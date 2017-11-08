#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <HeateR.h>
#include <Communications.h>

byte m_mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF };
byte m_ip[] = {10,4,11,250};
byte m_mask[] = {255,255,255,0};
byte m_gateway[] = {10,4,11,254};
byte m_dns[] = {8,8,8,8};
unsigned int portAPI = 12345;
unsigned int portCLI = 12346;

EthernetServer serverCLIoverTCP(portCLI);
EthernetServer serverAPI(portAPI);
EthernetClient clientAPI, clientCLI;
ObjCLI *SerialCLI = NULL;

void InitEthernet(){
  Ethernet.begin(m_mac, m_ip, m_dns, m_gateway, m_mask);
  serverCLIoverTCP.begin();
  serverAPI.begin();
}

void UpDate (){
  UpdataNextOne();
  clientAPI = serverAPI.available();
  CommAPI(clientAPI);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Inited serial.");
  InitRelayModule();
  InitHeateR();
  InitEthernet();
  Serial.println("MainController has started.");
  Serial.println("listen...");
  
}

void loop() {
  UpDate();
  clientCLI = serverCLIoverTCP.available();
  unsigned int num_room, state;
  if (clientCLI)
  {
    Serial.println("connectCLI");
    if (SerialCLI==NULL) {
      SerialCLI= new ObjCLI(&clientCLI);
      SerialCLI->InitMenu();
    }
    SerialCLI->MainMenu();
    Serial.println("listen...");
  }
}

