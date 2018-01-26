#include "HeateR.h"

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

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*SOCKETS*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

PCF8574 pcf8574port1;//Обьект1 управлене микросхемой расширения GPIO, PCF8574
PCF8574 pcf8574port2;//Обьект2 управлене микросхемой расширения GPIO, PCF8574
PCF8574 pcf8574port3;//Обьект3 управлене микросхемой расширения GPIO, PCF8574
ListRoom_c ListRoom_c::FirstRoom; // Первая комната в списке комнат
#if USE_NTP
unsigned long CurrentTimeRTC=0, RebootTime=0;
#endif
/*
 *   Функция для конфигурации микросхемы PCF8574
 */
void InitRelayModule()
{
#if TEST_MODE==1
	  for (int i=2;i<5;i++)
	  {
		pinMode(i,OUTPUT);
		digitalWrite(i,LOW);
	  }
#endif
#ifdef ADDR_1
  pcf8574port1.begin(ADDR_1);
#endif
#ifdef ADDR_2
  pcf8574port2.begin(ADDR_2);
#endif
#ifdef ADDR_3
  pcf8574port3.begin(ADDR_3);
#endif
  
  for (int i=0;i<8;i++)
  {
#ifdef ADDR_1
  pcf8574port1.digitalWrite(i, HIGH);
#endif
#ifdef ADDR_2
  pcf8574port2.digitalWrite(i, HIGH);
#endif
#ifdef ADDR_3
  pcf8574port3.digitalWrite(i, HIGH);
#endif
  }
  
  for (int i=0;i<8;i++)
  {
#ifdef ADDR_1
  pcf8574port1.pinMode(i, OUTPUT);
#endif
#ifdef ADDR_2
  pcf8574port2.pinMode(i, OUTPUT);
#endif
#ifdef ADDR_3
  pcf8574port3.pinMode(i, OUTPUT);
#endif
  }
  Serial.println("Inited relay module.");
}

void InitHeateR(){
	digitalWrite(RESET_PIN,HIGH);
	pinMode(RESET_PIN,OUTPUT);
	pinMode(RESET_BUTTON_PIN,INPUT_PULLUP);
	delay(1);
	if (digitalRead(RESET_BUTTON_PIN)==LOW) HeaterReBoot(ResetMode);
	RestoryListFromEEPROM();
}

void HeaterReBoot(restartMode mode)
{
	if (mode==RebootMode){
#if USE_WDT
		while(1);
#else
		DEBUG("Reboot unpossible. Please turn off manually or enable WDT");
#endif
	}
	else if (mode==ResetMode){
		for (int i=0;i<START_ADD_CONF_ROOMS;i++){
			EEPROM.write(i, 0);
			delay(1);
		}
#if USE_WDT
		while(1);
#else
		DEBUG("Reboot unpossible. Please turn off manually or enable WDT");
#endif
	}
}
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*class Sensor_c*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
 *   Обьект Sensor_c, содержит информацию о датчике функцю замера температуры и калибровки.
 * Конструктор получает OneWire и DallasTemperature интервейсы из списка. Класс является
 * базовым для обьекта комнаты.
 */
Sensor_c::Sensor_c(int wireInt, uint8_t* wireAdd)
{
	ListOneWire_c* p = GetOneWire(wireInt);
	OneWireInterface_p = p->OneWire_p;
	DallasTemperature_p = p->DallasTemperature_p;
	DallasTemperature_p->begin();
	OneWireInterfacePin = wireInt;
	for (int i=0; i<8;i++) OneWireAddresse[i]=wireAdd[i];

	DEBUG_TIME();DEBUG("Found ");
	DEBUG(DallasTemperature_p->getDeviceCount(), DEC);
	DEBUG(" sensor(s) on OneWire bus");
	DEBUG(".\n");
	DallasTemperature_p->setResolution(OneWireAddresse, RESOLUTION_SENSORS);
	DEBUG_TIME();DEBUG("Resolution of sensor: ");
	DEBUG(DallasTemperature_p->getResolution(OneWireAddresse), DEC);
	DEBUG(".\n");

	CalibrationTemperature = OldTemperature = CountBadTemperature = 0;
	delay(10);
	CurrentTemperature = DallasTemperature_p->getTempC(OneWireAddresse);
	TimeOutGetTemperature = GETTIME();
	MaxBadTemperature = 5;
}
/*
 *   Функция для получения температуры, возвращает первый удачный замер температуры. Предпринимает
 * MaxBadTemperature попыток замера
 */
