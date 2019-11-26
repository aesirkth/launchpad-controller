/* Embedded software for the Launch Pad Station Board

This code receives commands from a LoRa module RFM9XW and controls the rocket fuel sequence
Relevant commands are also forwarded to the rocket itself using logic levels on single wires

The Arduino can also receive commands from a Serial link

The commands are single bytes. See their definition below.
When a non-zero command is received, an action is triggered and the current state of the outputs
is sent back in a 5 bytes messages with the following content :
  - First byte: State
      LSb set to 1 if the fuel line is being filled
      bit 2 set to 1 if the fuel line is being vented
      bit 3 set to 1 if the ignition is armed
      bit 4 set to 1 if the ignition is active
      bit 5 set to 1 if the telemetry is enabled
  - Second and third byte: rssi
      int16_t holding the last rssi value in dBm
      Most Significant Byte sent first
  - Fourth byte: carriage return 0x0D ('\r')
  - Fifth byte: line feed 0x0A ('\n')

Hardware :
  - 1x Arduino Nano
  - 1x RFM9X LoRa transceiver
  - 3x Relay
  - 1x Rocket

Tested on Arduino Nano 3.0 boards

Wiring :
                Arduino      RFM95/96/97/98
                GND----------GND   (ground in)
interrupt 0 pin D2-----------DIO0  (interrupt request out)
                D9-----------RESET (reset pin)
         SS pin D10----------NSS   (CS chip select in)
        SCK pin D13----------SCK   (SPI clock in)
       MOSI pin D11----------MOSI  (SPI Data in)
       MISO pin D12----------MISO  (SPI Data out)
                The RFM9XW are 3V3 logic and are NOT 5V tolerant
                Connect everything through level shifters 5V <-> 3V3
                Use a 5V to 3V3 DC converter to power the RFM9X transceiver

                Arduino      Rocket
                GND----------GND   (ground in)
                A0-----------Ombilical 1 (Telemetry/FPV enable)
                A1-----------Ombilical 2 (Calibration start)

                Arduino      Ignition Relay
                GND----------GND   (ground in)
                A2-----------IN1   (Command pin for ignition)
                5V-----------VCC   (5V in)
                Connect the ignition circuit on Normally Open side

                Arduino      Solenoids Relay (NB: jumper between VCC and JD-VCC)
                GND----------GND   (ground in)
                A3-----------IN2   (Command pin for venting)
                A4-----------IN1   (Command pin for filling)
                5V-----------VCC   (5V in)
                Connect the solenoids on Normally Open side

*/

#include <SPI.h>
#include <RH_RF95.h> // RadioHead library  to control the LoRa transceiver

// Single wire ombilicals to the rocket
#define PIN_OMBI_TM A0 // Write LOW to this pin to disable the Telemetry and FPV transmitters
#define PIN_OMBI_CA A1 // Write LOW to this pin to start a sensor calibration on the rocket

// Pins where the relays are connected
#define PIN_RELAY_FIRE A2 // Write LOW to this pin to enable the ignition circuit
#define PIN_RELAY_FILL A3 // Write LOW to this pin to open solenoid 1
#define PIN_RELAY_VENT A4 // Write LOW to this pin to open solenoid 2

// Commands are single bytes
#define CMD_FILL_START 0x61 // 'a'
#define CMD_FILL_STOP  0x62 // 'b'
#define CMD_VENT_START 0x63 // 'c'
#define CMD_VENT_STOP  0x64 // 'd'
#define CMD_ARM        0x65 // 'e'
#define CMD_DISARM     0x66 // 'f'
#define CMD_FIRE_START 0x67 // 'g'
#define CMD_FIRE_STOP  0x68 // 'h'
#define CMD_TM_ENABLE  0x41 // 'A'
#define CMD_TM_DISABLE 0x42 // 'B'
#define CMD_CA_TRIGGER 0x43 // 'C'

// Defines for the Serial link
#define BAUDRATE 115200
#define BONJOUR "LAUNCHPADSTATION"

// Defines for the LoRa transceiver
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2
#define RF95_FREQ 915.0

RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Positions of the bit in the 'state' byte
#define BIT_FILLING_POS 0
#define BIT_VENTING_POS 1
#define BIT_ARMED_POS   2
#define BIT_FIRING_POS  3
#define BIT_TM_POS      4

uint8_t command = 0x00;
bool is_filling = false;
bool is_venting = false;
bool is_armed = false;
bool is_firing = false;
bool is_tm_enabled = true;

uint8_t line_feed = 0x0A;
uint8_t carriage_ret = 0x0D;

