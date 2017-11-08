#include "Communications.h"

char buff[SIZE_LOOP_BUF];
Room_c* room_p;

void CommAPI(EthernetClient client){
  if (client) 
  {
	int num_room=0, state=0;
    Serial.println("connectAPI");
    for(int i=0;i<SIZE_LOOP_BUF;i++)
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
        if (state==0) room_p->SetControlTemp(false);
        else if (state==1) {
          buff[14]='\0';
          state = atoi(&buff[9]);
          room_p->SetTimeOutCT(state);
        }
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
    while(client.read()!=(-1));
  Serial.println("listen...");
  }
}