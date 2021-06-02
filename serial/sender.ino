#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); // RX, TX

char mystr[10] = "Coucou"; //String data

void setup() {
  // Begin the Serial at 57600 Baud
  mySerial.begin(57600);
}

void loop() {
  mySerial.write(mystr,10); //Write the serial data
  delay(1000);
}
