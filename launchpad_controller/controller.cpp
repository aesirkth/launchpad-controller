#include "controller.h"

#include <Adafruit_NeoPixel.h>
#include <RH_RF95.h>  // RadioHead library  to control the LoRa transceiver
#include <SPI.h>

#include "hardware_definition.h"

#define BAUDRATE 115200
#define BONJOUR "LAUNCHPADSTATION"

#define RFM_TX_POWER 10  // Can be set between 5 and 23 dBm

#define BIT_RFM_INIT 0
#define BIT_OUTPUT1 1
#define BIT_OUTPUT2 2
#define BIT_OUTPUT3 3
#define BIT_OUTPUT4 4

#define CMD_OUTPUT1_EN 0x61   // 'a'
#define CMD_OUTPUT1_DIS 0x62  // 'b'
#define CMD_OUTPUT2_EN 0x63   // 'c'
#define CMD_OUTPUT2_DIS 0x64  // 'd'
#define CMD_OUTPUT3_EN 0x65   // 'e'
#define CMD_OUTPUT3_DIS 0x66  // 'f'
#define CMD_OUTPUT4_EN 0x67   // 'g'
#define CMD_OUTPUT4_DIS 0x68  // 'h'

RH_RF95 rfm(PIN_RFM_NSS, digitalPinToInterrupt(PIN_RFM_INT));

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_RGB_LEDS, PIN_LED_CTRL, NEO_GRB + NEO_KHZ400);

uint8_t rfm_init_success = 0;
uint8_t output1_state = 0;
uint8_t output2_state = 0;
uint8_t output3_state = 0;
uint8_t output4_state = 0;

/* Legacy */
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
      case CMD_OUTPUT1_EN:
        toggleOutput(PIN_OUTPUT1, 1);
        break;

      case CMD_OUTPUT1_DIS:
        toggleOutput(PIN_OUTPUT1, 0);
        break;

      case CMD_OUTPUT2_EN:
        toggleOutput(PIN_OUTPUT2, 1);
        break;

      case CMD_OUTPUT2_DIS:
        toggleOutput(PIN_OUTPUT2, 0);
        break;

      case CMD_OUTPUT3_EN:
        toggleOutput(PIN_OUTPUT3, 1);
        break;

      case CMD_OUTPUT3_DIS:
        toggleOutput(PIN_OUTPUT3, 0);
        break;

      case CMD_OUTPUT4_EN:
        toggleOutput(PIN_OUTPUT4, 1);
        break;

      case CMD_OUTPUT4_DIS:
        toggleOutput(PIN_OUTPUT4, 0);
        break;

      default:
        break;
    }
    sendState();
  }
  command = 0;
  delay(10);
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
  pinMode(PIN_OUTPUT1, OUTPUT);
  pinMode(PIN_OUTPUT2, OUTPUT);
  pinMode(PIN_OUTPUT3, OUTPUT);
  pinMode(PIN_OUTPUT4, OUTPUT);

  digitalWrite(PIN_OUTPUT1, LOW);
  digitalWrite(PIN_OUTPUT2, LOW);
  digitalWrite(PIN_OUTPUT3, LOW);
  digitalWrite(PIN_OUTPUT4, LOW);
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

void initRFM() {
  rfm_init_success = rfm.init();
  if (rfm_init_success) {
    // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
    rfm.setFrequency(RFM_FREQ);
    rfm.setTxPower(RFM_TX_POWER, false);
  }
  showStatus();
}

void initCommunications() {
  Serial.begin(BAUDRATE);

  while (!Serial && millis() < 4000) {
  }
  Serial.println(BONJOUR);

  resetRFM();
  initRFM();
}

void showStatus() {
  if (not rfm_init_success) {
    strip.setPixelColor(0, 0xff0000);
  } else {
    strip.setPixelColor(0, 0x00ff00);
  }
  strip.show();
}

void readByte(uint8_t* data) {  // Read one byte in the buffer
  if (rfm_init_success) {
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

void sendPayload(uint8_t payload[]) {  // Write the payload to the communication links
  delay(10);
  if (rfm_init_success) {
    rfm.send(payload, 5);
    rfm.waitPacketSent();
  }

  Serial.write(payload, 5);
}

void sendState() {  // Get the current state of the Launch Pad Station Board and
  // send it to the control interface
  uint8_t state = 0;
  state = state | rfm_init_success << BIT_RFM_INIT;
  state = state | output1_state << BIT_OUTPUT1;
  state = state | output2_state << BIT_OUTPUT2;
  state = state | output3_state << BIT_OUTPUT3;
  state = state | output4_state << BIT_OUTPUT4;
  int16_t rssi;
  if (rfm_init_success) {
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
  sendPayload(message);
}

void toggleOutput(uint8_t pin, uint8_t en) {
  switch (pin) {
    case PIN_OUTPUT1:
      output1_state = en;
      digitalWrite(pin, en);
      break;

    case PIN_OUTPUT2:
      output2_state = en;
      digitalWrite(pin, en);
      break;

    case PIN_OUTPUT3:
      output3_state = en;
      digitalWrite(pin, en);
      break;

    case PIN_OUTPUT4:
      output4_state = en;
      digitalWrite(pin, en);
      break;

    default:
      break;
  }
}