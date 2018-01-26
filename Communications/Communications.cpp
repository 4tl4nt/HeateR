#include "Communications.h"

#define DUBUGING_MODE
#ifdef DUBUGING_MODE
#define DEBUG(str, a...){ Serial.print(str, ##a);}
#define DEBUG_TIME(){\
	Serial.print((CurrentTimeRTC  % 86400L) / 3600); \
    Serial.print(':');\
    if (((CurrentTimeRTC % 3600) / 60) < 10) {\
      Serial.print('0');\
    }\
    Serial.print((CurrentTimeRTC  % 3600) / 60);\
    Serial.print(':');\
    if ((CurrentTimeRTC % 60) < 10) {\
      Serial.print('0');\
    }\
	Serial.print(CurrentTimeRTC % 60); \
	Serial.print("--"); \
}
#else
#define DEBUG(str, a...)
#define DEBUG_TIME()
#endif
//char timeServer[] = "ntp.time.in.ua";
//byte timeServerIP[] = {62,149,0,30};
unsigned int portAPI = 12345;
unsigned int portCLI = 12346;

EthernetServer serverCLIoverTCP(portCLI);
EthernetServer serverAPI(portAPI);
EthernetClient clientAPI, clientCLI;

char buff[SIZE_LOOP_BUF];
Room_c* room_p;

void InitEthernet(){
	NetworkSettings *p = new NetworkSettings;
	ReadNetworkSettingsEEPROM(p);
    DEBUG_TIME();DEBUG("IP:");
    DEBUG((int)p->ip[0]);
    DEBUG(".");
    DEBUG((int)p->ip[1]);
    DEBUG(".");
    DEBUG((int)p->ip[2]);
    DEBUG(".");
    DEBUG(p->ip[3]);
	DEBUG("\n");
	Ethernet.begin(p->mac, p->ip, p->dns, p->gateway, p->mask);
#if USE_WDT
	wdt_enable(WDTO_4S);
#endif
#if USE_NTP
	Udp.begin(localPortNTP);
	GetTime();
#endif
	serverCLIoverTCP.begin();
	serverAPI.begin();
	delete p;
}

void CommAPI(EthernetClient client){
  if (client) 
  {
	NewClientEthernet(client.getSocketNumber());
	int num_room=0, state=0;
    DEBUG_TIME();DEBUG("connectAPI");DEBUG("\n");
	int i=0;
    DEBUG_TIME();DEBUG("read: ");
    for(;i<SIZE_LOOP_BUF;i++)
    {
      buff[i]=client.read();
      if (buff[i]==';'||buff[i]==-1) break;
      DEBUG(buff[i]);
    }
    DEBUG("\n");
    buff[6]=buff[8]='\0';
    num_room = atoi(&buff[4]);
    state = atoi(&buff[7]);
    if (strncmp(buff,"SET,",4)==0)
    {
      room_p = getRoom(num_room);
      if (room_p != NULL) {
        client.print("OK;");
        if (state==0) room_p->SetControlTemp(false);
        else if (state==1) {
          buff[14]='\0';
          state = atoi(&buff[9]);
          room_p->SetTimeOutCT(state);
        }
		else client.print("ERROR;");
      }
      else client.print("ERROR;");
    }
    else if (strncmp(buff,"GET,",4)==0)
    {
      room_p = getRoom(num_room);
      if (room_p != NULL) {
        if (state==1){
        client.print("OK,");
        client.print(room_p->GetTemperature());
        client.print(";");
        }
        else if (state==2){
        client.print("OK,");
        client.print(room_p->GetControlTemp());
        client.print(";");
        }
        else client.print("ERROR;");
      }
      else client.print("ERROR;");
    }
	else client.print("ERROR;");
    while(client.read()!=(-1));
  DEBUG_TIME();DEBUG("listen...");DEBUG("\n");
  }
}

