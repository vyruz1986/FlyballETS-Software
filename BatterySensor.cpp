// file:	BatterySensor.cpp
//
// summary:	Implements the battery sensor class
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
#include "BatterySensor.h"
#include "global.h"

/// <summary>
///   Initialises this object.
/// </summary>
///
/// <param name="iBatterySensorPin">   Zero-based index of the battery sensor pin. </param>
void BatterySensorClass::init(uint8_t iBatterySensorPin)
{
   pinMode(iBatterySensorPin, INPUT);
   _iBatterySensorPin = iBatterySensorPin;
   _iNumberOfBatteryReadings = 0;
}

/// <summary>
///   Will check the current battery voltage. 10 readings will be taken to smooth out fluctuations.
/// </summary>
void BatterySensorClass::CheckBatteryVoltage()
{
   if (_iNumberOfBatteryReadings < 10)
   {
      _iBatteryReadings[_iNumberOfBatteryReadings] = analogRead(_iBatterySensorPin);
      _iNumberOfBatteryReadings++;
   }
   else
   {
      int iBatteryReadingsTotal = 0;
      for (int i = 0; i < _iNumberOfBatteryReadings; i++)
      {
         iBatteryReadingsTotal = iBatteryReadingsTotal + _iBatteryReadings[i];
      }
      int iAverageBatteryReading = iBatteryReadingsTotal / _iNumberOfBatteryReadings;
      float fMeasuredVoltage = iAverageBatteryReading * 0.0048828125;
      _iBatteryVoltage = fMeasuredVoltage * 2.5 * 100;
      _iNumberOfBatteryReadings = 0;
   }
}

/// <summary>
///   Gets battery voltage.
/// </summary>
///
/// <returns>
///   The battery voltage.
/// </returns>
uint16_t BatterySensorClass::GetBatteryVoltage()
{
   return _iBatteryVoltage;
}

/// <summary>
///   Gets battery percentage.
/// </summary>
///
/// <returns>
///   The battery percentage.
/// </returns>
uint16_t BatterySensorClass::GetBatteryPercentage()
{
   uint16_t iBatteryPercentage = map(_iBatteryVoltage, 960, 1260, 0, 100);
   return iBatteryPercentage;
}

/// <summary>
///   The battery sensor.
/// </summary>
BatterySensorClass BatterySensor;