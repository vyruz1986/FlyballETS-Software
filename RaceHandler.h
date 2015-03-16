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

   uint8_t iCurrentDog = 0;
   uint8_t iPreviousDog = 0;
   
   void Main();
   void StartTimers();
   void StartRace();
   void TriggerSensor1();
   void TriggerSensor2();

   double GetRaceTime();
   double GetDogTime(int iDogNumber);
   double GetCrossingTime(int iDognumber);
   double GetTotalCrossingTime();

   String GetRaceStateString();

private:
   long _lRaceStartTime = 0;
   long _lRaceEndTime = 0;
   long _lRaceTime = 0;
   long _lPerfectCrossingTime = 0;

   volatile long _lNewS1Time = 0;
   volatile long _lPrevS1Time = 0;
   bool _bS1TriggerState;
   int  _iS1Pin;

   volatile long _lNewS2Time = 0;
   volatile long _lPrevS2Time = 0;
   bool _bS2TriggerState;
   int  _iS2Pin;

   bool _bFault = false;
   long _lDogEnterTimes[4] = { 0, 0, 0, 0 };
   long _lDogExitTimes[4] = { 0, 0, 0, 0 };

   unsigned long _lDogTimes[4] = { 0, 0, 0, 0 };
   long _lCrossingTimes[4] = { 0, 0, 0, 0 };
   
   enum _byDogStates {
      GOINGIN,
      COMINGBACK
   };
   _byDogStates _byDogState = GOINGIN;
   void _ChangeRaceState(RaceStates _byNewRaceState);
   void _ChangeDogState(_byDogStates _byNewDogState);
   void _ChangeDogNumber(int _iNewDogNumber);

   

};

extern RaceHandlerClass RaceHandler;

#endif

