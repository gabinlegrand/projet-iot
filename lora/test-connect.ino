/*
#include <Sodaq_RN2483.h>
#include <Sodaq_wdt.h>
#include <StringLiterals.h>
#include <Switchable_Device.h>
#include <Utils.h> */

/*
* Copyright (c) 2015 SODAQ. All rights reserved.
*
* This file is part of Sodaq_RN2483.
*
* Sodaq_RN2483 is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation, either version 3 of
* the License, or(at your option) any later version.
*
* Sodaq_RN2483 is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with Sodaq_RN2483.  If not, see
* <http://www.gnu.org/licenses/>.
*/
#define DEBUG
#include <Sodaq_RN2483.h>

#define uplinkCnt 10

// LED color customization for all the application
// Should give color mix for Orange (0xFF6600) assuming color index is accurate
#define LED_RED_CUST    0x00
#define LED_GREEN_CUST  0x99
#define LED_BLUE_CUST   0xFF

#define debugSerial SerialUSB
#define loraSerial  Serial2

//#define ABP
#define OTAA

#ifdef ABP
  // ABP Keys - Use your own KEYS!  
  const uint8_t devAddr[4] =  {0x00, 0x00, 0x00, 0x00} ;
  const uint8_t nwkSKey[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB7, 0x93, 0xEE, 0x79, 0x2F, 0x05, 0xF7} ;
  const uint8_t appSKey[16] = {0xD4, 0x46, 0xCF, 0x21, 0x32, 0x8B, 0xB9, 0xEB, 0xFF, 0x7D, 0x83, 0x9D, 0xC8, 0x79, 0xE4, 0x10} ;
#else
  // OTAA Keys - Use your own KEYS!
  const uint8_t devEUI[8]  = {0x11, 0x22, 0x33, 0xFF, 0xD1, 0xD2, 0xD3, 0xD4} ;
  const uint8_t appEUI[8]  = {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x04, 0x31, 0xD8} ;
  const uint8_t appKey[16] = {0xAF, 0xCF, 0x06, 0x67, 0x06, 0x72, 0xD9, 0x2D, 0x31, 0x98, 0xD6, 0xE0, 0x42, 0xF0, 0xE4, 0x5D} ;
#endif

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

  //Initialize the LEDs and turn them all off
  pinMode(LED_RED, OUTPUT) ;
  pinMode(LED_GREEN, OUTPUT) ;
  pinMode(LED_BLUE, OUTPUT) ;

  digitalWrite(LED_RED, HIGH) ;
  digitalWrite(LED_GREEN, HIGH) ;
  digitalWrite(LED_BLUE, HIGH) ;

  // Power Up LED
  analogWrite(LED_RED,   LED_RED_CUST) ;
  analogWrite(LED_GREEN, LED_GREEN_CUST) ;
  analogWrite(LED_BLUE,  LED_BLUE_CUST) ;
  
  delay(10000) ;

  // Turn off the LEDs
  digitalWrite(LED_RED, HIGH) ;
  digitalWrite(LED_GREEN, HIGH) ;
  digitalWrite(LED_BLUE, HIGH) ;
  
	LoRaBee.setDiag(debugSerial) ; // optional
 
	#ifdef ABP
    if (LoRaBee.initABP(loraSerial, devAddr, appSKey, nwkSKey, true))
	  {
		  debugSerial.println("ABP Keys Accepted.") ;
	  }
	  else
	  {
		  debugSerial.println("ABP Key Setup Failed!") ;
	  }
  #else
    if (LoRaBee.initOTA(loraSerial, devEUI, appEUI, appKey, true))
    {
      debugSerial.println("OTAA Keys Accepted.") ;
    }
    else
    {
      debugSerial.println("OTAA Keys Setup Failed!") ;
    }
  #endif
  
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

      analogWrite(LED_RED,   LED_RED_CUST) ;
      analogWrite(LED_GREEN, LED_GREEN_CUST) ;
      analogWrite(LED_BLUE,  LED_BLUE_CUST) ;
  
      delay(500) ;
      
      digitalWrite(LED_RED, HIGH) ;
      digitalWrite(LED_GREEN, HIGH) ;
      digitalWrite(LED_BLUE, HIGH) ;
      
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
