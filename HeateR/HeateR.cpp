#include "HeateR.h"

#ifdef DUBUGING_MODE
#define DEBUG(str, a...){Serial.print(str, ##a);}
#else
#define DEBUG(str, a...)
#endif

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*SOCKETS*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

PCF8574 pcf8574port1;//Обьект1 управлене микросхемой расширения GPIO, PCF8574
PCF8574 pcf8574port2;//Обьект2 управлене микросхемой расширения GPIO, PCF8574
ListRoom_c ListRoom_c::FirstRoom; // Первая комната в списке комнат
/*
 *   Функция для конфигурации микросхемы PCF8574
 */
void InitRelayModule()
{
  pcf8574port1.begin(ADDR_1);
  pcf8574port2.begin(ADDR_2);
  
  for (int i=0;i<8;i++)
  {
  pcf8574port1.digitalWrite(i, HIGH);
  pcf8574port2.digitalWrite(i, HIGH);
  }
  
  for (int i=0;i<8;i++)
  {
  pcf8574port1.pinMode(i, OUTPUT);
  pcf8574port2.pinMode(i, OUTPUT);
  }
  
  Serial.println("Inited relay module.");
}

void InitHeateR(){
	digitalWrite(RESET_PIN,HIGH);
	pinMode(RESET_PIN,OUTPUT);
	RestoryListFromEEPROM();
}

