#include "Communications.h"

char timeServer[] = "ntp.time.in.ua";
byte timeServerIP[] = {62,149,0,30};
unsigned int portAPI = 12345;
unsigned int portCLI = 12346;

EthernetServer serverCLIoverTCP(portCLI);
EthernetServer serverAPI(portAPI);
EthernetClient clientAPI, clientCLI;

char buff[SIZE_LOOP_BUF];
Room_c* room_p;
EthernetUDP Udp;
#if USE_NTP
ntp_c *ntp;
#endif
void InitEthernet(){
	NetworkSetings *p = new NetworkSetings;
	ReadNetworkSetingsEEPROM(p);
    Serial.print("IP:");
    Serial.print((int)p->ip[0]);
    Serial.print(".");
    Serial.print((int)p->ip[1]);
    Serial.print(".");
    Serial.print((int)p->ip[2]);
    Serial.print(".");
    Serial.println(p->ip[3]);
	Ethernet.begin(p->mac, p->ip, p->dns, p->gateway, p->mask);
	serverCLIoverTCP.begin();
	serverAPI.begin();
	delete p;
#if USE_NTP
	ntp = new ntp_c;
	Udp.begin(ntp->localPort);
#endif
}

void CommAPI(EthernetClient client){
  if (client) 
  {
	NewClientEthernet(client.getSocketNumber());
	int num_room=0, state=0;
    Serial.println("connectAPI");
	int i=0;
    Serial.print("read: ");
    for(;i<SIZE_LOOP_BUF;i++)
    {
      buff[i]=client.read();
      if (buff[i]==';'||buff[i]==-1) break;
      Serial.print(buff[i]);
    }
    Serial.println();
    buff[6]=buff[8]='\0';
    num_room = atoi(&buff[4]);
    state = atoi(&buff[7]);
    if (strncmp(buff,"SET,",4)==0)
    {
      Serial.println("recive SET");
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
      Serial.println("recive GET");
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
  Serial.println("listen...");
  //client.stop();
  }
}

EthernetClient_list FirstEthernetClient;
EthernetClient_list::EthernetClient_list(){
	next_p = NULL;
	TimeToClose = 99;
	socketNUM = MAX_SOCK_NUM;
}
void UpdateClientEthernet(){
	static unsigned long Timer = 1000 + millis();
	if (millis()>Timer){
		EthernetClient_list *tmp_p, *p;
		tmp_p = p = &FirstEthernetClient;
		EthernetClient *Client;
		while (p->next_p != NULL) {
			if (p->TimeToClose>0) p->TimeToClose--;
			else {
				Client = new EthernetClient(p->socketNUM);
				Client->stop();
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
				break;
			}
			tmp_p = p;
			p = p->next_p;
		}
		Timer = 1000 + millis();
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

#if USE_NTP
ntp_c::ntp_c(){
	timeOut = 0;
	localPort = 8888;       // local port to listen for UDP packets
}
void ntp_c::getTime() {
	
  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);
  // while (!Udp.available());
  if (Udp.parsePacket()) {
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    // the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, extract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = ");
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);


    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
    Serial.print(':');
    if (((epoch % 3600) / 60) < 10) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ((epoch % 60) < 10) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch % 60); // print the second
  }
  // wait ten seconds before asking for the time again
  // delay(10000);
  // Ethernet.maintain();
}
void ntp_c::sendNTPpacket(char* address) {
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
  Serial.println("...");
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Serial.println("begin");
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Serial.println("write");
  Udp.endPacket();
  Serial.println("end");
}
#endif
