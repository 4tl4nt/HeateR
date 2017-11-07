#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <HeateR.h>;
#include <TimerOne.h>

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
void InitEthernet(){
  Ethernet.begin(m_mac, m_ip, m_dns, m_gateway, m_mask);
  serverCLIoverTCP.begin();
  serverAPI.begin();
  
  //Timer1.initialize(1500000);
  //Timer1.attachInterrupt(TimerFunc); 
}
ObjCLI *SerialCLI = NULL;;

void TimerFunc(void){
  UpdataNextOne();
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial);             // Leonardo: wait for serial monitor
  Serial.println("Inited serial.");
  InitRelayModule();
  InitHeateR();
  InitEthernet();
  Serial.println("MainController has started.");
  /*while(client.read()!=(-1));
  for(int i=0;i<1000;i++){
    if (Serial.available()){
      SerialCLI= new ObjCLI(&Serial);
      SerialCLI->InitMenu();
      SerialCLI->MainMenu();
    }
    delay(1);
  }*/
  
  Serial.println("listen...");
}
char buff[30];
void loop() {
  delay(10);
  
  UpdataNextOne();
  clientAPI = serverAPI.available();
  clientCLI = serverCLIoverTCP.available();
  int num_room, state;
  Room_c* room_p;
  
  if (clientAPI) 
  {
    Serial.println("connectAPI");
    for(int i=0;i<9;i++)
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
        if (state==1) room_p->SetRele();
        else if (state==0) room_p->ResetRele();
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
        clientAPI.print(room_p->CurrentState);
        clientAPI.print(";");
        }
        else clientAPI.print("ERROR;");
      }
      else clientAPI.print("ERROR;");
    }
    while(clientAPI.read()!=(-1));
    //clientAPI.print("\n");
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

