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
#include "Ciao.h"
#include <stdlib.h>
#include <string.h>
#include "../EthernetBonjour/EthernetBonjour.h"

#define BUFFER_INCREMENT 40
#define MAX_BUFFER_SIZE  400

CiaoClass::CiaoClass() {
  n = 0;
  txtString = (char *)malloc(BUFFER_INCREMENT);
  txtStringLength = 0;
  txtString[txtStringLength] = '\0';
  bufferSize = BUFFER_INCREMENT;
  char *formatStr = "format=Messenger";
  txtStringAppend(formatStr, strlen(formatStr));
}

void CiaoClass::begin(char *programName, int port) {
  begin("arduino", programName, port);
}

void CiaoClass::begin(char *hostname, char *programName, int listenPort) {
  int size = strlen(programName) + 7;
  name = (char *) malloc(size);
  memset (name, '\0', size);
  strncpy(name, programName, strlen(programName));
  strncat(name, "._ciao", strlen("._ciao"));
  port = listenPort;
  EthernetBonjour.begin(hostname);
}

void CiaoClass::addDisplay(char *label) {
  addDisplay(label, label);
}

void CiaoClass::addDisplay(char *label, char *prefix) {
  addPair(label, "label", prefix, "prefix");
}

void CiaoClass::addButton(char *label) {
  addButton(label, label);
}

void CiaoClass::addButton(char *label, char *send) {
  addPair(label, "label", send, "send");
}

void CiaoClass::addPair(char *value1, char *key1, char *value2, char *key2) {
  n++;
  
  char keyValStr[20];
  char nStr[6];
  memset (nStr, '\0', 6);
  itoa(n, nStr, 10);

  memset (keyValStr, '\0', 20);
  strncpy(keyValStr, key1, strlen(key1));
  strncat(keyValStr, nStr, strlen(nStr));
  strncat(keyValStr, "=", 1);
  strncat(keyValStr, value1, strlen(value1));
  txtStringAppend(keyValStr, strlen(keyValStr));
  
  memset (keyValStr, '\0', 20);
  strncpy(keyValStr, key2, strlen(key2));
  strncat(keyValStr, nStr, strlen(nStr));
  strncat(keyValStr, "=", 1);
  strncat(keyValStr, value2, strlen(value2));
  txtStringAppend(keyValStr, strlen(keyValStr));
}

void CiaoClass::announce() {
  EthernetBonjour.addServiceRecord(name,
                                   port,
                                   MDNSServiceTCP,
                                   getTxtString());
  freeTxtString();
}

void CiaoClass::run() {
  EthernetBonjour.run();
}

char * CiaoClass::getTxtString() {
  // Add the n= string
  char nStr[6];
  memset (nStr, '\0', 6);
  strncpy(nStr, "n=", 2);
  itoa(n, &nStr[2], 10);
  txtStringAppend(nStr, strlen(nStr));
  return txtString;
}

void CiaoClass::txtStringAppend(char *str, int length) {
  if (!checkTxtStringSize(length)) {
    return;
  }
  txtString[txtStringLength] = length;
  txtStringLength++;
  txtString[txtStringLength] = '\0';
  strncat(txtString, str, length);
  txtStringLength += length;
  txtString[txtStringLength] = '\0';
}

bool CiaoClass::checkTxtStringSize(int length) {
  if (bufferSize > MAX_BUFFER_SIZE || txtStringLength + length > MAX_BUFFER_SIZE)
    return false;

  while (txtStringLength + length > bufferSize) {
    bufferSize += BUFFER_INCREMENT;
    if (bufferSize > MAX_BUFFER_SIZE)
      return false;
    txtString = (char *)realloc(txtString, bufferSize);
  }
  return true;
}

void CiaoClass::freeTxtString() {
  free(txtString);
}

CiaoClass Ciao;