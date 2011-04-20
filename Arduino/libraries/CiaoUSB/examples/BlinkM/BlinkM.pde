//  Copyright (C) 2011 Mike Colagrosso
//  http://colagrosso.net
//
//  This file is part of Ciao: Communicate with an Arduino over Bonjour.
//
//  Ciao is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as
//  published by the Free Software Foundation, either version 3 of
//  the License, or (at your option) any later version.
//
//  Ciao is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with EthernetDHCP. If not, see
//  <http://www.gnu.org/licenses/>.
//

// Allows a Ciao app to view and change the color of a BlinkM

#include "Wire.h"
#include "BlinkM_funcs.h"

#include <Messenger.h>
#include <CiaoUSB.h>

#define BLINKM_ARDUINO_POWERED 1
byte blinkm_addr = 0x09; // the default address of all BlinkMs
#define DEBUG 0
#define SERVER_PORT 5282
#define LED_PIN 3

// Instantiate Messenger object with the message function and the default separator 
// (the space character)
Messenger message = Messenger(); 

// Define the max size of the string
// The size must be big enough to hold the longest string you are expecting
#define MAXSIZE 30 

// Create a char array (string) to hold the received string
char string[MAXSIZE];

void sendColor(int colorIndex) {
  Ciao.print("color ");
  switch(colorIndex) {
    case 1:
      Ciao.println("Blue");
      break;
    case 2:
      Ciao.println("Green");
      break;
    case 3:
      Ciao.println("Red");
      break;
  }
}

// Define messenger function
void messageCompleted() {

  if (DEBUG) Serial.println("message completed");

  while ( message.available() ) {
    if (message.checkString("blue") || message.checkString("CIAO")) {
      if (DEBUG) Serial.println("matched blue or CIAO");
      BlinkM_stopScript(blinkm_addr);
      BlinkM_setRGB(blinkm_addr, 0, 0, 0xff);
      sendColor(1);
    } 
    else if (message.checkString("green")) {
      if (DEBUG) Serial.println("matched green");
      BlinkM_stopScript(blinkm_addr);
      BlinkM_setRGB(blinkm_addr, 0, 0xff, 0);
      sendColor(2);
    } 
    else if (message.checkString("red")) {
      if (DEBUG) Serial.println("matched red");
      BlinkM_stopScript(blinkm_addr);
      BlinkM_setRGB(blinkm_addr, 0xff, 0, 0);
      sendColor(3);
    } 
    else {
      if (DEBUG) {
        Serial.println("didn't match CIAO, blue, green, or red");
        char buf[80];
        message.copyString(buf, 80);
        Serial.print("Got: ");
        Serial.println(buf);
      }
    }
  }
}


void setup()
{
  Serial.begin(115200);
  
  if( BLINKM_ARDUINO_POWERED )
    BlinkM_beginWithPower();
  else
    BlinkM_begin();

  delay(100); // wait a bit for things to stabilize
  
  byte addr = BlinkM_getAddress(blinkm_addr);
  if( addr != blinkm_addr ) {
    if( addr == -1 ) 
      Serial.println("\r\nerror: no response");
    else if( addr != blinkm_addr ) {
      Serial.print("\r\nerror: addr mismatch, addr received: ");
      Serial.println(addr, HEX);
    }
  }

  message.attach(messageCompleted);

  Ciao.begin("BlinkM", SERVER_PORT);
  Ciao.addDisplay("Color", "color");
  Ciao.addButton("Red", "red");
  Ciao.addButton("Green", "green");
  Ciao.addButton("Blue", "blue");
  Ciao.announce();
}

void loop()
{ 
  // Call this once per loop().
  Ciao.run();
  
  int available = Serial.available();
  if (available) {
    char c = Serial.read();
    message.process(c);
    delay(1);
  }
}

