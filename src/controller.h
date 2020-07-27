#include <Arduino.h>
#include <PWMServo.h>

void initMainOutputs();
void initServos();

void toggleOutput(uint8_t pin, uint8_t en);
void moveServo(PWMServo& servo, uint8_t angle);