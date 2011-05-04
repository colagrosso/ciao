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

// Allows a Ciao app to display and toggle the status of an LED

#if defined(ARDUINO) && ARDUINO > 18
#include <SPI.h>
#endif
#include <Ethernet.h>
//#include <EthernetDHCP.h>
#include <EthernetBonjour.h>
#include <Messenger.h>
#include <Ciao.h>

#define DEBUG 0

#define SERVER_PORT 5283

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

byte ip[] = {10, 0, 1, 129};

#define LED_PIN 3

int ledState = HIGH;

// Instantiate Messenger object with the message function and the default separator 
// (the space character)
Messenger message = Messenger(); 

// Define the max size of the string
// The size must be big enough to hold the longest string you are expecting
#define MAXSIZE 30 

Server server(SERVER_PORT);
Client client = Client(255);

// Create a char array (string) to hold the received string
char string[MAXSIZE];

void toggle() {
  ledState = !ledState;
  digitalWrite(LED_PIN, ledState);
}

// Define messenger function
void messageCompleted() {

  if (DEBUG) Serial.println("message completed");

  while ( message.available() ) {
    if (message.checkString("CIAO")) {
      if (DEBUG) Serial.println("CIAO");
    } 
    else if (message.checkString("toggle")) {
      if (DEBUG) Serial.println("matched toggle");
      toggle();
    } 
    else {
      if (DEBUG) {
        Serial.println("didn't match CIAO or toggle");
        char buf[MAXSIZE];
        message.copyString(buf, MAXSIZE);
        Serial.print("Got: ");
        Serial.println(buf);
      }
    }
    client.print("state ");
    if (ledState == HIGH) {
      client.println("On");      
    } else {
      client.println("Off");      
    }
  }
}


void setup()
{
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, ledState);
  
  Serial.begin(115200);
  message.attach(messageCompleted);
  if (DEBUG) Serial.println("Starting Ethernet...");
  Ethernet.begin(mac, ip);
  server.begin();

  Ciao.begin("Toggle", SERVER_PORT);
  Ciao.addDisplay("State", "state");
  Ciao.addButton("Toggle", "toggle");
  Ciao.announce();
}

void loop()
{ 
  // Call this once per loop().
  Ciao.run();
  client = server.available();
  if (client) {
    if (DEBUG) Serial.println("client connected");
    while (client.connected()) {
      while (client.available()) {
        char c = client.read();
        message.process(c);
        delay(1);
      }
    }
    delay(1);
    if (DEBUG) Serial.println("client stopping");
    client.stop();
  }
}

