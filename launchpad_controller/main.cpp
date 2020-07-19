#include "main.h"

#include <Adafruit_NeoPixel.h>
#include <RH_RF95.h>  // RadioHead library  to control the LoRa transceiver
#include <SPI.h>

#include "hardware_definition.h"

#define BAUDRATE 115200
#define RFM_TX_POWER 5  // Can be set between 5 and 23 dBm

RH_RF95 rfm(PIN_RFM_NSS, digitalPinToInterrupt(PIN_RFM_INT));

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_RGB_LEDS, PIN_LED_CTRL, NEO_GRB + NEO_KHZ400);

uint8_t status = 0;
uint8_t rfm_init_status = 0;

/* Legacy */
#define BONJOUR "LAUNCHPADSTATION"
// Single wire ombilicals to the rocket
#define PIN_OMBI_TM A0  // Write LOW to this pin to disable the Telemetry and FPV transmitters
#define PIN_OMBI_CA A1  // Write LOW to this pin to start a sensor calibration on the rocket

// Pins where the relays are connected
#define PIN_RELAY_FIRE A2  // Write LOW to this pin to enable the ignition circuit
#define PIN_RELAY_FILL A3  // Write LOW to this pin to open solenoid 1
#define PIN_RELAY_VENT A4  // Write LOW to this pin to open solenoid 2

// Commands are single bytes
#define CMD_FILL_START 0x61  // 'a'
#define CMD_FILL_STOP 0x62   // 'b'
#define CMD_VENT_START 0x63  // 'c'
#define CMD_VENT_STOP 0x64   // 'd'
#define CMD_ARM 0x65         // 'e'
#define CMD_DISARM 0x66      // 'f'
#define CMD_FIRE_START 0x67  // 'g'
#define CMD_FIRE_STOP 0x68   // 'h'
#define CMD_TM_ENABLE 0x41   // 'A'
#define CMD_TM_DISABLE 0x42  // 'B'
#define CMD_CA_TRIGGER 0x43  // 'C'
#define CMD_SAFE_IN 0x59     // 'Y'
#define CMD_SAFE_OUT 0x5A    // 'Z

// Positions of the bit in the 'state' byte
#define BIT_FILLING_POS 0
#define BIT_VENTING_POS 1
#define BIT_ARMED_POS 2
#define BIT_FIRING_POS 3
#define BIT_TM_POS 4
#define BIT_SAFE_MODE 5

uint8_t command = 0x00;
bool is_filling = false;
bool is_venting = false;
bool is_armed = false;
bool is_firing = false;
bool is_tm_enabled = true;
bool is_safe_mode = true;
uint8_t count = 0;

// bool rf95_init = false;

uint8_t line_feed = 0x0A;
uint8_t carriage_ret = 0x0D;
/* Legacy */

void setup() {
  initRGB();
  initMainOutputs();

  initCommunications();
}

void loop() {
  readByte(&command);

  if (command) {
    switch (command) {
      case CMD_FILL_START:
        start_filling();
        break;
      case CMD_FILL_STOP:
        stop_filling();
        break;
      case CMD_VENT_START:
        start_venting();
        break;
      case CMD_VENT_STOP:
        stop_venting();
        break;
      case CMD_ARM:
        arm();
        break;
      case CMD_DISARM:
        disarm();
        break;
      case CMD_FIRE_START:
        start_ignition();
        break;
      case CMD_FIRE_STOP:
        stop_ignition();
        break;
      case CMD_TM_ENABLE:
        enable_telemetry();
        break;
      case CMD_TM_DISABLE:
        disable_telemetry();
        break;
      case CMD_CA_TRIGGER:
        trigger_calibration();
        break;
      case CMD_SAFE_IN:
        enter_safe_mode();
        break;
      case CMD_SAFE_OUT:
        exit_safe_mode();
        break;
      default:
        break;
    }
    send_state();
  }
}

void initRGB() {
  strip.begin();
  delay(10);
  strip.clear();
  strip.setBrightness(20);
  delay(10);
  strip.setPixelColor(0, 0x000000);
  strip.show();
  delay(100);
  strip.setPixelColor(0, 0x66ccff);
  strip.show();
}

void initMainOutputs() {
  pinMode(PIN_IO1, OUTPUT);
  pinMode(PIN_IO2, OUTPUT);
  pinMode(PIN_IO3, OUTPUT);
  pinMode(PIN_IO4, OUTPUT);

  digitalWrite(PIN_IO1, LOW);
  digitalWrite(PIN_IO2, LOW);
  digitalWrite(PIN_IO3, LOW);
  digitalWrite(PIN_IO4, LOW);
}

void resetRFM() {
  pinMode(PIN_RFM_RESET, OUTPUT);
  digitalWrite(PIN_RFM_RESET, HIGH);
  delay(100);
  digitalWrite(PIN_RFM_RESET, LOW);
  delay(10);
  digitalWrite(PIN_RFM_RESET, HIGH);
  delay(100);
}

