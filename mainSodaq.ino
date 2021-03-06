#include <Sodaq_RN2483.h>
#include <Servo.h>

#define debugSerial  SerialUSB
#define loraSerial  Serial2

const uint8_t devEUI[8]  = {0x11, 0x22, 0x33, 0xFF, 0xD1, 0xD2, 0xD3, 0xD7} ;
const uint8_t appEUI[8]  = {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x04, 0x31, 0xD2} ;
const uint8_t appKey[16] = {0xAF, 0xCF, 0x06, 0x67, 0x06, 0x72, 0xD9, 0x2D, 0x31, 0x98, 0xD6, 0xE0, 0x42, 0xF0, 0xE4, 0x57} ;

// Payload is 'Microchip ExpLoRa' in HEX
const uint8_t testPayload[] = {0x4d, 0x69, 0x63, 0x72, 0x6f, 0x63, 0x68, 0x69, 0x70, 0x20, 0x45, 0x78, 0x70, 0x4c, 0x6f, 0x52, 0x61} ;

char rxStr[50];
String list[20];
int listCpt = 0;

Servo servoLeft;
Servo servoRight;

int pos = 0;

void setup()
{
  servoLeft.attach(3);
  servoRight.attach(4);
  servoLeft.write(5);
  servoRight.write(45);

  debugSerial.begin(57600);
  Serial.begin(57600);
  loraSerial.begin(LoRaBee.getDefaultBaudRate()) ;

  delay(10000) ;

  LoRaBee.setDiag(debugSerial) ;

  if (LoRaBee.initOTA(loraSerial, devEUI, appEUI, appKey, true))
  {
    debugSerial.println("OTAA Keys Accepted.") ;
  }
  else
  {
    debugSerial.println("OTAA Keys Setup Failed!") ;
  }

  debugSerial.println("Sleeping for 5 seconds before starting sending out test packets.");
  for (uint8_t i = 5; i > 0; i--)
  {
    debugSerial.println(i) ;
    delay(1000) ;
  }

  if (LoRaBee.send(1, testPayload, sizeof(testPayload)) != NoError)
  {
    debugSerial.println("First payload was sent") ;
  }
}

void loop()
{
  char dnbuf[16];
  char upbuf[16];

  LoRaBee.getMacParam("dnctr", dnbuf, 16);
  LoRaBee.getMacParam("upctr", upbuf, 16);

  debugSerial.print("Downlink frame counter: ");
  debugSerial.println(dnbuf);
  debugSerial.print("Uplink frame counter: ");
  debugSerial.println(upbuf);

  debugSerial.println("Sending message...");

  uint8_t buf[] = {'t', 'e', 's', 't'};
  uint8_t i = LoRaBee.send(1, buf, sizeof(buf));
  debugSerial.print("LoRa send: ");
  debugSerial.println(i);
  downlink();
  rxtx();
  debugSerial.println("");
  debugSerial.println("Need wait 3 seconds...");
  delay(3000);
}

void downlink()
{
  uint8_t payload[64];
  uint16_t len = LoRaBee.receive(payload, 64);
  String HEXPayload = "";
  if (payload[0] != 131)
  {
    HEXPayload += " ";
    for (int i = 0; i < len-1; i++)
    {
      HEXPayload += String(payload[i], DEC);
    }

    debugSerial.println(HEXPayload);

    if (HEXPayload == " 111112101") {
      servoMotor();
      return;
    }

    if(HEXPayload == " 99108111115"){
      closeDoor();
      return;
    }

    for (int i = 0; i < listCpt + 1; i ++)
    {
      debugSerial.println(list[i]);
      if (HEXPayload == list[i]) {
        debugSerial.println("it exist");
        return;
      }
    }
    list[listCpt] = HEXPayload;
    listCpt++;
  }
  else
  {
    debugSerial.println("no payload");
  }
}

void servoMotor() {
  servoLeft.write(65);
  servoRight.write(5);
  delay(2000);
}

void closeDoor() {
  servoLeft.write(5);
  servoRight.write(45);
  delay(2000);
}

void rxtx() {
  if (Serial.available() > 0)
  {
    Serial.readBytes(rxStr, 20);
    debugSerial.println(rxStr);
    for (int i = 0; i < listCpt + 1; i ++)
    {
      debugSerial.println(rxStr);
      if (String(rxStr) == list[i]) {
        servoMotor();
        return;
      }
    }
    delay(1000);
  }
}
