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

#if !defined(__ETHERNET_DHCP_H__)
#define __ETHERNET_DHCP_H__ 1

extern "C" {
   #include <inttypes.h>
}

typedef uint8_t byte;

typedef enum _BootpOpcode_t {
   BootpRequest      = 1,
   BootpReply        = 2
} BootpOpcode_t;

typedef enum _DhcpMessageType_t {
   DhcpMessageTryLater     = -2,
   DhcpMessageError        = -1,
   DhcpMessageIgnore       = 0,
   DhcpMessageDiscover     = 1,
   DhcpMessageOffer        = 2,
   DhcpMessageRequest      = 3,
   DhcpMessageDecline      = 4,
   DhcpMessageAck          = 5,
   DhcpMessageNak          = 6,
   DhcpMessageRelease      = 7,
   DhcpMessageInform       = 8
} DhcpMessageType_t;

typedef enum _DhcpFlags_t {
   DhcpFlagNone         = 0,
   DhcpFlagBroadcast    = 0x8000
} DhcpFlags_t;

// more info on options in rfc 2132
typedef enum _DhcpOption_t {
   DhcpOptionPadding          = 0,
   DhcpOptionSubnetMask       = 1,
   DhcpOptionRoutersOnSubnet  = 3,
   DhcpOptionDNS              = 6,
   DhcpOptionHostName         = 12,
   DhcpOptionDomainName       = 15,
	DhcpOptionRequestedIPaddr	= 50,
	DhcpOptionIPAddrLeaseTime	= 51,
	DhcpOptionOptionOverload	= 52,
	DhcpOptionMessageType		= 53,
	DhcpOptionServerIdentifier	= 54,
	DhcpOptionParamRequest	   = 55,
	DhcpOptionMsg		      	= 56,
	DhcpOptionMaxMsgSize 		= 57,
	DhcpOptionT1value	      	= 58,
	DhcpOptionT2value		      = 59,
	DhcpOptionClassIdentifier	= 60,
	DhcpOptionClientIdentifier	= 61,
	DhcpOptionEnd     		   = 255
} DhcpOption_t;

typedef enum _DhcpState_t {
   DhcpStateNone,
   DhcpStateDiscovering,
   DhcpStateRequesting,
   DhcpStateRenewing,
   DhcpStateLeased
} DhcpState_t;

typedef DhcpState_t DhcpState;

typedef struct _DhcpDataInternal_t {
   uint32_t    xid;
   uint8_t     macAddr[6];
   uint8_t     ipAddr[4]; 
   uint8_t     subnetMask[4];
   uint8_t     gatewayIpAddr[4];
   uint8_t     serverIpAddr[4];
   uint8_t     dnsIpAddr[4];
   uint32_t    leaseTime;
   uint32_t    t1, t2;
} DhcpDataInternal_t;

class EthernetDHCPClass
{
private:
   DhcpDataInternal_t   _dhcpData;
   int                  _socket;
   uint8_t*             _hostName;
   int                  _useHostNameAsClientIdentifier;
   DhcpState_t          _state;
   uint32_t             _secsUntilRenewal;
   unsigned long        _lastSendMillis;
   unsigned long        _lastPollMillis;
   unsigned long        _sessionBeginMillis;
   unsigned long        _millisecondsAccum;
   signed long          _secsUntilRenew, _secsUntilRebind;
   
   int _sendDhcpMessage(DhcpMessageType_t type);
   DhcpMessageType_t _processDhcpReply();
   int _requestDhcpLease();
   
   int _startDHCPSession();
   int _closeDHCPSession();
   void _resetDHCP();
   
   int _setHostNameWithLength(const char* newHostName, int length);
public:
   EthernetDHCPClass();
   ~EthernetDHCPClass();
   
   int begin(uint8_t* macAddr);
   int begin(uint8_t* macAddr, int pollingMode);
   
   DhcpState_t poll();
   void maintain();
   
   const byte* ipAddress();
   const byte* gatewayIpAddress();
   const byte* dnsIpAddress();
   
   const char* hostName();
   int setHostName(const char* newHostName);
   
   void useHostNameAsClientIdentifier(int yesOrNo);
};

extern EthernetDHCPClass EthernetDHCP;

#endif // __ETHERNET_DHCP_H__
