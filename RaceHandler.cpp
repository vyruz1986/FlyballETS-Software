// 
// 
// 

#include "RaceHandler.h"
#include <StopWatch.h>

void RaceHandlerClass::init()
{

}
void RaceHandlerClass::Main()
{
   if (_lNewS1Time > 0)
   {
      //Check what current state of race is
      if (RaceState == STARTING
         || RaceState == GOINGIN)
      {
         //Store crossing time
         lCrossingTimes[CurrentDog] = _lNewS1Time - _lPerfectCrossingTime;
         if (lCrossingTimes[CurrentDog] < 0)
         {
            //Negative crossing means fault
            //TODO: Trigger fault light
         }
         //And set race to expect next dog
         CurrentDog++;
      }
      if (RaceState == STARTING)
      {
         //This was the starting dog, set race to expect dog1 to come back
         RaceState = COMINGBACK;
         CurrentDog = 1;
      }
      _lNewS1Time = 0;
   }

   if (_lNewS2Time > 0)
   {
      //Only check 2nd sensor if we're expecting a dog to come back
      if (RaceState == COMINGBACK)
      {
         //Check how long current dog is away, we expect it to be away at least 2 seconds
         if (micros() - _lDogEnterTimes[CurrentDog] > 2000000)
         {
            _lDogExitTimes[CurrentDog] = _lNewS2Time;
            lDogTimes[CurrentDog] = _lDogEnterTimes[CurrentDog] - _lNewS2Time;
            //The time the dog came in is also the perfect crossing time
            _lPerfectCrossingTime = _lNewS2Time;
            //Set race back to expect incoming dog
            RaceState = GOINGIN;
            //And increase dog number
            CurrentDog++;
         }
      }
      _lNewS2Time = 0;
   }

}

void RaceHandlerClass::StartTimers()
{
   StartTime = micros();
}

void RaceHandlerClass::StartRace()
{
   RaceState = STARTING;
   StartTime = micros();
   _lPerfectCrossingTime = StartTime + 4000000;
   _lRaceStartTime = _lPerfectCrossingTime;
}

void RaceHandlerClass::TriggerSensor1()
{
   //Debounce code
   if (micros() - _lSensor1LastTriggerTime < 100000) //100ms debounce
   {
      return;
   }
   _lNewS1Time = micros();
   _lSensor1LastTriggerTime = micros();
}

void RaceHandlerClass::TriggerSensor2()
{
   //Debounce code
   if (micros() - _lSensor2LastTriggerTime < 100000) //100ms debounce
   {
      return;
   }
   _lNewS2Time = micros();
   _lSensor2LastTriggerTime = micros();
}

float RaceHandlerClass::GetElapsedTime()
{
   long ElapsedTimeMicros = micros() - StartTime;
   float ElapsedTimeSeconds = ElapsedTimeMicros / 1000000;
   return ElapsedTimeSeconds;
}

RaceHandlerClass RaceHandler;

