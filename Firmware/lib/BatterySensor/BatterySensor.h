// BatterySensor.h
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

#ifndef _BATTERYSENSOR_h
#define _BATTERYSENSOR_h

#include "Arduino.h"

class BatterySensorClass
{
protected:

public:
   void init(uint8_t iBatterySensorPin);
   void CheckBatteryVoltage();
   uint16_t GetBatteryVoltage();
   uint16_t GetBatteryPercentage();
   uint16_t GetLastAnalogRead();
   
private:
   uint8_t _iBatterySensorPin;

   int _iBatteryReadings[10];
   int _iNumberOfBatteryReadings = 0;
   uint16_t _iBatteryVoltage = 0;
   uint16_t _iAverageBatteryReading;
   
};

extern BatterySensorClass BatterySensor;

#endif

