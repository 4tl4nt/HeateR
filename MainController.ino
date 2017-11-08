#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <HeateR.h>

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
#define SIZE_LOOP_BUF 30
char buff[SIZE_LOOP_BUF];

void InitEthernet(){
  Ethernet.begin(m_mac, m_ip, m_dns, m_gateway, m_mask);
  serverCLIoverTCP.begin();
  serverAPI.begin();
}

void UpDate (){
  
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
  delay(10);
  UpdataNextOne();
  clientAPI = serverAPI.available();
  clientCLI = serverCLIoverTCP.available();
  unsigned int num_room, state;
  Room_c* room_p;
  
  if (clientAPI) 
  {
    Serial.println("connectAPI");
    for(int i=0;i<SIZE_LOOP_BUF;i++)
    {
      Serial.print("read ");
      buff[i]=clientAPI.read();
      Serial.println(buff[i]);
      if (buff[i]==';') break;
    }
    buff[6]=buff[8]='\0';
    num_room = atoi(&buff[4]);
    state = atoi(&buff[7]);
    if (strncmp(buff,"SET,",4)==0)
    {
      Serial.println("recive SET");
      room_p = getRoom(num_room);
      if (room_p != NULL) {
        clientAPI.print("OK;");
        if (state==0) room_p->SetControlTemp(false);
        else if (state==1) {
          buff[14]='\0';
          state = atoi(&buff[9]);
          room_p->SetTimeOutCT(state);
        }
      }
      else clientAPI.print("ERROR;");
    }
    else if (strncmp(buff,"GET,",4)==0)
    {
      Serial.println("recive GET");
      room_p = getRoom(num_room);
      if (room_p != NULL) {
        if (state==1){
        clientAPI.print("OK,");
        clientAPI.print(room_p->GetTemperature());
        clientAPI.print(";");
        }
        else if (state==2){
        clientAPI.print("OK,");
        clientAPI.print(room_p->GetControlTemp());
        clientAPI.print(";");
        }
        else clientAPI.print("ERROR;");
      }
      else clientAPI.print("ERROR;");
    }
    while(clientAPI.read()!=(-1));
  Serial.println("listen...");
  }
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