void setup()
{
  pinMode(PIN_RELAY_FIRE, OUTPUT);
  pinMode(PIN_RELAY_FILL, OUTPUT);
  pinMode(PIN_RELAY_VENT, OUTPUT);
  pinMode(PIN_OMBI_TM, OUTPUT);
  pinMode(PIN_OMBI_CA, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  // Disable the ignition circuit
  digitalWrite(PIN_RELAY_FIRE, HIGH);
  // Close the solenoids
  digitalWrite(PIN_RELAY_FILL, HIGH);
  digitalWrite(PIN_RELAY_VENT, HIGH);
  // Default state is HIGH for the ombilicals
  digitalWrite(PIN_OMBI_TM, HIGH);
  digitalWrite(PIN_OMBI_CA, HIGH);
  // Default state for this pin is HIGH
  digitalWrite(RFM95_RST, HIGH);
  
  init_communication();
}

void loop()
{
  read_byte(&command);
  
  if (command)
  {
    switch (command)
    {
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
    default:
      break;
    }
    send_state();
  }
  // Reset this to the default value
  command = 0x00;
  delay(100);
}

/*
 * Functions for communication with the Ground Station
 */

void init_communication()
{ // Initialize the communication links
  // Enable the Serial link and send 'BONJOUR' to be recognized by the control interface
  Serial.begin(115200);
  Serial.println(BONJOUR);
  // Trigger a reset of the transceiver
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
  // Initialize the RFM9X transceiver
  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
  rf95.setFrequency(RF95_FREQ);
  rf95.setTxPower(23, false); // Can be set between 5 and 23 dBm
}

void read_byte(uint8_t *data)
{ // Read one byte in the buffer

  if (rf95.waitAvailableTimeout(100))
  {
    uint8_t rf95_buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t rf95_len = sizeof(rf95_buf);

    if (rf95.recv(rf95_buf, &rf95_len))
    {
      *data = rf95_buf[0];
    }
  }
  else if (Serial.available() > 0)
  {
    *data = Serial.read();
  }
}

void send_payload(uint8_t payload[])
{ // Write the payload to the communication links
  delay(10);
  rf95.send(payload, 5);
  rf95.waitPacketSent();

  Serial.write(payload, 5);
}

void send_state()
{ // Get the current state of the Launch Pad Station Board and
  // send it to the control interface
  uint8_t state = 0;
  state = state | is_filling << BIT_FILLING_POS;
  state = state | is_venting << BIT_VENTING_POS;
  state = state | is_armed << BIT_ARMED_POS;
  state = state | is_firing << BIT_FIRING_POS;
  state = state | is_tm_enabled << BIT_TM_POS;
  int16_t rssi = rf95.lastRssi();
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

void start_filling()
{ // Enable solenoid 1 only if solenoid 2 is disabled
  if (!is_venting && !is_armed)
  {
    is_filling = true;
    digitalWrite(PIN_RELAY_FILL, LOW);
  }
}

void stop_filling()
{ // Disable solenoid 1
  is_filling = false;
  digitalWrite(PIN_RELAY_FILL, HIGH);
}

void start_venting()
{ // Enable solenoid 2 only if solenoid 1 is disabled
  if (!is_filling && !is_armed)
  {
    is_venting = true;
    digitalWrite(PIN_RELAY_VENT, LOW);
  }
}

void stop_venting()
{ // Disable solenoid 2
  is_venting = false;
  digitalWrite(PIN_RELAY_VENT, HIGH);
}

void arm()
{ // Set is_armed to true
  // is_armed must be true to allow ignition
  if (!is_filling && !is_venting)
  {
    is_armed = true;
  }
}

void disarm()
{ // Set is_armed to false
  // is_armed must be true to allow ignition
  is_armed = false;
  // Also stop firing, just in case
  is_firing = false;
  digitalWrite(PIN_RELAY_FIRE, HIGH);
}

void start_ignition()
{ // Enable ignition circuit
  // is_armed must be true to allow ignition
  // Solenoid 1 must be closed to allow ignition
  // Solenoid 2 must be closed to allow ignition
  if (is_armed)
  {
    is_firing = true;
    digitalWrite(PIN_RELAY_FIRE, LOW);
  }
}

void stop_ignition()
{ // Disable ignition circuit
  is_firing = false;
  digitalWrite(PIN_RELAY_FIRE, HIGH);
}

/*
 * Controls for the Rocket through the ombilicals
 */

void enable_telemetry()
{ // Enable the Telemetry and FPV transmitters (on the rocket)
  is_tm_enabled = true;
  digitalWrite(PIN_OMBI_TM, HIGH);
}

void disable_telemetry()
{ // Disable the Telemetry and FPV transmitters (on the rocket)
  is_tm_enabled = false;
  digitalWrite(PIN_OMBI_TM, LOW);
}

void trigger_calibration()
{ // Trigger a calibration routine (on the rocket)
  digitalWrite(PIN_OMBI_CA, LOW);
  delay(100);
  digitalWrite(PIN_OMBI_CA, HIGH);
}
