// file:	FlyballETS-Software.ino summary: FlyballETS-Software v1, by Alex Goris
//
// Flyball ETS (Electronic Training System) is an open source project which is designed to help
// teams who practice flyball (a dog sport). Read all about the project, including extensive
// information on how to build your own copy of Flyball ETS, on the following link: https://
// sparkydevices.wordpress.com/tag/flyball-ets/
//
// This part of the project (FlyballETS-Software) contains the Arduino source code for the Arduino
// Pro Mini which controls all components in the Flyball ETS These sources are originally
// distributed from: https://github.com/vyruz1986/FlyballETS-Software.
//
// Copyright (C) 2019 Alex Goris
// This file is part of FlyballETS-Software
// FlyballETS-Software is free software : you can redistribute it and / or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program.If not,
// see <http://www.gnu.org/licenses/>
#include "Arduino.h"

//Includes
#include "Structs.h"
#include "config.h"

//Public libs
#include <LiquidCrystal.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <WiFiUdp.h>
//#include <time.h>


//Private libs
#include <GPSHandler.h>
#include <SettingsManager.h>
#include <WebHandler.h>
#include <LCDController.h>
#include <RaceHandler.h>
#include <LightsController.h>
#include <BatterySensor.h>

/*List of pins and the ones used (Lolin32 board):
   - 34: S1 (handler side) photoelectric sensor. ESP32 has no pull-down resistor on 34 pin, but pull-down anyway by 1kohm resistor on sensor board
   - 33: S2 (box side) photoelectric sensor | with jtag port used for LCD Data4

   - 27: LCD Data7
   - 14: LCD Data6                       with jtag board switched to 32; jtag uses 14 for MTMS
   - 26: LCD Data5
   - 13: LCD Data4                       with jtag board switched to 33; jtag uses 13 for MTCK
   -  2: LCD1 (line 1&2) enable pin
   - 15: LCD2 (line 3&4) enable pin      with jtag board switched to  5; jtag uses 15 for MTDO
   - 25: LCD RS Pin

   -  0: WS2811B lights data pin / Lights 74HC595 clock pin
   - xx: <free> / Lights 74HC595 data pin
   - xx: <free> / Lights 74HC595 latch pin

   - 19: remote D0
   - 23: remote D1
   - 18: remote D2
   - 17: remote D3
   - 16: remote D4
   -  4: remote D5

   - 35: battery sensor pin               swithed to S2 (box side)

   -  5: Side switch button               disabled with jtag board to act as 15 pin LCD2 (line 3&4)

   - 32: Laser trigger button             disabled with jtag board to act as 14 pin LCD Data6
   - 12: Laser output                     disabled with jtag board; jtag uses 12 for MTDI
   
   -  1: free/TX, but used with jtag board
   -  3: free/RX, but used with jtag
   -  0: free to avoid flash programming difficulties via usb/serial
   - 21: GPS PPS signal
   - 36/VP: GPS rx (ESP tx)
   - 39/VN: GPS tx (ESP rx)
*/

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
uint8_t iS1Pin = 34;
#if !JTAG
uint8_t iS2Pin = 33;
#else
uint8_t iS2Pin = 35;
#endif
uint8_t iCurrentDog;
uint8_t iCurrentRaceState;

char cDogTime[8];
char cDogCrossingTime[8];
char cElapsedRaceTime[8];
char cTeamNetTime[8];
long long llHeapPreviousMillis = 0;
long long llHeapInterval = 30000;

//Initialise Lights stuff
#ifdef WS281x
uint8_t iLightsDataPin = 22; // escaped from 0 to avoid issues with flash programming
NeoPixelBus<NeoRgbFeature, WS_METHOD> LightsStrip(5 * LIGHTSCHAINS, iLightsDataPin);

#else
uint8_t iLightsClockPin = 8;
uint8_t iLightsDataPin = 9;
uint8_t iLightsLatchPin = 21;
#endif // WS281x

#if !JTAG
//Battery variables
int iBatterySensorPin = 35;
uint16_t iBatteryVoltage = 0;

//Other IO's
uint8_t iLaserTriggerPin = 32;
uint8_t iLaserOutputPin = 12;
boolean bLaserState = false;
uint8_t iSideSwitchPin = 5;
stInputSignal SideSwitch = {iSideSwitchPin, 0, 500};
#endif

