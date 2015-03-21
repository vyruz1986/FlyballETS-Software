// RaceHandler.h

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

   double GetRaceTime();
   double GetDogTime(int iDogNumber);
   String GetCrossingTime(int iDognumber);
   double GetTotalCrossingTime();

   String GetRaceStateString();

private:
   long _lRaceStartTime;
   long _lRaceEndTime;
   long _lRaceTime;
   long _lPerfectCrossingTime;

   volatile long _lNewS1Time;
   volatile long _lPrevS1Time;
   int _iS1TriggerState;
   int  _iS1Pin;

   volatile long _lNewS2Time;
   volatile long _lPrevS2Time;
   int _iS2TriggerState;
   int  _iS2Pin;

   bool _bFault = false;
   long _lDogEnterTimes[4];
   long _lDogExitTimes[4];

   unsigned long _lDogTimes[4];
   long _lCrossingTimes[4];
   
   enum _byDogStates {
      GOINGIN,
      COMINGBACK
   };
   _byDogStates _byDogState;
   void _ChangeRaceState(RaceStates _byNewRaceState);
   void _ChangeDogState(_byDogStates _byNewDogState);
   void _ChangeDogNumber(int _iNewDogNumber);
};

extern RaceHandlerClass RaceHandler;

#endif

