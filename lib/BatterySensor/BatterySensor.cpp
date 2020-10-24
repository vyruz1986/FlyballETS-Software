// file:	BatterySensor.cpp
//
// summary:	Implements the battery sensor class
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
#include "BatterySensor.h"
#include "config.h"

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
      _iAverageBatteryReading = iBatteryReadingsTotal / _iNumberOfBatteryReadings;

      //For ESP32 we're using +12V--R33k--PIN--R10k--GND circuit so divider is 3.3
      //ESP32 has 12-bit ADC (0-4095), theoretically 3.3V = 4095 in analogRead value
      //Theoreticallly this voltage divider allows reading 0-14.19V,
      //In practice 4095 analogRead is for 3.15V measured on R10k what correspond with 13,618V of battery
      //Minimum measurable value (ESP32 powered by USB) is 3.96V (battery) what correspond with 0,916V on R10k
      //I use 12.6V battery with operating range 10.8V-12.6V, but I assume OK range as 10.9V(0%)-12.6V(100%)
      //
      // battery     -->   R10k/pin35    --> analogRead   -> battery %
      //  3.961V              0.9164V               958
      //  9.901V              2.2915V              2638
      // 10.899V              2.5215V              2934            ~0%
      // 12.601V              2.9154V              3572          ~100%
      // 13.618V              3.1500V              4095
      //
      //Theoretical voltage multplier would be 14.19/3.3=4.3, but calculated based on measurements is 4.3223
      //For easy mapping of analogRead to R10k/pin35 voltage function map() can be used,
      //but in practice characteristic is not linear so for more accurate values calculated function should be used
      //I read equation from trend line of Excel X-Y chart for 4 highest measurements from the table above
      //(9.901V measurement was done explicitly for this purpose)
      
      //First calculate voltage at ADC pin
      //int iPinVoltage = map(_iAverageBatteryReading, 958, 4095, 916, 3150);
      double dPinVoltage = (-0.0001525)*pow(_iAverageBatteryReading,2) + 1.6155*_iAverageBatteryReading - 907.6;
      int iPinVoltage = dPinVoltage;
      _iBatteryVoltage = iPinVoltage * 4.3223;
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
   if (_iBatteryVoltage < 10900)
   {
      return 0;
   }
   else if (_iBatteryVoltage > 12460)
   {
      return 100;
   }
   else
   {
      uint16_t iBatteryPercentage = map(_iBatteryVoltage, 10900, 12460, 1, 100);
      return iBatteryPercentage;
   }
}

/// <summary>
///   Gets the last analogRead value from battery sense pin.
/// </summary>
///
/// <returns>
///   A value from 0-4095
/// </returns>
uint16_t BatterySensorClass::GetLastAnalogRead()
{
   return _iAverageBatteryReading;
}

/// <summary>
///   The battery sensor.
/// </summary>
BatterySensorClass BatterySensor;