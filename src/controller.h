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
void sendPayload(uint8_t payload[]);
void sendState();

void toggleOutput(uint8_t pin, uint8_t en);
void moveServo(PWMServo& servo, uint8_t angle);