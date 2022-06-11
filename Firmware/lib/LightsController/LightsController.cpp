// file:	LightsController.cpp
//
// summary:	Implements the lights controller class
// Copyright (C) 2019 Alex Goris
// This file is part of FlyballETS-Software
// FlyballETS-Software is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.If not, see <http://www.gnu.org/licenses/>

#include "LightsController.h"
#include "RaceHandler.h"
#include "config.h"
#include "Structs.h"
#include "WebHandler.h"
#include <NeoPixelBus.h>

/// <summary>
///   Initialises this object. This function needs to be passed the pin numbers for the shift
///   register which is used to control the lights.
/// </summary>
///
/// <param name="iLatchPin">  Zero-based index of the latch pin. </param>
/// <param name="iClockPin">  Zero-based index of the clock pin. </param>
/// <param name="iDataPin">   Zero-based index of the data pin. </param>
void LightsControllerClass::init(NeoPixelBus<NeoRgbFeature, WS_METHOD> *LightsStrip)
{
   if (SettingsManager.getSetting("StartingSequenceNAFA").equals("1"))
   {
      bModeNAFA = true;
      ESP_LOGI(__FILE__, "Starting sequence from settings: NAFA");
   }
   else
   {
      ESP_LOGI(__FILE__, "Starting sequence from settings: FCI");
   }
   //pinMode(iLightsPin, OUTPUT);
   _LightsStrip = LightsStrip;
   //_LightsStrip = NeoPixelBus<NeoRgbFeature, NeoWs2813Method>(5, iLightsPin);
   //_LightsStrip = Adafruit_NeoPixel(5, iLightsPin, NEO_RGB + NEO_KHZ800);
   _LightsStrip->Begin();
   //_LightsStrip->SetBrightness(255);
   _LightsStrip->Show(); // Initialize all pixels to 'off'
}

/// <summary>
///   Main entry-point for this application. It contains the main processing required for the
///   lights and should be called every time in the main loop of the project.
/// </summary>
void LightsControllerClass::Main()
{
   //Check if we have to toggle any lights
   for (int i = 0; i < 8; i++)
   {
      if (GET_MICROS / 1000 > _lLightsOnSchedule[i] && _lLightsOnSchedule[i] != 0)
      {
         ToggleLightState(_byLightsArray[i], ON);
         _lLightsOnSchedule[i] = 0; //Delete schedule
      }
      if (GET_MICROS / 1000 > _lLightsOutSchedule[i] && _lLightsOutSchedule[i] != 0)
      {
         ToggleLightState(_byLightsArray[i], OFF);
         _lLightsOutSchedule[i] = 0; //Delete schedule
      }
   }

   //Update lights
   if (_LightsStrip->CanShow())
   {
      _LightsStrip->Show();
   }

   if (_byCurrentLightsState != _byNewLightsState)
   {
      //ESP_LOGD(__FILE__, "New light states: %i", _byNewLightsState);
      _byCurrentLightsState = _byNewLightsState;
#ifdef WiFiON
      //Send data to websocket clients
      WebHandler._bUpdateLights = true;
#endif
   }
}

/// <summary>
///   Set start warning sequence used by NAFTA.
/// </summary>
void LightsControllerClass::WarningStartSequence()
{
   unsigned long lOffset = 10000; // Offset of 10ms introduced to assure proper synchronization
   //Set schedule for GREEN4 light
   _lLightsOnSchedule[7] = (GET_MICROS + lOffset) / 1000;         //Turn on "NOW" 
   _lLightsOutSchedule[7] = (GET_MICROS + lOffset)  / 1000 + 150; //Turn off after 200ms

   //Set schedule for YELLOW3 light
   _lLightsOnSchedule[6] = (GET_MICROS + lOffset) / 1000 + 150;   //Turn on after 200ms
   _lLightsOutSchedule[6] = (GET_MICROS + lOffset)  / 1000 + 300; //Turn off after 400ms
   
   //Set schedule for YELLOW2 light
   _lLightsOnSchedule[4] = (GET_MICROS + lOffset)  / 1000 + 300;  //Turn on after 400ms
   _lLightsOutSchedule[4] = (GET_MICROS + lOffset)  / 1000 + 450; //Turn off after 600ms

   //Set schedule for YELLOW1 light
   _lLightsOnSchedule[2] = (GET_MICROS + lOffset) / 1000 + 450;    //Turn on after 600ms
   _lLightsOutSchedule[2] = (GET_MICROS + lOffset)  / 1000 + 600;  //Turn on after 800ms

   //Set schedule for RED0 light
   _lLightsOnSchedule[1] = (GET_MICROS + lOffset) / 1000 + 600;    //Turn on after 800ms
   _lLightsOutSchedule[1] = (GET_MICROS + lOffset)  / 1000 + 750;  //Turn on after 1 second

   byOverallState = WARNING;
}