void initCommunications() {
  Serial.begin(BAUDRATE);
  while (!Serial.available()) {
  }

  Serial.println(BONJOUR);

  resetRFM();
  rfm_init_status = rfm.init();
  if (rfm_init_status) {
    // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
    rfm.setFrequency(RFM_FREQ);
    rfm.setTxPower(RFM_TX_POWER, false);
  } else {
    status |= 0b00000010;
    showStatus();
  }
}

void showStatus() {
  if (not rfm_init_status) {
    strip.setPixelColor(0, 0xff0000);
    strip.show();
  }
}

/* Legacy */

void readByte(uint8_t* data) {  // Read one byte in the buffer
  if (rfm_init_status) {
    if (rfm.waitAvailableTimeout(100)) {
      uint8_t rf95_buf[RH_RF95_MAX_MESSAGE_LEN];
      uint8_t rf95_len = sizeof(rf95_buf);

      if (rfm.recv(rf95_buf, &rf95_len)) {
        *data = rf95_buf[0];
      }
    } else if (Serial.available() > 0) {
      *data = Serial.read();
    }
  } else if (Serial.available() > 0) {
    *data = Serial.read();
  }
}

void send_payload(uint8_t payload[]) {  // Write the payload to the communication links
  delay(10);
  if (rfm_init_status) {
    rfm.send(payload, 5);
    rfm.waitPacketSent();
  }

  Serial.write(payload, 5);
}

void send_state() {  // Get the current state of the Launch Pad Station Board and
  // send it to the control interface
  uint8_t state = 0;
  state = state | is_filling << BIT_FILLING_POS;
  state = state | is_venting << BIT_VENTING_POS;
  state = state | is_armed << BIT_ARMED_POS;
  state = state | is_firing << BIT_FIRING_POS;
  state = state | is_tm_enabled << BIT_TM_POS;
  state = state | is_safe_mode << BIT_SAFE_MODE;
  int16_t rssi;
  if (rfm_init_status) {
    rssi = rfm.lastRssi();
  } else {
    rssi = 0;
  }

  uint8_t rssi_msb = (rssi & 0xFF00) >> 8;
  uint8_t rssi_lsb = rssi & 0x00FF;
  uint8_t message[5];
  message[0] = state;
  message[1] = rssi_msb;
  message[2] = rssi_lsb;
  message[3] = carriage_ret;
  message[4] = line_feed;
  send_payload(message);
}

/*
 * Controls for the Rocket fueling and ignition
 */

void start_filling() {  // Enable solenoid 1 only if solenoid 2 is disabled
  if (!is_safe_mode && !is_venting && !is_armed) {
    is_filling = true;
    digitalWrite(PIN_RELAY_FILL, LOW);
  }
}

void stop_filling() {  // Disable solenoid 1
  is_filling = false;
  digitalWrite(PIN_RELAY_FILL, HIGH);
}

void start_venting() {  // Enable solenoid 2 only if solenoid 1 is disabled
  if (!is_safe_mode && !is_filling && !is_armed) {
    is_venting = true;
    digitalWrite(PIN_RELAY_VENT, LOW);
  }
}

void stop_venting() {  // Disable solenoid 2
  is_venting = false;
  digitalWrite(PIN_RELAY_VENT, HIGH);
}

void arm() {  // Set is_armed to true
  // is_armed must be true to allow ignition
  if (!is_safe_mode && !is_filling && !is_venting) {
    is_armed = true;
  }
}

void disarm() {  // Set is_armed to false
  // is_armed must be true to allow ignition
  is_armed = false;
  // Also stop firing, just in case
  is_firing = false;
  digitalWrite(PIN_RELAY_FIRE, HIGH);
}

void start_ignition() {  // Enable ignition circuit
  // is_armed must be true to allow ignition
  // Solenoid 1 must be closed to allow ignition
  // Solenoid 2 must be closed to allow ignition
  if (is_armed) {
    is_firing = true;
    digitalWrite(PIN_RELAY_FIRE, LOW);
  }
}

void stop_ignition() {  // Disable ignition circuit
  is_firing = false;
  digitalWrite(PIN_RELAY_FIRE, HIGH);
}

/*
 * Controls for the Rocket through the ombilicals
 */

void enable_telemetry() {  // Enable the Telemetry and FPV transmitters (on the rocket)
  is_tm_enabled = true;
  digitalWrite(PIN_OMBI_TM, HIGH);
}

void disable_telemetry() {  // Disable the Telemetry and FPV transmitters (on the rocket)
  is_tm_enabled = false;
  digitalWrite(PIN_OMBI_TM, LOW);
}

void trigger_calibration() {  // Trigger a calibration routine (on the rocket)
  digitalWrite(PIN_OMBI_CA, LOW);
  delay(100);
  digitalWrite(PIN_OMBI_CA, HIGH);
}

/*
 * Safe mode
 */

void enter_safe_mode() {
  if (!is_filling && !is_venting && !is_armed && !is_firing) {
    is_safe_mode = true;
  }
}

void exit_safe_mode() {
  count++;
  if (count > 2) {
    count = 0;
    is_safe_mode = false;
  }
}