float Sensor_c::GetTemperature()
{
	CountBadTemperature=0;

	if ((TimeOutGetTemperature+TIMEOUT_GETTEMP) > GETTIME() && TimeOutGetTemperature < GETTIME()) return CurrentTemperature+CalibrationTemperature;
	do
	{
		TimeOutGetTemperature = GETTIME();
		DallasTemperature_p->requestTemperatures();
		CurrentTemperature = DallasTemperature_p->getTempC(OneWireAddresse);
		if (CurrentTemperature == -127)
		{
			if (CountBadTemperature==MaxBadTemperature) {
				return -127;
				DEBUG_TIME();
				DEBUG("Плохой замер температуры в группе портов №");
				DEBUG(OneWireInterfacePin);
				DEBUG(". Датчик №");
				for (int i=0;i<8;i++){
					Serial.print(OneWireAddresse[i],HEX);
					if (i<7)DEBUG(':');
				}
				DEBUG("\n");
			}
			else CountBadTemperature++;
			
		}
		delay(1);
	}while(CurrentTemperature == -127);
	if (CurrentTemperature != -127)return CurrentTemperature+CalibrationTemperature;
	return CurrentTemperature;
}
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-class Rele_c*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
Rele_c::Rele_c(int relePin)
{
	PinNumber=relePin;
	DEBUG_TIME();DEBUG("Used rele. Pin of rele is ");
	DEBUG(PinNumber);
	DEBUG(".\n");
	ResetRele();
}

void Rele_c::SetRele(bool State)
{
  int tmp = PinNumber-1;
#if TEST_MODE==1
	digitalWrite((PinNumber%3)+2,State);
  }
  else
#endif
#ifdef ADDR_1
  if (tmp<8) pcf8574port1.digitalWrite(tmp, State);
  else
#endif
#ifdef ADDR_2 
	  if (tmp<16) pcf8574port2.digitalWrite(tmp-8, State);
  else 
#endif
#ifdef ADDR_3
      if (tmp<24) pcf8574port3.digitalWrite(tmp-16, State);
  else
#endif
  {
    DEBUG_TIME();DEBUG("In function SetRalay is an incorrect PinNumber = ");
    DEBUG(tmp);
	DEBUG(".\n");
	return;
  }
  CurrentState=!State;
}

void Rele_c::ResetRele()
{
	SetRele(1);
}
/*
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*class Room_c*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
 *
 */
 
Room_c::Room_c(int room, int relePin, int wireInt, uint8_t* wireAdd)
:Sensor_c(wireInt, wireAdd),Rele_c(relePin)
{
	RoomNumber=room;
	MaxTemp = MINIMAL_TEMPERATURE+1;
	MinTemp = MINIMAL_TEMPERATURE;
	TimeOutCT = 0;
	EnableControlTemp=false;
	EnableMinTemp=false;
	DEBUG_TIME();DEBUG("Created new room. Number of room is ");
	DEBUG(RoomNumber);
	DEBUG(".\n");
}
/*
 *
 */
