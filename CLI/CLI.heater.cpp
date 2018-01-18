#include "CLI.heater.h"
#define DUBUGING_MODE
#ifdef DUBUGING_MODE
#define DEBUG(str, a...){ Serial.print(str, ##a);}
#else
#define DEBUG(str, a...)
#endif

const char INVALID_INPUT[] = "Не корректно...\n";

const char *MainItems[]=
{
	"HeateR v 2.4",
	"Управление аудиториями",
	"Показать температуру в аудиториях",
	"Показать включенные обогреватели",
	"Сохранить в постоянную память",
	"Сетевые настройки",
	"Сброс настроек",
	"Перезагрузить",
	"Выход"
};

const char *Menu_1Items[]=
{
	"Выберите действие(Enter - Обновить):",
	"Управлять одной аудиторией",
	"ВКЛ КК в аудитории",
	"ВКЛ КК во всех аудиториях",
	"ВЫКЛ КК в аудитории",
	"ВЫКЛ КК во всех аудиториях",
	"ВЫКЛ все обогреватели",
	"Добавить аудиторию",
	"Удалить Аудиторию",
	"Выход в главное меню"
};
const char *Menu_5Items[]=
{
	" ",
	"Настроить IP",
	"Настроить MASK",
	"Настроить GETAWEY",
	"Настроить DNS",
	"Настроить MAC",
	"Изменить Логин и Пароль",
	"Сохранить",
	Menu_1Items[9]
};
char const *Menu_1_1Items[]=
 {
	 "Обогреватель(Enter - Обновить):",
	 "ВКЛ",
	 "ВЫКЛ",
	 "Калибровать датчик",
	 "Cброс калибровки датчика",
	 "Настроить КК",
	 Menu_1Items[4],
	 Menu_1Items[8],
	 "Выход из управления"
 };
 char const *Reboot_Items[]=
 {
	"Вы точно хотите перезагрузить устройство?\nВсе не несохраненные настройки будут потеряны.\nПродолжить?",
	"Нет",
	"Да"
 };
 char const *Reset_Items[]=
 {
	 "Вы точно хотите сбросить настройки?",
	 "Нет",
	 "Да"
 };
ObjCLI::ObjCLI(Stream* s)
{
	CliStream=s;
	Status_flag = 0;
	DEBUG("New ObjCLI\n");
};

void ObjCLI::printMenu(char const **p, int size, bool clear)
{
	
	if (clear) printCLI(TOP_SIMBOL);
	printCLI((char*)(p[0]));
	printCLI("\n");
	int i;
	for (i=1;i<size-1;i++){
		printCLI(i);
		printCLI(". ");
		printCLI((char*)(p[i]));
		printCLI("\n");
	}
	printCLI(9);
	printCLI(". ");
	printCLI((char*)(p[i]));
	printCLI("\n");
}

