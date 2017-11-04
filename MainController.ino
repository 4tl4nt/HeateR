#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <HeateR.h>;

byte m_mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF };
byte m_ip[] = {192,168,1,250};
byte m_mask[] = {255,255,255,0};
byte m_gateway[] = {192,168,1,1};
byte m_dns[] = {8,8,8,8};
unsigned int port = 12345;

EthernetServer serverCLIoverTCP(port);
EthernetClient client;
void InitEthernet(){
  Ethernet.begin(m_mac, m_ip, m_dns, m_gateway, m_mask);
  serverCLIoverTCP.begin();
}
ObjCLI *SerialCLI = NULL;;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial);             // Leonardo: wait for serial monitor
  Serial.println("Inited serial.");
  InitRelayModule();
  InitHeateR();
  InitEthernet();
  Serial.println("MainController has started.");
  while(client.read()!=(-1));
  for(int i=0;i<1000;i++){
    if (Serial.available()){
      SerialCLI= new ObjCLI(&Serial);
      SerialCLI->InitMenu();
      SerialCLI->MainMenu();
    }
    delay(1);
  }
}
char buff[30];
void loop() {
  if (Serial.available()){
    if (SerialCLI==NULL) {
      SerialCLI= new ObjCLI(&Serial);
      SerialCLI->InitMenu();
    }
    SerialCLI->MainMenu();
  }
  EthernetClient client = serverCLIoverTCP.available();
  //SerialCLI= new ObjCLI(&client);
  Serial.println("listen");
  int num_room, state;
  Room_c* room_p;
  if (client) 
  {
    Serial.println("connect");
    for(int i=0;i<9;i++)
    {
      Serial.print("read ");
      buff[i]=client.read();
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
        client.print("OK;");
        if (state==1) room_p->SetRele();
        else if (state==0) room_p->ResetRele();
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
        client.print(room_p->CurrentState);
        client.print(";");
        }
        else client.print("ERROR;");
      }
      else client.print("ERROR;");
    }
    while(client.read()!=(-1));
    client.print("\n");
  }
}


