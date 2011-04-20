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

#ifndef CIAO_USB_H
#define CIAO_USB_H

class CiaoUsbClass {
  public:
    CiaoUsbClass();
    void begin(char *programName, int port);
    void begin(char *hostname, char *programName, int listenPort);
    void addDisplay(char *label);
    void addDisplay(char *label, char *prefix);
    void addButton(char *label);
    void addButton(char *label, char *send);
    void announce();
    void run();
    void print(const char[]);
    void println(const char[]);
    void println(int a);

  private:
};

extern CiaoUsbClass Ciao;

#endif /* CIAO_USB_H */
