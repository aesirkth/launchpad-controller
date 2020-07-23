#include <Arduino.h>

void initRGB();
void resetRFM();
void initRFM();
void initCommunications();
void showStatus();
uint8_t getSerialCommand(char* cmd);
void interpretSerialCommand(char* cmd);