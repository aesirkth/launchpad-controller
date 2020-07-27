#include "controller.h"

#include <Adafruit_NeoPixel.h>
#include <PWMServo.h>
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

Comms comms(Serial, rfm, PIN_RFM_RESET, RFM_FREQ, RFM_TX_POWER);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_RGB_LEDS, PIN_LED_CTRL, NEO_GRB + NEO_KHZ400);

PWMServo servo1, servo2, servo3;

uint8_t rfm_init_success = 0;
uint8_t output1_state = 0;
uint8_t output2_state = 0;
uint8_t output3_state = 0;
uint8_t output4_state = 0;

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

void sendPayload(uint8_t payload[]) {  // Write the payload to the communication links
  if (rfm_init_success) {
    strip.setPixelColor(0, 0x0000ff);
    strip.show();
    delay(20);
    rfm.send(payload, 5);
    rfm.waitPacketSent();
    showStatus();
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

void moveServo(PWMServo& servo, uint8_t angle) {
  if (angle < 180) {
    servo.write(angle);
  } else {
    servo.write(180);
  }
}