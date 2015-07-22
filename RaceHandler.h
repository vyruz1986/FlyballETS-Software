// RaceHandler.h
#include <StreamPrint.h>
#ifndef _RACEHANDLER_h
#define _RACEHANDLER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

class RaceHandlerClass
{
 protected:


 public:
	void init(int iS1Pin, int iS2Pin);
   enum RaceStates {
      STOPPED,
      STARTING,
      RUNNING
   };
   RaceStates RaceState = STOPPED;
   RaceStates PreviousRaceState = STOPPED;

   uint8_t iCurrentDog;
   uint8_t iPreviousDog;
   
   void Main();
   void StartTimers();
   void StartRace();
   void StopRace();
   void ResetRace();
   void TriggerSensor1();
   void TriggerSensor2();

   enum DogFaults {
      OFF,
      ON,
      TOGGLE
   };
   void SetDogFault(int iDogNumber, DogFaults State = TOGGLE);

   double GetRaceTime();
   double GetDogTime(int iDogNumber);
   String GetCrossingTime(int iDogNumber);
   String GetRerunInfo(int iDogNumber);
   double GetTotalCrossingTime();

   String GetRaceStateString();
   unsigned long* lRaceStartTime = &_lRaceStartTime;

private:
   unsigned long _lRaceStartTime;
   unsigned long _lRaceEndTime;
   unsigned long _lRaceTime;
   unsigned long _lPerfectCrossingTime;

   int  _iS1Pin;
   struct STriggerRecord
   {
      volatile int iSensorNumber;
      volatile unsigned long lTriggerTime;
      volatile int iSensorState;
   };
   STriggerRecord _STriggerQueue[10];

   volatile uint8_t _iQueueReadIndex;
   volatile uint8_t _iQueueWriteIndex;
   int  _iS2Pin;

   bool _bFault;
   bool _bDogFaults[4];
   bool _bRerunBusy;
   uint8_t _iDogRunCounters[4];
   unsigned long _lLastDogTimeReturnTimeStamp[4];
   uint8_t _iLastReturnedRunNumber[4];
   unsigned long _lDogEnterTimes[4];
   unsigned long _lDogExitTimes[4];

   unsigned long _lDogTimes[4][4];
   long _lCrossingTimes[4][4];

   String _strTransition;
   
   enum _byDogStates {
      GOINGIN,
      COMINGBACK
   };
   _byDogStates _byDogState;
   bool _bGatesClear = false;

   void _ChangeRaceState(RaceStates _byNewRaceState);
   void _ChangeDogState(_byDogStates _byNewDogState);
   void _ChangeDogNumber(int _iNewDogNumber);
   void _QueuePush(STriggerRecord _InterruptTrigger);
   STriggerRecord _QueuePop();
   bool _QueueEmpty();
   void _AddToTransitionString(STriggerRecord _InterruptTrigger);
};

extern RaceHandlerClass RaceHandler;

#endif

