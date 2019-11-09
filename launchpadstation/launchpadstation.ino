/* Embedded software for the Launch Pad Station

/!\ Power the Launch Pad Station Board BEFORE powering the relays /!\

This code receives commands from a LoRa module RFM9XW and controls the rocket fuel sequence
Commands are also forwarded to the rocket itself using logic levels on single wires

This is tested on Arduino Nano 3.0 boards

Hardware :
  - 1x Arduino Nano
  - 1x RFM9xW LoRa tranceiver
  - 3x Relay
  - 1x Rocket

Wiring :
                Arduino      RFM95/96/97/98
                GND----------GND   (ground in)
                3V3----------3.3V  (3.3V in)
interrupt 0 pin D2-----------DIO0  (interrupt request out)
                D9-----------RESET (reset pin)
         SS pin D10----------NSS   (CS chip select in)
        SCK pin D13----------SCK   (SPI clock in)
       MOSI pin D11----------MOSI  (SPI Data in)
       MISO pin D12----------MISO  (SPI Data out)
                Connect everything through level shifters 5V <-> 3V3
                The RFM9XW are 3V3 logic and are NOT 5V tolerant

                Arduino      Rocket
                GND----------GND   (ground in)
                A0-----------Ombilical 1 (Telemetry/FPV enable)
                A1-----------Ombilical 2 (Calibration start)

                Arduino      Ignition Relay
                GND----------GND   (ground in)
                A2-----------IN1   (Command pin)
                5V-----------VCC   (5V in)
                Connect the ignition circuit on Normally Open side

                Arduino      FILL/VENT Relay (NB: jumper between VCC and JD-VCC)
                GND----------GND   (ground in)
                A3-----------IN2   (Command pin for relay 1)
                A4-----------IN1   (Command pin for relay 2)
                5V-----------VCC   (5V in)
                Connect the solenoids on Normally Open side
*/

// Single wire ombilicals to the rocket
#define PIN_OMBI_TM A0 // Write LOW to this pin to disable the Telemetry and FPV transmitters
#define PIN_OMBI_CA A1 // Write LOW to this pin to start a sensor calibration on the rocket
// Pins where the relays are connected
#define PIN_RELAY_FIRE A2 // Write LOW to this pin to enable the ignition circuit
#define PIN_RELAY_FILL A3 // Write LOW to this pin to open solenoid 1
#define PIN_RELAY_VENT A4 // Write LOW to this pin to open solenoid 2

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

#define REPLY_ACK  0x3D // '=' This is returned if the command is successfuly executed
#define REPLY_NACK 0x21 // '!' This is returned if the command is not successfuly executed

uint8_t command = 0x00;
bool is_filling = false;
bool is_venting = false;
bool is_armed = false;

void setup()
{
  pinMode(PIN_RELAY_FIRE, OUTPUT);
  pinMode(PIN_RELAY_FILL, OUTPUT);
  pinMode(PIN_RELAY_VENT, OUTPUT);
  pinMode(PIN_OMBI_TM, OUTPUT);
  pinMode(PIN_OMBI_CA, OUTPUT);
  // Disable the ignition circuit
  digitalWrite(PIN_RELAY_FIRE, HIGH);
  // Close the solenoids
  digitalWrite(PIN_RELAY_FILL, HIGH);
  digitalWrite(PIN_RELAY_VENT, HIGH);
  // Default state is HIGH for the ombilicals
  digitalWrite(PIN_OMBI_TM, HIGH);
  digitalWrite(PIN_OMBI_CA, HIGH);

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
    write_byte(REPLY_NACK);
      break;
    }
  }
  // Reset this to the default value
  command = 0x00;
  delay(100);
}

void init_communication()
{ // Initialize the communication link
  Serial.begin(115200);
}

void read_byte(uint8_t *data)
{ // Read one byte in the buffer
  if (Serial.available() > 0)
  {
    *data = Serial.read();
  }
}

void write_byte(uint8_t data)
{ // Write one byte to the communication link
  Serial.write(data);
}

void start_filling()
{ // Enable solenoid 1 only if solenoid 2 is disabled
  if (!is_venting)
  {
    is_filling = true;
    digitalWrite(PIN_RELAY_FILL, LOW);
    write_byte(REPLY_ACK);
    write_byte(CMD_FILL_START);
  }
  else
  {
    write_byte(REPLY_NACK);
    write_byte(CMD_FILL_START);
  }
}

void stop_filling()
{ // Disable solenoid 1
  is_filling = false;
  digitalWrite(PIN_RELAY_FILL, HIGH);
  write_byte(REPLY_ACK);
  write_byte(CMD_FILL_STOP);
}

void start_venting()
{ // Enable solenoid 2 only if solenoid 1 is disabled
  if (!is_filling)
  {
    is_venting = true;
    digitalWrite(PIN_RELAY_VENT, LOW);
    write_byte(REPLY_ACK);
    write_byte(CMD_VENT_START);
  }
  else
  {
    write_byte(REPLY_NACK);
    write_byte(CMD_VENT_START);
  }
}

void stop_venting()
{ // Disable solenoid 2
  is_venting = false;
  digitalWrite(PIN_RELAY_VENT, HIGH);
  write_byte(REPLY_ACK);
  write_byte(CMD_VENT_STOP);
}

void arm()
{ // Set is_armed to true
  // is_armed must be true to allow ignition
  is_armed = true;
  write_byte(REPLY_ACK);
  write_byte(CMD_ARM);
}

void disarm()
{ // Set is_armed to false
  // is_armed must be true to allow ignition
  is_armed = false;
  // Also stop firing
  digitalWrite(PIN_RELAY_FIRE, HIGH);
  write_byte(REPLY_ACK);
  write_byte(CMD_DISARM);
}

void start_ignition()
{ // Enable ignition circuit
  // is_armed must be true to allow ignition
  // Solenoid 1 must be closed to allow ignition
  // Solenoid 2 must be closed to allow ignition
  if (is_armed && !is_filling && !is_venting)
  {
    digitalWrite(PIN_RELAY_FIRE, LOW);
    write_byte(REPLY_ACK);
    write_byte(CMD_FIRE_START);
  }
  else
  {
    write_byte(REPLY_NACK);
    write_byte(CMD_FIRE_START);
  }
}

void stop_ignition()
{ // Disable ignition circuit
  digitalWrite(PIN_RELAY_FIRE, HIGH);
  write_byte(REPLY_ACK);
  write_byte(CMD_FIRE_STOP);
}

void enable_telemetry()
{ // Enable the Telemetry and FPC transmitters (on the rocket)
  digitalWrite(PIN_OMBI_TM, HIGH);
  write_byte(REPLY_ACK);
  write_byte(CMD_TM_ENABLE);
}

void disable_telemetry()
{ // Disable the Telemetry and FPC transmitters (on the rocket)
  digitalWrite(PIN_OMBI_TM, LOW);
  write_byte(REPLY_ACK);
  write_byte(CMD_TM_DISABLE);
}

void trigger_calibration()
{ // Trigger a calibration routine (on the rocket)
  digitalWrite(PIN_OMBI_CA, LOW);
  delay(100);
  digitalWrite(PIN_OMBI_CA, HIGH);
  write_byte(REPLY_ACK);
  write_byte(CMD_CA_TRIGGER);
}