void ObjCLI::MainMenu (){
	ListRoom_c* tmp;
	Status_flag &= (~EXIT_STATUS);
	while(1){
		printCLI("\f");
		printCLI("user:");
		ReadString (&Buffer[0], LENGH_USERNAME+1);
		printCLI("password:");
		ReadString (&Buffer[LENGH_USERNAME+1], LENGH_PASSWORD+1);
		GetUserName(&Buffer[LENGH_USERNAME+1+LENGH_PASSWORD+1],&Buffer[LENGH_USERNAME+1+LENGH_PASSWORD+1+LENGH_USERNAME+1]);
		if (!strcmp(&Buffer[LENGH_USERNAME+1+LENGH_PASSWORD+1],&Buffer[0])&&!
						strcmp(&Buffer[LENGH_USERNAME+1+LENGH_PASSWORD+1+LENGH_USERNAME+1],&Buffer[LENGH_USERNAME+1])){
			break;
		}
	}
	while(1){
		CHECK_FOR_EXIT; 
		printMenu(MainItems, arraySize(MainItems), CLEAR_DISPLAY);
		switch(readNumCLI()){
		case 1:
			Menu_1_f ();
		break;
		case 2://"Показать температуру в адиториях",
			tmp = &ListRoom_c::FirstRoom;
			printCLI("\f");
			while(tmp->room_p!=NULL){
				printCLI("№");
				printCLI(tmp->room_p->RoomNumber);
				printCLI(".\t ");
				printCLI(tmp->room_p->GetTemperature());
				printCLI("\n");
				tmp = tmp->next_p;
			}
			if (WaitForAnyKey()=='c')Menu_1_1();
			break;
		case 3://"Показать включенные обогреватели",
			tmp = &ListRoom_c::FirstRoom;
			printCLI("\f");
			while(tmp->room_p!=NULL){
				printCLI("№");
				printCLI(tmp->room_p->RoomNumber);
				printCLI(".\t ");
				if(tmp->room_p->GetStateRele())printCLI("ВКЛ");
				else printCLI("ВЫКЛ");
				printCLI("\n");
				tmp = tmp->next_p;
			}
			if (WaitForAnyKey()=='c')Menu_1_1();
		break;
		case 4://"Сохранить в постоянную память",
			SaveListToEEPROM();
			printCLI("Сохраненно\n");
			WaitForAnyKey();
		break;
		case 5:
			PrintMainSettings();
		break;
		case 6://"Сброс настрек",
			printMenu(Reset_Items, arraySize(Reset_Items));
			if (readNumCLI()==9) HeaterReBoot(ResetMode);
		break;
		case 7://"Перезагрузить",
			printMenu(Reboot_Items, arraySize(Reboot_Items));
			if (readNumCLI()==9) HeaterReBoot(RebootMode);
		break;
		case 9:
			while(CliStream->read()!=(-1));
			return;//"Выход из меню"
		break;
		default:
			printCLI(INVALID_INPUT);
			printCLI("Используейте цифры 1-7\n");
			WaitForAnyKey();
		break;
		}
	}
	return;//"Выход из меню"
}
void ObjCLI::Menu_1_f (){
	int tmp;
	ListRoom_c* list_p;
	Room_c* room_p;
	while (1)
	{
		CHECK_FOR_EXIT;//here is allow
		printCLI(TOP_SIMBOL"Cписок аудиторий:\n");
		printAllRoom();
		printMenu(Menu_1Items, arraySize(Menu_1Items));
		switch(readNumCLI())
		{
		case 1:
			Menu_1_1();
		break;
		case 2:
			printCLI("Укажите Аудиторию: ");
			tmp = readDoubleCLI();
			room_p = getRoom(tmp);
			if (room_p==NULL) {
				printCLI(INVALID_INPUT);
				WaitForAnyKey();
			}
			else room_p->SetControlTemp(true);
		break;
		case 3:
			list_p = &ListRoom_c::FirstRoom;
			while (list_p->room_p!=NULL){
				list_p->room_p->SetControlTemp(true);
				list_p = list_p->next_p;
			}
		break;
		case 4:
			printCLI("Укажите Аудиторию: ");
			tmp = readDoubleCLI();
			room_p = getRoom(tmp);
			if (room_p==NULL) {
				printCLI(INVALID_INPUT);
				WaitForAnyKey();
			}
			else room_p->SetControlTemp(false);
		break;
		case 5:
			list_p = &ListRoom_c::FirstRoom;
			while (list_p->room_p!=NULL){
				list_p->room_p->SetControlTemp(false);
				list_p = list_p->next_p;
			}
		break;
		case 6:
			list_p = &ListRoom_c::FirstRoom;
			while (list_p->room_p!=NULL){
				list_p->room_p->ResetRele();
				list_p = list_p->next_p;
			}
		break;
		case 7:
			addNewRoom(NewRoom());
		break;
		case 8:
			RemoveRoomFromCLI();
		break;
		case 9:
			return;
		default:
			printCLI("Используейте цифры 1-9\n");
			break;
		}
	}
}

