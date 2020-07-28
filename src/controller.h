#include <Arduino.h>
#include <PWMServo.h>

void initRGB();
void initMainOutputs();
void initServos();
void resetRFM();
void initRFM();
void initCommunications();
void showStatus();

uint8_t getCommand(char* data);
void sendState();

void toggleOutput(uint8_t pin, uint8_t en);