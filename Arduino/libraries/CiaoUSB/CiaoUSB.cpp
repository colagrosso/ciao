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

#include "WProgram.h"
#include "CiaoUSB.h"
#include <stdlib.h>
#include <string.h>

CiaoUsbClass::CiaoUsbClass() {
}

void CiaoUsbClass::begin(char *programName, int port) {
  begin("arduino", programName, port);
}

void CiaoUsbClass::begin(char *hostname, char *programName, int listenPort) {
  Serial.println("");
  Serial.println("");
  Serial.print("CIAO_HOSTNAME ");
  Serial.println(hostname);
  Serial.print("CIAO_NAME ");
  Serial.println(programName);
  Serial.print("CIAO_PORT ");
  Serial.println(listenPort);
}

void CiaoUsbClass::addDisplay(char *label) {
  addDisplay(label, label);
}

void CiaoUsbClass::addDisplay(char *label, char *prefix) {
  Serial.println("CIAO_DISPLAY");
  Serial.println(label);
  Serial.println(prefix);
}

void CiaoUsbClass::addButton(char *label) {
  addButton(label, label);
}

void CiaoUsbClass::addButton(char *label, char *send) {
  Serial.println("CIAO_BUTTON");
  Serial.println(label);
  Serial.println(send);
}

void CiaoUsbClass::announce() {
  Serial.println("CIAO_ANNOUNCE");
}

void CiaoUsbClass::run() {
  // No-op when using USB
}

void CiaoUsbClass::print(const char* s) {
  Serial.print("CIAO_PRINT ");
  Serial.println(s);
}

void CiaoUsbClass::println(const char* s) {
  Serial.print("CIAO_PRINTLN ");
  Serial.println(s);
}

void CiaoUsbClass::println(int a) {
  Serial.print("CIAO_PRINTLN ");
  Serial.println(a);
}


CiaoUsbClass Ciao;