/// <summary>
///   Initiate start sequence, should be called if starting lights sequence should be initiated.
/// </summary>
void LightsControllerClass::InitiateStartSequence()
{
   //Set start sequence, we need to schedule the lights on/off times.
   if (bModeNAFA)
   {
      unsigned long lOffset = 1000000; // 1s offset after warning sequence ended
      //Set schedule for YELLOW1 light
      _lLightsOnSchedule[2] = (GET_MICROS + lOffset) / 1000;          //Turn on
      _lLightsOutSchedule[2] = (GET_MICROS + lOffset)  / 1000 + 1000; //Turn off after 1 seconds

      //Set schedule for YELLOW2 light
      _lLightsOnSchedule[4] = (GET_MICROS + lOffset)  / 1000 + 1000;  //Turn on after 1 second
      _lLightsOutSchedule[4] = (GET_MICROS + lOffset)  / 1000 + 2000; //Turn off after 2 seconds

      //Set schedule for YELLOW3 light
      _lLightsOnSchedule[6] = (GET_MICROS + lOffset) / 1000 + 2000;  //Turn on after 2 seconds
      _lLightsOutSchedule[6] = (GET_MICROS + lOffset)  / 1000 + 3000; //Turn off after 3 seconds

      //Set schedule for GREEN4 light
      _lLightsOnSchedule[7] = (GET_MICROS + lOffset)  / 1000 + 3000;  //Turn on after 3 seconds
      _lLightsOutSchedule[7] = (GET_MICROS + lOffset)  / 1000 + 4000; //Turn off after 4 seconds
   }
   else
   {
      unsigned long lOffset = 10000; // Offset of 10ms introduced to assure proper synchronization
      //Set schedule for RED1 light
      _lLightsOnSchedule[3] = (GET_MICROS + lOffset) / 1000;          //Turn on
      _lLightsOutSchedule[3] = (GET_MICROS + lOffset)  / 1000 + 1000; //keep on for 1 second

      //Set schedule for YELLOW2 light
      _lLightsOnSchedule[4] = (GET_MICROS + lOffset)  / 1000 + 1000;  //Turn on after 1 second
      _lLightsOutSchedule[4] = (GET_MICROS + lOffset)  / 1000 + 2000; //Turn off after 2 seconds

      //Set schedule for YELLOW3 light
      _lLightsOnSchedule[6] = (GET_MICROS + lOffset) / 1000 + 2000;  //Turn on after 2 seconds
      _lLightsOutSchedule[6] = (GET_MICROS + lOffset)  / 1000 + 3000; //Turn off after 3 seconds

      //Set schedule for GREEN4 light
      _lLightsOnSchedule[7] = (GET_MICROS + lOffset)  / 1000 + 3000;  //Turn on after 3 seconds
      _lLightsOutSchedule[7] = (GET_MICROS + lOffset)  / 1000 + 4000; //Turn off after 4 seconds
   }
   byOverallState = INITIATED;
}

/// <summary>
///   Resets the lights (turn everything OFF).
/// </summary>
void LightsControllerClass::ResetLights()
{
   byOverallState = RESET;

   //Set all lights off
   for (uint16_t i = 0; i < _LightsStrip->PixelCount(); i++)
   {
      _LightsStrip->SetPixelColor(i, 0);
   }
   _byNewLightsState = 0;
   DeleteSchedules();
}

