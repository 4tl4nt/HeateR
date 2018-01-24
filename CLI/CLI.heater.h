#ifndef CLI_HEATER_H
#define CLI_HEATER_H
#define printCLI CliStream->print

#include <PCF8574.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include <HeateR.h>

#define MAX_TIME_OUT 60000

#define EXIT_STATUS 0b10000000

#define CHECK_FOR_EXIT if (Status_flag&EXIT_STATUS) break
#define GO_TO_EXIT {\
Status_flag |= EXIT_STATUS;\
break;\
}
#define CLEAR_DISPLAY 1

#define TOP_SIMBOL "\f\t\t"

#if USE_NTP
extern unsigned long CurrentTimeRTC, RebootTime;
#endif

template<typename T, size_t n>
inline size_t arraySize(const T (&arr)[n])
{
    return n;
}
class ObjCLI
{
public:
	ObjCLI(Stream* s);
	Stream* CliStream;
	byte Status_flag;
	unsigned long Time_Out_Exit;
	char Buffer[100];
	int printAllSensors(DallasTemperature* DallasTemperature_p);
	int readNumCLI ();
	double readDoubleCLI();
	//void InitMenu ();
	void printMenu(char const **p, int size, bool clear = 0);
	void MainMenu ();
	void Menu_1_f();
	Room_c* NewRoom();
	void printAllRoom();
	void printRoom(Room_c* tmp_p);
	char WaitForAnyKey();
	void RemoveRoomFromCLI(int room = 99);
	void Menu_1_1();
	void ClimateControl(Room_c *p);
	void PrintMainSettings();
	int ReadString (char *buf, unsigned int length);
	int ReadIP (char *str, byte *b);
};
void UpDate ();
#endif