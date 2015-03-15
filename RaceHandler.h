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

   uint8_t iCurrentDog = 1;
   uint8_t iPreviousDog = 1;
   long lDogTimes[4] = { 0, 0, 0, 0 };
   long lCrossingTimes[4] = { 0, 0, 0, 0 };
   long lStartTime = 0;
   void Main();
   void StartTimers();
   void StartRace();
   void TriggerSensor1();
   void TriggerSensor2();

   double GetElapsedTime();
   double GetDogTime(int iDogNumber);

private:
   long _lRaceStartTime;
   long _lPerfectCrossingTime;

   long _lNewS1Time;
   long _lPrevS1Time;
   bool _bS1TriggerState;
   int  _iS1Pin;

   long _lNewS2Time;
   long _lPrevS2Time;
   bool _bS2TriggerState;
   int  _iS2Pin;

   bool _bFault = false;
   long _lDogEnterTimes[4];
   long _lDogExitTimes[4];
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