void Room_c::Update(){
	double temp = GetTemperature(); 
	if (TimeOutCT>0){
		unsigned long CurrentTime = millis();
		if (TimeOutCT < CurrentTime){
			if((CurrentTime-TimeOutCT) < 0xFFFF){
				SetControlTemp(false);
			}
		}
	}
	if (temp<(-27)) return;
	if (EnableControlTemp){
		if (temp<=MinTemp && !GetStateRele()) {
			DEBUG_TIME();DEBUG("Климат контроль: обогреватель №");
			DEBUG(RoomNumber);
			DEBUG(" ВКЛ\n");
			SetRele();
		}
		if (temp>=MaxTemp && GetStateRele()) {
			DEBUG_TIME();DEBUG("Климат контроль: обогреватель №");
			DEBUG(RoomNumber);
			DEBUG(" ВЫКЛ\n");
			ResetRele();
		}
	}
	else{
		if (temp<MINIMAL_TEMPERATURE && !GetStateRele()) {
			SetRele();
			EnableMinTemp=true;
			DEBUG_TIME();DEBUG("Автовключение системы обогрева №");
			DEBUG(RoomNumber);
			DEBUG("\n");
		}
		if ((temp>MAXIMAL_TEMPERATURE && GetStateRele()) || ((temp>(MINIMAL_TEMPERATURE+1)) && EnableMinTemp && GetStateRele())){
			ResetRele();
			DEBUG_TIME();DEBUG("Автовыключение системы обогрева №");
			DEBUG(RoomNumber);
			DEBUG("\n");
			EnableMinTemp=false;
		}
	}
}
/*
 *
 */
void Room_c::SetTimeOutCT(unsigned long i){
	DEBUG_TIME();DEBUG("SetTimeOutCT. ");
	DEBUG("i = ");
	DEBUG(i);
	DEBUG("\n");
	if (i==0){
		SetControlTemp(false);
		return;
	}
	i *= 1000;
	unsigned long CurrentTime = millis();
	if ((0xFFFFFFFF-CurrentTime)<i)
		TimeOutCT = i-(0xFFFFFFFF-CurrentTime);
	else 
		TimeOutCT = CurrentTime + i;
	
	SetControlTemp(true);
}
/*
 *
 */
void Room_c::SetControlTemp(double min, double max){
	if (min>max)return;
	if (min<MINIMAL_TEMPERATURE) {
		MinTemp = MINIMAL_TEMPERATURE;
		if (MinTemp>max) MaxTemp = MINIMAL_TEMPERATURE+1;
		else MaxTemp = max;
	}
	else if (max>MAXIMAL_TEMPERATURE) {
		MaxTemp = MAXIMAL_TEMPERATURE;
		if (MaxTemp<min) MinTemp = MAXIMAL_TEMPERATURE-1;
		else MinTemp = min;
	}
	else {
		MaxTemp = max;
		MinTemp = min;
	}
	EnableControlTemp=true;
}
/*
 *
 */
void Room_c::SetControlTemp(double temp){
	if ((temp-0.5)<MINIMAL_TEMPERATURE){
		MaxTemp = MINIMAL_TEMPERATURE+1;
		MinTemp = MINIMAL_TEMPERATURE;
	}
	else if ((temp+0.5)>MAXIMAL_TEMPERATURE){
		MinTemp = MAXIMAL_TEMPERATURE-1;
		MaxTemp = MAXIMAL_TEMPERATURE;
	}
	else {
		MinTemp = temp-0.5;
		MaxTemp = temp+0.5;
	}
	EnableControlTemp=true;
}
/*
 *
 */
void Room_c::SetControlTemp(bool state){
	EnableControlTemp=state;
	if (!state){
		TimeOutCT = 0;
		ResetRele();
	}
}
/*
 *
 */
 void UpdataNextOne(){
	 static ListRoom_c *curent_list = &ListRoom_c::FirstRoom;
	 if (curent_list->room_p != NULL) {
		 curent_list->room_p->Update();
		 curent_list = curent_list->next_p;
	 }
	 else curent_list = &ListRoom_c::FirstRoom;
 }
/*
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*class ListOneWire_c*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
 *
 */
