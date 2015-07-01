// Simulator.h
#include <StandardCplusplus.h>
#include <serstream>
#include <string>
#include <vector>
#include <iterator>
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
      struct SimulatorRecord
      {
         int iSensorNumber;
         unsigned long lTriggerTime;
         int iState;
      };
      std::vector<SimulatorRecord> SimulatorQueue;

      SimulatorRecord PendingRecord;
};

extern class SimulatorClass Simulator;

#endif

