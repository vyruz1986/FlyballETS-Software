// 
// 
// 

#include "RaceHandler.h"
#include "global.h"

void RaceHandlerClass::init(int iS1Pin, int iS2Pin)
{
   //Start in stopped state
   _ChangeRaceState(STOPPED);
   _iS1Pin = iS1Pin;
   _iS2Pin = iS2Pin;

}

void RaceHandlerClass::_ChangeRaceState(RaceStates _byNewRaceState)
{
   //First check if the new state (this function could be called superfluously)
   if (RaceState != _byNewRaceState)
   {
      PreviousRaceState = RaceState;
      RaceState = _byNewRaceState;
   }
}

void RaceHandlerClass::_ChangeDogState(_byDogStates _byNewDogState)
{
   if (_byDogState != _byNewDogState)
   {
      _byDogState = _byNewDogState;
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
   if (_lNewS1Time > 0 && _bS1TriggerState == HIGH)
   {
      bDEBUG ? printf("%lu: S1 %d\r\n", millis(), _lNewS1Time, _bS1TriggerState) : NULL;
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
            bDEBUG ? printf("%lu: Fault by dog %i!\r\n", millis(), iCurrentDog) : NULL;
         }
         else
         {
            //Only change state when there was no fault
            //set state to expect the dog back
            _ChangeDogState(COMINGBACK);
         }
      }
      _lPrevS1Time = _lNewS1Time;
      _lNewS1Time = 0;
   }

   if (_lNewS2Time > 0 && _bS2TriggerState == HIGH)
   {
      bDEBUG ? printf("%lu: S2 %d\r\n", millis(), _lNewS2Time, _bS2TriggerState) : NULL;
      //Only check 2nd sensor if we're expecting a dog to come back
      if (_byDogState == COMINGBACK)
      {
         //Check how long current dog is away, we expect it to be away at least 2 seconds
         if (micros() - _lDogEnterTimes[iCurrentDog] > 2000000)
         {
            _lDogExitTimes[iCurrentDog] = _lNewS2Time;
            lDogTimes[iCurrentDog] = _lDogEnterTimes[iCurrentDog] - _lNewS2Time;
            //The time the dog came in is also the perfect crossing time
            _lPerfectCrossingTime = _lNewS2Time;
            //Set race back to expect incoming dog
            _ChangeDogState(GOINGIN);
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
   _ChangeRaceState(RUNNING);
   lStartTime = micros();
   _lPerfectCrossingTime = lStartTime;
   _lRaceStartTime = _lPerfectCrossingTime;
}

void RaceHandlerClass::StartRace()
{
   _ChangeRaceState(STARTING);
   lStartTime = micros();
   _lPerfectCrossingTime = lStartTime + 3000000;
   _lRaceStartTime = _lPerfectCrossingTime;
}

void RaceHandlerClass::TriggerSensor1()
{
   /*Disable debounce for now
   //Debounce code
   if ((micros() - _lPrevS1Time) < 100000) //100ms debounce
   {
      return;
   }*/
   _lNewS1Time = micros();
   _bS1TriggerState = digitalRead(_iS1Pin);
}

void RaceHandlerClass::TriggerSensor2()
{
   /*Disable debounce for now
   //Debounce code
   if ((micros() - _lPrevS2Time) < 100000) //100ms debounce
   {
      return;
   }*/
   _lNewS2Time = micros();
   _bS2TriggerState = digitalRead(_iS2Pin);
}

double RaceHandlerClass::GetElapsedTime()
{
   double dElapsedTimeSeconds = 0;
   if (RaceState != STOPPED
      && RaceState != STARTING)
   {
      long ElapsedTimeMicros = micros() - lStartTime;
      dElapsedTimeSeconds = ElapsedTimeMicros / 1000000.0, 3;
   }

   return dElapsedTimeSeconds;
}

double RaceHandlerClass::GetDogTime(int iDogNumber)
{
   double dDogTimeSeconds = 0;
   if (lDogTimes[iDogNumber] > 0)
   {
      dDogTimeSeconds = lDogTimes[iDogNumber] / 1000000.0, 3;
   }
   return dDogTimeSeconds;
}

RaceHandlerClass RaceHandler;

