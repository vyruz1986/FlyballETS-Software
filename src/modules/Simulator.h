// Simulator.h
#ifndef _SIMULATOR_h
#define _SIMULATOR_h

#include "config.h"
#include "Arduino.h"

class SimulatorClass
{
protected:
public:
   void init();
   void ChangeSimulatedRaceID(uint iSimulatedRaceID);
   void Main();

private:
   unsigned int _iDataPos;
   unsigned int _iDataStartPos;
   typedef struct SimulatorRecord
   {
      uint8_t iSimSensorNumber;
      long long llSimTriggerTime;
      uint8_t iSimState;
   } SimulatorRecord;
   static const SimulatorRecord SimulatorQueue[60 * NumSimulatedRaces] PROGMEM;

   SimulatorRecord PendingRecord;
};

extern class SimulatorClass Simulator;

#endif
