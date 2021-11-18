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
#ifdef WiFiON
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <WiFiUdp.h>
#endif
#include <EEPROM.h>
#include <FS.h>
#include <SD_MMC.h>
#include <NeoPixelBus.h>
#include <ESPmDNS.h>
//#include <time.h>

//Private libs
#include <GPSHandler.h>
#include <SettingsManager.h>
#ifdef WiFiON
#include <WebHandler.h>
#endif
#include <LCDController.h>
#include <RaceHandler.h>
#include <LightsController.h>
#include <BatterySensor.h>

/*List of pins and the ones used (Lolin32 board):
   - 34: S1 (handler side) photoelectric sensor. ESP32 has no pull-down resistor on 34 pin, but pull-down anyway by 1kohm resistor on sensor board
   - 33: S2 (box side) photoelectric sensor

   - 27: LCD Data7
   - 32: LCD Data6
   - 26: LCD Data5
   - 13: LCD Data4
   - 16: LCD1 (line 1&2) enable pin
   - 17: LCD2 (line 3&4) enable pin
   - 25: LCD RS Pin

   - 22: WS2811B lights data pin

   - 35: battery sensor pin

   - 12: Laser output
   
   -  5: set high to turn off build in led
   -  1: free/TX
   -  3: free/RX
   
   - 21:    GPS PPS signal
   - 36/VP: GPS rx (ESP tx)
   - 39/VN: GPS tx (ESP rx)

   -  2: SD card Data0
   -  4: SD card Data1
   - 14: SD card Clock
   - 15: SD card Command

   - 19: dataOutPin (Q7) for 74HC166
   - 23: latchPin (CE)   for 74HC166
   - 18: clockPin (CP)   for 74HC166

74HC166 pinouts
   - D0: Side switch button
   - D1: remote D0 start/stop
   - D2: remote D1 reset
   - D3: remote D2 dog 1 fault
   - D4: remote D5 dog 4 fault
   - D5: remote D4 dog 3 fault
   - D6: remote D3 dog 2 fault
   - D7: Laser trigger button
*/

//Set simulate to true to enable simulator class (see Simulator.cpp/h)
#if Simulate
#include "Simulator.h"
#endif

uint8_t iS1Pin = 34;
uint8_t iS2Pin = 33;
uint8_t iCurrentDog;
uint8_t iCurrentRaceState;

char cDogCrossingTime[8];
char cElapsedRaceTime[8];
char cTeamNetTime[8];
long long llHeapPreviousMillis = 0;
long long llHeapInterval = 5000;
bool error = false;

//Initialise Lights stuff
uint8_t iLightsDataPin = 22;
NeoPixelBus<NeoRgbFeature, WS_METHOD> LightsStrip(5 * LIGHTSCHAINS, iLightsDataPin);

//Battery variables
int iBatterySensorPin = 35;
uint16_t iBatteryVoltage = 0;

//Other IO's
uint8_t iLaserOutputPin = 12;
boolean bLaserActive = false;
uint16_t SideSwitchCoolDownTime = 500;

//Set last serial output variable
unsigned long lLastSerialOutput = 0;

//Battery % update on LCD timer variable
long long llLastBatteryLCDupdate = -25000;

//control pins for 74HC166 (remote + side switch)
uint8_t iLatchPin = 23;
uint8_t iClockPin = 18;
uint8_t iDataInPin = 19;
byte bDataIn = 0;

//control pins for SD card
uint8_t iSDdata0Pin = 2;
uint8_t iSDdata1Pin = 4;
uint8_t iSDclockPin = 14;
uint8_t iSDcmdPin = 15;

//GPS module pins
uint8_t iGPStxPin = 36;
uint8_t iGPSrxPin = 39;
uint8_t iGPSppsPin = 21;

//Array to hold last time button presses
unsigned long long llLastRCPress[8] = {0, 0, 0, 0, 0, 0, 0, 0};

//control pins for LCD
uint8_t iLCDE1Pin = 16;
uint8_t iLCDE2Pin = 17;
uint8_t iLCDData4Pin = 13;
uint8_t iLCDData5Pin = 26;
uint8_t iLCDData6Pin = 32;
uint8_t iLCDData7Pin = 27;
uint8_t iLCDRSPin = 25;

