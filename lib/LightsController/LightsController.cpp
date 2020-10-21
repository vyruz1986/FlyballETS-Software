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
#ifdef WS281x
void LightsControllerClass::init(NeoPixelBus<NeoRgbFeature, WS_METHOD> *LightsStrip)
#else
void LightsControllerClass::init(uint8_t iLatchPin, uint8_t iClockPin, uint8_t iDataPin)
#endif // WS281x
{
#ifdef WS281x
   //pinMode(iLightsPin, OUTPUT);
   _LightsStrip = LightsStrip;
   //_LightsStrip = NeoPixelBus<NeoRgbFeature, NeoWs2813Method>(5, iLightsPin);
   //_LightsStrip = Adafruit_NeoPixel(5, iLightsPin, NEO_RGB + NEO_KHZ800);
   _LightsStrip->Begin();
   //_LightsStrip->SetBrightness(255);
   _LightsStrip->Show(); // Initialize all pixels to 'off'
#else
   //Initialize pins for shift register
   _iLatchPin = iLatchPin;
   _iClockPin = iClockPin;
   _iDataPin = iDataPin;

   pinMode(_iLatchPin, OUTPUT);
   pinMode(_iClockPin, OUTPUT);
   pinMode(_iDataPin, OUTPUT);

   //Write 0 to shift register to turn off all lights
   digitalWrite(_iLatchPin, LOW);
   shiftOut(_iDataPin, _iClockPin, MSBFIRST, 0);
   digitalWrite(_iLatchPin, HIGH);
#endif // WS281x
}

/// <summary>
///   Main entry-point for this application. It contains the main processing required for the
///   lights and should be called every time in the main loop of the project.
/// </summary>
void LightsControllerClass::Main()
{
   HandleStartSequence();

   //Check if we have to toggle any lights
   for (int i = 0; i < 6; i++)
   {
      if (GET_MICROS / 1000 > _llLightsOnSchedule[i] && _llLightsOnSchedule[i] != 0)
      {
         ToggleLightState(_byLightsArray[i], ON);
         _llLightsOnSchedule[i] = 0; //Delete schedule
      }
      if (GET_MICROS / 1000 > _llLightsOutSchedule[i] && _llLightsOutSchedule[i] != 0)
      {
         ToggleLightState(_byLightsArray[i], OFF);
         _llLightsOutSchedule[i] = 0; //Delete schedule
      }
   }
#ifdef WS281x
   //Update lights
   if (_LightsStrip->CanShow())
   {
      _LightsStrip->Show();
   }
#endif // WS281x

   if (_byCurrentLightsState != _byNewLightsState)
   {
      //ESP_LOGD(__FILE__, " %llu: New light states: %i", GET_MICROS / 1000, _byNewLightsState);
      _byCurrentLightsState = _byNewLightsState;
#ifndef WS281x
      digitalWrite(_iLatchPin, LOW);
      shiftOut(_iDataPin, _iClockPin, MSBFIRST, _byCurrentLightsState);
      digitalWrite(_iLatchPin, HIGH);
#endif // WS281x
      //Send data to websocket clients
      WebHandler.SendLightsData(GetLightsState());
   }
}

/// <summary>
///   Handles the start sequence, will be called by main function when oceral race state is
///   STARTING.
/// </summary>
void LightsControllerClass::HandleStartSequence()
{
   //This function takes care of the starting lights sequence
   //First check if the overall state of this class is 'STARTING'
   if (byOverallState == STARTING)
   {
      //The class is in the 'STARTING' state, check if the lights have been programmed yet
      if (!_bStartSequenceStarted)
      {
         //Start sequence is not yet started, we need to schedule the lights on/off times

         //Set schedule for RED light
         _llLightsOnSchedule[1] = GET_MICROS / 1000;         //Turn on NOW
         _llLightsOutSchedule[1] = GET_MICROS / 1000 + 1000; //keep on for 1 second

         //Set schedule for YELLOW1 light
         _llLightsOnSchedule[2] = GET_MICROS / 1000 + 1000;  //Turn on after 1 second
         _llLightsOutSchedule[2] = GET_MICROS / 1000 + 2000; //Turn off after 2 seconds

         //Set schedule for YELLOW2 light
         _llLightsOnSchedule[4] = GET_MICROS / 1000 + 2000;  //Turn on after 2 seconds
         _llLightsOutSchedule[4] = GET_MICROS / 1000 + 3000; //Turn off after 3 seconds

         //Set schedule for GREEN light
         _llLightsOnSchedule[5] = GET_MICROS / 1000 + 3000;  //Turn on after 3 seconds
         _llLightsOutSchedule[5] = GET_MICROS / 1000 + 4000; //Turn off after 4 seconds

         _bStartSequenceStarted = true;
      }
      //Check if the start sequence is busy
      bool bStartSequenceBusy = false;
      for (int i = 0; i < 6; i++)
      {
         if (_llLightsOnSchedule[i] > 0 || _llLightsOutSchedule[i] > 0)
         {
            bStartSequenceBusy = true;
         }
      }
      //Check if we should start the timer (GREEN light on)
      if (CheckLightState(GREEN) == ON && RaceHandler.RaceState == RaceHandler.STARTING)
      {
         RaceHandler.StartTimers();
         ESP_LOGD(__FILE__, "%llu: GREEN light is ON!", GET_MICROS / 1000);
      }
      if (!bStartSequenceBusy)
      {
         _bStartSequenceStarted = false;
         byOverallState = STARTED;
      }
   }
}

