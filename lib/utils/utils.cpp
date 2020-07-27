#include "utils.h"

/*
  Class to encapsulate the communications to and from the Launchpad Controller

  Works with both the Serial ser and the RFM9xW LoRa transceiver

  @param ser      Stream object (Serial) to read data from
  @param rfm      RH_RF95 object instanciated with the right arguments for the board
  @param rst_pin  Reset pin of the LoRa transceiver
  @param freq     Frequency of the LoRa transceiver
  @param pow      Tx Power of the LoRa transceiver
*/
Comms::Comms(Stream& ser, RH_RF95& rfm, uint8_t rst_pin, float freq, int8_t pow) {
  _ser = &ser;
  _rfm = &rfm;
  _freq = freq;
  _pow = pow;

  pinMode(_rst, OUTPUT);
  digitalWrite(_rst, HIGH);  // Default state
  delay(10);
}

/* ---------------- Functions to deal with the LoRa Transceiver ---------------- */

/*
  Get everything ready to use the communication interfaces
*/
void Comms::begin() {
  reset();

  rfm_success = _rfm->init();
  if (rfm_success) {
    // Defaults after init are:
    // 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
    _rfm->setFrequency(_freq);
    _rfm->setTxPower(_pow);
  }
}

/*
  Trigger a reset of the LoRa transceiver
*/
void Comms::reset() {
  digitalWrite(_rst, LOW);
  delay(10);
  digitalWrite(_rst, HIGH);
  delay(100);
}

/*
  Read a message from the RFM buffer

  @param data char array of size CMD_DATA_LEN to store the data

  @return 1 if a message has been read
          0 if no message is available
*/
uint8_t Comms::readRFMBuffer(char* data) {
  if (rfm_success) {
    if (_rfm->waitAvailableTimeout(100)) {
      uint8_t rf95_buf[RH_RF95_MAX_MESSAGE_LEN];
      uint8_t rf95_len = sizeof(rf95_buf);

      if (_rfm->recv(rf95_buf, &rf95_len)) {
        // All messages have a fixed length CMD_DATA_LEN
        for (uint8_t i = 0; i < CMD_DATA_LEN; i++) {
          data[i] = rf95_buf[i];
        }
      }
      return 1;
    }
  }
  return 0;
}

/* ---------------- Functions to deal with the serial interface ---------------- */

/*
  Read the data in the serial buffer and extract one valid command frame 

  Read the serial buffer byte by byte and return the data bytes of a command frame 
  if one is found at the beginning of the buffer. If the first bytes of the buffer 
  do not match a valid frame structure, they are removed from the buffer and the 
  extraction stops. Call this function again to retry an extraction from the *new* 
  beginning of the serial buffer

  A command frame over Serial is made of the following structure:
    --------------------------------------------------------------------------
    | Byte position |     0      |    1    |      2      | ... |     n+1     |
    | Description   | Start byte | ID byte | Data byte 1 | ... | Data byte n |
    -------------------------------------------------------------------------- 

  The size of the Data field is given by CMD_DATA_LEN

  @param data array to store the data bytes of the command
  @param id   pointer to the identifier of the command frame. id is updated to the ID 
              field of the command frame is a valid frame is read

  @return 1 if a command has been read
          0 if the buffer is empty or if the first byte is not a valid frame start
*/
uint8_t Comms::readSerialCommand(char* data, uint8_t* id) {
  if (_ser->available() < CMD_FRAME_LEN) {
    return 0;  // Not enough data to make a command frame in the buffer, abort reading

  } else {
    char tmp = _ser->read();
    if (tmp != CMD_START_OF_FRAME) {
      return 0;  // First byte is not a frame start -> stop reading
    } else {
      {
        char tmp = _ser->read();
        if (tmp == CMD_ID_CONTROLLER) {
          _ser->readBytes(data, CMD_DATA_LEN);
          *id = CMD_ID_CONTROLLER;
          return 1;
        } else if (tmp == CMD_ID_GATEWAY) {
          _ser->readBytes(data, CMD_DATA_LEN);
          *id = CMD_ID_GATEWAY;
          return 1;
        } else {  // ID does not match -> stop reading
          return 0;
        }
      }
    }
  }
}

/* 
  Read a command in the LoRa buffer and the Serial buffer

  The LoRa buffer has priority over the Serial buffer. The Serial buffer is read 
  only if the LoRa buffer is empty or the LoRa transceiver is not initialized

  The value of id is updated to the ID field of the command frame is a valid frame 
  is read from the Serial buffer. If a valid frame is read from the Lora buffer, the
  value of id is set to 0

  @param data array to store the data bytes of the command
  @param id   pointer to the identifier of the command frame

  @return 1 if a command has been read
          0 if the buffers are empty or do not hold any command
*/
uint8_t Comms::readCommand(char* data, uint8_t* id) {
  if (readRFMBuffer(data)) {
    *id = 0;
    return 1;
  } else if (readSerialCommand(data, id)) {
    return 1;
  } else {
    return 0;
  }
}