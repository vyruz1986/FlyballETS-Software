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

      //For ESP32 we're using +12V--R33k--PIN--R10k--GND circuit, from equation Uo/10000 = (Ui - Uo)/33000 --> we have divider value Ui/Uo = 4.3
      //ESP32 has 12-bit ADC (0-4095), theoretically 3.3V = 4095 in analogRead value
      //Theoreticallly this voltage divider allows reading 0-14.19V (3.3V multplied by divider 4.3),
      //In order to measure true range I recommend to activate in conifg.h "BatteryCalibration" and build table
      //of true ready by ESP32 analogRead value for different power supply (lab power supply) points. In my case it was
      // power supply (V)   -->  analogRead from LCD
      //  9,5                       2497
      // 10,5                       2768
      // 11,5                       3071
      // 12,0                       3251
      // 12,5                       3456
      // 13,0                       3683
      // 13,5                       3952
      // 13,75                      4094
      //Theoretical voltage divider is 4.3, but true one based on comparision of
      //supply voltage and pin35/R10K voltage was in my case 4.3223
      //
      //For easy simplification of analogRead mapping to R10k/pin35 voltage linear function map() can be used,
      //but in practice characteristic is not linear so for more accurate values calculated function should be used.
      //I use Excel for that with first selecting best maching trend line and later calucate it using this fucntion
      //where y is range of cells (in one row, not column!) with values of R10k/pin35 voltage (supply voltage divided by 4.3223)
      //and x is range of cells (in one row, not column!) with analogRead values
      // equation: y = c2 * x^2 + c1 * x + b
      // c2: =INDEKS(REGLINP(y; x^{1;2});1)
      // c1: =INDEKS(REGLINP(y; x^{1;2});1;2)
      //  b: =INDEKS(REGLINP(y; x^{1;2});1;3)
      
      //First calculate voltage at ADC pin
      //int iPinVoltage = map(_iAverageBatteryReading, 958, 4095, 916, 3150);
      double dPinVoltage = (-0.00017784)*pow(_iAverageBatteryReading,2) + 1.783677*_iAverageBatteryReading - 1145.2;
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
   #if BatteryCalibration
      return _iAverageBatteryReading;
   #else
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
   #endif
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