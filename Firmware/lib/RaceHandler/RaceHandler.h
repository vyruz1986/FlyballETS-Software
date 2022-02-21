// RaceHandler.h
#ifndef _RACEHANDLER_h
#define _RACEHANDLER_h

#include "Structs.h"
#include "Arduino.h"


#define NUM_HISTORIC_RACE_RECORDS 100

class RaceHandlerClass
{
   friend class SimulatorClass;
   friend class WebHandlerClass;
protected:


public:
	void init(uint8_t iS1Pin, uint8_t iS2Pin);
   enum RaceStates {
      RESET,
      STARTING,
      RUNNING,
      STOPPED
   };
   RaceStates RaceState = RESET;
   RaceStates PreviousRaceState = RESET;

   uint8_t iCurrentDog;
   uint8_t iPreviousDog;
   uint8_t iNextDog;
   uint8_t iNumberOfRacingDogs = 4;
   uint8_t iDogRunCounters[4];  //Number of (re-)runs for each dog
   long long llRaceStartTime;
   int iCurrentRaceId = -1;
   char* cRaceStartTimestamp;
   
   void Main();
   void ChangeRaceStateToRunning();
   void StartRaceTimer();
   void StopRace();
   void StopRace(long long llStopTime);
   void ResetRace();
   void PrintRaceTriggerRecords();
   void PrintRaceTriggerRecordsToFile();
   void TriggerSensor1();
   void TriggerSensor2();

   enum DogFaults {
      OFF,
      ON,
      TOGGLE
   };
   void SetDogFault(uint8_t iDogNumber, DogFaults State = TOGGLE);

   double GetRaceTime();
   String GetDogTime(uint8_t iDogNumber, int8_t iRunNumber = -1);
   String GetStoredDogTimes(uint8_t iDogNumber, int8_t iRunNumber = -1);
   int8_t SelectRunNumber(uint8_t iDogNumber, int8_t iRunNumber = -1);
   String GetCrossingTime(uint8_t iDogNumber, int8_t iRunNumber = -1);
   String TransformCrossingTime(uint8_t iDogNumber, int8_t iRunNumber, boolean bToFile = false);
   String GetRerunInfo(uint8_t iDogNumber);
   double GetNetTime();

   String GetRaceStateString();

   stRaceData GetRaceData();
   stRaceData GetRaceData(int iRaceId);
   boolean GetRunDirection();
   void ToggleRunDirection();
   void ToggleNumberOfDogs();


private:
   long long _llRaceEndTime;
   long long _llRaceTime;
   long long _llLastDogExitTime;
   long long _llS2CrossedSafeTime;
   long long _llS2CrossedUnsafeTime;
   long long _llLastTransitionStringUpdate;
   long long _llRaceElapsedTime;

   uint8_t  _iS1Pin;
   uint8_t  _iS2Pin;
   boolean _bRunDirectionInverted = false;
   boolean _bNextDogFound = false;

   struct STriggerRecord
   {
      volatile int iSensorNumber;
      volatile long long llTriggerTime;
      volatile int iSensorState;
   };
   
   #define TRIGGER_QUEUE_LENGTH 60
   STriggerRecord _InputTriggerQueue[TRIGGER_QUEUE_LENGTH];
   STriggerRecord _OutputTriggerQueue[TRIGGER_QUEUE_LENGTH];

   volatile uint8_t _iOutputQueueReadIndex = 0;
   volatile uint8_t _iInputQueueReadIndex = 0;
   volatile uint8_t _iOutputQueueWriteIndex = 0;
   volatile uint8_t _iInputQueueWriteIndex = 0;

   bool _bFault;
   bool _bDogFaults[4];
   bool _bDogManualFaults[4];
   bool _bDogPerfectCross[4][4];
   bool _bDogBigOK[4][4];
   bool _bDogSmallok[4][4];
   bool _bDogMissedGateGoingin[4][4];
   bool _bDogMissedGateComingback[4][4];
   bool _bRerunBusy;
   bool _bS1isSafe;
   bool _bS1StillSafe;
   bool _bNegativeCrossDetected;
   bool _bPotentialNegativeCrossDetected;
   long long _llLastDogTimeReturnTimeStamp[4];
   uint8_t _iLastReturnedRunNumber[4];
   long long _llDogEnterTimes[4];
   long long _llDogExitTimes[4];
   long long _llDogTimes[4][4];
   long long _llCrossingTimes[4][4];

   String _strTransition;
   
   enum _byDogStates {
      GOINGIN,
      COMINGBACK
   };
   _byDogStates _byDogState;
   bool _bGatesClear = true;

   stRaceData _HistoricRaceData[NUM_HISTORIC_RACE_RECORDS];

   void _ChangeRaceState(RaceStates _byNewRaceState);
   void _ChangeDogState(_byDogStates _byNewDogState);
   void _ChangeDogNumber(uint8_t _iNewDogNumber);
   void _QueuePush(STriggerRecord _InterruptTrigger);
   void _QueueFilter();
   STriggerRecord _QueuePop();
   bool _QueueEmpty();
   void _AddToTransitionString(STriggerRecord _InterruptTrigger);
};

extern RaceHandlerClass RaceHandler;

#endif
