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
//
//  Based on previous work by Jordan Terrell
//  http://blog.jordanterrell.com/post/Arduino-DHCP-Library-Version-04.aspx

#include <string.h>
#include <stdlib.h>

extern "C" {
   #include "wiring.h"
   #include <utility/EthernetUtil.h>
}

#include <utility/EthernetCompat.h>
#include "EthernetDHCP.h"

#define     DEFAULT_HOST_NAME ("Arduino")
#define     MAX_HOST_NAME_LEN (255)

#define     RETRY_TIMEOUT     (3000)

#define     DHCP_SERVER_PORT  (67)
#define     DHCP_CLIENT_PORT  (68)

#define     DHCP_HTYPE10MB    (1)
#define     DHCP_HTYPE100MB   (2)
#define     DHCP_HWADDRLEN    (6)
#define     DHCP_HOPS         (0)
#define     MAX_DHCP_MSG_SIZE (1500)

#define     DEFAULT_T1        (900)    // default time until renew, 15 minutes (in seconds)

#define     DHCP_OPTS_MAGIC_COOKIE     (0x63825363)

#define     NUM_SOCKETS       (4)
#define     RX_TX_BUF_SIZE    (0x55)

typedef struct _BootpPacket_t {
   uint8_t     opCode, hwType, hwAddrLen, hops;
   uint32_t    xid;
   uint16_t    secs, flags;
   uint8_t     clientIpAddr[4];
   uint8_t     yourIpAddr[4];
   uint8_t     serverIpAddr[4];
   uint8_t     gwIpAddr[4];
   uint8_t     clientHwAddr[16];
} __attribute__((__packed__)) BootpPacket_t;

EthernetDHCPClass::EthernetDHCPClass()
{
   this->_state = DhcpStateNone;
   this->_hostName = NULL;
   this->_useHostNameAsClientIdentifier = 0;
   this->_millisecondsAccum = 0;
   this->_secsUntilRenew = 0;
   this->_secsUntilRebind = 0;
   this->_lastSendMillis = 0;
   this->_lastPollMillis = 0;
   this->_sessionBeginMillis = 0;
   this->_socket = -1;
}

EthernetDHCPClass::~EthernetDHCPClass()
{
   if (NULL != this->_hostName) {
      free(this->_hostName);
      this->_hostName = NULL;
   }
}

// return values:
// 1 on success
// 0 on error
int EthernetDHCPClass::begin(uint8_t* macAddr)
{
   return this->begin(macAddr, 0);
}

// return values:
// 1 on success
// 0 on error
int EthernetDHCPClass::begin(uint8_t* macAddr, int pollingMode)
{
   int rv = 0;
 	 	   
   memset(&this->_dhcpData, 0, sizeof(DhcpDataInternal_t));
   this->_state = DhcpStateNone;
   this->_lastSendMillis = 0;
   
   memcpy(this->_dhcpData.macAddr, macAddr, 6);

   ethernet_compat_init(this->_dhcpData.macAddr,
                        this->_dhcpData.ipAddr,
                        RX_TX_BUF_SIZE);
   
   this->_socket = -1;
    	
 	if (0 < this->_requestDhcpLease()) {
      rv = 1;
 	   
      if (!pollingMode)
         while(DhcpStateLeased != this->poll())
            delay(10);
   }
 	   
   return rv;
}

// return values:
// 1 on success
// 0 on error
int EthernetDHCPClass::_startDHCPSession()
{
   (void)this->_closeDHCPSession();
   
   this->_dhcpData.xid++;
   
   int i;
   for (i = NUM_SOCKETS-1; i>=0; i--)
      if (ECSockClosed == ethernet_compat_read_SnSr(i)) {
         if (ethernet_compat_socket(i, ECSnMrUDP, DHCP_CLIENT_PORT, 0) > 0) {
            this->_socket = i;
            break;
         }
      }

   if (this->_socket < 0)
      return 0;
	
	uint16_t port = DHCP_SERVER_PORT;

   if (DhcpStateRenewing == this->_state || DhcpStateRequesting == this->_state) {
      ethernet_compat_write_SnDIPR(this->_socket, this->_dhcpData.serverIpAddr);
   } else {
      uint8_t broadcast[] = { 255, 255, 255, 255 };
      ethernet_compat_write_SnDIPR(this->_socket, broadcast);
   }

   ethernet_compat_write_SnDPORT(this->_socket, port);
   
   this->_sessionBeginMillis = millis();
   
   return 1;
}

// return values:
// 1 on success
// 0 on error
int EthernetDHCPClass::_closeDHCPSession()
{
   if (this->_socket > -1)
      ethernet_compat_close(this->_socket);
   
   this->_socket = -1;
}

