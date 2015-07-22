#include "Simulator.h"
#include "global.h"
#include "RaceHandler.h"
#include "PROGMEM_readAnything.h"
#include <avr/pgmspace.h>

void SimulatorClass::init(int iS1Pin, int iS2Pin)
{
   _iS1Pin = iS1Pin;
   pinMode(_iS1Pin, OUTPUT);
   digitalWrite(_iS1Pin, LOW);
   _iS2Pin = iS2Pin;
   pinMode(_iS2Pin, OUTPUT);
   digitalWrite(_iS2Pin, LOW);
}

void SimulatorClass::Main()
{
   SimulatorRecord PendingRecord;
   if (RaceHandler.RaceState == RaceHandler.STOPPED)
   {
      return;
   }
   if (RaceHandler.RaceState == RaceHandler.STARTING)
   {
      //We're starting a new race, reset data position to 0
      _iDataPos = 0;
      //memcpy_P(&PendingRecord, &SimulatorQueue[_iDataPos], sizeof(PendingRecord));
      PROGMEM_readAnything(&SimulatorQueue[_iDataPos], PendingRecord);
   }
   
   unsigned long lRaceElapsedTime = micros() - *RaceHandler.lRaceStartTime;
   int iSimPin;

   //Simulate sensor
   if (PendingRecord.lTriggerTime <= lRaceElapsedTime)
   {
      if (PendingRecord.iSensorNumber == 1)
      {
         iSimPin = _iS1Pin;
      }
      else if (PendingRecord.iSensorNumber == 2)
      {
         iSimPin = _iS2Pin;
      }
      //simulate signal
      digitalWrite(iSimPin, PendingRecord.iState);

      //And increase pending record
      _iDataPos++;
      //memcpy_P(&PendingRecord, &SimulatorQueue[_iDataPos], sizeof(PendingRecord));
      PROGMEM_readAnything(&SimulatorQueue[_iDataPos], PendingRecord);
   }
}

SimulatorClass Simulator;
