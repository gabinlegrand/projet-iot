#define DEBUG
#include <Sodaq_RN2483.h>

#define uplinkCnt 10

#define debugSerial SerialUSB
#define loraSerial  Serial2

const uint8_t devEUI[8]  = {0x11, 0x22, 0x33, 0xFF, 0xD1, 0xD2, 0xD3, 0xD5} ;
const uint8_t appEUI[8]  = {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x04, 0x31, 0xD8} ;
const uint8_t appKey[16] = {0xAF, 0xCF, 0x06, 0x67, 0x06, 0x72, 0xD9, 0x2D, 0x31, 0x98, 0xD6, 0xE0, 0x42, 0xF0, 0xE4, 0x5E} ;

// Payload is 'Microchip ExpLoRa' in HEX
const uint8_t testPayload[] = {0x4d, 0x69, 0x63, 0x72, 0x6f, 0x63, 0x68, 0x69, 0x70, 0x20, 0x45, 0x78, 0x70, 0x4c, 0x6f, 0x52, 0x61} ;

void setup()
{  
  debugSerial.begin(57600) ;
  loraSerial.begin(LoRaBee.getDefaultBaudRate()) ;

  //Set the temperature sensor pin as input
  pinMode(TEMP_SENSOR, INPUT) ;

  //Set ADC resolution to 12 bits
  analogReadResolution(12) ;
  
  delay(10000) ;
  
  LoRaBee.setDiag(debugSerial) ; // optional
 
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
  // Initialize voltage for temperature sensor
  float sum = 0.0;
  float voltage = 0.0;
  float temperature = 0.0;
  uint8_t samples = 10 ;
  uint8_t payload[] = {0x00, 0x00, 0x00} ;
  uint8_t data ;
  
  // Send x packets, with at least a 10 seconds delay after each transmission (more seconds if the device is busy)
  uint8_t i = uplinkCnt ;
  while (i > 0)
  {
    sum = 0.0 ;    // reset averaged reading to start at 0
    // This loop does 10 readings of the voltage, and adds this to the variable
    for(int i = 0; i < samples; i++)
    {
      sum += 3.3/4095.0 * (float)analogRead(TEMP_SENSOR) ;
    }

    // Divide the voltage to get average
    voltage = sum/samples ;

    debugSerial.println(voltage, 1) ;

    // Convert voltage to temperature
    temperature = (voltage - 0.5) * 100.0 ;

    // Print temperature to serial terminal
    debugSerial.print(temperature, 1) ;
    debugSerial.println(" C") ;

    if (temperature >= 0)
    {
      payload[0] = '+' ;
    }
    else
    {
      payload[0] = '-' ;
    }

    data = (int)temperature ;
    payload[2] = '0' + (data % 10) ;
    data /= 10 ;
    payload[1] = '0' + (data % 10) ;
  
    switch (LoRaBee.send(1, payload, 3))
    {
    case NoError:
      debugSerial.println("Successful transmission.");

  
      delay(500) ;
      
      
      i-- ;
      break ;
    case NoResponse:
      debugSerial.println("There was no response from the device.") ;
      break ;
    case Timeout:
      debugSerial.println("Connection timed-out. Check your serial connection to the device! Sleeping for 20sec.") ;
      delay(20000) ;
      break ;
    case PayloadSizeError:
      debugSerial.println("The size of the payload is greater than allowed. Transmission failed!") ;
      break ;
    case InternalError:
      debugSerial.println("Oh No! This shouldn't happen. Something is really wrong! Try restarting the device!\r\nThe program will now halt.") ;
      while (1) {} ;
      break ;
    case Busy:
      debugSerial.println("The device is busy. Sleeping for 10 extra seconds.") ;
      delay(10000) ;
      break ;
    case NetworkFatalError:
      debugSerial.println("There is a non-recoverable error with the network connection. You should re-connect.\r\nThe program will now halt.") ;
      while (1) {} ;
      break ;
    case NotConnected:
      debugSerial.println("The device is not connected to the network. Please connect to the network before attempting to send data.\r\nThe program will now halt.") ;
      while (1) {} ;
      break ;
    case NoAcknowledgment:
      debugSerial.println("There was no acknowledgment sent back!") ;
      break ;
    default:
      break ;
    }
   
    delay(10000) ;
  }

  debugSerial.println("End!");

  //LoRaBee.sleep() ;
 
  while (1) 
  { } // block forever
}
