#include <StandardCplusplus.h>
#include <serstream>
#include <string>
#include <vector>
#include <iterator>
#include "Simulator.h"
#include "global.h"
#include "RaceHandler.h"

void SimulatorClass::init(int iS1Pin, int iS2Pin)
{
   _iS1Pin = iS1Pin;
   pinMode(_iS1Pin, OUTPUT);
   digitalWrite(_iS1Pin, LOW);
   _iS2Pin = iS2Pin;
   pinMode(_iS2Pin, OUTPUT);
   digitalWrite(_iS2Pin, LOW);

   SimulatorQueue.push_back({ 1, 59652, 1 });
   SimulatorQueue.push_back({ 2, 77600, 1 });
   SimulatorQueue.push_back({ 1, 167584, 1 });
   SimulatorQueue.push_back({ 1, 184580, 0 });
   SimulatorQueue.push_back({ 2, 197480, 0 });
   SimulatorQueue.push_back({ 2, 4185496, 1 });
   SimulatorQueue.push_back({ 1, 4196104, 1 });
   SimulatorQueue.push_back({ 2, 4314804, 0 });
   SimulatorQueue.push_back({ 1, 4321668, 0 });
   SimulatorQueue.push_back({ 1, 4343980, 1 });
   SimulatorQueue.push_back({ 2, 4364040, 1 });
   SimulatorQueue.push_back({ 1, 4462872, 0 });
   SimulatorQueue.push_back({ 2, 4479716, 0 });
   SimulatorQueue.push_back({ 2, 8386584, 1 });
   SimulatorQueue.push_back({ 1, 8403856, 1 });
   SimulatorQueue.push_back({ 1, 8592560, 0 });
   SimulatorQueue.push_back({ 2, 8611116, 0 });
   SimulatorQueue.push_back({ 1, 13270968, 1 });
   SimulatorQueue.push_back({ 2, 13284460, 1 });
   SimulatorQueue.push_back({ 2, 13490444, 0 });
   SimulatorQueue.push_back({ 1, 13509776, 0 });
   SimulatorQueue.push_back({ 2, 17602856, 1 });
   SimulatorQueue.push_back({ 1, 17621064, 1 });
   SimulatorQueue.push_back({ 2, 17705768, 0 });
   SimulatorQueue.push_back({ 2, 17714540, 1 });
   SimulatorQueue.push_back({ 2, 17725920, 0 });
   SimulatorQueue.push_back({ 1, 17763264, 0 });
}

void SimulatorClass::Main()
{
   if (RaceHandler.RaceState != RaceHandler.RUNNING)
   {
      return;
   }
   if (RaceHandler.RaceState == RaceHandler.STARTING)
   {
      //We're starting a new race, reset data position to 0
      _iDataPos = 0;
      PendingRecord = SimulatorQueue[_iDataPos];
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
      PendingRecord = SimulatorQueue[_iDataPos];
   }
}

SimulatorClass Simulator;
