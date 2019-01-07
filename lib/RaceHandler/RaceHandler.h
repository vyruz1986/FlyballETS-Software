// RaceHandler.h
#include "Structs.h"
#ifndef _RACEHANDLER_h
#define _RACEHANDLER_h

#ifdef ESP32
#define GET_MICROS esp_timer_get_time()
#elif
#define GET_MICROS micros()
#endif

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#define NUM_HISTORIC_RACE_RECORDS 100

class RaceHandlerClass
{
   friend class SimulatorClass;
   friend class WebHandlerClass;
 protected:


 public:
	void init(uint8_t iS1Pin, uint8_t iS2Pin);
   enum RaceStates {
      STOPPED,
      STARTING,
      RUNNING
   };
   RaceStates RaceState = STOPPED;
   RaceStates PreviousRaceState = STOPPED;

   uint8_t iCurrentDog;
   uint8_t iPreviousDog;
   uint8_t iNextDog;
   
   void Main();
   void StartTimers();
   void StartRace();
   void StopRace();
   void StopRace(unsigned long lStopTime);
   void ResetRace();
   void TriggerSensor1();
   void TriggerSensor2();

   enum DogFaults {
      OFF,
      ON,
      TOGGLE
   };
   void SetDogFault(uint8_t iDogNumber, DogFaults State = TOGGLE);

   double GetRaceTime();
   double GetDogTime(uint8_t iDogNumber, int8_t iRunNumber = -1);
   unsigned long GetDogTimeMillis(uint8_t iDogNumber, int8_t iRunNumber = -1);
   String GetCrossingTime(uint8_t iDogNumber, int8_t iRunNumber = -1);
   unsigned long GetCrossingTimeMillis(uint8_t iDogNumber, int8_t iRunNumber = -1);
   String GetRerunInfo(uint8_t iDogNumber);
   long GetTotalCrossingTimeMillis();
   double GetTotalCrossingTime();

   String GetRaceStateString();

   stRaceData GetRaceData();
   stRaceData GetRaceData(uint iRaceId);
   void ToggleRunDirection();
   boolean GetRunDirection();

private:
   unsigned long _lRaceStartTime;
   unsigned long _lRaceEndTime;
   unsigned long _lRaceTime;
   unsigned long _lPerfectCrossingTime;
   unsigned long _lLastTransitionStringUpdate;

   uint8_t  _iS1Pin;
   uint8_t  _iS2Pin;
   boolean _bRunDirectionInverted = false;

   struct STriggerRecord
   {
      volatile uint8_t iSensorNumber;
      volatile unsigned long lTriggerTime;
      volatile int iSensorState;
   };
#define TRIGGER_QUEUE_LENGTH 50
   STriggerRecord _STriggerQueue[TRIGGER_QUEUE_LENGTH];

   volatile uint8_t _iQueueReadIndex;
   volatile uint8_t _iQueueWriteIndex;

   bool _bFault;
   bool _bDogFaults[4];
   bool _bRerunBusy;
   uint8_t _iDogRunCounters[4];  //Number of (re-)runs for each dog
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

   stRaceData _HistoricRaceData[NUM_HISTORIC_RACE_RECORDS];
   uint _iCurrentRaceId;

   void _ChangeRaceState(RaceStates _byNewRaceState);
   void _ChangeDogState(_byDogStates _byNewDogState);
   void _ChangeDogNumber(uint8_t _iNewDogNumber);
   void _QueuePush(STriggerRecord _InterruptTrigger);
   STriggerRecord _QueuePop();
   bool _QueueEmpty();
   void _AddToTransitionString(STriggerRecord _InterruptTrigger);
};

extern RaceHandlerClass RaceHandler;

#endif

