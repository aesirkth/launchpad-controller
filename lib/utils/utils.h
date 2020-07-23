#include <Arduino.h>

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

uint8_t getSerialCommand(Stream& port, char* data);