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
#ifdef WS281x

#endif // WS281x

class LightsControllerClass
{
protected:
public:
#ifdef WS281x
   void init(NeoPixelBus<NeoRgbFeature, WS_METHOD> *LightsStrip);
#else
   void init(uint8_t iLatchPin, uint8_t iClockPin, uint8_t iDataPin);
#endif
   //Overal state of this class
   enum OverallStates
   {
      RESET,
      STARTING,
      STARTED
   };
   OverallStates byOverallState = RESET;

   //Decimal values of lights connected to 74HC595
   enum Lights
   {
      WHITE = 130,  //74HC595 QH (128) + QB (2). I made a boo-boo on my prototype, WHITE should be wired to QB.
      RED = 64,     //74HC595 QG
      YELLOW1 = 32, //74HC595 QF
      BLUE = 16,    //74HC595 QE
      YELLOW2 = 8,  //74HC595 QD
      GREEN = 4     //74HC595 QC
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
   void ToggleLightState(Lights byLight, LightStates byLightState = TOGGLE);
   void ResetLights();
   void DeleteSchedules();
   void ToggleFaultLight(uint8_t iDogNumber, LightStates byLightState);
   void ReaceReadyFault(LightStates byLightState);

   stLightsState GetLightsState();

private:
#ifdef WS281x
   //Neopixel object
   //Adafruit_NeoPixel _LightsStrip;
   NeoPixelBus<NeoRgbFeature, WS_METHOD> *_LightsStrip;
#else
   //Pin connected to ST_CP of 74HC595
   uint8_t _iLatchPin = 12;
   //Pin connected to SH_CP of 74HC595
   uint8_t _iClockPin = 13;
   //Pin connected to DS of 74HC595
   uint8_t _iDataPin = 11;
#endif // WS281x

   //This byte contains the combined states of all ligths at any given time
   byte _byCurrentLightsState = 255;
   byte _byNewLightsState = 0;

   bool _bStartSequenceStarted = 0;

   long long _llLightsOnSchedule[6];
   long long _llLightsOutSchedule[6];

   Lights _byLightsArray[6] = {
      WHITE,
      RED,
      YELLOW1,
      BLUE,
      YELLOW2,
      GREEN};

   Lights _byDogErrorLigths[4] = {
      RED,
      BLUE,
      YELLOW2,
      GREEN};

#ifdef WS281x
   struct SNeoPixelConfig
   {
      //uint32_t iColor;
      RgbColor iColor;
      uint8_t iPixelNumber;
   };
   SNeoPixelConfig _GetNeoPixelConfig(Lights byLight);
#endif // WS281x
};

extern LightsControllerClass LightsController;

#endif
