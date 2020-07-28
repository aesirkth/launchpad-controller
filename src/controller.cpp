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

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_RGB_LEDS, PIN_LED_CTRL, NEO_GRB + NEO_KHZ400);

PWMServo servo1, servo2, servo3;

uint8_t rfm_init_success = 0;
uint8_t output1_state = 0;
uint8_t output2_state = 0;
uint8_t output3_state = 0;
uint8_t output4_state = 0;
uint8_t servo1_angle = 0;
uint8_t servo2_angle = 0;
uint8_t servo3_angle = 0;

char data[CMD_DATA_LEN];

void setup() {
  initRGB();
  initMainOutputs();
  initServos();

  initCommunications();
}

void loop() {
  if (getCommand(data)) {
    switch (data[0]) {
      case CMD_SEND_BONJOUR:
        Serial.println(BONJOUR);
        break;

      case CMD_TOGGLE_OUTPUT1:
        output1_state = data[1] & 0x01;
        digitalWrite(PIN_OUTPUT1, output1_state);
        break;

      case CMD_TOGGLE_OUTPUT2:
        output2_state = data[1] & 0x01;
        digitalWrite(PIN_OUTPUT2, output2_state);
        break;

      case CMD_TOGGLE_OUTPUT3:
        output3_state = data[1] & 0x01;
        digitalWrite(PIN_OUTPUT3, output3_state);
        break;

      case CMD_TOGGLE_OUTPUT4:
        output4_state = data[1] & 0x01;
        digitalWrite(PIN_OUTPUT4, output4_state);
        break;

      case CMD_MOVE_SERVO1:
        servo1_angle = constrain(data[1], 0, 180);
        servo1.write(servo1_angle);
        break;

      case CMD_MOVE_SERVO2:
        servo2_angle = constrain(data[1], 0, 180);
        servo2.write(servo2_angle);
        break;

      case CMD_MOVE_SERVO3:
        servo3_angle = constrain(data[1], 0, 180);
        servo3.write(servo3_angle);
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
        strip.setPixelColor(0, 0xff0000);
        strip.show();
        delay(30);
        showStatus();
      }
      return 1;
    } else {
      return getSerialCommand(Serial, data);  // Ignore the ID
    }
  } else {
    return getSerialCommand(Serial, data);  // Ignore the ID
  }
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

  uint8_t m_size = 6;
  uint8_t message[m_size];

  message[0] = state;
  message[1] = servo1_angle;
  message[2] = servo2_angle;
  message[3] = servo3_angle;
  message[4] = rssi_msb;
  message[5] = rssi_lsb;

  if (rfm_init_success) {
    strip.setPixelColor(0, 0x0000ff);
    strip.show();
    delay(20);
    rfm.send(message, m_size);
    rfm.waitPacketSent();
    showStatus();
  }

  Serial.write(message, m_size);
  Serial.write(0x00);
  Serial.write(0x00);
  // Include a carriage return and a line feed so the receiver can split out frames
  Serial.write(0x0D);
  Serial.write(0x0A);
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