//Set last serial output variable
long long llLastSerialOutput = 0;

//remote control pins
int iRC0Pin = 19;
int iRC1Pin = 23;
int iRC2Pin = 18;
int iRC3Pin = 17;
int iRC4Pin = 16;
int iRC5Pin = 4;

//Array to hold last time button presses
long long llLastRCPress[6] = {0, 0, 0, 0, 0, 0};

#if !JTAG
uint8_t iLCDData4Pin = 13;
uint8_t iLCDData6Pin = 14;
uint8_t iLCDE2Pin = 15;
#else
uint8_t iLCDData4Pin = 33;
uint8_t iLCDData6Pin = 32;
uint8_t iLCDE2Pin = 5;
#endif
uint8_t iLCDData5Pin = 26;
uint8_t iLCDData7Pin = 27;
uint8_t iLCDE1Pin = 2;
uint8_t iLCDRSPin = 25;

LiquidCrystal lcd(iLCDRSPin, iLCDE1Pin, iLCDData4Pin, iLCDData5Pin, iLCDData6Pin, iLCDData7Pin);  //declare two LCD's, this will be line 1&2
LiquidCrystal lcd2(iLCDRSPin, iLCDE2Pin, iLCDData4Pin, iLCDData5Pin, iLCDData6Pin, iLCDData7Pin); //declare two LCD's, this will be line 1&2

//String for serial comms storage
String strSerialData;
uint iSimulatedRaceID = 0;
byte bySerialIndex = 0;
boolean bSerialStringComplete = false;

//Wifi stuff
//WiFiMulti wm;
IPAddress IPGateway(192, 168, 20, 1);
IPAddress IPNetwork(192, 168, 20, 0);
IPAddress IPSubnet(255, 255, 255, 0);

//Define serial pins for GPS module
HardwareSerial GPSSerial(1);

//Keep last reported OTA progress so we can send message for every % increment
unsigned int uiLastProgress = 0;

//Function prototypes
void Sensor1Wrapper();
void Sensor2Wrapper();
void WiFiEvent(WiFiEvent_t event);
void ResetRace();
void mdnsServerSetup();
void StartStopRace();
void StopRaceMain();
void StartRaceMain();
void serialEvent();

