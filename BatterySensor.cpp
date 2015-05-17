#include "BatterySensor.h"
#include "global.h"

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
      _fBatteryVoltage = fMeasuredVoltage * 2.5 * 100;
      _iNumberOfBatteryReadings = 0;
   }
}

uint16_t BatterySensorClass::GetBatteryVoltage()
{
   return _fBatteryVoltage;
}

uint16_t BatterySensorClass::GetBatteryPercentage()
{
   uint16_t iBatteryPercentage = map(_fBatteryVoltage, 960, 1260, 0, 100);
   return iBatteryPercentage;
}

BatterySensorClass BatterySensor;