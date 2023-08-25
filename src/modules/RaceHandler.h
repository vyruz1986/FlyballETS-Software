// RaceHandler.h
#ifndef _RACEHANDLER_h
#define _RACEHANDLER_h

#include "Structs.h"
#include "Arduino.h"
#include "config.h"
#if Simulate
#include "Simulator.h"
#endif

class RaceHandlerClass
{
   friend class SimulatorClass;
   friend class WebHandlerClass;

protected:
public:
   void init(uint8_t iS1Pin, uint8_t iS2Pin);
   enum RaceStates
   {
      RESET,
      STARTING,
      RUNNING,
      STOPPED
   };
   RaceStates RaceState = RESET;
   String strRaceState;

   uint8_t iCurrentDog;
   uint8_t iPreviousDog;
   uint8_t iNextDog = 1;
   uint8_t iNumberOfRacingDogs = 4;
   uint8_t iDogRunCounters[4]; // Number of (re-)runs for each dog
   long long llRaceStartTime;
   uint16_t iCurrentRaceId = 0;
   char *cRaceStartTimestamp;
   bool bRerunsOff = false;
   bool bRunDirectionInverted = false;
   bool bIgnoreSensors = false;
   volatile bool bExecuteStopRace;
   volatile bool bExecuteResetRace;
   volatile bool bExecuteStartRaceTimer;

   void Main();
   void StartRaceTimer();
   void StopRace();
   void StopRace(long long llStopTime);
   void ResetRace();
   void IRAM_ATTR TriggerSensor1(portMUX_TYPE *spinlock);
   void IRAM_ATTR TriggerSensor2(portMUX_TYPE *spinlock);

   enum DogFaults
   {
      OFF,
      ON,
      TOGGLE
   };
   void SetDogFault(uint8_t iDogNumber, DogFaults State = TOGGLE);

   String GetRaceTime();
   String GetDogTime(uint8_t iDogNumber, int8_t iRunNumber = -1);
   String GetStoredDogTimes(uint8_t iDogNumber, int8_t iRunNumber = -1);
   int8_t SelectRunNumber(uint8_t iDogNumber, int8_t iRunNumber = -1);
   String GetCrossingTime(uint8_t iDogNumber, int8_t iRunNumber = -1);
   String TransformCrossingTime(uint8_t iDogNumber, int8_t iRunNumber, bool bToFile = false);
   String GetRerunInfo(uint8_t iDogNumber, int8_t iRunNumber = -1);
   String GetCleanTime();

   void ToggleRunDirection();
   void ToggleAccuracy();
   void ToggleRerunsOffOn(uint8_t _iState);
   void SetNumberOfDogs(uint8_t _iNumberOfRacingDogs);

private:
   long long _llRaceTime;
   long long _llRaceEndTime;
   long long _llLastDogExitTime;
   long long _llS2CrossedSafeTime;
   long long _llS2CrossedUnsafeTriggerTime;
   long long _llS2CrossedUnsafeGetMicrosTime;
   long long _llLastTransitionStringUpdate;
   long long _llGatesClearedTime;
   long long _llRaceElapsedTime;

   uint8_t _iS1Pin;
   uint8_t _iS2Pin;

   struct STriggerRecord
   {
      volatile int iSensorNumber;
      volatile long long llTriggerTime;
      volatile int iSensorState;
   };

   STriggerRecord _InputTriggerQueue[TRIGGER_QUEUE_LENGTH];
   STriggerRecord _OutputTriggerQueue[TRIGGER_QUEUE_LENGTH];

   volatile uint8_t _iOutputQueueReadIndex = 0;
   volatile uint8_t _iInputQueueReadIndex = 0;
   volatile uint8_t _iOutputQueueWriteIndex = 0;
   volatile uint8_t _iInputQueueWriteIndex = 0;

   bool _bAccuracy3digits = false;
   bool _bNextDogFound = false;
   bool _bSensorNoise = false;
   bool _bLastStringBAba = false;
   bool _bNoValidCleanTime = false;
   bool _bFault;
   bool _bDogFaults[5];
   bool _bDogManualFaults[4];
   bool _bDogDetectedFaults[5][4];
   bool _bDogPerfectCross[5][4];
   bool _bDogBigOK[5][4];
   bool _bDogSmallok[5][4];
   bool _bDogFakeTime[4][4];
   bool _bDogMissedGateGoingin[4][4];
   bool _bDogMissedGateComingback[4][4];
   bool _bRerunBusy;
   bool _bS1isSafe;
   bool _bS1StillSafe;
   bool _bNegativeCrossDetected;
   bool _bPotentialNegativeCrossDetected;
   bool _bRaceSummaryPrinted = false; // race summary printed indicator
   bool _bWrongRunDirectionDetected = false;
   long long _llLastDogTimeReturnTimeStamp[4];
   int8_t _iLastReturnedRunNumber[4];
   long long _llDogEnterTimes[5];
   long long _llDogExitTimes[4];
   long long _llDogTimes[4][4];
   long long _llCrossingTimes[5][4];

   String _strTransition;
   String _strPreviousTransitionFirstLetter = ""; // fix for simulated race 18-41

   enum _byDogStates
   {
      GOINGIN,
      COMINGBACK
   };
   _byDogStates _byDogState;
   bool _bGatesClear = true;

   void _ChangeRaceState(RaceStates _byNewRaceState);
   void _ChangeDogState(_byDogStates _byNewDogState);
   void _ChangeDogNumber(uint8_t _iNewDogNumber);
   void _QueuePush(STriggerRecord _InterruptTrigger);
   void _QueueFilter();
   STriggerRecord _QueuePop();
   bool _QueueEmpty();
   void _AddToTransitionString(STriggerRecord _InterruptTrigger);
   void _PrintRaceSummary();
   void _PrintRaceTriggerRecords();
   void _PrintRaceTriggerRecordsToFile();
};

extern RaceHandlerClass RaceHandler;

#endif