void HeaterReBoot(restartMode mode)
{
	if (mode==RebootMode){
		digitalWrite(RESET_PIN, LOW);
	}
	else if (mode==ResetMode){
		EEPROM.write(5, 0);
		digitalWrite(RESET_PIN, LOW);
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

	DEBUG("Found ");
	DEBUG(DallasTemperature_p->getDeviceCount(), DEC);
	DEBUG(" sensor(s) on OneWire bus");
	DEBUG(".\n");
	DallasTemperature_p->setResolution(OneWireAddresse, RESOLUTION_SENSORS);
	DEBUG("Resolution of sensor: ");
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
			if (CountBadTemperature==MaxBadTemperature) return -127;
			else CountBadTemperature++;
			DEBUG("Плохой замер температуры в группе портов №");
			DEBUG(OneWireInterfacePin-21);
			DEBUG("\n");
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
	DEBUG("Used rele. Pin of rele is ");
	DEBUG(PinNumber);
	DEBUG(".\n");
	ResetRele();
}

void Rele_c::SetRele()
{
  int tmp = PinNumber-1;
  if (tmp<8) pcf8574port1.digitalWrite(tmp, LOW);
  else if (tmp<16) pcf8574port2.digitalWrite(tmp-8, LOW);
  else
  {
    DEBUG("In function SetRalay is an incorrect PinNumber = ");
    DEBUG(tmp);
	DEBUG(".\n");
	return;
  }
  CurrentState=1;
}

void Rele_c::ResetRele()
{
  int tmp = PinNumber-1;
  if (tmp<8) pcf8574port1.digitalWrite(tmp, HIGH);
  else if (tmp<16) pcf8574port2.digitalWrite(tmp-8, HIGH);
  else
  {
    DEBUG("In function ResetRalay is an incorrect PinNumber = ");
    DEBUG(tmp);
	DEBUG(".\n");
	return;
  }
  CurrentState=0;
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
	EnableControlTemp=false;
	DEBUG("Created new room. Number of room is ");
	DEBUG(RoomNumber);
	DEBUG(".\n");
}
/*
 *
 */
void Room_c::Update(){
	double temp = GetTemperature(); 
	if (temp<(-27)) return;
	if (EnableControlTemp){
		if (temp<=MinTemp && !GetStateRele()) {
			DEBUG("Климат контроль: обогреватель №");
			DEBUG(RoomNumber);
			DEBUG(" ВКЛ\n");
			SetRele();
		}
		if (temp>=MaxTemp && GetStateRele()) {
			DEBUG("Климат контроль: обогреватель №");
			DEBUG(RoomNumber);
			DEBUG(" ВЫКЛ\n");
			ResetRele();
		}
	}
	else{
		if (temp<MINIMAL_TEMPERATURE && !GetStateRele()) {
			SetRele();
			DEBUG("Автовключение системы обогрева №");
			DEBUG(RoomNumber);
			DEBUG("\n");
		}
		if (temp>MAXIMAL_TEMPERATURE && GetStateRele()) {
			ResetRele();
			DEBUG("Автовыключение системы обогрева №");
			DEBUG(RoomNumber);
			DEBUG("\n");
		}
	}
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
 *
void Room_c::SetControlTemp(bool state){
	EnableControlTemp=state;
	if (!state){
		MaxTemp = MINIMAL_TEMPERATURE+1;
		MinTemp = MINIMAL_TEMPERATURE;
		ResetRele();
	}
}
 *
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
		DEBUG("Used the created OneWire. Pin of OneWire is ");
		DEBUG(curentListOneWire->Pin);
		DEBUG(".\n");
		return curentListOneWire;
	}
	else
	{
		DEBUG("Created a new OneWire. Pin of OneWire is ");
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
				DEBUG("delete room\n");
			} else { 
				if (p->room_p== NULL) return;
				delete p->room_p; 
				if (p->next_p == NULL) return;
				tmp_p = p->next_p;
				p->next_p = tmp_p->next_p;
				p->room_p = tmp_p->room_p;
				delete tmp_p;
				DEBUG("not delete? room is first\n");
			}
			break;
		}
		tmp_p = p;
		p=p->next_p;
	}
}
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
	if (EEPROM.read(0) != VERSION_HEATER) return;
	int addres=6, count = EEPROM.read(5);
	ListRoom_c *p = &ListRoom_c::FirstRoom;
	if (p->room_p!=NULL) return;
	int room, relePin, wireInt;
	DeviceAddress wireAdd;
	double tmp, tmp2;
	
	for (int i=1; i<=count; i++){
		room = EEPROM.read(addres++);
		relePin = EEPROM.read(addres++);
		wireInt = EEPROM.read(addres++);
		
		DEBUG("Read addres ");
		for(int i=0; i<8; i++){
		wireAdd[i] = EEPROM.read(addres++);
		DEBUG(wireAdd[i], HEX);
		DEBUG(":");
		}
		DEBUG("\n");
		p->room_p = new Room_c(room, relePin, wireInt, wireAdd);
		DEBUG("Read CurrentState\n");
		p->room_p->CurrentState = EEPROM.read(addres++);
		delay(4);
		if (p->room_p->CurrentState)p->room_p->SetRele();
		else p->room_p->ResetRele();
		
		DEBUG("Read CalibrationTemperature: ");
		tmp = RDFromEEPROM(addres);
		p->room_p->SetCalibration(tmp);
		DEBUG(tmp);
		DEBUG("\nRead Minimal Temperature: ");
		tmp = RDFromEEPROM(addres);
		DEBUG(tmp);
		DEBUG("\nRead Maximal Temperature: ");
		tmp2 = RDFromEEPROM(addres);
		DEBUG(tmp2);
		DEBUG("\n");
		p->room_p->SetControlTemp(tmp, tmp2);
		p->room_p->SetControlTemp((bool)(EEPROM.read(addres++)));
		p->next_p = new ListRoom_c;
		DEBUG("\n---------------------------\n");
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
	int addres=6, count = 0;
	EEPROM.write(0, VERSION_HEATER);
	delay(4);
	ListRoom_c *p = &ListRoom_c::FirstRoom;
	double DoubleNum;
	while(p->room_p!=NULL){
		count++;
		
		EEPROM.write(addres++, p->room_p->RoomNumber);
		delay(4);DEBUG("Write RoomNumber\n");
		EEPROM.write(addres++, p->room_p->PinNumber);
		delay(4);DEBUG("Write PinNumber\n");
		EEPROM.write(addres++, p->room_p->OneWireInterfacePin);
		delay(4);DEBUG("Write OneWireInterfacePin\nWrite Add = ");
		for(int i=0; i<8; i++){
			EEPROM.write(addres++, p->room_p->OneWireAddresse[i]);
			delay(4);
			DEBUG(p->room_p->OneWireAddresse[i], HEX);
			DEBUG(":");
		}
		delay(4);DEBUG("\nWrite CurrentState");
		EEPROM.write(addres++, p->room_p->CurrentState);
		delay(4);
		
		DoubleNum = p->room_p->GetCalibration();
		DEBUG("\nWrite CalibrationTemperature: ");
		SDToEEPROM(addres, DoubleNum);
		DEBUG(DoubleNum);
		delay(4);
		DoubleNum = p->room_p->GetMinTemp();
		DEBUG("\nWrite Minimal Temperature: ");
		SDToEEPROM(addres, DoubleNum);
		DEBUG(DoubleNum);
		delay(4);
		DoubleNum = p->room_p->GetMaxTemp();
		DEBUG("\nWrite Maximal Temperature: ");
		SDToEEPROM(addres, DoubleNum);
		DEBUG(DoubleNum);
		delay(4);
		DEBUG("\nWrite State Climate Control\n");
		EEPROM.write(addres++, p->room_p->GetControlTemp());
		DEBUG("Wroten ");
		DEBUG(addres);
		DEBUG(" bytes");
		delay(4);
		DEBUG("\n---------------------------\n");
		p=p->next_p;
	}
	EEPROM.write(5, count);
	delay(4);
	DEBUG("Wroten ");
	
	DEBUG(count);
	DEBUG(" rooms.\nSaveListToEEPROM is done.\n");
}
