// LCDController.h
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

#ifndef _LCDCONTROLLER_h
#define _LCDCONTROLLER_h

#include <Arduino.h>
#include <LiquidCrystal.h>
#include "LightsController.h"
#include "RaceHandler.h"
#include "BatterySensor.h"

class LCDControllerClass
{
protected:
public:
   uint16_t iLCDUpdateInterval = 451; // 451ms is LCD update interval
   bool bUpdateTimerLCDdata = false;
   bool bExecuteLCDUpdate;
   bool bUpdateNonTimerLCDdata = false;
   void init(LiquidCrystal *Clcd1, LiquidCrystal *Clcd2);
   void reInit();
   void FirmwareUpdateInit();
   void FirmwareUpdateProgress(String strNewValue);
   void FirmwareUpdateSuccess();
   void FirmwareUpdateError();
   void UpdateNumberOfDogsOnLCD(uint8_t iNumberOfDogs);
   void Main();
   enum LCDFields
   {
      D1Time,      // 0
      D2Time,      // 1
      D3Time,      // 2
      D4Time,      // 3
      D1CrossTime, // 4
      D2CrossTime, // 5
      D3CrossTime, // 6
      D4CrossTime, // 7
      D1RerunInfo, // 8
      D2RerunInfo, // 9
      D3RerunInfo, // 10
      D4RerunInfo, // 11
      TeamTime,    // 12
      CleanTime,   // 13
      RaceState,   // 14
      RaceID,      // 15
      BattLevel,   // 16
      WifiState,   // 17
      GpsState,    // 18
      SDcardState, // 19
      BoxDirection // 20
   };
   volatile bool bUpdateThisLCDField[21];

   void UpdateField(LCDFields lcdfieldField, String strNewValue);

private:
   void _UpdateLCD(int iLine, int iPosition, String strText, int iFieldLength);
   void _HandleLCDUpdates();
   LiquidCrystal *_Clcd1;
   LiquidCrystal *_Clcd2;
   unsigned long _ulLastLCDUpdate = 0;
   unsigned long _ulLastLCDUpdateWithTimes = 0;
   long long llLastBatteryLCDupdate = -25000; // Initial offset for battery value upate on LCD
   bool bLCDRefresh = false;

   struct SLCDField
   {
      bool bUpdateFlag;
      uint8_t iLine;
      uint8_t iStartingPosition;
      uint8_t iFieldLength;
      String strText;
   };
   struct SLCDField _SlcdfieldFields[21];
};

extern LCDControllerClass LCDController;

#endif
