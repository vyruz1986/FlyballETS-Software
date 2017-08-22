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
// Copyright (C) 2017 Alex Goris
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
#include "config.h"
#include "StreamPrint.h"
#include "LCDController.h"
#include "RaceHandler.h"
#include "LightsController.h"
#include "BatterySensor.h"
#include "global.h"
#include <LiquidCrystal.h>
#include <avr/pgmspace.h>

/*List of pins and the ones used:
   - D0: Reserved for RX
   - D1: Reserved for TX
   - D2: S1 (handler side) photoelectric sensor
   - D3: S2 (box side) photoelectric sensor
   - D4: LCD Data7
   - D5: LCD Data6
   - D6: LCD Data5
   - D7: LCD Data4
   - D8: WS2811B lights data pin / Lights 74HC595 clock pin
   - D9: <free> / Lights 74HC595 data pin
   - D10: LCD2 (line 3&4) enable pin
   - D11: LCD1 (line 1&2) enable pin
   - D12: LCD RS Pin
   - D13: <free> / Lights 74HC595 latch pin
   - A0: remote D5
   - A1: remote D2
   - A2: remote D1
   - A3: remote D0
   - A4: remote D3
   - A5: remote D4
   - A6: battery sensor pin
   - A7: <free>
*/

//Set simulate to true to enable simulator class (see Simulator.cpp/h)
#define Simulate false
#if Simulate
   #include "Simulator.h"
#endif

#ifdef WS281x
   #include <Adafruit_NeoPixel.h>
#endif // WS281x

uint8_t iS1Pin = 2;
uint8_t iS2Pin = 3;
uint8_t iCurrentDog;
uint8_t iCurrentRaceState;

char cDogTime[8];
char cDogCrossingTime[8];
char cElapsedRaceTime[8];
char cTotalCrossingTime[8];

//Battery variables
int iBatterySensorPin = A6;
uint16_t iBatteryVoltage = 0;

//Initialise Lights stuff
#ifdef WS281x
   uint8_t iLightsDataPin = 8;
   Adafruit_NeoPixel LightsStrip = Adafruit_NeoPixel(5, iLightsDataPin, NEO_RGB);
#else
   uint8_t iLightsClockPin = 8;
   uint8_t iLightsDataPin = 9;
   uint8_t iLightsLatchPin = 13;
#endif // WS281x



//Set last serial output variable
long lLastSerialOutput = 0;

//remote control pins
int iRC0Pin = A3;
int iRC1Pin = A2;
int iRC2Pin = A1;
int iRC3Pin = A4;
int iRC4Pin = A5;
int iRC5Pin = A0;
//Array to hold last time button presses
unsigned long lLastRCPress[6] = {0, 0, 0, 0, 0, 0};

LiquidCrystal lcd(12, 11, 7, 6, 5, 4);  //declare two LCD's, this will be line 1&2
LiquidCrystal lcd2(12, 10, 7, 6, 5, 4); //This is the second, this will be line 3&4

//String for serial comms storage
String strSerialData;
byte bySerialIndex = 0;
boolean bSerialStringComplete = false;
   
void setup()
{
   Serial.begin(57600);

   pinMode(iS1Pin, INPUT);
   pinMode(iS2Pin, INPUT);

   //Set ISR's with wrapper functions
   attachInterrupt(1, Sensor2Wrapper, CHANGE);
   attachInterrupt(0, Sensor1Wrapper, CHANGE);

   //Initialize BatterySensor class with correct pin
   BatterySensor.init(iBatterySensorPin);

   //Initialize LightsController class with shift register pins
#ifdef WS281x
   LightsController.init(&LightsStrip);
#else
   LightsController.init(iLightsLatchPin, iLightsClockPin, iLightsDataPin);
#endif

   //Initialize RaceHandler class with S1 and S2 pins
   RaceHandler.init(iS1Pin, iS2Pin);

   //Initialize LCDController class with lcd1 and lcd2 objects
   LCDController.init(&lcd, &lcd2);

   //Initialize simulatorclass pins if applicable
   #if Simulate
      Simulator.init(iS1Pin, iS2Pin);
   #endif

   //initialize pins for remote control
   pinMode(iRC0Pin, INPUT);
   pinMode(iRC1Pin, INPUT);
   pinMode(iRC2Pin, INPUT);
   pinMode(iRC3Pin, INPUT);
   pinMode(iRC4Pin, INPUT);
   pinMode(iRC5Pin, INPUT);
   
   strSerialData[0] = 0;

   Serialprint("Ready!\r\n");
}