/// <summary>
///   Deletes any scheduled light timings.
/// </summary>
void LightsControllerClass::DeleteSchedules()
{
   //Delete any set schedules
   for (int i = 0; i < 8; i++)
   {
      _lLightsOnSchedule[i] = 0;  //Delete schedule
      _lLightsOutSchedule[i] = 0; //Delete schedule
   }
   //Blink WHITE light to indicate manual STOP or RESET execution
   _lLightsOnSchedule[0] = GET_MICROS / 1000;         //Turn on NOW
   _lLightsOutSchedule[0] = GET_MICROS / 1000 + 100;  //keep on for 100ms
}

/// <summary>
///   Toggle a given light to a given state.
/// </summary>
///
/// <param name="byLight">       The by light. </param>
/// <param name="byLightState">  State of the by light. </param>
void LightsControllerClass::ToggleLightState(Lights byLight, LightStates byLightState)
{
   bool byCurrentLightState = CheckLightState(byLight);
   if (byLightState == TOGGLE)
   {
      //We have to toggle the lights
      if (byCurrentLightState == ON)
      {
         byLightState = OFF;
      }
      else
      {
         byLightState = ON;
      }
   }

   SNeoPixelConfig LightConfig = _GetNeoPixelConfig(byLight);

   if (byLightState == OFF)
   {
      LightConfig.iColor = RgbColor(0);
      // If start warning sequence is initiated and we're going to turn off RED0 light we need to initiate start sequence
      if (byOverallState == WARNING && LightConfig.iPixelNumber == 0)
         InitiateStartSequence();
      //ESP_LOGD(__FILE__, "Light %d is OFF", LightConfig.iPixelNumber);
   }
   else
   {
      //ESP_LOGD(__FILE__, "Light %d is ON", LightConfig.iPixelNumber);
      // If start sequence is initiated and we're going to turn on RED light we need to start race timer
      if (byOverallState == INITIATED && LightConfig.iPixelNumber == 1)
      {
         RaceHandler.StartRaceTimer();
         byOverallState = STARTING;
      }

      // If start sequence is in progress and we're going to trun on GREEN light we need to change race state to RUNNING
      if (byOverallState == STARTING && LightConfig.iPixelNumber == 4)
      {
         RaceHandler.ChangeRaceStateToRunning();
         byOverallState = STARTED;
      }
   }

   for (int lightschain = 0; lightschain < LIGHTSCHAINS; lightschain++)
   {
      _LightsStrip->SetPixelColor(LightConfig.iPixelNumber + 5 * lightschain, LightConfig.iColor);
      ESP_LOGI(__FILE__, "Light %d is now %d", LightConfig.iPixelNumber, byLightState);
   }

   if (byCurrentLightState != byLightState)
   {

      if (byLightState == ON)
      {
         _byNewLightsState = _byNewLightsState + byLight;
      }
      else
      {
         _byNewLightsState = _byNewLightsState - byLight;
      }
   }
}

/// <summary>
///   Toggle fault light for a given dog number. This function will take a dog number and a light
///   state, and determine by itself which light should be set to the given state.
/// </summary>
///
/// <param name="DogNumber">     Zero-indexed dog number. </param>
/// <param name="byLightState">  State of the by light. </param>
void LightsControllerClass::ToggleFaultLight(uint8_t DogNumber, LightStates byLightState)
{
   //Get error light for dog number from array
   Lights byLight = _byDogErrorLigths[DogNumber];
   if (byLightState == ON)
   {
      //If a fault lamp is turned on we have to light the white light for 1 sec
      //Set schedule for FAULT light
      if (bModeNAFA)
      {
         _lLightsOnSchedule[1] = GET_MICROS / 1000;         //Turn on NOW
         _lLightsOutSchedule[1] = GET_MICROS / 1000 + 1000; //keep on for 1 second
      }
      else
      {
         _lLightsOnSchedule[0] = GET_MICROS / 1000;         //Turn on NOW
         _lLightsOutSchedule[0] = GET_MICROS / 1000 + 1000; //keep on for 1 second
      }
   }
   ToggleLightState(byLight, byLightState);
   //ESP_LOGD(__FILE__, "Fault light for dog %i: %i", DogNumber, byLightState);
}

