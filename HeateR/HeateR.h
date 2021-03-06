#ifndef HEATER_H
#define HEATER_H

#if ARDUINO >= 100
#include "Arduino.h"
#define I2CWRITE(x) Wire.write(x)
#define I2CREAD() Wire.read()
#else
#include "WProgram.h"
#define I2CWRITE(x) Wire.send(x)
#define I2CREAD() Wire.receive()
#define INPUT_PULLUP 2
#endif

#define NUMBER_OF_SOCKETS 24
#define PCF8574_BASE_ADD  0x20
#define LIST_MAX_ROOM 32 // Максимально количество комнат в списке ListRoom_c
#define LIST_MAX_WIRE 31  // Максимально количество интерфейсов в списке ListOneWire_c
#define RESOLUTION_SENSORS 10// Разрядность сенсоров
#define DUBUGING_MODE // Закоментировать чтобы отключить вывод отладочной информации в терминал
#define VERSION_HEATER 24 // Версия прошивки влияет на сохраненные настройки
#define RESET_PIN 8 // пин который выведен на контакт ресет
#define RESET_BUTTON_PIN 7 // пин кнопки сброса настроек, кнопку нужно зажимать во время старта
#define TIMEOUT_GETTEMP 5000 // количество мелискунд для повторного запроса температуры
#define MINIMAL_TEMPERATURE 15.00
#define MAXIMAL_TEMPERATURE 30.00


#define ADD_VERSION 4
#define ADD_COUNT_SAVE_LIST 5
#define START_ADD_CONF_IP 6
#define START_ADD_CONF_MASK 10
#define START_ADD_CONF_GATEWAY 14
#define START_ADD_CONF_DNS 18
#define START_ADD_CONF_PORT_API 22
#define START_ADD_CONF_PORT_CLI 24
#define START_ADD_CONF_MAC 26
#define LENGH_USERNAME 10
#define LENGH_PASSWORD 10
#define START_ADD_CONF_USERNAME 32
#define START_ADD_CONF_PASSWORD LENGH_USERNAME+START_ADD_CONF_USERNAME
#define START_ADD_CONF_ROOMS LENGH_PASSWORD+START_ADD_CONF_PASSWORD
#define USE_NTP 1
#define USE_WDT 1 

#if USE_WDT
#include <avr/wdt.h>
#endif
#if USE_NTP
extern unsigned long CurrentTimeRTC, RebootTime;
#endif
typedef struct{
	byte mac[6];
	byte ip[4];
    byte mask[4];
	byte gateway[4];
	byte dns[4];
} NetworkSettings;
const byte DEFAULT_MAC[] = { 0x00, 0xFD, 0xBF, 0xFF, 0xEE, 0xEF };
const byte DEFAULT_IP[] = {192,168,1,100};
const byte DEFAULT_MASK[] = {255,255,255,0};
const byte DEFAULT_GATEWAY[] = {192,168,1,1};
const byte DEFAULT_DNS[] = {8,8,8,8};
const unsigned int DEFAULT_portAPI = 12345;
const unsigned int DEFAULT_portCLI = 12346;


#define GETTIME() millis()

#include <inttypes.h>
#include <EEPROM.h>
#include <PCF8574.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Communications.h>

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*SOCKETS*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

#define ADDR_1  PCF8574_BASE_ADD|0
#define ADDR_2  PCF8574_BASE_ADD|7
#define ADDR_3  PCF8574_BASE_ADD|3
#define TEST_MODE 0
/*
 *
 */
enum restartMode{
	ResetMode,
	RebootMode,
	ReloadMode,
	SleepMode
};
/*
 * функции инициализации и остановки устройства. 
 */
void InitRelayModule();
void InitHeateR();
void HeaterReBoot(restartMode mode);
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*TEMPERATURE*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
 * Клас Датчика
 * Базовый клас для класа комнаты, хранит всю информацию о датчике
 * Имеет функции управления, калибровки и запроса температоры.
 */
