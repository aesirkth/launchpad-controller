#include <Arduino.h>
#include <RH_RF95.h>

#define CMD_FRAME_LEN 4
#define CMD_DATA_LEN 2
#define CMD_START_OF_FRAME 0x26  // '&'
#define CMD_ID_CONTROLLER 0x63   // 'c'
#define CMD_ID_GATEWAY 0x67      // 'g'

#define CMD_SEND_BONJOUR 0x42    // 'B'
#define CMD_TOGGLE_OUTPUT1 0x61  // 'a'
#define CMD_TOGGLE_OUTPUT2 0x62  // 'b'
#define CMD_TOGGLE_OUTPUT3 0x63  // 'c'
#define CMD_TOGGLE_OUTPUT4 0x64  // 'd'
#define CMD_MOVE_SERVO1 0x6A     // 'j'
#define CMD_MOVE_SERVO2 0x6B     // 'k'
#define CMD_MOVE_SERVO3 0x6C     // 'l'

class Comms {
 public:
  Comms(Stream& ser, RH_RF95& rfm, uint8_t rst_pin, float freq, int8_t pow);

  void begin();
  uint8_t readCommand(char* data, uint8_t* id);
  void sendState(uint8_t out, uint8_t* servo);

  uint8_t rfm_success;  // True when the RFM transeiver is successfully initiated

 private:
  void reset();
  uint8_t readRFMBuffer(char* data);
  uint8_t readSerialCommand(char* data, uint8_t* id);
  void sendPayload(uint8_t* payload, uint8_t len);

  Stream* _ser;
  RH_RF95* _rfm;

  uint8_t _rst;
  float _freq;
  int8_t _pow;
};