/// <summary>
///   Switch fault light ON/OFF to indicate unexpected sensor read while in READY/RESET state.
/// </summary>
void LightsControllerClass::ReaceReadyFault(LightStates byLightState)
{
   Lights byLight = _byLightsArray[0];
   ToggleLightState(byLight, byLightState);
}

stLightsState LightsControllerClass::GetLightsState()
{
   stLightsState CurrentLightsState;

      //1st light can have 2 colors
   if (CheckLightState(WHITE0))
   {
      CurrentLightsState.State[0] = 1;
   }
   else if (CheckLightState(RED0))
   {
      CurrentLightsState.State[0] = 2;
   }
   else
   {
      CurrentLightsState.State[0] = 0;
   }
   //2nd light can have 2 colors
   if (CheckLightState(YELLOW1))
   {
      CurrentLightsState.State[1] = 1;
   }
   else if (CheckLightState(RED1))
   {
      CurrentLightsState.State[1] = 2;
   }
   else
   {
      CurrentLightsState.State[1] = 0;
   }
   //3rd light can have 2 colors
   if (CheckLightState(YELLOW2))
   {
      CurrentLightsState.State[2] = 1;
   }
   else if (CheckLightState(BLUE2))
   {
      CurrentLightsState.State[2] = 2;
   }
   else
   {
      CurrentLightsState.State[2] = 0;
   }
   CurrentLightsState.State[3] = CheckLightState(YELLOW3) == 1 ? 1 : 0;
   CurrentLightsState.State[4] = CheckLightState(GREEN4) == 1 ? 1 : 0;

   return CurrentLightsState;
}

/// <summary>
///   Check light state for a given light.
/// </summary>
///
/// <param name="byLight"> The light for which the state should be returned. </param>
///
/// <returns>
///   The LightsControllerClass::LightStates state for the given light number.
/// </returns>
LightsControllerClass::LightStates LightsControllerClass::CheckLightState(Lights byLight)
{
   RgbColor iCurrentColor;
   SNeoPixelConfig TargetConfig = _GetNeoPixelConfig(byLight);
   iCurrentColor = _LightsStrip->GetPixelColor(TargetConfig.iPixelNumber);
   if (iCurrentColor == TargetConfig.iColor)
   {
      return ON;
   }
   else
   {
      return OFF;
   }
}

/// <summary>
///   Toggles the direction the system expects dogs to run in
/// </summary>
void LightsControllerClass::ToggleStartingSequence()
{
   bModeNAFA = !bModeNAFA;
   SettingsManager.setSetting("StartingSequenceNAFA", String(bModeNAFA));
   if (bModeNAFA)
   {
      ESP_LOGI(__FILE__, "Starting sequence: NAFA");
   }
   else
   {
      ESP_LOGI(__FILE__, "Starting sequence: FCI");
   }
   LCDController.reInit();
   #ifdef WiFiON
   WebHandler._bSendRaceData = true;
   #endif
}

LightsControllerClass::SNeoPixelConfig LightsControllerClass::_GetNeoPixelConfig(LightsControllerClass::Lights byLight)
{
   SNeoPixelConfig Config;
   switch (byLight)
   {
   case WHITE0:
      Config.iPixelNumber = 0;
      Config.iColor = RgbColor(255, 255, 255);
      break;
   case RED0:
      Config.iPixelNumber = 0;
      Config.iColor = RgbColor(255, 0, 0);
      break;
   case YELLOW1:
      Config.iPixelNumber = 1;
      Config.iColor = RgbColor(255, 100, 0);
      break;
   case RED1:
      Config.iPixelNumber = 1;
      Config.iColor = RgbColor(255, 0, 0);
      break;
   case YELLOW2:
      Config.iPixelNumber = 2;
      Config.iColor = RgbColor(255, 100, 0);
      break;
   case BLUE2:
      Config.iPixelNumber = 2;
      Config.iColor = RgbColor(0, 0, 255);
      break;
   case YELLOW3:
      Config.iPixelNumber = 3;
      Config.iColor = RgbColor(255, 100, 0);
      break;
   case GREEN4:
      Config.iPixelNumber = 4;
      Config.iColor = RgbColor(0, 255, 0);
      break;
   }

   return Config;
}

/// <summary>
///   The lights controller.
/// </summary>
LightsControllerClass LightsController;
