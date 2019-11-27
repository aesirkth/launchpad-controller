/* Embedded software for the Launch Pad Station Gateway

This code forwards data between the Serial link of an Arduino board and a RFM9X LoRa transceiver
connected to it.

When a byte is available on the Serial link, it is transmitted via the LoRa transceiver
When a packet is available on the LoRa transceiver, it is sent to the Serial link

Hardware :
  - 1x Arduino Nano
  - 1x RFM9X LoRa transceiver

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

*/
#include <SPI.h>
#include <RH_RF95.h> // RadioHead library  to control the LoRa transceiver

// Defines for the Serial link
#define BAUDRATE 115200
#define BONJOUR "LAUNCHPADSTATION"

// Defines for the LoRa transceiver
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2
#define RF95_FREQ 915.0

RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup()
{  
  init_communication();
}

void loop()
{
  if (Serial.available() > 0)
  {
    uint8_t radiopacket = Serial.read();
    
    delay(10);
    rf95.send(&radiopacket, 1);
    rf95.waitPacketSent();
    delay(10);
  }
  if (rf95.waitAvailableTimeout(100))
  {  
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len))
    {
      Serial.write(buf, len);
    }
  }
}

void init_communication()
{ // Initialize the communication links
  // Enable the Serial link and send 'BONJOUR' to be recognized by the control interface
  Serial.begin(BAUDRATE);
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
