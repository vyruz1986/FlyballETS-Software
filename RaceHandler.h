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
	void init();
   enum RaceStates {
      STOPPED,
      STARTING,
      COMINGBACK,
      GOINGIN
   };
   RaceStates RaceState = STOPPED;
   RaceStates PreviousRaceState = STOPPED;

   uint8_t iCurrentDog = 1;
   uint8_t iPreviousDog = 1;
   long lDogTimes[4];
   long lCrossingTimes[4];
   long StartTime = 0;
   void Main();
   void StartTimers();
   void StartRace();
   void TriggerSensor1();
   void TriggerSensor2();

   float GetElapsedTime();


private:
   long _lRaceStartTime;
   long _lPerfectCrossingTime;
   long _lSensor1LastTriggerTime;
   long _lSensor2LastTriggerTime;
   long _lNewS1Time;
   long _lPrevS1Time;
   long _lNewS2Time;
   long _lPrevS2Time;
   bool _bFault = false;
   long _lDogEnterTimes[4];
   long _lDogExitTimes[4];
   void _ChangeState(RaceStates _byNewState);
   void _ChangeDogNumber(int _iNewDogNumber);

};

extern RaceHandlerClass RaceHandler;

#endif

