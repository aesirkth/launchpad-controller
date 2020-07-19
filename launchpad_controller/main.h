#include <Arduino.h>

void initRGB();
void initMainOutputs();
void resetRFM();
void initCommunications();
void showStatus();

void readByte(uint8_t *data);

void send_payload(uint8_t payload[]);
void send_state();
void start_filling();
void stop_filling();
void start_venting();
void stop_venting();
void arm();
void disarm();
void start_ignition();
void stop_ignition();
void enable_telemetry();
void disable_telemetry();
void trigger_calibration();
void enter_safe_mode();
void exit_safe_mode();