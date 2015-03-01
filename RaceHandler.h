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

   int CurrentDog = 1;
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
   long _lNewS2Time;
   bool _bFault = false;
   long _lDogEnterTimes[4];
   long _lDogExitTimes[4];

};

extern RaceHandlerClass RaceHandler;

#endif

