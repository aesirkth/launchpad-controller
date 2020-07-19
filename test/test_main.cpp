#include "test_main.h"

#include <Adafruit_NeoPixel.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <unity.h>

#include "hardware_definition.h"

RH_RF95 rf95(PIN_RFM_NSS, PIN_RFM_INT);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_RGB_LEDS, PIN_LED_CTRL, NEO_GRB + NEO_KHZ400);

void setup() {
  delay(2000);

  UNITY_BEGIN();

  initRGB();
  RUN_TEST(test_rgb);

  initOutputs();
  RUN_TEST(testOutputs);

  UNITY_END();
}

void loop() {
}

/* Tests */
void test_rgb() {
  TEST_MESSAGE("Testing the RGB leds, do you see them flashing ?");
  delay(1000);
  uint32_t color[6] = {
      0xff0000,
      0x00ff00,
      0x0000ff,
      0xcc00ff,
      0xcc9900,
      0x66ccff,
  };

  for (uint8_t i = 0; i < 6; i++) {
    for (uint8_t j = 0; j < NUM_RGB_LEDS; j++) {
      strip.setPixelColor(j, color[i]);
    }
    strip.show();
    delay(500);
  }

  strip.clear();
  strip.show();
  TEST_PASS_MESSAGE("Test passed if saw it");
}

void testOutputs() {
  TEST_MESSAGE("Testing the main outputs, do you see them activate/deactivate ?");
  delay(1000);
  digitalWrite(PIN_IO1, HIGH);
  delay(1000);
  digitalWrite(PIN_IO1, LOW);
  digitalWrite(PIN_IO2, HIGH);
  delay(1000);
  digitalWrite(PIN_IO2, LOW);
  digitalWrite(PIN_IO3, HIGH);
  delay(1000);
  digitalWrite(PIN_IO3, LOW);
  digitalWrite(PIN_IO4, HIGH);
  delay(1000);
  digitalWrite(PIN_IO4, LOW);

  TEST_PASS_MESSAGE("Test passed if saw it");
}

/* Utils */
void initRGB() {
  strip.begin();
  delay(10);
  strip.clear();
  strip.setBrightness(20);
  delay(10);
  strip.show();
}

void initOutputs() {
  pinMode(PIN_IO1, OUTPUT);
  pinMode(PIN_IO2, OUTPUT);
  pinMode(PIN_IO3, OUTPUT);
  pinMode(PIN_IO4, OUTPUT);

  digitalWrite(PIN_IO1, LOW);
  digitalWrite(PIN_IO2, LOW);
  digitalWrite(PIN_IO3, LOW);
  digitalWrite(PIN_IO4, LOW);
}