/// <summary>
///   Initiate start sequence, should be called if starting lights sequence should be initiated.
/// </summary>
void LightsControllerClass::InitiateStartSequence()
{
   byOverallState = STARTING;
}

/// <summary>
///   Resets the lights (turn everything OFF).
/// </summary>
void LightsControllerClass::ResetLights()
{
   byOverallState = RESET;

   //Set all lights off
#ifdef WS281x
   for (uint16_t i = 0; i < _LightsStrip->PixelCount(); i++)
   {
      _LightsStrip->SetPixelColor(i, 0);
   }
#endif // WS281x
   _byNewLightsState = 0;
   DeleteSchedules();
}

/// <summary>
///   Deletes any scheduled light timings.
/// </summary>
void LightsControllerClass::DeleteSchedules()
{
   //Delete any set schedules
   for (int i = 0; i < 6; i++)
   {
      _llLightsOnSchedule[i] = 0;  //Delete schedule
      _llLightsOutSchedule[i] = 0; //Delete schedule
   }
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
#ifdef WS281x
   SNeoPixelConfig LightConfig = _GetNeoPixelConfig(byLight);

   if (byLightState == OFF)
   {
      LightConfig.iColor = RgbColor(0);
      ESP_LOGD(__FILE__, "%llu: Light %d is OFF", GET_MICROS / 1000, LightConfig.iPixelNumber);
   }
   else
   {
      ESP_LOGD(__FILE__, "%llu: Light %d is ON", GET_MICROS / 1000, LightConfig.iPixelNumber);
   }

   for (int lightschain = 0; lightschain < LIGHTSCHAINS; lightschain++)
   {
      _LightsStrip->SetPixelColor(LightConfig.iPixelNumber + 5 * lightschain, LightConfig.iColor);
   }

#endif // WS281x
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
      //Set schedule for WHITE light
      _llLightsOnSchedule[0] = GET_MICROS / 1000;         //Turn on NOW
      _llLightsOutSchedule[0] = GET_MICROS / 1000 + 1000; //keep on for 1 second
   }
   ToggleLightState(byLight, byLightState);
   //ESP_LOGD(__FILE__, "Fault light for dog %i: %i", DogNumber, byLightState);
}

/// <summary>
///   Switch white fault light ON/OFF to indicate unexpected sensor read while in READY/RESET state.
/// </summary>
void LightsControllerClass::ReaceReadyFault(LightStates byLightState)
{
   Lights byLight = _byLightsArray[0];
   ToggleLightState(byLight, byLightState);
}

stLightsState LightsControllerClass::GetLightsState()
{
   stLightsState CurrentLightsState;

   CurrentLightsState.State[0] = CheckLightState(WHITE) == 1 ? 1 : 0;
   CurrentLightsState.State[1] = CheckLightState(RED) == 1 ? 1 : 0;
   //3rd light can have 2 colors
   if (CheckLightState(YELLOW1) == 1)
   {
      CurrentLightsState.State[2] = 1;
   }
   else if (CheckLightState(BLUE))
   {
      CurrentLightsState.State[2] = 2;
   }
   else
   {
      CurrentLightsState.State[2] = 0;
   }
   CurrentLightsState.State[3] = CheckLightState(YELLOW2) == 1 ? 1 : 0;
   CurrentLightsState.State[4] = CheckLightState(GREEN) == 1 ? 1 : 0;

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
#ifdef WS281x
   RgbColor iCurrentColor;
   SNeoPixelConfig TargetConfig = _GetNeoPixelConfig(byLight);
   iCurrentColor = _LightsStrip->GetPixelColor(TargetConfig.iPixelNumber);
   if (iCurrentColor == TargetConfig.iColor)
#else
   if ((byLight & _byNewLightsState) == byLight)
#endif // WS281x
   {
      return ON;
   }
   else
   {
      return OFF;
   }
}

#ifdef WS281x
LightsControllerClass::SNeoPixelConfig LightsControllerClass::_GetNeoPixelConfig(LightsControllerClass::Lights byLight)
{
   SNeoPixelConfig Config;
   switch (byLight)
   {
   case WHITE:
      Config.iPixelNumber = 0;
      Config.iColor = RgbColor(255, 255, 255);
      break;
   case RED:
      Config.iPixelNumber = 1;
      Config.iColor = RgbColor(255, 0, 0);
      break;
   case YELLOW1:
      Config.iPixelNumber = 2;
      Config.iColor = RgbColor(255, 100, 0);
      break;
   case BLUE:
      Config.iPixelNumber = 2;
      Config.iColor = RgbColor(0, 0, 255);
      break;
   case YELLOW2:
      Config.iPixelNumber = 3;
      Config.iColor = RgbColor(255, 100, 0);
      break;
   case GREEN:
      Config.iPixelNumber = 4;
      Config.iColor = RgbColor(0, 255, 0);
      break;
   }

   return Config;
}
#endif // WS281x

/// <summary>
///   The lights controller.
/// </summary>
LightsControllerClass LightsController;