EthernetClient_list FirstEthernetClient;
EthernetClient_list::EthernetClient_list(){
	next_p = NULL;
	TimeToClose = 99;
	socketNUM = MAX_SOCK_NUM;
}
void UpdateClientEthernet(){
	static unsigned long Timer = (millis()/1000+1)*1000;
	if (millis()>Timer){
		EthernetClient_list *tmp_p, *p;
		tmp_p = p = &FirstEthernetClient;
		EthernetClient *Client;
		CurrentTimeRTC++;
#if USE_WDT
UpdateWDT (FLAG_1);
#endif
		while (p->next_p != NULL) {
			if (p->TimeToClose>0) p->TimeToClose--;
			else {
				Client = new EthernetClient(p->socketNUM);
				if (Client->connected()) Client->stop();
				if (p->next_p->next_p!=NULL){
					p->socketNUM = p->next_p->socketNUM;
					p->TimeToClose = p->next_p->TimeToClose;
					tmp_p = p->next_p;
					p->next_p = p->next_p->next_p;
					delete tmp_p;
				}
				else {
					delete p->next_p;
					p->next_p = NULL;
					p->TimeToClose = 99;
					p->socketNUM = MAX_SOCK_NUM;
				}
				delete Client;
				break;
			}
			tmp_p = p;
			p = p->next_p;
		}
		Timer = (millis()/1000+1)*1000;
	}
}

void NewClientEthernet(uint8_t Socket){
	EthernetClient_list* p = &FirstEthernetClient;
	while (p->next_p != NULL) {
		if (p->socketNUM == Socket) {
			p->TimeToClose = TIME_OUT_SOCKET;
			return;
		}
		p = p->next_p;
	}
	if (p->next_p==NULL) {
		p->next_p = new EthernetClient_list;
	}
	p->socketNUM = Socket;
	p->TimeToClose = TIME_OUT_SOCKET;
}

#if USE_WDT
void UpdateWDT (uint8_t i){
	static uint8_t flags = 0;
	flags |= i;
	if (flags & (FLAG_1|FLAG_2|FLAG_3|FLAG_4)){
		wdt_reset();
		flags = 0;
	}
	if (CurrentTimeRTC>=RebootTime) while(1);
}
#endif
#if USE_NTP

unsigned int localPortNTP = 8888;       // local port to listen for UDP packets
char timeServer[] = "time.nist.gov"; // time.nist.gov NTP server
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
EthernetUDP Udp;// A UDP instance to let us send and receive packets over UDP
void GetTime() {
	DEBUG_TIME();DEBUG("GetTime.\n");
	sendNTPpacket(timeServer); // send an NTP packet to a time server
	DEBUG_TIME();DEBUG("SendNTPpacket\n");
	delay(1000);// wait to see if a reply is available
	while (!(Udp.parsePacket()));// We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
    // the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, extract the two words:
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
	DEBUG_TIME();DEBUG("Seconds since Jan 1 1900 = ");
	DEBUG(secsSince1900);
	DEBUG("\n");
    // now convert NTP time into everyday time:
    DEBUG_TIME();DEBUG("Unix time = ");// Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL; // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears + 7200;
	CurrentTimeRTC = epoch;
	RebootTime = ((epoch / 86400L)+1)*86400L;
	DEBUG(epoch);
	DEBUG("\n");
	//prin the time
	DEBUG_TIME();DEBUG("The UTC time is ");
	DEBUG((epoch  % 86400L) / 3600);
	DEBUG(':');
    if (((epoch % 3600) / 60) < 10) {
		DEBUG('0');
    }
	DEBUG((epoch  % 3600) / 60); 
	DEBUG(':');
    if ((epoch % 60) < 10) {
		DEBUG('0');
    }
	DEBUG(epoch % 60);
	DEBUG("\n");
	
	//prin the reboot time
	DEBUG_TIME();DEBUG("Reboot time is ");       // UTC is the time at Greenwich Meridian (GMT)
    DEBUG((RebootTime  % 86400L) / 3600); // print the hour (86400 equals secs per day)
    DEBUG(':');
    if (((RebootTime % 3600) / 60) < 10) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      DEBUG('0');
    }
    DEBUG((RebootTime  % 3600) / 60); // print the minute (3600 equals secs per minute)
    DEBUG(':');
    if ((RebootTime % 60) < 10) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      DEBUG('0');
    }
    DEBUG(RebootTime % 60); // print the second
	DEBUG("\n");
}

// send an NTP request to the time server at the given address
void sendNTPpacket(char* address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

#endif
