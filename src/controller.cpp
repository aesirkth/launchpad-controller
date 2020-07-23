#include "controller.h"

#include <Adafruit_NeoPixel.h>
#include <RH_RF95.h>  // RadioHead library  to control the LoRa transceiver
#include <SPI.h>

#include "hardware_definition.h"
#include "utils.h"

#define BIT_RFM_INIT 0
#define BIT_OUTPUT1 1
#define BIT_OUTPUT2 2
#define BIT_OUTPUT3 3
#define BIT_OUTPUT4 4

RH_RF95 rfm(PIN_RFM_NSS, digitalPinToInterrupt(PIN_RFM_INT));

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_RGB_LEDS, PIN_LED_CTRL, NEO_GRB + NEO_KHZ400);

uint8_t rfm_init_success = 0;
uint8_t output1_state = 0;
uint8_t output2_state = 0;
uint8_t output3_state = 0;
uint8_t output4_state = 0;

uint8_t line_feed = 0x0A;
uint8_t carriage_ret = 0x0D;

char data[CMD_DATA_LEN];

void setup() {
  initRGB();
  initMainOutputs();

  initCommunications();
}

void loop() {
  if (getCommand(data)) {
    switch (data[0]) {
      case CMD_SEND_BONJOUR:
        Serial.println(BONJOUR);
        break;

      case CMD_TOGGLE_OUTPUT1:
        toggleOutput(PIN_OUTPUT1, data[1] & 0x01);
        break;

      case CMD_TOGGLE_OUTPUT2:
        toggleOutput(PIN_OUTPUT2, data[1] & 0x01);
        break;

      case CMD_TOGGLE_OUTPUT3:
        toggleOutput(PIN_OUTPUT3, data[1] & 0x01);
        break;

      case CMD_TOGGLE_OUTPUT4:
        toggleOutput(PIN_OUTPUT4, data[1] & 0x01);
        break;

      default:
        break;
    }
    sendState();
  }
}

void initRGB() {
  strip.begin();
  delay(10);
  strip.clear();
  strip.setBrightness(20);
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

uint8_t getCommand(char* data) {
  if (rfm_init_success) {
    if (rfm.waitAvailableTimeout(100)) {
      uint8_t rf95_buf[RH_RF95_MAX_MESSAGE_LEN];
      uint8_t rf95_len = sizeof(rf95_buf);

      if (rfm.recv(rf95_buf, &rf95_len)) {
        for (uint8_t i = 0; i < CMD_DATA_LEN; i++) {
          data[i] = rf95_buf[i];
        }
      }
      return 1;
    } else {
      return getSerialCommand(Serial, data);  // Ignore the ID
    }
  } else {
    return getSerialCommand(Serial, data);  // Ignore the ID
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