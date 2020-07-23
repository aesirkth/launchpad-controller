#include "utils.h"

#include <Arduino.h>

/* ---------------- Functions to deal with the serial interface ---------------- */

/*
  Read the data in the serial buffer and extract one valid frame

  Read the serial buffer byte by byte and return the data bytes of a command frame
  if one is found at the beginning of the buffer. If the first bytes of the buffer
  do not match a valid frame structure, they are removed from the buffer and the
  extraction stops. Call this function again to retry an extraction from the *new*
  beginning of the serial buffer

  @param port Stream object (Serial) to read data from

  @param data array to store the data bytes of the command

  @return 0 if the buffer is empty or if the first byte is not a valid frame start
          CMD_ID_CONTROLLER if the frame is a controller command
          CMD_ID_GATEWAY if the frame is a gateway command
*/
uint8_t getSerialCommand(Stream& port, char* data) {
  if (port.available() >= CMD_FRAME_LEN) {
    char tmp = port.read();
    if (tmp == CMD_START_OF_FRAME) {
      {
        char tmp = port.read();
        if (tmp == CMD_ID_CONTROLLER) {
          port.readBytes(data, CMD_DATA_LEN);
          return CMD_ID_CONTROLLER;
        } else if (tmp == CMD_ID_GATEWAY) {
          port.readBytes(data, CMD_DATA_LEN);
          return CMD_ID_GATEWAY;
        } else {  // ID does not match -> stop reading
          return 0;
        }
      }
    } else {  // First byte is not a frame start -> stop reading
      return 0;
    }

  } else {  // Not enough data to read in the buffer
    return 0;
  }
}