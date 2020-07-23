#include "gateway.h"

#include <Adafruit_NeoPixel.h>
#include <RH_RF95.h>  // RadioHead library  to control the LoRa transceiver
#include <SPI.h>

#include "hardware_definition.h"
#include "utils.h"

RH_RF95 rfm(PIN_RFM_NSS, digitalPinToInterrupt(PIN_RFM_INT));

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_RGB_LEDS, PIN_LED_CTRL, NEO_GRB + NEO_KHZ400);

uint8_t rfm_init_success = 0;

char data[CMD_DATA_LEN];

void setup() {
  initRGB();
  initCommunications();
}

void loop() {
  switch (getSerialCommand(Serial, data)) {
    case CMD_ID_CONTROLLER:
      if (rfm_init_success) {
        strip.setPixelColor(0, 0x0000ff);
        strip.show();
        delay(20);
        rfm.send((uint8_t*)data, CMD_DATA_LEN);
        rfm.waitPacketSent();
        showStatus();
      }
      break;

    case CMD_ID_GATEWAY:
      interpretSerialCommand(data);
      break;

    default:
      break;
  }

  if (rfm_init_success) {
    if (rfm.waitAvailableTimeout(100)) {
      uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
      uint8_t len = sizeof(buf);
      if (rfm.recv(buf, &len)) {
        Serial.write(buf, len);
        strip.setPixelColor(0, 0xff0000);
        strip.show();
        delay(30);
        showStatus();
      }
    }
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

void interpretSerialCommand(char* data) {
  switch (data[0]) {
    case CMD_SEND_BONJOUR:
      Serial.println(BONJOUR);
      break;

    default:
      break;
  }
}