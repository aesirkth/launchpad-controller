
// Including  RadioHead library 
#include <SPI.h>
#include <RH_RF95.h>
#include <RHReliableDatagram.h>

#define BAUDRATE 115200
#define BONJOUR "LAUNCHPADSTATION"
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
    if (rf95.waitAvailableTimeout(1000))
    {  
      uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
      uint8_t len = sizeof(buf);
      if (rf95.recv(buf, &len))
      {
        Serial.write(*buf);Serial.write('\r');Serial.write('\n');
      }
    }
  }
}

void init_communication()
{ // Initialize the communication link

  /*
   * Serial link
   */
  Serial.begin(BAUDRATE);
  Serial.println(BONJOUR);

  /*
   * RFM9xW module
   */
  // Reset of RFM95
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
 
  rf95.setFrequency(RF95_FREQ);
  rf95.setTxPower(23, false);
}
