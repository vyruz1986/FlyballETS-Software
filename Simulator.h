#include <avr/pgmspace.h>
//#include "PROGMEM_readAnything.h"

// Simulator.h
#ifndef _SIMULATOR_h
#define _SIMULATOR_h

#if defined(ARDUINO) && ARDUINO >= 100
   #include "Arduino.h"
#else
   #include "WProgram.h"
#endif

class SimulatorClass
{
protected:


public:
   void init(int iS1Pin, int iS2Pin);
   void Main();


private:
   uint8_t _iS1Pin;
   uint8_t _iS2Pin;
   unsigned int _iDataPos;
   typedef struct SimulatorRecord
   {
      uint8_t iSensorNumber;
      unsigned long lTriggerTime;
      uint8_t iState;
   } SimulatorRecord;
   constexpr static SimulatorRecord SimulatorQueue[28] PROGMEM = {
      { 1, 41708, 1 }
      , { 2, 60692, 1 }
      , { 1, 176848, 0 }
      , { 2, 197732, 0 }
      , { 1, 4675580, 1 }
      , { 2, 4692252, 1 }
      , { 2, 4830180, 0 }
      , { 1, 4849032, 0 }
      , { 2, 9058416, 1 }
      , { 1, 9074780, 1 }
      , { 1, 9215868, 0 }
      , { 2, 9234968, 0 }
      , { 2, 13497276, 1 }
      , { 2, 13500924, 0 }
      , { 2, 13502728, 1 }
      , { 1, 13514992, 1 }
      , { 1, 13668000, 0 }
      , { 1, 13669344, 1 }
      , { 1, 13681828, 0 }
      , { 2, 13701464, 0 }
      , { 2, 18028788, 1 }
      , { 2, 18034252, 0 }
      , { 2, 18036540, 1 }
      , { 1, 18046892, 1 }
      , { 2, 18160312, 0 }
      , { 1, 18178224, 0 }
      , { 1, 20454580, 1 }
      , { 1, 20456368, 0 }
   };

   SimulatorRecord PendingRecord;
};

extern class SimulatorClass Simulator;

#endif