void EthernetDHCPClass::_resetDHCP()
{
   memset(this->_dhcpData.ipAddr, 0, sizeof(this->_dhcpData.ipAddr));
   memset(this->_dhcpData.serverIpAddr, 0, sizeof(this->_dhcpData.serverIpAddr));
}

// return values:
// 1  ... success
// -1 ... out of memory
int EthernetDHCPClass::_sendDhcpMessage(DhcpMessageType_t type)
{
   uint8_t statusCode = 1;
   uint16_t ptr = 0;
   BootpPacket_t* bootpPacket = NULL;
   int sndBytes, cnt;
   uint8_t* buf;
   const uint8_t* hostName;
   int addedClientIdentifier = 0;
   unsigned long now = millis();
   
  	ptr = ethernet_compat_read_SnTX_WR(this->_socket);

   bootpPacket = (BootpPacket_t*)malloc(sizeof(BootpPacket_t));
   if (NULL == bootpPacket) {
      statusCode = -1;
      goto errorReturn;
   }
   
   memset(bootpPacket, 0, sizeof(BootpPacket_t));
   
   bootpPacket->opCode      = BootpRequest;
   bootpPacket->hwType      = DHCP_HTYPE10MB;
   bootpPacket->hwAddrLen   = DHCP_HWADDRLEN;
   bootpPacket->hops        = DHCP_HOPS;
   
   bootpPacket->xid = ethutil_htonl(this->_dhcpData.xid);
   
   bootpPacket->secs = ethutil_htons((now - this->_sessionBeginMillis) / 1000);
   bootpPacket->flags = ethutil_htons(DhcpFlagNone | DhcpFlagBroadcast);

   memcpy(bootpPacket->clientHwAddr, this->_dhcpData.macAddr, sizeof(this->_dhcpData.macAddr));
   
   if (this->_state == DhcpStateRenewing)
      memcpy(bootpPacket->clientIpAddr, this->_dhcpData.ipAddr, sizeof(bootpPacket->clientIpAddr));
   
   ethernet_compat_write_data(this->_socket, (uint8_t*)bootpPacket, (uint8_t*)ptr, sizeof(BootpPacket_t));
   ptr += sizeof(BootpPacket_t);
   
   // send server name (64 bytes) and boot file (128 bytes) as all zeroes
   cnt = 64 + 128;
   memset(bootpPacket, 0, sizeof(BootpPacket_t));
   while (cnt > 0) {
      sndBytes = (cnt > sizeof(BootpPacket_t)) ? sizeof(BootpPacket_t) : cnt;
      ethernet_compat_write_data(this->_socket, (uint8_t*)bootpPacket, (uint8_t*)ptr, sndBytes);
      ptr += sndBytes;
      cnt -= sndBytes;
   }
   
   buf = (uint8_t*)bootpPacket;
   
   // OPTION: magic cookie
   *((uint32_t*)buf) = ethutil_htonl(DHCP_OPTS_MAGIC_COOKIE);
   ethernet_compat_write_data(this->_socket, (uint8_t*)buf, (uint8_t*)ptr, 4);
   ptr += 4;
   
   // OPTION: message type
   buf[0] = DhcpOptionMessageType;
   buf[1] = 0x01;
   buf[2] = type;
   ethernet_compat_write_data(this->_socket, (uint8_t*)buf, (uint8_t*)ptr, 3);
   ptr += 3;
   
   // OPTION: host name
   hostName = (const uint8_t*)this->hostName();
   buf[0] = DhcpOptionHostName;
   buf[1] = strlen((const char*)hostName);
   ethernet_compat_write_data(this->_socket, (uint8_t*)buf, (uint8_t*)ptr, 2);
   ptr += 2;
   ethernet_compat_write_data(this->_socket, (uint8_t*)hostName, (uint8_t*)ptr, strlen((const char*)hostName));
   ptr += strlen((const char*)hostName);
   
   // OPTION: client identifier (we use the MAC address for this, since it's already unique)
   //         however, if specified, we use the host name as client identifier as well
   buf[0] = DhcpOptionClientIdentifier;
   if (this->_useHostNameAsClientIdentifier) {
      buf[1] = strlen((const char*)hostName) + 1;
      buf[2] = 0x00;
      ethernet_compat_write_data(this->_socket, (uint8_t*)buf, (uint8_t*)ptr, 3);
      ptr += 3;
      ethernet_compat_write_data(this->_socket, (uint8_t*)hostName, (uint8_t*)ptr, strlen((const char*)hostName));
      ptr += strlen((const char*)hostName);
   } else {
      buf[1] = 0x07;
      buf[2] = 0x01;
      memcpy(buf+3, this->_dhcpData.macAddr, sizeof(this->_dhcpData.macAddr));
      ethernet_compat_write_data(this->_socket, (uint8_t*)buf, (uint8_t*)ptr, 3 + sizeof(this->_dhcpData.macAddr));
      ptr += 3 + sizeof(this->_dhcpData.macAddr);
   }
   
   // OPTION: max dhcp message size
   buf[0] = DhcpOptionMaxMsgSize;
   buf[1] = 0x02;
   buf[2] = (MAX_DHCP_MSG_SIZE >> 8) & 0xff;
   buf[3] = MAX_DHCP_MSG_SIZE & 0xff;
   ethernet_compat_write_data(this->_socket, (uint8_t*)buf, (uint8_t*)ptr, 4);
   ptr += 4;
   
   if (DhcpMessageRequest == type && DhcpStateRenewing != this->_state) {
      // OPTION: requested IP address
      buf[0] = DhcpOptionRequestedIPaddr;
      buf[1] = 0x04;
      memcpy(buf+2, this->_dhcpData.ipAddr, 4);
      
      // OPTION: server identifier
      buf[6] = DhcpOptionServerIdentifier;
      buf[7] = 0x04;
      memcpy(buf+8, this->_dhcpData.serverIpAddr, 4);
      
      ethernet_compat_write_data(this->_socket, (uint8_t*)buf, (uint8_t*)ptr, 12);
      ptr += 12;
   }
   
   // OPTION: DHCP Parameters
   buf[0] = DhcpOptionParamRequest;
   buf[1] = 0x07;
   buf[2] = DhcpOptionSubnetMask;
   buf[3] = DhcpOptionRoutersOnSubnet;
   buf[4] = DhcpOptionDNS;
   buf[5] = DhcpOptionDomainName;
   buf[6] = DhcpOptionHostName;
   buf[7] = DhcpOptionT1value;
   buf[8] = DhcpOptionT2value;
   
   buf[9] = DhcpOptionEnd;
   
   ethernet_compat_write_data(this->_socket, (uint8_t*)buf, (uint8_t*)ptr, 10);
   ptr += 10;
   
   ethernet_compat_write_SnTX_WR(this->_socket, ptr);
   ethernet_compat_write_SnCR(this->_socket, ECSnCrSockSend);

   while(ethernet_compat_read_SnCR(this->_socket));
        
errorReturn:
   this->_lastSendMillis = now;

   if (NULL != bootpPacket)
      free(bootpPacket);

   return statusCode;
}

