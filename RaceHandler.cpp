// 
// 
// 

#include "RaceHandler.h"
#include "global.h"

void RaceHandlerClass::init()
{
   //Start in stopped state
   _ChangeState(STOPPED);
}

void RaceHandlerClass::_ChangeState(RaceStates _byNewState)
{
   //First check if the new state (this function could be called superfluously)
   if (RaceState != _byNewState)
   {
      PreviousRaceState = RaceState;
      RaceState = _byNewState;
   }
}

void RaceHandlerClass::_ChangeDogNumber(int _iNewDogNumber)
{
   //Check if the dog really changed (this function could be called superfluously)
   if (_iNewDogNumber != iCurrentDog)
   {
      iPreviousDog = iCurrentDog;
      iCurrentDog = _iNewDogNumber;
   }

}

void RaceHandlerClass::Main()
{
   if (_lNewS1Time > 0)
   {
      bDEBUG ? printf("%lu: Sensor1 triggered: %lu\r\n", millis(), _lNewS1Time) : NULL;
      //Check what current state of race is
      if (RaceState != STOPPED
         && (_lNewS1Time - _lPrevS2Time) > 100000) //100 ms debounce to avoid dog coming back immediately triggering the next dog
      {
         //Store crossing time
         lCrossingTimes[iCurrentDog] = _lNewS1Time - _lPerfectCrossingTime;
         //Store dog enter time
         _lDogEnterTimes[iCurrentDog] = _lNewS1Time;
         if (lCrossingTimes[iCurrentDog] < 0)
         {
            //Negative crossing means fault
            //TODO: Trigger fault light
            bDEBUG ? printf("%lu: Fault by dog %i!", millis(), iCurrentDog) : NULL;
         }
         else
         {
            //Only change state when there was no fault
            //set state to expect the dog back
            _ChangeState(COMINGBACK);
         }
      }
      _lPrevS1Time = _lNewS1Time;
      _lNewS1Time = 0;
   }

   if (_lNewS2Time > 0)
   {
      bDEBUG ? printf("%lu: Sensor2 triggered: %lu\r\n", millis(), _lNewS2Time) : NULL;
      //Only check 2nd sensor if we're expecting a dog to come back
      if (RaceState == COMINGBACK)
      {
         //Check how long current dog is away, we expect it to be away at least 2 seconds
         if (micros() - _lDogEnterTimes[iCurrentDog] > 2000000)
         {
            _lDogExitTimes[iCurrentDog] = _lNewS2Time;
            lDogTimes[iCurrentDog] = _lDogEnterTimes[iCurrentDog] - _lNewS2Time;
            //The time the dog came in is also the perfect crossing time
            _lPerfectCrossingTime = _lNewS2Time;
            //Set race back to expect incoming dog
            _ChangeState(GOINGIN);
            //And increase dog number
            _ChangeDogNumber(iCurrentDog + 1);
         }
      }
      _lPrevS2Time = _lNewS2Time;
      _lNewS2Time = 0;
   }

}

void RaceHandlerClass::StartTimers()
{
   _ChangeState(GOINGIN);
   lStartTime = micros();
   _lPerfectCrossingTime = lStartTime;
   _lRaceStartTime = _lPerfectCrossingTime;
}

void RaceHandlerClass::StartRace()
{
   _ChangeState(STARTING);
   lStartTime = micros();
   _lPerfectCrossingTime = lStartTime + 3000000;
   _lRaceStartTime = _lPerfectCrossingTime;
}

void RaceHandlerClass::TriggerSensor1()
{
   //Debounce code
   if ((micros() - _lSensor1LastTriggerTime) < 100000) //100ms debounce
   {
      return;
   }
   _lNewS1Time = micros();
   _lSensor1LastTriggerTime = micros();
}

void RaceHandlerClass::TriggerSensor2()
{
   //Debounce code
   if ((micros() - _lSensor2LastTriggerTime) < 100000) //100ms debounce
   {
      return;
   }
   _lNewS2Time = micros();
   _lSensor2LastTriggerTime = micros();
}

float RaceHandlerClass::GetElapsedTime()
{
   long ElapsedTimeMicros = micros() - lStartTime;
   float ElapsedTimeSeconds = ElapsedTimeMicros / 1000000;
   return ElapsedTimeSeconds;
}

RaceHandlerClass RaceHandler;