void loop()
{
   //Handle Race main processing
   RaceHandler.Main();
   
   //Handle lights main processing
   LightsController.Main();
   
   //Handle battery sensor main processing
   BatterySensor.CheckBatteryVoltage();
   
   //Handle LCD processing
   LCDController.Main();
   
#if Simulate
   //Run simulator
   Simulator.Main();
#endif
   
   //Race start/stop button (remote D0 output) or serial command
   if ((digitalRead(iRC0Pin) == HIGH
      && (millis() - lLastRCPress[0] > 2000))
      || (bSerialStringComplete
         && (strSerialData == "START"
            || strSerialData == "STOP")))
   {
      StartStopRace();
   }

   //Race reset button (remote D1 output)
   if (digitalRead(iRC1Pin) == HIGH
      && (millis() - lLastRCPress[1] > 2000)
      || (bSerialStringComplete && strSerialData == "RESET"))
   {
      ResetRace();
   }

   //Dog0 fault RC button
   if (digitalRead(iRC2Pin) == HIGH
      && (millis() - lLastRCPress[2] > 2000)
      || (bSerialStringComplete && strSerialData == "D0F"))
   {
      lLastRCPress[2] = millis();
      //Toggle fault for dog
      RaceHandler.SetDogFault(0);
   }

   //Dog1 fault RC button
   if (digitalRead(iRC3Pin) == HIGH
      && (millis() - lLastRCPress[3] > 2000)
      || (bSerialStringComplete && strSerialData == "D1F"))
   {
      lLastRCPress[3] = millis();
      //Toggle fault for dog
      RaceHandler.SetDogFault(1);
   }
   //Dog2 fault RC button
   if (digitalRead(iRC4Pin) == HIGH
      && (millis() - lLastRCPress[4] > 2000)
      || (bSerialStringComplete && strSerialData == "D2F"))
   {
      lLastRCPress[4] = millis();
      //Toggle fault for dog
      RaceHandler.SetDogFault(2);
   }

   //Dog3 fault RC button
   if (digitalRead(iRC5Pin) == HIGH
      && (millis() - lLastRCPress[5] > 2000)
      || (bSerialStringComplete && strSerialData == "D3F"))
   {
      lLastRCPress[5] = millis();
      //Toggle fault for dog
      RaceHandler.SetDogFault(3);
   }

   /*Update LCD Display fields*/
   //Update team time to display
   dtostrf(RaceHandler.GetRaceTime(), 7, 3, cElapsedRaceTime);
   LCDController.UpdateField(LCDController.TeamTime, cElapsedRaceTime);

   //Update battery percentage to display
   iBatteryVoltage = BatterySensor.GetBatteryVoltage();
   uint16_t iBatteryPercentage = BatterySensor.GetBatteryPercentage();
   LCDController.UpdateField(LCDController.BattLevel, String(iBatteryPercentage));

   //Update total crossing time
   dtostrf(RaceHandler.GetTotalCrossingTime(), 7, 3, cTotalCrossingTime);
   LCDController.UpdateField(LCDController.TotalCrossTime, cTotalCrossingTime);

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
         Serialprint("D%i: %s|CR: %s\r\n", RaceHandler.iCurrentDog, cDogTime, RaceHandler.GetCrossingTime(RaceHandler.iCurrentDog, -2).c_str());
         Serialprint("RT:%s\r\n", cElapsedRaceTime);
      }
      Serialprint("RS: %i\r\n", RaceHandler.RaceState);
   }

   if (RaceHandler.iCurrentDog != iCurrentDog)
   {
      dtostrf(RaceHandler.GetDogTime(RaceHandler.iPreviousDog, -2), 7, 3, cDogTime);
      Serialprint("D%i: %s|CR: %s\r\n", RaceHandler.iPreviousDog, cDogTime, RaceHandler.GetCrossingTime(RaceHandler.iPreviousDog, -2).c_str());
      Serialprint("D: %i\r\n", RaceHandler.iCurrentDog);
      Serialprint("RT:%s\r\n", cElapsedRaceTime);
   }

   //Enable (uncomment) the following if you want periodic status updates on the serial port
   /*
   if ((millis() - lLastSerialOutput) > 500)
   {
      //Serialprint("%lu: ping! voltage is: %.2u, this is %i%%\r\n", millis(), iBatteryVoltage, iBatteryPercentage);
      //Serialprint("%lu: Elapsed time: %s\r\n", millis(), cElapsedRaceTime);
      if (RaceHandler.RaceState == RaceHandler.RUNNING)
      {
         dtostrf(RaceHandler.GetDogTime(RaceHandler.iCurrentDog), 7, 3, cDogTime);
         Serialprint("Dog %i: %ss\r\n", RaceHandler.iCurrentDog, cDogTime);
      }
      lLastSerialOutput = millis();
   }
   */

   //Cleanup variables used for checking if something changed
   iCurrentDog = RaceHandler.iCurrentDog;
   iCurrentRaceState = RaceHandler.RaceState;

   //Check if we have serial data which we should handle
   if (strSerialData.length() > 0
       && bSerialStringComplete)
   {
      if (bDEBUG) Serialprint("cSer: '%s'\r\n", strSerialData.c_str());

      if (strSerialData == "DEBUG")
      {
         bDEBUG = !bDEBUG;
      }

      strSerialData = "";
      bSerialStringComplete = false;
   }
}

void serialEvent()
{
   //Listen on serial port
   Serial.flush();
   while (Serial.available() > 0)
   {
      char cInChar = Serial.read(); // Read a character
	  //Check if buffer contains complete serial message, terminated by newline (\n)
      if (cInChar == '\n')
      {
		   //Serial message in buffer is complete, null terminate it and store it for further handling
         bSerialStringComplete = true;
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
///   Starts (if stopped) or stops (if started) a race. Start is only allowed if race is stopped and reset.
/// </summary>
void StartStopRace()
{
   lLastRCPress[0] = millis();
   if (RaceHandler.RaceState == RaceHandler.STOPPED //If race is stopped
      && RaceHandler.GetRaceTime() == 0)           //and timers are zero
   {
      //Then start the race
      if (bDEBUG) Serialprint("%lu: START!\r\n", millis());
      LightsController.InitiateStartSequence();
      RaceHandler.StartRace();
   }
   else //If race state is running or starting, we should stop it
   {
      RaceHandler.StopRace(micros());
      LightsController.DeleteSchedules();
   }
}

/// <summary>
///   Reset race so new one can be started, reset is only allowed when race is stopped
/// </summary>
void ResetRace()
{
   if (RaceHandler.RaceState != RaceHandler.STOPPED)   //Only allow reset when race is stopped first
   {
      return;
   }
   lLastRCPress[1] = millis();
   LightsController.ResetLights();
   RaceHandler.ResetRace();
}