Room_c* ObjCLI::NewRoom(){
	Room_c* room=NULL;
	int c=0, r, d, w, n=99;
	DeviceAddress a;
	ListOneWire_c* ListOneWire_p=NULL;
	while(true){
		printCLI(TOP_SIMBOL"Добавление новой аудитории.\nАудитория: ");
		r = readDoubleCLI();
		CHECK_FOR_EXIT;
		printCLI("Номер розетки: ");
		d = readDoubleCLI();
		CHECK_FOR_EXIT;
		printCLI("Номер группы портов датчика: ");
		w = readDoubleCLI();
		CHECK_FOR_EXIT;
		ListOneWire_p = GetOneWire(w);
		while(CliStream->read()!=(-1));
		while (n==99) 
		{
			CHECK_FOR_EXIT;
			Time_Out_Exit = MAX_TIME_OUT + millis();
			while (!c){
				c = printAllSensors(ListOneWire_p->DallasTemperature_p);
				if (CliStream->available()) return NULL;
				if (!c) printCLI("Поиск...");
				delay(10);
				if (Time_Out_Exit<millis())GO_TO_EXIT;
			}
			CHECK_FOR_EXIT;
			printCLI("Выберите:\n(0...98)датчик\n(99)искать заново\n(100)выход\nСделайте свой выбор: ");
			n = readDoubleCLI();
			CHECK_FOR_EXIT;
			if (n==100) return NULL;
			if (n > c-1) {
				n=99;
				c=0;
				continue;
			}
			CHECK_FOR_EXIT;
		}
		CHECK_FOR_EXIT;
		ListOneWire_p->DallasTemperature_p->getAddress(a, n);
		if (ListOneWire_p->DallasTemperature_p->validAddress(a)) room = new Room_c(r,d,w,a);
		else {printCLI(INVALID_INPUT); WaitForAnyKey(); return NULL;}
		printCLI("Готово!!!\n");
		return room;
	}
}

int ObjCLI::printAllSensors(DallasTemperature* DallasTemperature_p)
{
	DallasTemperature_p->begin();
	int count = DallasTemperature_p->getDeviceCount();
	printCLI(TOP_SIMBOL"Найдено ");
	printCLI(count);
	if ((count%100!=11)&&(count%10==1))printCLI(" датчик.\n");
	else if (
			(
				(count%100!=13)&&
				(count%100!=12)&&
				(count%100!=14)
			)
			&&
			(
				(count%10==2)||
				(count%10==3)||
				(count%10==4)
			)
		)
		printCLI(" датчика.\n");
	else printCLI(" датчиков.\n");
	for (int i=0; i!=count; i++)
	{
		DeviceAddress tmp;
		printCLI(i);
		printCLI(". ");
		DallasTemperature_p->getAddress(tmp, i);
		for (int j=0;j<7;j++){printCLI(tmp[j], HEX);printCLI(":");}
		printCLI(tmp[7], HEX);
		printCLI("\n");
	}
	return count;
}

int ObjCLI::readNumCLI ()
{
	Time_Out_Exit = MAX_TIME_OUT + millis();
	while(CliStream->read()!=(-1));
	delay(1);
	char tmp=0;
	while (CliStream->available() == 0){
		UpDate();
		if (Time_Out_Exit<millis()) GO_TO_EXIT;
	}
	CliStream->readBytes(&tmp, 1);
	printCLI(tmp);
    printCLI("\n");
	while(CliStream->read()!=(-1));
	if((tmp<0x30)&&(tmp>0x39)) return -1;
    return tmp-0x30;
}
double ObjCLI::readDoubleCLI()
{
	Time_Out_Exit = MAX_TIME_OUT + millis();
	while(CliStream->read()!=(-1));
	char tmp=0;
	int i=0;
	while (tmp != 0x0D&&tmp != 0x0A){
		while (CliStream->available() == 0){
			UpDate();
			if (Time_Out_Exit<millis()) GO_TO_EXIT;
		}
		CliStream->readBytes(&tmp, 1);
		Buffer[i++]=tmp;
		printCLI(tmp);
	}
	if (i==1) return 0;
    Buffer[i]='\0';
    printCLI("\n");
	while(CliStream->read()!=(-1));
    return strtod(Buffer, NULL);
}

void ObjCLI::printAllRoom()
{
	ListRoom_c *tmp_p = &ListRoom_c::FirstRoom;
	while(tmp_p->room_p != NULL)
	{
		printRoom(tmp_p->room_p);
		tmp_p=tmp_p->next_p;
	}
}
void ObjCLI::printRoom(Room_c* tmp_p)
{
	printCLI("№");
	printCLI(tmp_p->RoomNumber);
	if (tmp_p->RoomNumber>9) printCLI(", C:");
	else printCLI(",  C:");
	printCLI(tmp_p->GetTemperature());
	printCLI(", Heater:");
	if(tmp_p->GetStateRele()) printCLI(" ВКЛ");
	else printCLI("ВЫКЛ");
	printCLI(", ");
	printCLI(tmp_p->GetMinTemp());
	if (tmp_p->GetControlTemp()) printCLI("<<AUTO>>");
	else printCLI("<<    >>");
	printCLI(tmp_p->GetMaxTemp());
	printCLI("\n");
}