ListOneWire_c FirstOneWire; // Обьект FirstOneWire это первый елемент в списке
ListOneWire_c::ListOneWire_c()
{
	Pin=-1;
	OneWire_p=NULL;
	next_p=NULL;
	DallasTemperature_p=NULL;
};
/*  
 *  Функция добавления нового интерефейса. принимает параметр номер пина. вовращает указаль на обект,
 * который содержит ссылку на OneWire и на DallasTemperature. Внутри функция ищет уже открытые интер-
 * фейсы, сопоставляя их с пинами. Нельзя открыть больше чемь 32 OneWire интерфейса.
*/
ListOneWire_c* GetOneWire(int wireInt)
{
	wireInt+=21;
	static int countList = 0;
	if (countList>LIST_MAX_WIRE) { Serial.println("List OneWire is crowded..."); return NULL; }
	ListOneWire_c* curentListOneWire = &FirstOneWire;
	while (curentListOneWire->Pin != wireInt && curentListOneWire->next_p!=NULL) curentListOneWire = curentListOneWire->next_p;
	if (curentListOneWire->OneWire_p!=NULL)
	{
		DEBUG_TIME();DEBUG("Used the created OneWire. Pin of OneWire is ");
		DEBUG(curentListOneWire->Pin);
		DEBUG(".\n");
		return curentListOneWire;
	}
	else
	{
		DEBUG_TIME();DEBUG("Created a new OneWire. Pin of OneWire is ");
		DEBUG(wireInt);
		DEBUG(".\n");
		curentListOneWire->Pin=wireInt;
		curentListOneWire->OneWire_p=new OneWire(wireInt);
		curentListOneWire->next_p=new ListOneWire_c();
		curentListOneWire->DallasTemperature_p = new DallasTemperature(curentListOneWire->OneWire_p);
		countList++;
		return curentListOneWire;
	}
}

ListRoom_c::ListRoom_c()
{
	next_p = NULL;
	room_p = NULL;
}

void addNewRoom(Room_c *room)
{
	ListRoom_c *list, *curent_list = &ListRoom_c::FirstRoom;
	int n, c, r;
	if (room != NULL)
	{	
		if (curent_list->room_p != NULL) //Проверяю не первая ли это комната
			c = curent_list->room_p->RoomNumber; //Номер текущей комнаты
		else {//Если комнат еще не было, создаю первую
			curent_list->room_p = room;
			curent_list->next_p = new ListRoom_c;
			return;
		}
		r = room->RoomNumber; // номер комнаты которую нужно добавить
		while(1){
			if (curent_list->next_p->room_p!=NULL){ // Проверяю существует ли следующая комната
				n = curent_list->next_p->room_p->RoomNumber;
			}
			else // если не существует, то проверяю куда нужно вписать новую комнату, до или после текущей?
				if (r>=c){ // после
					curent_list->next_p->room_p = room; 
					curent_list->next_p->next_p = new ListRoom_c;
					break;
				}
				else {  // до
					curent_list->next_p->next_p = new ListRoom_c;
					curent_list->next_p->room_p = curent_list->room_p;
					curent_list->room_p = room;
				}
			if (r>=n){ // так как следующая существует, проверяю стоит ли к ней перейти
				curent_list = curent_list->next_p;
			}
			else { 
				list = curent_list->next_p; // сохраняю следующий
				curent_list->next_p = new ListRoom_c; // на его место создаю новый
				curent_list->next_p->next_p = list; // старый следующий записую в поле новому следующему
				if (r<c){ // проверяю куда нужно вписать новую комнату, до или после текущей?
					curent_list->next_p->room_p=curent_list->room_p; // передвигаю комнату вперед
					curent_list->room_p = room; // присваиваю новую комнату в текущий
				}
				else{ // после
					curent_list->next_p->room_p = room; // присваиваю новую комнату на следующий елемент
				}
				break;
			}
		}
	}
}

