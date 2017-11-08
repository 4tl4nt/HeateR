#include "CLI.heater.h"
#define DUBUGING_MODE
#ifdef DUBUGING_MODE
#define DEBUG(str, a...){ Serial.print(str, ##a);}
#else
#define DEBUG(str, a...)
#endif

#define NUMBER_ITEM_MAIN 7
menu_c mainMenu[NUMBER_ITEM_MAIN+1];
const char *MainItems[NUMBER_ITEM_MAIN+1]=
{
	"\f\n\t\tHeateR v 2.3",
	"Управление аудиториями",
	"Показать температуру в аудиториях",
	"Показать включенные обогреватели",
	"Сохранить в постоянную память",
	"Сброс настроек",
	"Перезагрузить",
	"Выход из меню"
};
#define NUMBER_ITEM_MENU_001 9
menu_c Menu_1[NUMBER_ITEM_MENU_001+1];
const char *Menu_1Items[NUMBER_ITEM_MENU_001+1]=
{
	"Выберите действие:",
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
#define NUMBER_ITEM_MENU_011 3
menu_c Menu_1_1[NUMBER_ITEM_MENU_011+1];
char const *Menu_1_1Items[NUMBER_ITEM_MENU_011+1]=
{
	Menu_1Items[0],
	"Управление обогревателем",
	"Калибровка датчика",
	Menu_1Items[4],
 };

ObjCLI::ObjCLI(Stream* s)
{
	CliStream=s;
	DEBUG("New ObjCLI\n");
}

void ObjCLI::InitMenu ()
{
	for (int i=0;i<=NUMBER_ITEM_MAIN;i++)
	{
		mainMenu[i].field_p = MainItems[i];
		mainMenu[i].numField = i;
	}
	 for (int i=0;i<=NUMBER_ITEM_MENU_001;i++)
	{
		Menu_1[i].field_p = Menu_1Items[i];
		Menu_1[i].numField = i;
	}
	 for (int i=0;i<=NUMBER_ITEM_MENU_011;i++)
	{
		Menu_1_1[i].field_p = Menu_1_1Items[i];
		Menu_1_1[i].numField = i;
	}
}
 
void ObjCLI::printMenu(menu_c menu[],int size)
{
	printCLI(menu[0].field_p);
	printCLI("\n");
	for (int i=1;i<=size;i++)
	{
		printCLI(menu[i].numField);
		printCLI(". ");
		printCLI(menu[i].field_p);
		printCLI("\n");
	}
	//printCLI("Сделайте свой выбор: ");
}
void ObjCLI::MainMenu (){
	ListRoom_c* tmp;
	while(1){
		printMenu(mainMenu, NUMBER_ITEM_MAIN);
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
			if (WaitForAnyKey()=='c')ControlRoomCLI();
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
			if (WaitForAnyKey()=='c')ControlRoomCLI();
		break;
		case 4://"Сохранить в постоянную память",
			SaveListToEEPROM();
			printCLI("Сохраненно\n");
			WaitForAnyKey();
		break;
		case 5://"Сброс настрек",
			printCLI("Вы точно хотите сбросить настройки?\n9. Да\n1. Нет\n");
			if (readNumCLI()==9) HeaterReBoot(ResetMode);
		break;
		case 6://"Перезагрузить",
			printCLI("Вы точно хотите перезагрузить устройство?\nВсе не несохраненные настройки будут потеряны.\nПродолжить?\n9. Да\n1. Нет\n");
			if (readNumCLI()==9) HeaterReBoot(ResetMode);
		break;
		case 7:
			while(CliStream->read()!=(-1));
			return;//"Выход из меню"
		break;
		default:
			printCLI("Используейте цифры 1-7\n");
		break;
		}
	}
}
//void getAllRoom(ptrFunc stream);
void ObjCLI::Menu_1_f (){
	int tmp;
	ListRoom_c* list_p;
	Room_c* room_p;
	while (1)
	{
		printCLI("\f\n\t\tCписок аудиторий:\n");
		printAllRoom();
		printMenu(Menu_1, NUMBER_ITEM_MENU_001);
		switch(readNumCLI())
		{
		case 1:
			ControlRoomCLI();
		break;
		case 2:
			printCLI("Укажите Аудиторию: ");
			tmp = readDoubleCLI();
			room_p = getRoom(tmp);
			if (room_p==NULL) {
				printCLI("Не корректно...\n");
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
				printCLI("Не корректно...");
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
	
	printCLI("\f\n\t\tДобавление новой аудитории.\nАудитория: ");
	r = readDoubleCLI();
	printCLI("Номер розетки: ");
	d = readDoubleCLI();
	printCLI("Номер группы портов датчика: ");
	w = readDoubleCLI();
	ListOneWire_p = GetOneWire(w);
	while(CliStream->read()!=(-1));
	while (n==99) 
	{
		
		while (!c){
			c = printAllSensors(ListOneWire_p->DallasTemperature_p);
			if (CliStream->available()) return NULL;
			if (!c) printCLI("Поиск...");
			delay(10);
		}
		printCLI("Выберите:\n(0...98)датчик\n(99)искать заново\n(100)выход\nСделайте свой выбор: ");
		n = readDoubleCLI();
		if (n==100) return NULL;
		if (n > c-1) {
			n=99;
			c=0;
			continue;
		}
	}
	ListOneWire_p->DallasTemperature_p->getAddress(a, n);
	if (ListOneWire_p->DallasTemperature_p->validAddress(a)) room = new Room_c(r,d,w,a);
	else {printCLI("Упссс... Что-то произошло, комната не добавленна!"); return NULL;}
	printCLI("Готово!!!\n");
	return room;
}

int ObjCLI::printAllSensors(DallasTemperature* DallasTemperature_p)
{
	DallasTemperature_p->begin();
	int count = DallasTemperature_p->getDeviceCount();
	printCLI("\f\n\t\tНайдено ");
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
	while(CliStream->read()!=(-1));
	delay(1);
	char tmp=0;
	while (CliStream->available() == 0)UpDate();
	CliStream->readBytes(&tmp, 1);
	printCLI(tmp);
    printCLI("\n");
	while(CliStream->read()!=(-1));
	if((tmp<0x30)&&(tmp>0x39)) return -1;
    return tmp-0x30;
}
double ObjCLI::readDoubleCLI()
{
	while(CliStream->read()!=(-1));
	char tmp=0;
	int i=0;
	while (tmp != 0x0D&&tmp != 0x0A){
		while (CliStream->available() == 0)UpDate();
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
	while (CliStream->available() == 0)UpDate();
		Buffer[0] = CliStream->read();
	return Buffer[0];
}

void ObjCLI::RemoveRoomFromCLI(){
	int tmp, tmp_room;
	while(1){
		printCLI("\f\n\t\tУдаление Аудитории\n");
		printAllRoom();
		printCLI("Ведите номер аудитории(99. выход):");
		tmp_room = readDoubleCLI();
		if (tmp_room==99)return;
		if (getRoom(tmp_room)==NULL){
			printCLI("Аудитории не существует.\n");
			WaitForAnyKey();
			continue;
		}
		printCLI("Вы точно хотите ");
		printCLI(tmp_room);
		printCLI("-ю аудиторию?\n1-ДА\n0-НЕТ\n");
		tmp = readNumCLI();
		if (!tmp) return;
		printCLI("Точно точно???\n1-ДА\n0-НЕТ\n");
		tmp = readNumCLI();
		if (tmp==0) {return; DEBUG("В RemoveRoomFromCLI(), tmp==0");}
		DeleteRoom(tmp_room);
		return;
	}
}
void ObjCLI::ControlRoomCLI()
{
	printCLI("Укажите № аудитории:");
	Room_c *p = getRoom(readDoubleCLI());
	double d_tmp;
	if (p==NULL)return;
	while(1)
	{
		printCLI("\f");
		printRoom(p);
		printCLI("\ВКЛ/ВЫКЛ обогреватель\n0-ВЫКЛ\n1-ВКЛ\n2. Обновить\n3. Калибровать датчик\n4. Cброс калибровки датчика\n5. Настроить КК\n6. Выключить КК\n7. Удалить аудиторию\n9. Выход из управления\n");
		printCLI("Сделайте свой выбор: ");
		switch(readNumCLI()){
			case 0: p->ResetRele();
		break;
			case 1: p->SetRele();
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
			DeleteRoom(p->RoomNumber);
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
	double temp = readDoubleCLI();
	if (temp < MINIMAL_TEMPERATURE || temp > MAXIMAL_TEMPERATURE){
		printCLI("Указана некорректно!\n");
		WaitForAnyKey();
		return;
	}
	p->SetControlTemp(temp);
}