char ObjCLI::WaitForAnyKey(){
	printCLI("Нажмите любую клавишу...\n");
	while (CliStream->available() == 0){
		UpDate();
		if (Time_Out_Exit<millis()) GO_TO_EXIT;
	}
	Buffer[0] = CliStream->read();
	return Buffer[0];
}

void ObjCLI::RemoveRoomFromCLI(int room){
	int tmp;
	while(1){
		if (room==99){
			printCLI(TOP_SIMBOL"Удаление Аудитории\n");
			printAllRoom();
			printCLI("Ведите номер аудитории(99. выход):");
			room = readDoubleCLI();
			CHECK_FOR_EXIT;
			if (room==99)return;
			if (getRoom(room)==NULL){
				printCLI(INVALID_INPUT);
				WaitForAnyKey();
				continue;
			}
		}
		printCLI("Вы точно хотите удалить ");
		printCLI(room);
		printCLI("-ю аудиторию?\n1-ДА\n0-НЕТ\n");
		tmp = readNumCLI();
		CHECK_FOR_EXIT;
		if (!tmp) return;
		printCLI("Точно точно???\n1-ДА\n0-НЕТ\n");
		tmp = readNumCLI();
		CHECK_FOR_EXIT;
		DeleteRoom(room);
		return;
	}
}
void ObjCLI::Menu_1_1()
{
	printCLI("Укажите № аудитории:");
	Room_c *p = getRoom(readDoubleCLI());
	double d_tmp;
	if (p==NULL){
		printCLI(INVALID_INPUT);
		WaitForAnyKey();
		return;
	}
	while(1)
	{
		CHECK_FOR_EXIT;
		printCLI("\f");
		printRoom(p);
		printMenu(Menu_1_1Items, arraySize(Menu_1_1Items));
		printCLI("Сделайте свой выбор: ");
		switch(readNumCLI()){
			case 0:
		break;
			case 1: p->SetRele();
		break;
			case 2: p->ResetRele(); 
		break;
			case 3: 
			printCLI("Укажите поправку температуры:");
			p->SetCalibration(readDoubleCLI());
		break;
			case 4: 
			p->ResetCalibration();
			printCLI("Выполнен сброс калибровки\n");
			WaitForAnyKey();
		break;
			case 5: 
			ClimateControl(p);
		break;
			case 6: 
			p->SetControlTemp(false);
			printCLI("КК отключен\n");
			WaitForAnyKey();
		break;
			case 7: 
			RemoveRoomFromCLI(p->RoomNumber);
			return;
		break;
			case 9: return;
		break;
			default: 
		break;
		}
	}
}
void ObjCLI::ClimateControl(Room_c *p){
	printCLI("Укажите температуру: ");
	while (true){
		double temp = readDoubleCLI();
		CHECK_FOR_EXIT;
		if (temp < MINIMAL_TEMPERATURE || temp > MAXIMAL_TEMPERATURE){
			printCLI(INVALID_INPUT);
			WaitForAnyKey();
			break;
		}
		p->SetControlTemp(temp);
		break;
	}
	return;
}
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*Main Settings*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
void ObjCLI::PrintMainSettings() {
	NetworkSettings NetSets;
	ReadNetworkSettingsEEPROM(&NetSets);
	byte b[4];
	while(1)
	{
		CHECK_FOR_EXIT;
		printCLI(TOP_SIMBOL"Network Settings\n");
		printCLI("IP:\t ");
		for (int i=0;i<4;i++){ printCLI(NetSets.ip[i]);if(i<3)printCLI('.');}
		printCLI("\nMASK:\t ");
		for (int i=0;i<4;i++){ printCLI(NetSets.mask[i]);if(i<3)printCLI('.');}
		printCLI("\nGETAWEY: ");
		for (int i=0;i<4;i++){ printCLI(NetSets.gateway[i]);if(i<3)printCLI('.');}
		printCLI("\nDNS:\t ");
		for (int i=0;i<4;i++){ printCLI(NetSets.dns[i]);if(i<3)printCLI('.');}
		printCLI("\nMAC:\t ");
		for (int i=0;i<6;i++){ printCLI(NetSets.mac[i]);if(i<5)printCLI(':');}
		
		printMenu(Menu_5Items, arraySize(Menu_5Items));
		switch(readNumCLI()){
			case 1: 
			printCLI("Введите IP: ");
			if (ReadString(&Buffer[0], 20)==-1){
				DEBUG("ReadString -1\n");
				break;
			}
			if (ReadIP(&Buffer[0], &b[0])==-1){
				DEBUG("ReadIP -1\n");
				break;
			}
			for (int i=0;i<4;i++){ NetSets.ip[i] = b[i];}
		break;
			case 2: 
			printCLI("Введите MASK: ");
			if (ReadString(&Buffer[0], 20)==-1){
				DEBUG("ReadString -1\n");
				break;
			}
			if (ReadIP(&Buffer[0], &b[0])==-1){
				DEBUG("ReadIP -1\n");
				break;
			}
			for (int i=0;i<4;i++){ NetSets.mask[i] = b[i];}
		break;
			case 3: 
			printCLI("Введите GETAWEY: ");
			if (ReadString(&Buffer[0], 20)==-1){
				DEBUG("ReadString -1\n");
				break;
			}
			if (ReadIP(&Buffer[0], &b[0])==-1){
				DEBUG("ReadIP -1\n");
				break;
			}
			for (int i=0;i<4;i++){ NetSets.gateway[i] = b[i];}
		break;
			case 4: 
			printCLI("Введите DNS: ");
			if (ReadString(&Buffer[0], 20)==-1){
				DEBUG("ReadString -1\n");
				break;
			}
			if (ReadIP(&Buffer[0], &b[0])==-1){
				DEBUG("ReadIP -1\n");
				break;
			}
			for (int i=0;i<4;i++){ NetSets.dns[i] = b[i];}
		break;
			case 5: 
			printCLI("Пункт не доступен");
		break;
		
		
		
			case 6: 
			printCLI("\nЛогин: ");
			if (ReadString(&Buffer[0], LENGH_USERNAME+1)==-1){
				printCLI(INVALID_INPUT);
				WaitForAnyKey();
				break;
			}
			printCLI("\nПароль: ");
			if (ReadString(&Buffer[LENGH_USERNAME+1], LENGH_PASSWORD+1)==-1){
				printCLI(INVALID_INPUT);
				WaitForAnyKey();
				break;
			}
			SaveUserName(&Buffer[0],&Buffer[LENGH_USERNAME+1]);
			
			
			
		break;
			case 7: 
			WriteNetworkSettingsEEPROM(&NetSets);
			printCLI("\nСохраненно.\nНастройки будут применены после перезагрузки!\n");
			WaitForAnyKey();
		break;
			case 9: 
			return;
		break;
			default: 
		break;
		}
	}
}
int ObjCLI::ReadString (char *buf, unsigned int length){
	int i;
	for (i=0;i<length;i++)buf[i]=0;
	i=0;
	Time_Out_Exit = MAX_TIME_OUT + millis();
	while(CliStream->read()!=(-1));
	while (i<length){		
		while (CliStream->available() == 0){
			UpDate();
			if (Time_Out_Exit<millis()) GO_TO_EXIT;
		}
		CliStream->readBytes(&buf[i], 1);
		DEBUG(buf[i]);
		if (buf[i] == 0x0D || buf[i] == 0x0A) {buf[i]=0;break;}
		else i++;
	}
	if (i<length) return i;
	else return -1;
}
int ObjCLI::ReadIP (char *str, byte *b){
	unsigned int addr=0;
	char tmp[4];
	int i=0, count=0;
	while (1){
		while (str[addr]!='.'&&str[addr]!=0){
			DEBUG(str[addr]);
			if (str[addr]>='0'&&str[addr]<='9')tmp[i++]=str[addr++];
			else return -1;
			if (i>3) return -1;
		}
		if (str[addr]==0&&count<3) return -1;
		if (str[addr]=='.')str[addr]=0;
		b[count++] = atoi(&str[addr-i]);
		if (count==4) return addr;
		addr++;
		i=0;
	}
}