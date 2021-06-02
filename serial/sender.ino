#include <SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9
#define SS_PIN          10

MFRC522 mfrc522(SS_PIN, RST_PIN);
SoftwareSerial mySerial(6, 7); // RX, TX

unsigned long UID = 0;
char buf[16];
void setup() {
  // Begin the Serial at 57600 Baud
  mySerial.begin(57600);
  Serial.begin(9600);   // Initialize serial communications with the PC
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}

void loop() {
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  if (*((unsigned long *)mfrc522.uid.uidByte) != UID) {
    UID = *((unsigned long *)mfrc522.uid.uidByte); //recup UID  unsigned long
    ltoa(UID, buf, 10);
    Serial.println(buf);
    mySerial.write(buf, 10);
  }
}
