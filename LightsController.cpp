// file:	LightsController.cpp
//
// summary:	Implements the lights controller class
// Copyright (C) 2015  Alex Goris
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

#include "StreamPrint.h"
#include "LightsController.h"
#include "RaceHandler.h"
#include "global.h"
#include <Adafruit_NeoPixel.h>

/// <summary>
///   Initialises this object. This function needs to be passed the pin numbers for the shift
///   register which is used to control the lights.
/// </summary>
///
/// <param name="iLatchPin">  Zero-based index of the latch pin. </param>
/// <param name="iClockPin">  Zero-based index of the clock pin. </param>
/// <param name="iDataPin">   Zero-based index of the data pin. </param>
void LightsControllerClass::init(Adafruit_NeoPixel* LightsStrip)
{
   _LightsStrip = LightsStrip;
   _LightsStrip->begin();
   _LightsStrip->setBrightness(255);
   _LightsStrip->show(); // Initialize all pixels to 'off'
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
      if (millis() > _lLightsOnSchedule[i]
         && _lLightsOnSchedule[i] != 0)
      {
         ToggleLightState(_byLightsArray[i], ON);
         _lLightsOnSchedule[i] = 0; //Delete schedule
      }
      if (millis() > _lLightsOutSchedule[i]
         && _lLightsOutSchedule[i] != 0)
      {
         ToggleLightState(_byLightsArray[i], OFF);
         _lLightsOutSchedule[i] = 0; //Delete schedule
      }
   }
   //Update lights
   _LightsStrip->show();

   /*
   if (_byCurrentLightsState != _byNewLightsState)
   {
      if (bDEBUG) Serialprint("%lu: New light states: %i\r\n", millis(), _byNewLightsState);
      _byCurrentLightsState = _byNewLightsState;
      digitalWrite(_iLatchPin, LOW);
      shiftOut(_iDataPin, _iClockPin, MSBFIRST, _byCurrentLightsState);
      digitalWrite(_iLatchPin, HIGH);
   }*/
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
         _lLightsOnSchedule[1] = millis(); //Turn on NOW
         _lLightsOutSchedule[1] = millis() + 1000; //keep on for 1 second

         //Set schedule for YELLOW1 light
         _lLightsOnSchedule[2] = millis() + 1000; //Turn on after 1 second
         _lLightsOutSchedule[2] = millis() + 2000; //Turn off after 2 seconds

         //Set schedule for YELLOW2 light
         _lLightsOnSchedule[4] = millis() + 2000; //Turn on after 2 seconds
         _lLightsOutSchedule[4] = millis() + 3000; //Turn off after 2 seconds

         //Set schedule for GREEN light
         _lLightsOnSchedule[5] = millis() + 3000; //Turn on after 3 seconds
         _lLightsOutSchedule[5] = millis() + 4000; //Turn off after 4 seconds

         _bStartSequenceStarted = true;
      }
      //Check if the start sequence is busy
      bool bStartSequenceBusy = false;
      for (int i = 0; i < 6; i++)
      {
         if (_lLightsOnSchedule[i] > 0
            || _lLightsOutSchedule[i] > 0)
         {
            bStartSequenceBusy = true;
         }
      }
      //Check if we should start the timer (GREEN light on)
      if (CheckLightState(GREEN) == ON
         && RaceHandler.RaceState == RaceHandler.STARTING)
      {
         RaceHandler.StartTimers();
         if (bDEBUG) Serialprint("%lu: GREEN light is ON!\r\n", millis());
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
   byOverallState = STOPPED;
   
   //Set all lights off
   for (uint16_t i = 0; i < _LightsStrip->numPixels(); i++)
   {
      _LightsStrip->setPixelColor(i, 0);
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
   for (int i = 0; i < 6; i++)
   {
      _lLightsOnSchedule[i] = 0; //Delete schedule
      _lLightsOutSchedule[i] = 0; //Delete schedule
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
   uint8_t iPixelNumber;
   uint32_t iColor;


   switch (byLight)
   {
   case WHITE:
      iPixelNumber = 0;
      iColor = _LightsStrip->Color(255, 255, 255);
      break;
   case RED:
      iPixelNumber = 1;
      iColor = _LightsStrip->Color(255, 0, 0);
      break;
   case YELLOW1:
      iPixelNumber = 2;
      iColor = _LightsStrip->Color(255, 100, 0);
      break;
   case BLUE:
      iPixelNumber = 2;
      iColor = _LightsStrip->Color(0, 0, 255);
      break;
   case YELLOW2:
      iPixelNumber = 3;
      iColor = _LightsStrip->Color(255, 100, 0);
      break;
   case GREEN:
      iPixelNumber = 4;
      iColor = _LightsStrip->Color(0, 255, 0);
      break;
   }

   if (byLightState == OFF)
   {
      iColor = _LightsStrip->Color(0, 0, 0);
   }
   /*
   strip.setPixelColor(0, strip.Color( 0,0,0 ));
   strip.setPixelColor(1, strip.Color(0, 0, 0));
   strip.setPixelColor(2, strip.Color(0, 0, 0));
   strip.setPixelColor(3, strip.Color(0, 0, 0));
   strip.setPixelColor(4, strip.Color(0, 0, 0));
   */
   /*
   Serial.print("Color value: ");
   Serial.println(byLight);

   Serial.print("State value: ");
   Serial.println(byLightState);

   Serial.print("Setting pixel ");
   Serial.print(iPixelNumber);
   Serial.print(" to color ");
   Serial.println(iColor);*/
   _LightsStrip->setPixelColor(iPixelNumber, iColor);
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
      _lLightsOnSchedule[0] = millis(); //Turn on NOW
      _lLightsOutSchedule[0] = millis() + 1000; //keep on for 1 second
   }
   ToggleLightState(byLight, byLightState);
   if (bDEBUG) Serialprint("Fault light for dog %i: %i\r\n", DogNumber, byLightState);
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
   uint8_t iPixelNumber;
   uint32_t iCurrentColor;
   uint32_t iIntendedColor;
   switch (byLight)
   {
   case WHITE:
      iPixelNumber = 0;
      iIntendedColor = _LightsStrip->Color(255, 255, 255);
      break;
   case RED:
      iPixelNumber = 1;
      iIntendedColor = _LightsStrip->Color(255, 0, 0);
      break;
   case YELLOW1:
      iPixelNumber = 2;
      iIntendedColor = _LightsStrip->Color(255, 100, 0);
      break;
   case BLUE:
      iPixelNumber = 2;
      iIntendedColor = _LightsStrip->Color(0, 0, 255);
      break;
   case YELLOW2:
      iPixelNumber = 3;
      iIntendedColor = _LightsStrip->Color(255, 100, 0);
      break;
   case GREEN:
      iPixelNumber = 4;
      iIntendedColor = _LightsStrip->Color(0, 255, 0);
      break;
   }
   iCurrentColor = _LightsStrip->getPixelColor(iPixelNumber);
   if (iCurrentColor == iIntendedColor)
   {
      return ON;
   }
   else
   {
      return OFF;
   }
}

/// <summary>
///   The lights controller.
/// </summary>
LightsControllerClass LightsController;