void setup()
{
   EEPROM.begin(EEPROM_SIZE);
   Serial.begin(115200);
   SettingsManager.init();

   pinMode(iS1Pin, INPUT_PULLDOWN);
   pinMode(iS2Pin, INPUT_PULLDOWN);

   //Set light data pin as output
   pinMode(iLightsDataPin, OUTPUT);

   //initialize pins for remote control
   pinMode(iRC0Pin, INPUT_PULLDOWN);
   pinMode(iRC1Pin, INPUT_PULLDOWN);
   pinMode(iRC2Pin, INPUT_PULLDOWN);
   pinMode(iRC3Pin, INPUT_PULLDOWN);
   pinMode(iRC4Pin, INPUT_PULLDOWN);
   pinMode(iRC5Pin, INPUT_PULLDOWN);

   //LCD pins as output
   pinMode(iLCDData4Pin, OUTPUT);
   pinMode(iLCDData5Pin, OUTPUT);
   pinMode(iLCDData6Pin, OUTPUT);
   pinMode(iLCDData7Pin, OUTPUT);
   pinMode(iLCDE1Pin, OUTPUT);
   pinMode(iLCDE2Pin, OUTPUT);
   pinMode(iLCDRSPin, OUTPUT);

   //Set ISR's with wrapper functions
#if !Simulate
   attachInterrupt(digitalPinToInterrupt(iS2Pin), Sensor2Wrapper, CHANGE);
   attachInterrupt(digitalPinToInterrupt(iS1Pin), Sensor1Wrapper, CHANGE);
#endif

#if !JTAG
   //Initialize other I/O's
   pinMode(iLaserTriggerPin, INPUT_PULLUP);
   pinMode(iLaserOutputPin, OUTPUT);
   pinMode(SideSwitch.Pin, INPUT_PULLUP);

   //Initialize BatterySensor class with correct pin
   BatterySensor.init(iBatterySensorPin);
#endif

   //Initialize LightsController class with shift register pins
#ifdef WS281x
   LightsController.init(&LightsStrip);
#else
   LightsController.init(iLightsLatchPin, iLightsClockPin, iLightsDataPin);
#endif

   //Initialize LCDController class with lcd1 and lcd2 objects
   LCDController.init(&lcd, &lcd2);

   strSerialData[0] = 0;

   //Setup AP
   WiFi.onEvent(WiFiEvent);
   WiFi.mode(WIFI_MODE_AP);
   //SettingsManager.setSetting("APName", "FlyballETS");
   //SettingsManager.setSetting("APPass", "FlyballETS.1234");
   String strAPName = SettingsManager.getSetting("APName");
   String strAPPass = SettingsManager.getSetting("APPass");

   if (!WiFi.softAP(strAPName.c_str(), strAPPass.c_str()))
   {
      ESP_LOGW(__FILE__, "Error initializing softAP!");
   }
   else
   {
      ESP_LOGI(__FILE__, "Wifi started successfully, AP name: %s, pass: %s!", strAPName.c_str(), strAPPass.c_str());
   }
   WiFi.softAPConfig(IPGateway, IPGateway, IPSubnet);

   //configure webserver
   WebHandler.init(80);

   //Initialize RaceHandler class with S1 and S2 pins
   RaceHandler.init(iS1Pin, iS2Pin);

   //Initialize simulatorclass pins if applicable
#if Simulate
   Simulator.init(iS1Pin, iS2Pin);
#endif

   //Ota setup
   ArduinoOTA.setPassword("FlyballETS.1234");
   ArduinoOTA.setPort(3232);
   ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == 0) //Instead of 0 it was U_FLASH
         type = "sketch";
      else // U_SPIFFS
         type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      ESP_LOGI(__FILE__, "Start updating %s", type);
   });

   ArduinoOTA.onEnd([]() {
      ESP_LOGI(__FILE__, "[OTA]: End");
   });
   ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      unsigned int progressPercentage = (progress / (total / 100));
      if (uiLastProgress != progressPercentage)
      {
         ESP_LOGI(__FILE__, "[OTA]: Progress: %u%%", progressPercentage);
         uiLastProgress = progressPercentage;
      }
   });
   ArduinoOTA.onError([](ota_error_t error) {
      ESP_LOGE(__FILE__, "");
      ESP_LOGE(__FILE__, "[OTA]: Error[%u]: ", error);
   });
   ArduinoOTA.begin();

   //Initialize GPS Serial port and class
   GPSSerial.begin(9600, SERIAL_8N1, 39, 36);
   GPSHandler.init(&GPSSerial);

#ifdef ESP32
   mdnsServerSetup();
#endif //  ESP32

#if !WiFiActivation
   WiFi.mode(WIFI_OFF);
   //btStop();
#endif
}

