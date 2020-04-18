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

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include <LiquidCrystal.h>
class LCDControllerClass
{
protected:
public:
   void init(LiquidCrystal *Clcd1, LiquidCrystal *Clcd2);
   void Main();
   enum LCDFields
   {
      D1Time,
      D1RerunInfo,
      D2Time,
      D2RerunInfo,
      D3Time,
      D3RerunInfo,
      D4Time,
      D4RerunInfo,
      D1CrossTime,
      D2CrossTime,
      D3CrossTime,
      D4CrossTime,
      RaceState,
      BattLevel,
      TeamTime,
      CleanTime,
      BoxDirection
   };

   void UpdateField(LCDFields lcdfieldField, String strNewValue);

private:
   void _UpdateLCD(int iLine, int iPosition, String strText, int iFieldLength);
   LiquidCrystal *_Clcd1;
   LiquidCrystal *_Clcd2;
   unsigned long _lLastLCDUpdate = 0;
   unsigned int _iLCDUpdateInterval = 500; //500ms update interval

   struct SLCDField
   {
      int iLine;
      int iStartingPosition;
      int iFieldLength;
      String strText;
   };
   struct SLCDField _SlcdfieldFields[17];
};

extern LCDControllerClass LCDController;

#endif