class Sensor_c {
	OneWire* OneWireInterface_p;
	DallasTemperature* DallasTemperature_p;
	float CalibrationTemperature;
	float CurrentTemperature;
	float OldTemperature;
	long TimeOutGetTemperature;
	int CountBadTemperature;
	int MaxBadTemperature;
public:
	DeviceAddress OneWireAddresse;
	int OneWireInterfacePin;
	Sensor_c(int wireInt, uint8_t* wireAdd);
	float GetTemperature();
	float GetCalibration(){return CalibrationTemperature;};
	bool SetCalibration(double i){CalibrationTemperature+=i;};
	bool ResetCalibration(){CalibrationTemperature=0;};
};
/*
 * Клас Розетки
 * Базовый клас для класа комнаты, хранит всю информацию о розетке
 * Имеет функции управления розеткой
 */
class Rele_c {
public:
	int PinNumber;
	int CurrentState;
	Rele_c(int relePin);
	void SetRele(bool State=0);
	void ResetRele();
	int GetStateRele(){return CurrentState;};
};
/*
 * Клас Комнаты. 
 * Хранит в себе информацию об одном датчике, розетке и номере комнаты
 * Обьект Room_c. Принимает 4 параметра, первый номер комнаты, второй номер розетки на патч 
 * панели(1-16), третий OneWire пин к которому подключен датчик в этой комнате, четвертый 
 * адрес датчика(адрес может быть NULL, тогда запуститься функция поиска датчика на указаном
 * OneWire интерфейсе)
 */
class Room_c: public Sensor_c, public Rele_c{
	double MinTemp;
	double MaxTemp;
	bool EnableControlTemp;
	bool EnableMinTemp;
public:
	Room_c(int room, int relePin, int wireInt, uint8_t* wireAdd);
	void Update();
	void SetControlTemp(bool state);
	void SetControlTemp(double min, double max);
	void SetControlTemp(double temp);
	bool GetControlTemp(){return EnableControlTemp;};
	double GetMaxTemp(){return MaxTemp;};
	double GetMinTemp(){return MinTemp;};
	void SetTimeOutCT(unsigned long i);
	unsigned long TimeOutCT;
	int RoomNumber;
	friend void UpdateNextOne();
};
void UpdataNextOne();
/*
 * Односвязный список. Используеться для хранения списка  указателей на интерфесы OneWire и DallasTemperature
 * Не хранит адрес датчика
 */
class ListOneWire_c {
public:
	ListOneWire_c* next_p;
	int Pin;
	ListOneWire_c();
	OneWire* OneWire_p;
	DallasTemperature* DallasTemperature_p;
	friend ListOneWire_c* GetOneWire(int pin); // фукция используеться как для добавления нового, так и для получения уже имеющегося интерфейса
};
ListOneWire_c* GetOneWire(int pin);
/*
 * Односвязный список. Используеться для хранения списка комнат, комнаты записываются в отсортерованом виде.
 * Сортировка по номеру комнаты, если номера комнаты совпадают то новая комната добавлятся после существующих
 * Удаляние комнаты по первому совпадениюЮ то есть удаляються более старые.
 */
class ListRoom_c{
public:
	ListRoom_c* next_p;
	static ListRoom_c FirstRoom;// используется как указатель на первый елемент
	ListRoom_c();
	Room_c* room_p;
	friend Room_c* getRoom(int i); //возвращает первую комнату с номером i, то есть, если номера дублируются то функция вернет самую старую.
	friend void addNewRoom(Room_c* room);
	friend void DeleteRoom(int room);
	friend void SaveListToEEPROM();
	friend void RestoryListFromEEPROM();
};
/*
 * Функции для работы со списком комнат
 */
Room_c* getRoom(int i);
void addNewRoom(Room_c* room);
void DeleteRoom(int room);
void RestoryListFromEEPROM();
void SaveListToEEPROM();
void ReadNetworkSettingsEEPROM(NetworkSettings *p);
void WriteNetworkSettingsEEPROM(NetworkSettings *p);
void GetUserName(char *user, char *pass);
void SaveUserName(char *user, char *pass);

#include <CLI.heater.h>
#endif