void loop()
{
   //Handle OTA update if incoming
   ArduinoOTA.handle();

   //Handle lights main processing
   LightsController.Main();

   //Check for serial events
   serialEvent();

 #if Simulate
   //Run simulator
   Simulator.Main();
#endif

  //Handle Race main processing
   RaceHandler.Main();

#if !JTAG
   //Handle battery sensor main processing
   BatterySensor.CheckBatteryVoltage();
#endif

   //Handle LCD processing
   LCDController.Main();

   //Handle WebSocket server
   WebHandler.loop();

   //Handle settings manager loop
   SettingsManager.loop();

   //Handle GPS
   GPSHandler.loop();

   //Race start/stop button (remote D0 output)
   if (digitalRead(iRC0Pin) == HIGH && (GET_MICROS / 1000 - llLastRCPress[0]) > 2000)
   {
      StartStopRace();
   }

   //Race start (serial command only)
   if (bSerialStringComplete && strSerialData == "START" && (RaceHandler.RaceState == RaceHandler.RESET))
   {
      StartRaceMain();
   }

   //Race stop (serial command only)
   if (bSerialStringComplete && strSerialData == "STOP" && ((RaceHandler.RaceState == RaceHandler.STARTING) || (RaceHandler.RaceState == RaceHandler.RUNNING)))
   {
      StopRaceMain();
   }

   //Race reset button (remote D1 output)
   if ((digitalRead(iRC1Pin) == HIGH && (GET_MICROS / 1000 - llLastRCPress[1] > 2000)) || (bSerialStringComplete && strSerialData == "RESET"))
   {
      ResetRace();
   }

#if Simulate
   //Change Race ID (only serial command), e.g. RACE 1 or RACE 2
   if (bSerialStringComplete && strSerialData.startsWith("RACE"))
   {
      strSerialData.remove(0, 5);
      iSimulatedRaceID = strSerialData.toInt();
      if (iSimulatedRaceID < 0 || iSimulatedRaceID >= NumSimulatedRaces)
      {
         iSimulatedRaceID = 0;
      }
      Simulator.ChangeSimulatedRaceID(iSimulatedRaceID);
   }
#endif

   //Dog0 fault RC button
   if ((digitalRead(iRC2Pin) == HIGH && (GET_MICROS / 1000 - llLastRCPress[2] > 2000)) || (bSerialStringComplete && strSerialData == "D0F"))
   {
      llLastRCPress[2] = GET_MICROS / 1000;
      //Toggle fault for dog
      RaceHandler.SetDogFault(0);
   }

   //Dog1 fault RC button
   if ((digitalRead(iRC3Pin) == HIGH && (GET_MICROS / 1000 - llLastRCPress[3] > 2000)) || (bSerialStringComplete && strSerialData == "D1F"))
   {
      llLastRCPress[3] = GET_MICROS / 1000;
      //Toggle fault for dog
      RaceHandler.SetDogFault(1);
   }
   //Dog2 fault RC button
   if ((digitalRead(iRC4Pin) == HIGH && (GET_MICROS / 1000 - llLastRCPress[4] > 2000)) || (bSerialStringComplete && strSerialData == "D2F"))
   {
      llLastRCPress[4] = GET_MICROS / 1000;
      //Toggle fault for dog
      RaceHandler.SetDogFault(2);
   }

   //Dog3 fault RC button
   if ((digitalRead(iRC5Pin) == HIGH && (GET_MICROS / 1000 - llLastRCPress[5] > 2000)) || (bSerialStringComplete && strSerialData == "D3F"))
   {
      llLastRCPress[5] = GET_MICROS / 1000;
      //Toggle fault for dog
      RaceHandler.SetDogFault(3);
   }

   //Update LCD Display fields
   //Update team time to display
   dtostrf(RaceHandler.GetRaceTime(), 7, 3, cElapsedRaceTime);
   LCDController.UpdateField(LCDController.TeamTime, cElapsedRaceTime);

#if !JTAG
   //Update battery percentage to display
   iBatteryVoltage = BatterySensor.GetBatteryVoltage();
   uint16_t iBatteryPercentage = BatterySensor.GetBatteryPercentage();
   String sBatteryPercentage;
   if (iBatteryPercentage == 0)
   {
      sBatteryPercentage = "LOW";
   }
   else
   {
      sBatteryPercentage = String(iBatteryPercentage);
   }
   while (sBatteryPercentage.length() < 3)
   {
      sBatteryPercentage = " " + sBatteryPercentage;
   }
   LCDController.UpdateField(LCDController.BattLevel, sBatteryPercentage);
#endif

   //Update team netto time
   dtostrf(RaceHandler.GetNetTime(), 7, 3, cTeamNetTime);
   LCDController.UpdateField(LCDController.NetTime, cTeamNetTime);

   //Update race status to display
   LCDController.UpdateField(LCDController.RaceState, RaceHandler.GetRaceStateString());

   //Handle individual dog info
   dtostrf(RaceHandler.GetDogTime(0), 7, 3, cDogTime);
   LCDController.UpdateField(LCDController.D1Time, cDogTime);
   LCDController.UpdateField(LCDController.D1CrossTime, RaceHandler.GetCrossingTime(0));
   LCDController.UpdateField(LCDController.D1RerunInfo, RaceHandler.GetRerunInfo(0));

   dtostrf(RaceHandler.GetDogTime(1), 7, 3, cDogTime);
   LCDController.UpdateField(LCDController.D2Time, cDogTime);
   LCDController.UpdateField(LCDController.D2CrossTime, RaceHandler.GetCrossingTime(1));
   LCDController.UpdateField(LCDController.D2RerunInfo, RaceHandler.GetRerunInfo(1));

   dtostrf(RaceHandler.GetDogTime(2), 7, 3, cDogTime);
   LCDController.UpdateField(LCDController.D3Time, cDogTime);
   LCDController.UpdateField(LCDController.D3CrossTime, RaceHandler.GetCrossingTime(2));
   LCDController.UpdateField(LCDController.D3RerunInfo, RaceHandler.GetRerunInfo(2));

   dtostrf(RaceHandler.GetDogTime(3), 7, 3, cDogTime);
   LCDController.UpdateField(LCDController.D4Time, cDogTime);
   LCDController.UpdateField(LCDController.D4CrossTime, RaceHandler.GetCrossingTime(3));
   LCDController.UpdateField(LCDController.D4RerunInfo, RaceHandler.GetRerunInfo(3));

   if (iCurrentRaceState != RaceHandler.RaceState)
   {
      if (RaceHandler.RaceState == RaceHandler.STOPPED)
      {
         //Race is finished, put final data on screen
         dtostrf(RaceHandler.GetDogTime(RaceHandler.iCurrentDog, -2), 7, 3, cDogTime);
         ESP_LOGI(__FILE__, "Dog %i: %s|CR: %s", RaceHandler.iCurrentDog+1, cDogTime, RaceHandler.GetCrossingTime(RaceHandler.iCurrentDog, -2).c_str());
         ESP_LOGI(__FILE__, "Team:%s", cElapsedRaceTime);
         ESP_LOGI(__FILE__, "Net:%s", cTeamNetTime);
      }
      ESP_LOGI(__FILE__, "RS: %s", RaceHandler.GetRaceStateString());
   }

   //heap memory monitor
   /*long long llCurrentMillis = GET_MICROS / 1000;
   if (llCurrentMillis - llHeapPreviousMillis > llHeapInterval)
   {
      ESP_LOGI(__FILE__, "Elapsed system time: %llu. Heap caps free size: %i\n", GET_MICROS / 1000, heap_caps_get_free_size(MALLOC_CAP_8BIT));
      llHeapPreviousMillis = llCurrentMillis;
   }
   */
   if (RaceHandler.iCurrentDog != iCurrentDog)
   {
      dtostrf(RaceHandler.GetDogTime(RaceHandler.iPreviousDog, -2), 7, 3, cDogTime);

      ESP_LOGI(__FILE__, "Dog %i: %s|CR: %s", RaceHandler.iPreviousDog+1, cDogTime, RaceHandler.GetCrossingTime(RaceHandler.iPreviousDog, -2).c_str());
      ESP_LOGI(__FILE__, "Next dog: %i", RaceHandler.iCurrentDog+1);
      ESP_LOGI(__FILE__, "RT:%s", cElapsedRaceTime);
   }

   //Enable (uncomment) the following if you want periodic status updates on the serial port
   if ((GET_MICROS / 1000 - llLastSerialOutput) > 60000)
   {
      ESP_LOGI(__FILE__, "Battery: analog: %i ,voltage: %i, level: %i%%", BatterySensor.GetLastAnalogRead(), iBatteryVoltage, iBatteryPercentage);
      //ESP_LOGI(__FILE__, "%llu: Elapsed time: %s", GET_MICROS / 1000, cElapsedRaceTime);
      //ESP_LOGI(__FILE__, "Free heap: %i", system_get_free_heap_size());
      /*
      if (RaceHandler.RaceState == RaceHandler.RUNNING)Ś
      {
         dtostrf(RaceHandler.GetDogTime(RaceHandler.iCurrentDog), 7, 3, cDogTime);
         ESP_LOGI(__FILE__, "Dog %i: %ss", RaceHandler.iCurrentDog, cDogTime);
      }
      */
      llLastSerialOutput = GET_MICROS / 1000;
   }
   //Cleanup variables used for checking if something changed
   iCurrentDog = RaceHandler.iCurrentDog;
   iCurrentRaceState = RaceHandler.RaceState;

   //Check if we have serial data which we should handle
   if (strSerialData.length() > 0 && bSerialStringComplete)
   {
      strSerialData = "";
      bSerialStringComplete = false;
   }

#if !JTAG
   //Handle laser output
   digitalWrite(iLaserOutputPin, !digitalRead(iLaserTriggerPin));

   //Handle side switch button
   if (digitalRead(SideSwitch.Pin) == LOW && GET_MICROS / 1000 - SideSwitch.LastTriggerTime > SideSwitch.CoolDownTime)
   {
      SideSwitch.LastTriggerTime = GET_MICROS / 1000;
      ESP_LOGI(__FILE__, "Switching sides!");
      RaceHandler.ToggleRunDirection();
   }
#endif
}

