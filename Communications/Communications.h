#ifndef COMMUNICATIONS_H
#define COMMUNICATIONS_H
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <HeateR.h>

#define SIZE_LOOP_BUF 30
#define USE_NTP 0
#if USE_NTP
extern char timeServer[];
#endif

extern byte m_mac[];
extern byte m_ip[];
extern byte m_mask[];
extern byte m_gateway[];
extern byte m_dns[];
extern unsigned int portAPI;
extern unsigned int portCLI;

extern EthernetServer serverCLIoverTCP;
extern EthernetServer serverAPI;
extern EthernetClient clientAPI, clientCLI;

void InitEthernet();
void CommAPI(EthernetClient client);
void UpdateSocketTimer(){
	
}

class ListEthernetClient_c {
public:
	ListEthernetClient_c* next_p;
	unsigned long TimeToClose;
};

#if USE_NTP
#define NTP_PACKET_SIZE 48 // NTP time stamp is in the first 48 bytes of the message
class ntp_c {
	byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
	EthernetUDP Udp;
	void sendNTPpacket(char* address);
		public: 
	unsigned int localPort;       // local port to listen for UDP packets
	ntp_c();	
	void getTime();
	unsigned long timeOut;
};
extern ntp_c *ntp;
#endif
#endif