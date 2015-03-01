// 
// 
// 

#include "BatterySensor.h"

void BatterySensorClass::init(int iBatterySensorPin)
{
   pinMode(iBatterySensorPin, INPUT);
   _iBatterySensorPin = iBatterySensorPin;
   _iNumberOfBatteryReadings = 0;
}

void BatterySensorClass::CheckBatteryVoltage()
{
   if (_iNumberOfBatteryReadings < 10)
   {
      _iBatteryReadings[_iNumberOfBatteryReadings] = _iRawBatteryLevel = analogRead(_iBatterySensorPin);
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
      _fBatteryVoltage = fMeasuredVoltage * 2.5;
      _iNumberOfBatteryReadings = 0;
   }
}

float BatterySensorClass::GetBatteryVoltage()
{
   return _fBatteryVoltage;
}

BatterySensorClass BatterySensor;