void serialEvent()
{
   //Listen on serial port

   while (Serial.available() > 0)
   {
      char cInChar = Serial.read(); // Read a character
                                    //Check if buffer contains complete serial message, terminated by newline (\n)
      if (cInChar == '\n')
      {
         //Serial message in buffer is complete, null terminate it and store it for further handling
         bSerialStringComplete = true;
         ESP_LOGD(__FILE__, "SERIAL received: '%s'", strSerialData.c_str());
         strSerialData += '\0'; // Null terminate the string
         break;
      }
      strSerialData += cInChar; // Store it
   }
}

/// <summary>
///   These are wrapper functions which are necessary because it's not allowed to use a class member function directly as an ISR
/// </summary>
void Sensor2Wrapper()
{
   RaceHandler.TriggerSensor2();
}

/// <summary>
///   These are wrapper functions which are necessary because it's not allowed to use a class member function directly as an ISR
/// </summary>
void Sensor1Wrapper()
{
   RaceHandler.TriggerSensor1();
}

/// <summary>
///   Start a race.
/// </summary>
void StartRaceMain()
{
   ESP_LOGI(__FILE__, "%s", GPSHandler.GetUTCTimestamp());
   ESP_LOGD(__FILE__, "%llu: START!", GET_MICROS / 1000);
   LightsController.InitiateStartSequence();
   RaceHandler.StartRace();
}

