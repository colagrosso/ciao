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

// Reads analog input from the Arduino and displays them to the Ciao app

#include <CiaoUSB.h>

#define SERVER_PORT 5285

void setup()
{
  Serial.begin(115200);
  Ciao.begin("Analog Read", SERVER_PORT);
  Ciao.addDisplay("Analog IN 0", "AIN0");
  Ciao.addDisplay("Analog IN 1", "AIN1");
  Ciao.announce();
}

void loop()
{ 
  // Call this once per loop().
  Ciao.run();
  int sensorValue = analogRead(A0);
  Ciao.print("AIN0 ");
  Ciao.println(sensorValue);
  sensorValue = analogRead(A1);
  Ciao.print("AIN1 ");
  Ciao.println(sensorValue);
  delay(500);
}

