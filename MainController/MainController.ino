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
#if USE_NTP
  Serial.println("Start ntp request...");
  ntp->getTime();
  Serial.println("End ntp request."); 
#endif
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
    clientCLI.stop();
    Serial.println("listen...");
  }
}

