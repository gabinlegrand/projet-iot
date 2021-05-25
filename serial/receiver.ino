char mystr[50]; //Initialized variable to store recieved data

void setup() {
  // Begin the Serial at 9600 Baud
  Serial.begin(9600);
  SerialUSB.begin(9600);
}

void loop() {
  if(Serial.available() > 0){
    Serial.readBytes(mystr,10); //Read the serial data and store in var
    SerialUSB.println(mystr); //Print data on Serial Monitor
    delay(1000);
  }
}
