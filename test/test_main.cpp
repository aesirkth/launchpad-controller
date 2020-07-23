#include "test_main.h"

#include <Adafruit_NeoPixel.h>
#include <PWMServo.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <unity.h>

#include "hardware_definition.h"

RH_RF95 rfm(PIN_RFM_NSS, digitalPinToInterrupt(PIN_RFM_INT));

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_RGB_LEDS, PIN_LED_CTRL, NEO_GRB + NEO_KHZ400);

PWMServo servo1, servo2, servo3;

void setup() {
  delay(2000);

  UNITY_BEGIN();

  initRFM();
  RUN_TEST(test_RFM);

  initRGB();
  RUN_TEST(test_rgb);

  initOutputs();
  RUN_TEST(test_outputs);

  initServos();
  RUN_TEST(test_servos);

  UNITY_END();
}

void loop() {
}

/* Tests */
void test_RFM() {
  TEST_ASSERT_TRUE(rfm.init());
}

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
  TEST_PASS_MESSAGE("Test passed if you saw it");
}

void test_outputs() {
  TEST_MESSAGE("Testing the main outputs, do you see them activate/deactivate ?");
  delay(1000);
  TEST_MESSAGE("P1");
  digitalWrite(PIN_OUTPUT1, HIGH);
  delay(1000);
  digitalWrite(PIN_OUTPUT1, LOW);
  TEST_MESSAGE("P2");
  digitalWrite(PIN_OUTPUT2, HIGH);
  delay(1000);
  digitalWrite(PIN_OUTPUT2, LOW);
  TEST_MESSAGE("P3");
  digitalWrite(PIN_OUTPUT3, HIGH);
  delay(1000);
  digitalWrite(PIN_OUTPUT3, LOW);
  TEST_MESSAGE("P4");
  digitalWrite(PIN_OUTPUT4, HIGH);
  delay(1000);
  digitalWrite(PIN_OUTPUT4, LOW);

  TEST_PASS_MESSAGE("Test passed if saw it");
}

void test_servos() {
  TEST_MESSAGE("Testing the servo outputs, do you see the servos move ?");
  delay(1000);
  TEST_MESSAGE("P5");
  servo1.write(0);
  delay(1000);
  servo1.write(180);
  delay(1000);
  servo1.write(90);
  TEST_MESSAGE("P6");
  servo2.write(0);
  delay(1000);
  servo2.write(180);
  delay(1000);
  servo2.write(90);
  TEST_MESSAGE("P7");
  servo3.write(0);
  delay(1000);
  servo3.write(180);
  delay(1000);
  servo3.write(90);

  TEST_PASS_MESSAGE("Test passed if saw it");
}

/* Utils */
void initRFM() {
  pinMode(PIN_RFM_RESET, OUTPUT);
  digitalWrite(PIN_RFM_RESET, HIGH);
  delay(100);
  digitalWrite(PIN_RFM_RESET, LOW);
  delay(10);
  digitalWrite(PIN_RFM_RESET, HIGH);
  delay(100);
}

void initRGB() {
  strip.begin();
  delay(10);
  strip.clear();
  strip.setBrightness(20);
  delay(10);
  strip.show();
}

void initOutputs() {
  pinMode(PIN_OUTPUT1, OUTPUT);
  pinMode(PIN_OUTPUT2, OUTPUT);
  pinMode(PIN_OUTPUT3, OUTPUT);
  pinMode(PIN_OUTPUT4, OUTPUT);

  digitalWrite(PIN_OUTPUT1, LOW);
  digitalWrite(PIN_OUTPUT2, LOW);
  digitalWrite(PIN_OUTPUT3, LOW);
  digitalWrite(PIN_OUTPUT4, LOW);
}

void initServos() {
  servo1.attach(PIN_PWM1, SERVO_MIN_PULSE_WIDTH, SERVO_MAX_PULSE_WIDTH);
  servo2.attach(PIN_PWM2, SERVO_MIN_PULSE_WIDTH, SERVO_MAX_PULSE_WIDTH);
  servo3.attach(PIN_PWM3, SERVO_MIN_PULSE_WIDTH, SERVO_MAX_PULSE_WIDTH);
}