/// <summary>
///   Stop a race.
/// </summary>
void StopRaceMain()
{
   RaceHandler.StopRace();
   LightsController.DeleteSchedules();
}

/// <summary>
///   Starts (if stopped) or stops (if started) a race. Start is only allowed if race is stopped and reset.
/// </summary>
void StartStopRace()
{
   llLastRCPress[0] = GET_MICROS / 1000;
   if (RaceHandler.RaceState == RaceHandler.RESET) //If race is reset
   {
      //Then start the race
      StartRaceMain();
   }
   else //If race state is running or starting, we should stop it
   {
      StopRaceMain();
   }
}

/// <summary>
///   Reset race so new one can be started, reset is only allowed when race is stopped
/// </summary>
void ResetRace()
{
   if (RaceHandler.RaceState != RaceHandler.STOPPED) //Only allow reset when race is stopped first
   {
      return;
   }
   llLastRCPress[1] = GET_MICROS / 1000;
   LightsController.ResetLights();
   RaceHandler.ResetRace();
   iCurrentDog = RaceHandler.iCurrentDog;
   iCurrentRaceState = RaceHandler.RaceState;
}

void WiFiEvent(WiFiEvent_t event)
{
   Serial.printf("Wifi event %i\r\n", event);
   switch (event)
   {
   case SYSTEM_EVENT_AP_START:
      ESP_LOGI(__FILE__, "AP Started");
      WiFi.softAPConfig(IPGateway, IPGateway, IPSubnet);

      if (WiFi.softAPIP() != IPGateway)
      {
         ESP_LOGE(__FILE__, "I am not running on the correct IP (%s instead of %s), rebooting!", WiFi.softAPIP().toString().c_str(), IPGateway.toString().c_str());
         ESP.restart();
      }

      ESP_LOGI(__FILE__, "Ready on IP: %s, v%s", WiFi.softAPIP().toString().c_str(), APP_VER);
      break;
   case SYSTEM_EVENT_AP_STOP:
      ESP_LOGI(__FILE__, "AP Stopped");
      break;

   default:
      break;
   }
}

#ifdef ESP32
void mdnsServerSetup()
{
   MDNS.addService("http", "tcp", 80);
   MDNS.addServiceTxt("arduino", "tcp", "app_version", APP_VER);
   MDNS.begin("FlyballETS");
}
#endif