//  Copyright (C) 2010 Georg Kaindl
//  http://gkaindl.com
//
//  This file is part of Arduino EthernetDHCP.
//
//  EthernetDHCP is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as
//  published by the Free Software Foundation, either version 3 of
//  the License, or (at your option) any later version.
//
//  EthernetDHCP is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with EthernetDHCP. If not, see
//  <http://www.gnu.org/licenses/>.
//

//  Illustrates how to use EthernetDHCP in synchronous (blocking)
//  mode.

#if defined(ARDUINO) && ARDUINO > 18
#include <SPI.h>
#endif
#include <Ethernet.h>
#include <EthernetDHCP.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

const char* ip_to_str(const uint8_t*);

void setup()
{
  Serial.begin(9600);

  Serial.println("Attempting to obtain a DHCP lease...");
  
  // Initiate a DHCP session. The argument is the MAC (hardware) address that
  // you want your Ethernet shield to use. This call will block until a DHCP
  // lease has been obtained. The request will be periodically resent until
  // a lease is granted, but if there is no DHCP server on the network or if
  // the server fails to respond, this call will block forever.
  // Thus, you can alternatively use polling mode to check whether a DHCP
  // lease has been obtained, so that you can react if the server does not
  // respond (see the PollingDHCP example).
  EthernetDHCP.begin(mac);

  // Since we're here, it means that we now have a DHCP lease, so we print
  // out some information.
  const byte* ipAddr = EthernetDHCP.ipAddress();
  const byte* gatewayAddr = EthernetDHCP.gatewayIpAddress();
  const byte* dnsAddr = EthernetDHCP.dnsIpAddress();
  
  Serial.println("A DHCP lease has been obtained.");

  Serial.print("My IP address is ");
  Serial.println(ip_to_str(ipAddr));
  
  Serial.print("Gateway IP address is ");
  Serial.println(ip_to_str(gatewayAddr));
  
  Serial.print("DNS IP address is ");
  Serial.println(ip_to_str(dnsAddr));
}

void loop()
{
  // You should periodically call this method in your loop(): It will allow
  // the DHCP library to maintain your DHCP lease, which means that it will
  // periodically renew the lease and rebind if the lease cannot be renewed.
  // Thus, unless you call this somewhere in your loop, your DHCP lease might
  // expire, which you probably do not want :-)
  EthernetDHCP.maintain();
}

// Just a utility function to nicely format an IP address.
const char* ip_to_str(const uint8_t* ipAddr)
{
  static char buf[16];
  sprintf(buf, "%d.%d.%d.%d\0", ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
  return buf;
}