// return value:
// a DhcpMessageType_t indicating either a DHCP response or an internal error
DhcpMessageType_t EthernetDHCPClass::_processDhcpReply()
{
   DhcpMessageType_t rv = DhcpMessageIgnore;
   uint16_t ptr = 0;
   BootpPacket_t* bootpPacket = NULL;
   uint8_t* buf, *p, *e;
   uint8_t svr_addr[4], l;
   uint16_t svr_port;
   uint16_t udp_len, opt_len;
   uint32_t magicCookie = ethutil_htonl(DHCP_OPTS_MAGIC_COOKIE);
    
   if (0 == ethernet_compat_read_SnRX_RSR(this->_socket)) {
      rv = DhcpMessageTryLater;
      goto errorReturn;
   }
      
   bootpPacket = (BootpPacket_t*)malloc(sizeof(BootpPacket_t));
   if (NULL == bootpPacket) {
      rv = DhcpMessageError;
      goto errorReturn;
   }
   
   ptr = ethernet_compat_read_SnRX_RD(this->_socket);

   // read UDP header
   buf = (uint8_t*)bootpPacket;
   ethernet_compat_read_data(this->_socket, (uint8_t*)ptr, (uint8_t*)buf, 8);
   ptr += 8;

   memcpy(&svr_addr, buf, sizeof(uint32_t));
   *((uint16_t*)&svr_port) = ethutil_ntohs(*((uint32_t*)(buf+4)));
   *((uint16_t*)&udp_len) = ethutil_ntohs(*((uint32_t*)(buf+6)));
   
   ethernet_compat_read_data(this->_socket, (uint8_t*)ptr, (uint8_t*)bootpPacket, sizeof(BootpPacket_t));
   
   if (BootpReply == bootpPacket->opCode && DHCP_SERVER_PORT == svr_port &&
       0 == memcmp(bootpPacket->clientHwAddr,
                   this->_dhcpData.macAddr,
                   sizeof(this->_dhcpData.macAddr)) &&
       ethutil_ntohl(bootpPacket->xid) == this->_dhcpData.xid) {
         
         // check for magic cookie
         ethernet_compat_read_data(this->_socket, (uint8_t*)ptr + 236, (uint8_t*)buf, sizeof(uint32_t));
         if (0 == memcmp(buf, &magicCookie, sizeof(uint32_t))) {
            memcpy(this->_dhcpData.ipAddr, bootpPacket->yourIpAddr, sizeof(this->_dhcpData.ipAddr));

            uint16_t opt_len = udp_len - 240;
            if (sizeof(BootpPacket_t) < opt_len) {
               free(bootpPacket);
               bootpPacket = (BootpPacket_t*)malloc(opt_len);
            }
            buf = (uint8_t*)bootpPacket;

            ethernet_compat_read_data(this->_socket, (uint8_t*)ptr + 240, (uint8_t*)buf, opt_len);

            p = buf;
            e = p + opt_len;
            
            while (p < e) {
               l = *(p+1);
               DhcpOption_t o = (DhcpOption_t)*p;      
               switch (*p++) {
                  case DhcpOptionPadding:
                     l = 0;
                     break;
                  case DhcpOptionEnd:
                     l = 0;
                     p = e;
                     break;
                  case DhcpOptionMessageType:
                     rv = (DhcpMessageType_t)*(p+1);
                     break;
                  case DhcpOptionSubnetMask:
                     memcpy(this->_dhcpData.subnetMask, p+1, sizeof(this->_dhcpData.subnetMask));
                     break;
                  case DhcpOptionRoutersOnSubnet:
                     memcpy(this->_dhcpData.gatewayIpAddr, p+1,
                            sizeof(this->_dhcpData.gatewayIpAddr));
                     break;
                  case DhcpOptionDNS:
                     memcpy(this->_dhcpData.dnsIpAddr, p+1, sizeof(this->_dhcpData.dnsIpAddr));
                     break;
                  case DhcpOptionIPAddrLeaseTime:
                     this->_dhcpData.leaseTime = ethutil_ntohl(*((uint32_t*)(p+1)));
                     break;
                  case DhcpOptionT1value:
                     this->_dhcpData.t1 = ethutil_ntohl(*((uint32_t*)(p+1)));
                     break;
                  case DhcpOptionT2value:
                     this->_dhcpData.t2 = ethutil_ntohl(*((uint32_t*)(p+1)));
                     break;
                  case DhcpOptionHostName:
                     this->_setHostNameWithLength((char*)&p[1], l);
                     break;
                  case DhcpOptionServerIdentifier:
                     if ((uint32_t)0 == *(uint32_t*)this->_dhcpData.serverIpAddr ||
                         0 == memcmp(this->_dhcpData.serverIpAddr, svr_addr, sizeof(svr_addr)))
                         memcpy(this->_dhcpData.serverIpAddr, p+1, 4);
                     break;
                  default:
                     break;
               }
            
               p += l + (l ? 1 : 0);
            }
         }
   }
   
   ptr += udp_len;
   
   ethernet_compat_write_SnRX_RD(this->_socket, ptr);
   ethernet_compat_write_SnCR(this->_socket, ECSnCrSockRecv);

   while(ethernet_compat_read_SnCR(this->_socket));
   
errorReturn:
   if (NULL != bootpPacket)
      free(bootpPacket);

   return rv;
}

