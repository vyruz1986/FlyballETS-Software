//#include <avr/pgmspace.h>
//#include "PROGMEM_readAnything.h"

// Simulator.h
#ifndef _SIMULATOR_h
#define _SIMULATOR_h

#include "config.h"

#if defined(ARDUINO) && ARDUINO >= 100
   #include "Arduino.h"
#else
   #include "WProgram.h"
#endif

class SimulatorClass
{
protected:


public:
   void init(uint8_t iS1Pin, uint8_t iS2Pin);
   void ChangeSimulatedRaceID(unsigned long iSimulatedRaceID);
   void Main();


private:
   uint8_t _iS1Pin;
   uint8_t _iS2Pin;
   unsigned int _iDataPos;
   unsigned int _iDataStartPos;
   typedef struct SimulatorRecord
   {
      uint8_t iSensorNumber;
      long lTriggerTime;
      uint8_t iState;
   } SimulatorRecord;
   static const SimulatorRecord SimulatorQueue[60 * NumSimulatedRaces] PROGMEM;

   SimulatorRecord PendingRecord;
};

extern class SimulatorClass Simulator;

#endif

