/* gateway.cpp

Author: Erwan Caffier for ÆSIR

Version: v3.0

Date: August 2020

Description:
  Embedded software on the gateway for the Launchpad Controller. This code listens to the commands
  received on the USB Serial interface and forwards them to the Launchpad Controller over a Lora
  link. The data packets received through the LoRa link are sent back on the USB Serial interface.
  This code has been tested on the Teensy LC microcontroller and the Launchpad Controller of the
  Mjollnir project.

License:
  MIT License

  Copyright (c) 2020 Association of Engineering Students in Rocketry

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

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
        int8_t rssi;
        if (rfm_init_success) {
          rssi = rfm.lastRssi();
        } else {
          rssi = 0;
        }
        Serial.write(buf, len);
        Serial.write(rssi);
        // Include a carriage return and a line feed so the receiver can split out frames
        Serial.write(0x0D);
        Serial.write(0x0A);
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
  strip.setPixelColor(0, STARTUP_COLOR);
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