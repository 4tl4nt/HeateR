#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <HeateR.h>
#include <Communications.h>

ObjCLI *SerialCLI = NULL;
void UpDate (){
  
#if USE_WDT
  UpdateWDT (FLAG_4);
#endif
  UpdataNextOne();
  clientAPI = serverAPI.available();
  CommAPI(clientAPI);
  UpdateClientEthernet();
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
      //SerialCLI->InitMenu();
    }
    SerialCLI->MainMenu();
    if (clientCLI.connected()) clientCLI.stop();
    Serial.println("listen...");
  }
}

