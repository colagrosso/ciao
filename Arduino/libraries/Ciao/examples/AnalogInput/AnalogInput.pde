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
//  License along with Ciao. If not, see
//  <http://www.gnu.org/licenses/>.
//

// Reads analog input from the Arduino and displays them to the Ciao app

#if defined(ARDUINO) && ARDUINO > 18
#include <SPI.h>
#endif
#include <Ethernet.h>
//#include <EthernetDHCP.h>
#include <EthernetBonjour.h>
#include <Ciao.h>

#define DEBUG 0
#define SERVER_PORT 5285

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

byte ip[] = {10, 0, 1, 129};

Server server(SERVER_PORT);
Client client = Client(255);

void setup()
{
  Serial.begin(115200);
  if (DEBUG) Serial.println("Starting Ethernet...");
  Ethernet.begin(mac, ip);
  server.begin();
  Ciao.begin("Analog Read", SERVER_PORT);
  Ciao.addDisplay("Analog IN 0", "AIN0");
  Ciao.addDisplay("Analog IN 1", "AIN1");
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
      int sensorValue = analogRead(A0);
      client.print("AIN0 ");
      client.println(sensorValue);
      sensorValue = analogRead(A1);
      client.print("AIN1 ");
      client.println(sensorValue);
      delay(500);
    }
    delay(1);
    if (DEBUG) Serial.println("client stopping");
    client.stop();
  }
}