LiquidCrystal lcd(iLCDRSPin, iLCDE1Pin, iLCDData4Pin, iLCDData5Pin, iLCDData6Pin, iLCDData7Pin);  //declare two LCD's, this will be line 1&2
LiquidCrystal lcd2(iLCDRSPin, iLCDE2Pin, iLCDData4Pin, iLCDData5Pin, iLCDData6Pin, iLCDData7Pin); //declare two LCD's, this will be line 1&2

//String for serial comms storage
String strSerialData;
uint iSimulatedRaceID = 0;
byte bySerialIndex = 0;
boolean bSerialStringComplete = false;
boolean bRaceSummaryPrinted = false;

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
#ifdef WiFiON
void WiFiEvent(WiFiEvent_t event);
#endif
void ResetRace();
void mdnsServerSetup();
void StartStopRace();
void StopRaceMain();
void StartRaceMain();
void serialEvent();
void ButtonsRead();
String GetButtonString();

void setup()
{
   EEPROM.begin(EEPROM_SIZE);
   Serial.begin(115200);
   SettingsManager.init();

   pinMode(5, OUTPUT);

   pinMode(iS1Pin, INPUT_PULLDOWN);
   pinMode(iS2Pin, INPUT_PULLDOWN);

   //Set light data pin as output
   pinMode(iLightsDataPin, OUTPUT);

   //initialize pins for 74HC166
   pinMode(iLatchPin, OUTPUT);
   pinMode(iClockPin, OUTPUT);
   pinMode(iDataInPin, INPUT_PULLDOWN);

   //initialize pins for SD Card
   pinMode(iSDdata0Pin, INPUT_PULLUP);
   pinMode(iSDdata1Pin, INPUT_PULLUP);
   pinMode(iSDcmdPin, INPUT_PULLUP);

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

   //Initialize other I/O's
   pinMode(iLaserOutputPin, OUTPUT);

   //Turn off build-in ESP32 Loli32 LED
   digitalWrite(5, HIGH);

   //Initialize BatterySensor class with correct pin
   BatterySensor.init(iBatterySensorPin);

   //Initialize LightsController class with shift register pins
   LightsController.init(&LightsStrip);

   //Initialize LCDController class with lcd1 and lcd2 objects
   LCDController.init(&lcd, &lcd2);

   strSerialData[0] = 0;

   //SD card init
 #ifdef SDcard
   if (!SD_MMC.begin("/sdcard", true))
   {
      Serial.println("Card Mount Failed");
      return;
   }
   else
   {
      uint8_t cardType = SD_MMC.cardType();

      if (cardType == CARD_NONE)
      {
         Serial.println("No SD_MMC card attached");
         return;
      }
      Serial.print("SD_MMC Card Type: ");
      if (cardType == CARD_MMC)
      {
         Serial.println("MMC");
      }
      else if (cardType == CARD_SD)
      {
         Serial.println("SDSC");
      }
      else if (cardType == CARD_SDHC)
      {
         Serial.println("SDHC");
      }
      else
      {
         Serial.println("UNKNOWN");
      }
      uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
      Serial.printf("SD_MMC Card Size: %lluMB\n", cardSize);
   }
#endif

#ifdef WiFiON
   //Setup AP
   WiFi.onEvent(WiFiEvent);
   WiFi.mode(WIFI_MODE_AP);
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
#endif

   //Initialize RaceHandler class with S1 and S2 pins
   RaceHandler.init(iS1Pin, iS2Pin);

   //Initialize simulatorclass pins if applicable
#if Simulate
   Simulator.init(iS1Pin, iS2Pin);
#endif

#ifdef WiFiON
   //Ota setup
   ArduinoOTA.setPassword("FlyballETS.1234");
   ArduinoOTA.setPort(3232);
   ArduinoOTA.onStart([]()
   {
      String type;
      if (ArduinoOTA.getCommand() == 0) //VSCode constantly can't read properly value of U_FLASH, therefore replacing with "0"
         type = "sketch";
      else // U_SPIFFS
         type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      ESP_LOGI(__FILE__, "Start updating %s", type);
   });

   ArduinoOTA.onEnd([]()
   {
      ESP_LOGI(__FILE__, "[OTA]: End");
   });
   ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
   {
      unsigned int progressPercentage = (progress / (total / 100));
      if (uiLastProgress != progressPercentage)
      {
         ESP_LOGI(__FILE__, "[OTA]: Progress: %u%%", progressPercentage);
         uiLastProgress = progressPercentage;
      }
   });
   ArduinoOTA.onError([](ota_error_t error)
   {
      ESP_LOGE(__FILE__, "");
      ESP_LOGE(__FILE__, "[OTA]: Error[%u]: ", error);
   });
   ArduinoOTA.begin();

