#include "controller.h"

#include <Adafruit_NeoPixel.h>
#include <PWMServo.h>
#include <RH_RF95.h>  // RadioHead library  to control the LoRa transceiver
#include <SPI.h>

#include "comms.h"
#include "hardware_definition.h"

#define BIT_OUTPUT1 1
#define BIT_OUTPUT2 2
#define BIT_OUTPUT3 3
#define BIT_OUTPUT4 4

#define MASK_OUTPUT1 ~(1 << BIT_OUTPUT1)
#define MASK_OUTPUT2 ~(1 << BIT_OUTPUT2)
#define MASK_OUTPUT3 ~(1 << BIT_OUTPUT3)
#define MASK_OUTPUT4 ~(1 << BIT_OUTPUT4)

RH_RF95 rfm(PIN_RFM_NSS, digitalPinToInterrupt(PIN_RFM_INT));

Comms comms(Serial, rfm, PIN_RFM_RESET, RFM_FREQ, RFM_TX_POWER);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_RGB_LEDS, PIN_LED_CTRL, NEO_GRB + NEO_KHZ400);

PWMServo servo1, servo2, servo3;

uint8_t rfm_init_success = 0;

uint8_t output_state = 0;
uint8_t servo_state[3] = {0, 0, 0};

uint8_t line_feed = 0x0A;
uint8_t carriage_ret = 0x0D;

char data[CMD_DATA_LEN];
uint8_t id;

void setup() {
  initRGB();
  initMainOutputs();
  initServos();

  Serial.begin(BAUDRATE);

  comms.begin();
}

void loop() {
  if (comms.readCommand(data, &id)) {
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

      case CMD_MOVE_SERVO1:
        moveServo(servo1, data[1]);
        break;

      case CMD_MOVE_SERVO2:
        moveServo(servo2, data[1]);
        break;

      case CMD_MOVE_SERVO3:
        moveServo(servo3, data[1]);
        break;

      default:
        break;
    }
    comms.sendState(output_state, servo_state);
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

void initServos() {
  servo1.attach(PIN_PWM1, SERVO_MIN_PULSE_WIDTH, SERVO_MAX_PULSE_WIDTH);
  servo2.attach(PIN_PWM2, SERVO_MIN_PULSE_WIDTH, SERVO_MAX_PULSE_WIDTH);
  servo3.attach(PIN_PWM3, SERVO_MIN_PULSE_WIDTH, SERVO_MAX_PULSE_WIDTH);
}

void showStatus() {
  if (not rfm_init_success) {
    strip.setPixelColor(0, 0xff0000);
  } else {
    strip.setPixelColor(0, 0x00ff00);
  }
  strip.show();
}

void toggleOutput(uint8_t pin, uint8_t en) {
  switch (pin) {
    case PIN_OUTPUT1:
      output_state = (output_state & MASK_OUTPUT1) | (en << BIT_OUTPUT1);
      digitalWrite(pin, en);
      break;

    case PIN_OUTPUT2:
      output_state = (output_state & MASK_OUTPUT2) | (en << BIT_OUTPUT2);
      digitalWrite(pin, en);
      break;

    case PIN_OUTPUT3:
      output_state = (output_state & MASK_OUTPUT3) | (en << BIT_OUTPUT3);
      digitalWrite(pin, en);
      break;

    case PIN_OUTPUT4:
      output_state = (output_state & MASK_OUTPUT4) | (en << BIT_OUTPUT4);
      digitalWrite(pin, en);
      break;

    default:
      break;
  }
}

void moveServo(PWMServo& servo, uint8_t angle) {
  if (angle < 180) {
    servo.write(angle);
  } else {
    servo.write(180);
  }
}