// return values:
// 1 on success
// 0 otherwise
int EthernetDHCPClass::_requestDhcpLease()
{
   int rv = 0;
   
   if (this->_startDHCPSession()) {      
      DhcpMessageType_t t;
      switch (this->_state) {
         case DhcpStateRenewing:
            t = DhcpMessageRequest;
            break;
         default:
            t = DhcpMessageDiscover;
            break;
      }
      
      if (this->_state != DhcpStateRenewing)
         this->_state = DhcpStateDiscovering;
      
      if (this->_sendDhcpMessage(t) > 0)
         rv = 1;
   }
   
   return rv;
}

void EthernetDHCPClass::maintain()
{
   (void)this->poll();
}

// return value:
// the current DHCP state
DhcpState_t EthernetDHCPClass::poll()
{
   unsigned long now = millis();
   
   if (0 != this->_lastPollMillis) {
      this->_millisecondsAccum += now - this->_lastPollMillis;
      while(this->_millisecondsAccum > 1000) {
         this->_secsUntilRenew -= 1;
         this->_secsUntilRebind -= 1;
         this->_millisecondsAccum -= 1000;
      }
   }
   
   this->_lastPollMillis = now;
   
   if (this->_secsUntilRenew <= 0 && DhcpStateLeased == this->_state) {
      this->_state = DhcpStateRenewing;
      this->_requestDhcpLease();
   } 
   
   if (this->_secsUntilRebind <= 0 &&
              (DhcpStateLeased == this->_state || DhcpStateRenewing == this->_state)) {
      this->_resetDHCP();
      this->_state = DhcpStateDiscovering;
      this->_requestDhcpLease();
   }
   
   if (DhcpStateLeased != this->_state) {
      DhcpMessageType_t type = this->_processDhcpReply();
      
      if (DhcpMessageTryLater != type && DhcpMessageIgnore != type && DhcpMessageError != type) {
         switch (this->_state) {
            case DhcpStateDiscovering:
               if (type = DhcpMessageOffer) {
                  this->_state = DhcpStateRequesting;
                  this->_sendDhcpMessage(DhcpMessageRequest);
               }
               break;
            case DhcpStateRequesting:
            case DhcpStateRenewing:
               if (type == DhcpMessageAck) {
                  this->_state = DhcpStateLeased;
                  
                  // set t1 and t2, even if they were not provided by the DHCP server
                  if (0 == this->_dhcpData.t1) {
                     if (0 != this->_dhcpData.t2)
                        this->_dhcpData.t1 = this->_dhcpData.t2 >> 1;
                     else if (0 != this->_dhcpData.leaseTime)
                        this->_dhcpData.t1 = this->_dhcpData.leaseTime >> 1;
                     else
                        this->_dhcpData.t1 = DEFAULT_T1;
                  }
                  if (0 == this->_dhcpData.t2)
                     this->_dhcpData.t2 = this->_dhcpData.t1 << 1;
                  
                  this->_secsUntilRenew = this->_dhcpData.t1;
                  this->_secsUntilRebind = this->_dhcpData.t2;
                  
                  ethernet_compat_write_SIPR(this->_dhcpData.ipAddr);
                 	ethernet_compat_write_GAR(this->_dhcpData.gatewayIpAddr);
                  ethernet_compat_write_SUBR(this->_dhcpData.subnetMask);
                  
                  (void)this->_closeDHCPSession();
               } else {
                  // clear DHCP data and retry from the beginning
                  this->_resetDHCP();
                  this->_state = DhcpStateDiscovering;
                  this->_requestDhcpLease();
               }
               break;
            default:
               break;
         }
      } else {
         if (now - this->_lastSendMillis > RETRY_TIMEOUT) {
            DhcpMessageType_t t = DhcpMessageDiscover;
            
            switch (this->_state) {
               case DhcpStateRequesting:
               case DhcpStateRenewing:
                  t = DhcpMessageRequest;
                  break;
            }
            
            this->_sendDhcpMessage(t);
         }
      }
   }
   
   return this->_state;
}

