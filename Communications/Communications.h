#define SIZE_LOOP_BUF 30
#define TIME_OUT_SOCKET 10

#ifndef COMMUNICATIONS_H
#define COMMUNICATIONS_H
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <HeateR.h>
#if USE_WDT
#include <avr/wdt.h>
#endif

#if USE_NTP
extern char timeServer[];
#endif

extern unsigned int portAPI;
extern unsigned int portCLI;

extern EthernetServer serverCLIoverTCP;
extern EthernetServer serverAPI;
extern EthernetClient clientAPI, clientCLI;

void InitEthernet();
void CommAPI(EthernetClient client);

class EthernetClient_list {
public:
    EthernetClient_list();
	EthernetClient_list* next_p;
	int TimeToClose;
	uint8_t socketNUM;
};
void UpdateClientEthernet();
void NewClientEthernet(uint8_t Socket);

#if USE_NTP
/*

 Udp NTP Client

 Get the time from a Network Time Protocol (NTP) time server
 Demonstrates use of UDP sendPacket and ReceivePacket
 For more on NTP time servers and the messages needed to communicate with them,
 see http://en.wikipedia.org/wiki/Network_Time_Protocol

 created 4 Sep 2010
 by Michael Margolis
 modified 9 Apr 2012
 by Tom Igoe
 modified 02 Sept 2015
 by Arturo Guadalupi

 This code is in the public domain.

 */

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
extern unsigned int localPortNTP;       // local port to listen for UDP packets

extern char timeServer[]; // time.nist.gov NTP server

//extern const int NTP_PACKET_SIZE; // NTP time stamp is in the first 48 bytes of the message

extern byte packetBuffer[]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
extern EthernetUDP Udp;

extern unsigned long CurrentTimeRTC, RebootTime;

void GetTime();
void sendNTPpacket(char* address);

#endif
#endif