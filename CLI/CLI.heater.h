#ifndef CLI_HEATER_H
#define CLI_HEATER_H
#define printCLI CliStream->print

#include <PCF8574.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include <HeateR.h>

class item_c 
{
public:
	uint8_t numField; 
	char* field_p;
}; 
 
class menu_c: public item_c 
{
public:
	void action_p(void* function);
};
class ObjCLI/*: protected Stream*/ 
{
public:
	ObjCLI(Stream* s);
	Stream* CliStream;
	char Buffer[100];
	int printAllSensors(DallasTemperature* DallasTemperature_p);
	int readNumCLI ();
	double readDoubleCLI();
	void InitMenu ();
	void printMenu(menu_c* menu, int size);
	void MainMenu ();
	void Menu_1_f();
	Room_c* NewRoom();
	void printAllRoom();
	void printRoom(Room_c* tmp_p);
	char WaitForAnyKey();
	void RemoveRoomFromCLI();
	void ControlRoomCLI();
	void ClimateControl(Room_c *p);
};
void UpDate ();
#endif