#ifdef ESP32
   mdnsServerSetup();
#endif //  ESP32
#endif

   //Initialize GPS Serial port and class
   GPSSerial.begin(9600, SERIAL_8N1, iGPSrxPin, iGPStxPin);
   GPSHandler.init(&GPSSerial);

   ESP_LOGI(__FILE__, "Setup running on core %d", xPortGetCoreID());
}

void loop()
{
   //Exclude handling of those services in loop while race is running
   if (RaceHandler.RaceState == RaceHandler.STOPPED || RaceHandler.RaceState == RaceHandler.RESET)
   {
      //Handle settings manager loop
      SettingsManager.loop();

#ifdef WiFiON
      //Handle OTA update if incoming
      ArduinoOTA.handle();
#endif

      //Handle GPS
      GPSHandler.loop();

      //Handle battery sensor main processing
      BatterySensor.CheckBatteryVoltage();
   }

   //Check for serial events
   serialEvent();

   //Check Remote and Side switch buttons state
   ButtonsRead();

   //Handle lights main processing
   LightsController.Main();

   //Handle Race main processing
   RaceHandler.Main();

#if Simulate
   //Run simulator
   Simulator.Main();
#endif

   //Handle LCD processing
   LCDController.Main();

#ifdef WiFiON
   //Handle WebSocket server
   WebHandler.loop();
#endif

   //Reset variables when state RESET
   if (RaceHandler.RaceState == RaceHandler.RESET)
   {
      iCurrentDog = RaceHandler.iCurrentDog;
      iCurrentRaceState = RaceHandler.RaceState;
      bRaceSummaryPrinted = false;
   }

   //Race start/stop button (remote D0 output)
   if (bitRead(bDataIn, 1) == HIGH && (GET_MICROS / 1000 - llLastRCPress[1]) > 2000)
   {
      StartStopRace();
   }

   //Race start (serial command only)
   if (bSerialStringComplete && strSerialData == "start" && (RaceHandler.RaceState == RaceHandler.RESET))
   {
      StartRaceMain();
   }

   //Race stop (serial command only)
   if (bSerialStringComplete && strSerialData == "stop" && ((RaceHandler.RaceState == RaceHandler.STARTING) || (RaceHandler.RaceState == RaceHandler.RUNNING)))
   {
      StopRaceMain();
   }

   //Race reset button (remote D1 output)
   if ((bitRead(bDataIn, 2) == HIGH && (GET_MICROS / 1000 - llLastRCPress[2] > 2000)) || (bSerialStringComplete && strSerialData == "reset"))
   {
      ResetRace();
   }

   //Reboot ESP32
   if (bSerialStringComplete && strSerialData == "reboot")
   {
      ESP.restart();
   }

#if Simulate
   //Change Race ID (only serial command), e.g. race 1 or race 2
   if (bSerialStringComplete && strSerialData.startsWith("race"))
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

   //Dog 1 fault RC button
   if ((bitRead(bDataIn, 3) == HIGH && (GET_MICROS / 1000 - llLastRCPress[3] > 2000)) || (bSerialStringComplete && strSerialData == "d1f"))
   {
      llLastRCPress[3] = GET_MICROS / 1000;
      //Toggle fault for dog
      RaceHandler.SetDogFault(0);
   }

   //Dog 2 fault RC button
   if ((bitRead(bDataIn, 6) == HIGH && (GET_MICROS / 1000 - llLastRCPress[6] > 2000)) || (bSerialStringComplete && strSerialData == "d2f"))
   {
      llLastRCPress[6] = GET_MICROS / 1000;
      //Toggle fault for dog
      RaceHandler.SetDogFault(1);
   }
   //Dog 3 fault RC button
   if ((bitRead(bDataIn, 5) == HIGH && (GET_MICROS / 1000 - llLastRCPress[5] > 2000)) || (bSerialStringComplete && strSerialData == "d3f"))
   {
      llLastRCPress[5] = GET_MICROS / 1000;
      //Toggle fault for dog
      RaceHandler.SetDogFault(2);
   }

   //Dog 4 fault RC button
   if ((bitRead(bDataIn, 4) == HIGH && (GET_MICROS / 1000 - llLastRCPress[4] > 2000)) || (bSerialStringComplete && strSerialData == "d4f"))
   {
      llLastRCPress[4] = GET_MICROS / 1000;
      //Toggle fault for dog
      RaceHandler.SetDogFault(3);
   }

//Update LCD Display fields
//Update team time to display
#if Accuracy2digits
   {
      dtostrf(RaceHandler.GetRaceTime(), 6, 2, cElapsedRaceTime);
   }
#else
   {
      dtostrf(RaceHandler.GetRaceTime(), 7, 3, cElapsedRaceTime);
   }
#endif
   LCDController.UpdateField(LCDController.TeamTime, cElapsedRaceTime);

   //Update battery percentage to display
   if ((GET_MICROS / 1000 < 2000 || ((GET_MICROS / 1000 - llLastBatteryLCDupdate) > 30000)) //
      && (RaceHandler.RaceState == RaceHandler.STOPPED || RaceHandler.RaceState == RaceHandler.RESET))
   {
      iBatteryVoltage = BatterySensor.GetBatteryVoltage();
      uint16_t iBatteryPercentage = BatterySensor.GetBatteryPercentage();
      String sBatteryPercentage;
      if (iBatteryPercentage == 9999)
      {
         sBatteryPercentage = "!!!";
         LCDController.UpdateField(LCDController.BattLevel, sBatteryPercentage);
         LightsController.ResetLights();
         delay(3000);
         esp_deep_sleep_start();
      }
      else if (iBatteryPercentage == 9911)
      {
         sBatteryPercentage = "USB";
      }
      else if (iBatteryPercentage == 0)
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
      //ESP_LOGD(__FILE__, "Battery: analog: %i ,voltage: %i, level: %i%%", BatterySensor.GetLastAnalogRead(), iBatteryVoltage, iBatteryPercentage);
      llLastBatteryLCDupdate = GET_MICROS / 1000;
   }

//Update team netto time
#if Accuracy2digits
   {
      dtostrf(RaceHandler.GetNetTime(), 6, 2, cTeamNetTime);
   }
#else
   {
      dtostrf(RaceHandler.GetNetTime(), 7, 3, cTeamNetTime);
   }
#endif
   LCDController.UpdateField(LCDController.NetTime, cTeamNetTime);

   //Update race status to display
   LCDController.UpdateField(LCDController.RaceState, RaceHandler.GetRaceStateString());

   //Handle individual dog info
   LCDController.UpdateField(LCDController.D1Time, RaceHandler.GetDogTime(0));
   LCDController.UpdateField(LCDController.D1CrossTime, RaceHandler.GetCrossingTime(0));
   LCDController.UpdateField(LCDController.D1RerunInfo, RaceHandler.GetRerunInfo(0));

   LCDController.UpdateField(LCDController.D2Time, RaceHandler.GetDogTime(1));
   LCDController.UpdateField(LCDController.D2CrossTime, RaceHandler.GetCrossingTime(1));
   LCDController.UpdateField(LCDController.D2RerunInfo, RaceHandler.GetRerunInfo(1));

   LCDController.UpdateField(LCDController.D3Time, RaceHandler.GetDogTime(2));
   LCDController.UpdateField(LCDController.D3CrossTime, RaceHandler.GetCrossingTime(2));
   LCDController.UpdateField(LCDController.D3RerunInfo, RaceHandler.GetRerunInfo(2));

   LCDController.UpdateField(LCDController.D4Time, RaceHandler.GetDogTime(3));
   LCDController.UpdateField(LCDController.D4CrossTime, RaceHandler.GetCrossingTime(3));
   LCDController.UpdateField(LCDController.D4RerunInfo, RaceHandler.GetRerunInfo(3));

   if (iCurrentRaceState != RaceHandler.RaceState)
   {
      ESP_LOGI(__FILE__, "RS: %s", RaceHandler.GetRaceStateString());
   }

   if (RaceHandler.RaceState == RaceHandler.STOPPED && ((GET_MICROS / 1000 - (RaceHandler.llRaceStartTime / 1000 + RaceHandler.GetRaceTime() * 1000)) > 1500) && !bRaceSummaryPrinted)
   {
      //Race has been stopped 1 second ago: print race summary to console
      for (uint8_t i = 0; i < 4; i++)
      {
         //ESP_LOGD(__FILE__, "Dog %i -> %i run(s).", i + 1, RaceHandler.iDogRunCounters[i] + 1);
         for (uint8_t i2 = 0; i2 < (RaceHandler.iDogRunCounters[i] + 1); i2++)
         {
            ESP_LOGI(__FILE__, "Dog %i: %s | CR: %s", i + 1, RaceHandler.GetStoredDogTimes(i, i2), RaceHandler.TransformCrossingTime(i, i2).c_str());
         }
      }
      ESP_LOGI(__FILE__, " Team: %s", cElapsedRaceTime);
      ESP_LOGI(__FILE__, "  Net: %s\n", cTeamNetTime);
#if !Simulate
      RaceHandler.PrintRaceTriggerRecords();
#endif
      bRaceSummaryPrinted = true;
   }

   /*//heap memory monitor
   long long llCurrentMillis = GET_MICROS / 1000;
   if (llCurrentMillis - llHeapPreviousMillis > llHeapInterval)
   {
      ESP_LOGI(__FILE__, "Elapsed system time: %llu. Heap caps free size: %i", GET_MICROS / 1000, heap_caps_get_free_size(MALLOC_CAP_8BIT));
      ESP_LOGI(__FILE__, "Heap integrity OK? %i", heap_caps_check_integrity_all(error));
      llHeapPreviousMillis = llCurrentMillis;
   }
   */
   if (RaceHandler.iCurrentDog != iCurrentDog && RaceHandler.RaceState == RaceHandler.RUNNING)
   {
      ESP_LOGI(__FILE__, "Dog %i: %s | CR: %s", RaceHandler.iPreviousDog + 1, RaceHandler.GetDogTime(RaceHandler.iPreviousDog, -2), RaceHandler.GetCrossingTime(RaceHandler.iPreviousDog, -2).c_str());
      ESP_LOGI(__FILE__, "Running dog: %i.", RaceHandler.iCurrentDog + 1);
   }

   //Enable (uncomment) the following if you want periodic status updates on the serial port
   /*
   if ((GET_MICROS / 1000 - lLastSerialOutput) > 60000)
   {
      //ESP_LOGI(__FILE__, "%llu: Elapsed time: %s", GET_MICROS / 1000, cElapsedRaceTime);
      ESP_LOGI(__FILE__, "Free heap: %i", esp_get_free_heap_size());
      lLastSerialOutput = GET_MICROS / 1000;
   }*/

   //Cleanup variables used for checking if something changed
   iCurrentDog = RaceHandler.iCurrentDog;
   iCurrentRaceState = RaceHandler.RaceState;

   //Check if we have serial data which we should handle
   if (strSerialData.length() > 0 && bSerialStringComplete)
   {
      strSerialData = "";
      bSerialStringComplete = false;
   }

   //Laser activation
   if (bitRead(bDataIn, 7) == HIGH && ((GET_MICROS / 1000 - llLastRCPress[7] > LaserOutputTimer * 1000) || llLastRCPress[7] == 0) //
      && (RaceHandler.RaceState == RaceHandler.STOPPED || RaceHandler.RaceState == RaceHandler.RESET))
   {
      llLastRCPress[7] = GET_MICROS / 1000;
      digitalWrite(iLaserOutputPin, HIGH);
      bLaserActive = true;
      ESP_LOGI(__FILE__, "Turn Laser ON.");
   }
   //Laser deativation
   if ((bLaserActive) && ((GET_MICROS / 1000 - llLastRCPress[7] > LaserOutputTimer * 1000) || RaceHandler.RaceState == RaceHandler.STARTING || RaceHandler.RaceState == RaceHandler.RUNNING))
   {
      digitalWrite(iLaserOutputPin, LOW);
      bLaserActive = false;
      ESP_LOGI(__FILE__, "Turn Laser OFF.");
   }

   //Handle side switch button (when race is not running)
   if (bitRead(bDataIn, 0) == HIGH && (GET_MICROS / 1000 - llLastRCPress[0] > SideSwitchCoolDownTime) //
      && (RaceHandler.RaceState == RaceHandler.STOPPED || RaceHandler.RaceState == RaceHandler.RESET))
   {
      llLastRCPress[0] = GET_MICROS / 1000;
      //ESP_LOGI(__FILE__, "Switching sides!");
      RaceHandler.ToggleRunDirection();
   }
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
///   Read 74HC166 pins status to detect remote buttons or switch side button press.
/// </summary>
void ButtonsRead()
{
   byte bOldDataIn = bDataIn;
   bDataIn = 0;
   digitalWrite(iLatchPin, LOW);
   digitalWrite(iClockPin, LOW);
   digitalWrite(iClockPin, HIGH);
   digitalWrite(iLatchPin, HIGH);
   for (uint8_t i = 0; i < 8; ++i)
   {
      bDataIn |= digitalRead(iDataInPin) << (7 - i);
      digitalWrite(iClockPin, LOW);
      digitalWrite(iClockPin, HIGH);
   }
   // It's assumed that only one button can be pressed at the same time, therefore any multiple high states are treated as false read and ingored
   if (bDataIn != 0 && bDataIn != 1 && bDataIn != 2 && bDataIn != 4 && bDataIn != 8 && bDataIn != 16 && bDataIn != 32 && bDataIn != 64 && bDataIn != 128)
   {
      ESP_LOGD(__FILE__, "%s", GetButtonString().c_str());
      bDataIn = 0;
   }
   // Print to console if button press detected
   if (bDataIn != bOldDataIn && bDataIn != 0)
   {
      ESP_LOGD(__FILE__, "%s", GetButtonString().c_str());
   }
}

/// <summary>
///   Gets pressed button string for consol printing.
/// </summary>
String GetButtonString()
{
   String strButton;
   switch (bDataIn)
   {
   case 1:
      strButton = "Side switch";
      break;
   case 2:
      strButton = "Remote 1: start/stop";
      break;
   case 4:
      strButton = "Remote 2: reset";
      break;
   case 8:
      strButton = "Remote 3: dog 1 fault";
      break;
   case 16:
      strButton = "Remote 6: dog 4 fault";
      break;
   case 32:
      strButton = "Remote 5: dog 3 fault";
      break;
   case 64:
      strButton = "Remote 4: dog 2 fault";
      break;
   case 128:
      strButton = "Laser trigger";
      break;
   default:
      strButton = "Unknown --> Ingored";
      break;
   }

   return strButton;
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
   LightsController.InitiateStartSequence();
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
   llLastRCPress[1] = GET_MICROS / 1000;
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
   llLastRCPress[2] = GET_MICROS / 1000;
   LightsController.ResetLights();
   RaceHandler.ResetRace();
}

#ifdef WiFiON
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

void mdnsServerSetup()
{
   MDNS.addService("http", "tcp", 80);
   MDNS.addServiceTxt("arduino", "tcp", "app_version", APP_VER);
   MDNS.begin("FlyballETS");
}
#endif