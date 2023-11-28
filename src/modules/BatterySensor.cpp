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
      //of true ready by ESP32 analogRead value for different power supply (lab power supply) points. Example of mapping:
      // power supply (V)   -->  analogRead from LCD
      // 4.979                   1231
      // 8.978                   2370  
      // 9.477                   2512
      // 10.478                  2806   
      // 10.676                  2865
      // 10.878                  2928
      // 11.080                  2990
      // 11.578                  3156
      // 11.782                  3230
      // 11.976                  3300
      // 12.278                  3422
      // 12.577                  3550
      //Theoretical voltage divider is 4.3, but true one based on comparision of
      //supply voltage and pin35/R10K voltage was (with load) 4.3172
      //
      //For easy simplification of analogRead mapping to R10k/pin35 voltage linear function map() can be used,
      //but in practice characteristic is not linear so for more accurate values calculated function should be used.
      //I use Excel for that with first selecting best maching trend line and later calucate it using this fucntion
      //where y is range of cells (in one row, not column!) with values of R10k/pin35 voltage (supply voltage divided by 4.3172)
      //and x is range of cells (in one row, not column!) with analogRead values
      // equation: y = c2 * x^2 + c1 * x + b
      // c2: =INDEKS(REGLINP(y; x^{1;2});1)
      // c1: =INDEKS(REGLINP(y; x^{1;2});1;2)
      //  b: =INDEKS(REGLINP(y; x^{1;2});1;3)

      //First calculate voltage at ADC pin
      //int iPinVoltage = map(_iAverageBatteryReading, 958, 4095, 916, 3150);
      double dPinVoltage = (-0.00012493) * pow(_iAverageBatteryReading, 2) + 1.4559 * _iAverageBatteryReading - 671.7;
      int iPinVoltage = dPinVoltage;
      _iBatteryVoltage = iPinVoltage * 4.3172;
      _iNumberOfBatteryReadings = 0;
      //log_d("_iBatteryVoltage: %i", _iBatteryVoltage);
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
///   Gets battery percentage or analog pin read if calibrqtion mode.
///   Assumed working range is 10.5V - 12.3V what is save for 3S2P and 3S4P li-ion batteries

/// </summary>
///
/// <returns>
///   The battery percentage or analog pin read or special tags.
///   Tag 0    - when voltage is below 10.5V and above 10V "LOW" will be displayed
///   Tag 9911 - when voltage is below 5V or above 60V (ESP32 standalone) it would be assumed
///              that ESP32 is powered by USB
///   Tag 9999 - when voltage is below 10V (and above 5V) we are close to danger li-ion 9V so
///               "!!!" will be displayed and ESP32 turn into deep sleep mode 
/// </returns>
uint16_t BatterySensorClass::GetBatteryPercentage()
{
#if BatteryCalibration
   return _iAverageBatteryReading;
#else
   if (_iBatteryVoltage < 5000 || _iBatteryVoltage >= 60000)
   {
      return 9911;
   }
   else if (_iBatteryVoltage >= 5000 && _iBatteryVoltage < 10000)
   {
      return 9999;
   }
   else if (_iBatteryVoltage >= 10000 && _iBatteryVoltage < 10500)
   {
      return 0;
   }
   else if (_iBatteryVoltage > 12250 || _iBatteryVoltage < 60000)
   {
      return 100;
   }
   else
   {
      uint16_t iBatteryPercentage = map(_iBatteryVoltage, 10500, 12250, 1, 100);
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