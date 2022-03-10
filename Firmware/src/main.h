#include "Arduino.h"

//Includes
#include <Structs.h>
#include <config.h>

//Public libs
#include <LiquidCrystal.h>
#ifdef WiFiON
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>
#endif
#include <EEPROM.h>
#include <NeoPixelBus.h>
#include <ESPmDNS.h>
//#include <time.h>

//Private libs
#include "GPSHandler.h"
#include "SettingsManager.h"
#ifdef WiFiON
#include "WebHandler.h"
#endif
#include "LCDController.h"
#include "RaceHandler.h"
#include "LightsController.h"
#include "BatterySensor.h"
#include "SDcardController.h"
//#include "SystemManager.h"
//#include "SlaveHandler.h"
//#include "WifiManager.h"

//Set simulate to true to enable simulator class (see Simulator.cpp/h)
#if Simulate
#include "Simulator.h"
#endif

//Function prototypes
void Sensor1Wrapper();
void Sensor2Wrapper();
void ResetRace();
void StartStopRace();
void StartRaceMain();
void StopRaceMain();
void mdnsServerSetup();
void serialEvent();
//void Core0Loop(void *parameter);
void HandleSerialCommands();
void HandleRemoteAndButtons();
void HandleLCDUpdates();
String GetButtonString(uint8_t _iActiveBit);
#ifdef WiFiON
void WiFiEvent(WiFiEvent_t event);
#endif