// return value:
// current local IP address as uint8_t[4]
const byte* EthernetDHCPClass::ipAddress()
{
   return this->_dhcpData.ipAddr;
}

// return value:
// current gateway IP address as uint8_t[4]
const byte* EthernetDHCPClass::gatewayIpAddress()
{
   return this->_dhcpData.gatewayIpAddr;
}

// return value:
// current DNS server IP address as uint8_t[4]
const byte* EthernetDHCPClass::dnsIpAddress()
{
   return this->_dhcpData.dnsIpAddr;
}

// return value:
// current host name as string
const char* EthernetDHCPClass::hostName()
{
   const char* rv = (char*)this->_hostName;
   if (NULL == rv)
      rv = (const char*)DEFAULT_HOST_NAME;
   
   return rv;
}

// return values:
// 1 on success
// 0 on error
int EthernetDHCPClass::setHostName(const char* newHostName)
{
   return this->_setHostNameWithLength(newHostName, 0);
}

// return values:
// 1 on success
// 0 on error
int EthernetDHCPClass::_setHostNameWithLength(const char* newHostName, int length)
{  
   if (NULL != this->_hostName) {
      free(this->_hostName);
      this->_hostName = NULL;
   }
   
   if (NULL == newHostName)
      return 1;
   
   int l = (length > 0) ? length+1 : strlen(newHostName)+1;
   l = (l > MAX_HOST_NAME_LEN) ? MAX_HOST_NAME_LEN : l;
   
   this->_hostName = (uint8_t*)malloc(l);
   if (NULL == this->_hostName)
      return 0;
   
   memcpy(this->_hostName, newHostName, l-1);
   this->_hostName[l-1] = '\0';
      
   return 1;
}

void EthernetDHCPClass::useHostNameAsClientIdentifier(int yesOrNo)
{
   this->_useHostNameAsClientIdentifier = yesOrNo;
}

EthernetDHCPClass EthernetDHCP;