Room_c* getRoom(int i)
{
	ListRoom_c* tmp = &ListRoom_c::FirstRoom;
	while(1)
	{
		if (tmp->room_p == NULL || tmp->room_p->RoomNumber == i) break;
		tmp = tmp->next_p;
	}
	return tmp->room_p;
}
void DeleteRoom(int room){
	ListRoom_c *tmp_p, *p;
	tmp_p = p = &ListRoom_c::FirstRoom;
	while(p->room_p!=NULL){
		if (p->room_p->RoomNumber==room){
			if (&ListRoom_c::FirstRoom != p){
				tmp_p->next_p=p->next_p;
				delete p->room_p;
				delete p;
			} else { 
				if (p->room_p== NULL) return;
				delete p->room_p; 
				if (p->next_p == NULL) return;
				tmp_p = p->next_p;
				p->next_p = tmp_p->next_p;
				p->room_p = tmp_p->room_p;
				delete tmp_p;
			}
			break;
		}
		tmp_p = p;
		p=p->next_p;
	}
}
/*
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* EEPROM *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
 *
 */
double RDFromEEPROM(int &addr){//RestoryDoubleFromEEPROM
	double tmp = EEPROM.read(addr++);
	delay(4);
	tmp += (double)(EEPROM.read(addr++))/100;
	delay(4);
	if (EEPROM.read(addr++))return(-tmp);
	else return(tmp);
}
void RestoryListFromEEPROM()
{
	DEBUG_TIME();DEBUG("RestoryListFromEEPROM\n");
	if (EEPROM.read(ADD_VERSION) != VERSION_HEATER) return;
	int addres=START_ADD_CONF_ROOMS, count = EEPROM.read(ADD_COUNT_SAVE_LIST);
	ListRoom_c *p = &ListRoom_c::FirstRoom;
	if (p->room_p!=NULL) return;
	int room, relePin, wireInt;
	DeviceAddress wireAdd;
	double tmp, tmp2;
	
	for (int i=1; i<=count; i++){
		room = EEPROM.read(addres++);
		relePin = EEPROM.read(addres++);
		wireInt = EEPROM.read(addres++);
		
		DEBUG_TIME();DEBUG("Read addres ");
		for(int i=0; i<8; i++){
		wireAdd[i] = EEPROM.read(addres++);
		DEBUG(wireAdd[i], HEX);
		DEBUG(":");
		}
		DEBUG("\n");
		p->room_p = new Room_c(room, relePin, wireInt, wireAdd);
		p->room_p->ResetRele();
		
		DEBUG_TIME();DEBUG("Read CalibrationTemperature: ");
		tmp = RDFromEEPROM(addres);
		p->room_p->SetCalibration(tmp);
		DEBUG(tmp);
		DEBUG("\n")
		DEBUG_TIME();DEBUG("Read Minimal Temperature: ");
		tmp = RDFromEEPROM(addres);
		DEBUG(tmp);
		DEBUG("\n")
		DEBUG_TIME();DEBUG("Read Maximal Temperature: ");
		tmp2 = RDFromEEPROM(addres);
		DEBUG(tmp2);
		DEBUG("\n");
		p->room_p->SetControlTemp(tmp, tmp2);
		p->room_p->SetControlTemp(false);
		p->next_p = new ListRoom_c;
		DEBUG("---------------------------------\n");
		p=p->next_p;
	}
}
void SDToEEPROM(int &addr, double num){//SaveDoubleToEEPROM
	double aBs = num<0?-num:num;
	EEPROM.write(addr++, (int)aBs);
	delay(4);
	EEPROM.write(addr++, (int)((aBs-(int)aBs)*100));
	delay(4);
	EEPROM.write(addr++, num<0?1:0);
	delay(4);
}
void SaveListToEEPROM()
{
	DEBUG_TIME();DEBUG("SaveListToEEPROM\n");
	int addres=START_ADD_CONF_ROOMS, count = 0;
	EEPROM.write(ADD_VERSION, VERSION_HEATER);
	delay(4);
	ListRoom_c *p = &ListRoom_c::FirstRoom;
	double DoubleNum;
	while(p->room_p!=NULL){
		count++;
		
		EEPROM.write(addres++, p->room_p->RoomNumber);
		delay(4);
		EEPROM.write(addres++, p->room_p->PinNumber);
		delay(4);
		EEPROM.write(addres++, p->room_p->OneWireInterfacePin);
		delay(4);
		for(int i=0; i<8; i++){
			EEPROM.write(addres++, p->room_p->OneWireAddresse[i]);
			delay(4);
		}
		delay(4);
		
		DoubleNum = p->room_p->GetCalibration();
		SDToEEPROM(addres, DoubleNum);
		DEBUG(DoubleNum);
		delay(4);
		DoubleNum = p->room_p->GetMinTemp();
		SDToEEPROM(addres, DoubleNum);
		DEBUG(DoubleNum);
		delay(4);
		DoubleNum = p->room_p->GetMaxTemp();
		SDToEEPROM(addres, DoubleNum);
		DEBUG(DoubleNum);
		delay(4);
		DEBUG_TIME();DEBUG("Write room: ");
		DEBUG(p->room_p->RoomNumber);
		DEBUG("\n");
		delay(4);
		DEBUG("\n---------------------------\n");
		p=p->next_p;
	}
	EEPROM.write(ADD_COUNT_SAVE_LIST, count);
	delay(4);
	DEBUG_TIME();DEBUG("Wroten ");
	
	DEBUG(count);
	DEBUG(" rooms.\n");
	DEBUG_TIME();DEBUG("SaveListToEEPROM is done.\n");
}
void ReadNetworkSettingsEEPROM(NetworkSettings *p){
	if (EEPROM.read(START_ADD_CONF_IP)==0){
		for (int i=0;i<4;i++)
		{
			p->ip[i] = DEFAULT_IP[i];
			p->mask[i] = DEFAULT_MASK[i];
			p->gateway[i] = DEFAULT_GATEWAY[i];
			p->dns[i] = DEFAULT_DNS[i];
		}
		for (int i=0;i<6;i++)p->mac[i] = DEFAULT_MAC[i];
		return;
	}
	for (int i=0;i<4;i++){
		p->ip[i] = EEPROM.read(START_ADD_CONF_IP+i);
		p->mask[i] = EEPROM.read(START_ADD_CONF_MASK+i);
		p->gateway[i] = EEPROM.read(START_ADD_CONF_GATEWAY+i);
		p->dns[i] = EEPROM.read(START_ADD_CONF_DNS+i);
	}
	for (int i=0;i<6;i++)p->mac[i] = EEPROM.read(START_ADD_CONF_MAC+i);
}
void WriteNetworkSettingsEEPROM(NetworkSettings *p){
	for (int i=0;i<4;i++){
		EEPROM.write(START_ADD_CONF_IP+i,p->ip[i]);
		EEPROM.write(START_ADD_CONF_MASK+i,p->mask[i]);
		EEPROM.write(START_ADD_CONF_GATEWAY+i,p->gateway[i]);
		EEPROM.write(START_ADD_CONF_DNS+i,p->dns[i]);
	}
	for (int i=0;i<6;i++)EEPROM.write(START_ADD_CONF_MAC+i,p->mac[i]);
}
void GetUserName(char *user, char *pass){
	int i;
	for (i=0; i<LENGH_USERNAME;i++){
		user[i]=EEPROM.read(START_ADD_CONF_USERNAME+i);
	}
	for (i=0; i<LENGH_PASSWORD;i++){
		pass[i]=EEPROM.read(START_ADD_CONF_PASSWORD+i);
	}
}
void SaveUserName(char *user, char *pass){
	int i;
	for (i=0; i<LENGH_USERNAME;i++){
		EEPROM.write(START_ADD_CONF_USERNAME+i,user[i]);
		if (user[i]==0)break;
	}
	for (; i<LENGH_USERNAME;i++)EEPROM.write(START_ADD_CONF_USERNAME+i,0);
	for (i=0; i<LENGH_PASSWORD;i++){
		EEPROM.write(START_ADD_CONF_PASSWORD+i,pass[i]);
		if (pass[i]==0)break;
	}
	for (; i<LENGH_PASSWORD;i++)EEPROM.write(START_ADD_CONF_PASSWORD+i,0);
}















