#include "Arduino.h"

//Public libs
#include <LiquidCrystal.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include "WiFiUdp.h"
#include "WiFiClientSecure.h"

//Private libs
#include <GPSHandler.h>
#include <SettingsManager.h>
#include <WebHandler.h>
#include <RaceHandler.h>
#include <LCDController.h>
#include <LightsController.h>
#include <BatterySensor.h>
#include <SystemManager.h>
#include <SlaveHandler.h>

//Includes
#include <Structs.h>
#include <config.h>

//Set simulate to true to enable simulator class (see Simulator.cpp/h)
#if Simulate
#include "Simulator.h"
#endif

#ifdef WS281x
//#include <Adafruit_NeoPixel.h>
#include <NeoPixelBus.h>
#endif // WS281x

#ifdef ESP32
#include <ESPmDNS.h>
#endif

//Function prototypes
void Sensor1Wrapper();
void Sensor2Wrapper();
void ResetRace();
void mdnsServerSetup();
void StartStopRace();
void serialEvent();
void Core0Loop(void *parameter);
void HandleSerialMessages();
void HandleRemoteControl();
void HandleLCDUpdates();