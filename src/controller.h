#include <Arduino.h>

void initRGB();
void initMainOutputs();
void resetRFM();
void initRFM();
void initCommunications();
void showStatus();

void readByte(uint8_t *data);
void sendPayload(uint8_t payload[]);
void sendState();

void toggleOutput(uint8_t pin, uint8_t en);