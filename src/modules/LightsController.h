// LightsController.h
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

#ifndef _LIGHTSCONTROLLER_h
#define _LIGHTSCONTROLLER_h

#include "Arduino.h"
#include <NeoPixelBus.h>
#include "config.h"
#include "Structs.h"

class LightsControllerClass
{
   friend class WebHandlerClass;

protected:
public:
   void init(NeoPixelBus<NeoRgbFeature, WS_METHOD> *LightsStrip);

   bool bModeNAFA = false;
   volatile bool bExecuteRaceReadyFaultON;
   volatile bool bExecuteRaceReadyFaultOFF;
   volatile bool bExecuteResetLights;

   //Overal state of this class
   enum OverallStates
   {
      WARNING,
      INITIATED,
      RESET,
      STARTING,
      STARTED
   };
   
   OverallStates byOverallState = RESET;

   //Possible pixel colors (unique names needed)
   enum Lights
   {
      WHITE0, // pixel 0
      RED0,   // pixel 0
      YELLOW1,// pixel 1
      RED1,   // pixel 1
      YELLOW2,// pixel 2
      BLUE2,  // pixel 2
      YELLOW3,// pixel 3
      GREEN4  // pixel 4
   };
   
   enum LightStates
   {
      OFF,
      ON,
      TOGGLE
   };

   LightStates CheckLightState(Lights byLight);
   void Main();
   void HandleStartSequence();
   void InitiateStartSequence();
   void WarningStartSequence();
   void ToggleLightState(Lights byLight, LightStates byLightState = TOGGLE);
   void ResetLights();
   void DeleteSchedules();
   void ToggleFaultLight(uint8_t iDogNumber, LightStates byLightState);
   void ReaceReadyFault(LightStates byLightState);
   void ToggleStartingSequence();

   stLightsState GetLightsState();

private:
   //Neopixel object
   //Adafruit_NeoPixel _LightsStrip;
   NeoPixelBus<NeoRgbFeature, WS_METHOD> *_LightsStrip;

   //This byte contains the combined states of all ligths at any given time
   byte _byCurrentLightsState = 255;
   byte _byNewLightsState = 0;

   unsigned long _lLightsOnSchedule[8];
   unsigned long _lLightsOutSchedule[8];

   Lights _byLightsArray[8] = {
      WHITE0, // 0
      RED0,   // 1
      YELLOW1,// 2
      RED1,   // 3
      YELLOW2,// 4
      BLUE2,  // 5
      YELLOW3,// 6
      GREEN4};// 7

   Lights _byDogErrorLigths[4] = {
      RED1,
      BLUE2,
      YELLOW3,
      GREEN4};

   struct SNeoPixelConfig
   {
      //uint32_t iColor;
      RgbColor iColor;
      uint8_t iPixelNumber;
   };
   SNeoPixelConfig _GetNeoPixelConfig(Lights byLight);
};

extern LightsControllerClass LightsController;

#endif
