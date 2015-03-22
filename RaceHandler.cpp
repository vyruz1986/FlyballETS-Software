// 
// 
// 
#include "LightsController.h"
#include "RaceHandler.h"
#include "global.h"

void RaceHandlerClass::init(int iS1Pin, int iS2Pin)
{
   //Start in stopped state
   _ChangeRaceState(STOPPED);
   _iS1Pin = iS1Pin;
   _iS2Pin = iS2Pin;

   ResetRace();

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
   //Don't handle anything if race is stopped
   if (RaceState == STOPPED)
   {
      return;
   }
   if (_lNewS1Time > 0)
   {
      bDEBUG ? printf("%lu: S1|Time: %lu|State: %i\r\n", millis(), _lNewS1Time, _iS1TriggerState) : NULL;
      //Check what current state of race is
      if (RaceState != STOPPED
         && (_lNewS1Time - _lPrevS2Time) > 100000 //100 ms debounce to avoid dog coming back immediately triggering the next dog
         && _iS1TriggerState == 1)
      {
         //Potential fault occured
         //Special handling for dog 0 when it's not yet in the lane
         if (iCurrentDog == 0
            && _byDogState == GOINGIN
            && _lNewS1Time < _lPerfectCrossingTime)
         {
            //Dog 0 is too early!
            LightsController.ToggleFaultLight(iCurrentDog, LightsController.ON);
            bDEBUG ? printf("%lu: Fault by dog %i!\r\n", millis(), iCurrentDog) : NULL;
            _lCrossingTimes[iCurrentDog] = _lNewS1Time - _lPerfectCrossingTime;
            _ChangeDogState(COMINGBACK);
            _bFault = true;
         }
         //Check if this is a next dog which is too early (we are expecting a dog to come back
         else if (_byDogState == COMINGBACK)
         {
            //This dog is too early!
            //We assume previous dog came in at the more or less the same time
            //TODO: Since the previous dog didn't come in yet, we don't know what the perfect crossing time is
            //Therefor we can't display a negative crossing time on the display
            //So we don't set the CrossingTime for this dog (it stays +0.000s)
            _lDogExitTimes[iCurrentDog] = _lNewS1Time;
            _lDogTimes[iCurrentDog] = _lNewS1Time - _lDogEnterTimes[iCurrentDog];
            _ChangeDogNumber(iCurrentDog + 1);
            _lDogEnterTimes[iCurrentDog] = _lNewS1Time;
            bDEBUG ? printf("%lu: Fault by dog %i!\r\n", millis(), iCurrentDog) : NULL;
            LightsController.ToggleFaultLight(iCurrentDog, LightsController.ON);
            _bFault = true;
         }

         //Normal race handling (no faults)
         if (_byDogState == GOINGIN)
         {
            //Store crossing time only when dogstate was GOINGIN
            _lCrossingTimes[iCurrentDog] = _lNewS1Time - _lPerfectCrossingTime;
            _ChangeDogState(COMINGBACK);
         }
      }
      _lPrevS1Time = _lNewS1Time;
      _lNewS1Time = 0;
   }

   if (_lNewS2Time > 0)
   {
      bDEBUG ? printf("%lu: S2|Time: %lu|State: %i\r\n", millis(), _lNewS2Time, _iS2TriggerState) : NULL;
      //Only check 2nd sensor if we're expecting a dog to come back
      if (_byDogState == COMINGBACK)
      {
         //Check how long current dog is away, we expect it to be away at least 2 seconds
         if (_lNewS2Time - _lPrevS1Time > 2000000
            && _iS2TriggerState == 1)
         {
            _lDogExitTimes[iCurrentDog] = _lNewS2Time;
            _lDogTimes[iCurrentDog] = _lNewS2Time - _lDogEnterTimes[iCurrentDog];
            //If this is the 4th dog and there is no fault we have to stop the race
            if (iCurrentDog == 3 && _bFault == false)
            {
               _lRaceEndTime = _lNewS2Time;
               _lRaceTime = _lRaceEndTime - _lRaceStartTime;
               _ChangeRaceState(STOPPED);
            }
            else
            {
               //The time the dog came in is also the perfect crossing time
               _lPerfectCrossingTime = _lNewS2Time;
               //Set race back to expect incoming dog
               _ChangeDogState(GOINGIN);
               //And increase dog number
               _ChangeDogNumber(iCurrentDog + 1);
               //Store next dog enter time
               _lDogEnterTimes[iCurrentDog] = _lNewS2Time;
            }
         }
      }
      _lPrevS2Time = _lNewS2Time;
      _lNewS2Time = 0;
   }
   //Update racetime
   if (RaceState == RUNNING)
   {
      _lRaceTime = micros() - _lRaceStartTime;
   }
}

void RaceHandlerClass::StartTimers()
{
   _ChangeRaceState(RUNNING);
   _lRaceStartTime = micros();
   _lPerfectCrossingTime = _lRaceStartTime;
   //Store dog 1 enter time
   _lDogEnterTimes[0] = _lRaceStartTime;
}

void RaceHandlerClass::StartRace()
{
   _ChangeRaceState(STARTING);
   _lRaceStartTime = micros();
   _lPerfectCrossingTime = _lRaceStartTime + 3000000;
}

void RaceHandlerClass::StopRace()
{
   if (RaceState == RUNNING)
   {
      //Race is running, so we have to record the EndTime
      _lRaceEndTime = micros();
      _lRaceTime = _lRaceEndTime - _lRaceStartTime;
   }
   _ChangeRaceState(STOPPED);
}

void RaceHandlerClass::ResetRace()
{
   if (RaceState == STOPPED)
   {
      iCurrentDog = 0;
      iPreviousDog = 0;
      _lRaceStartTime = 0;
      _lRaceEndTime = 0;
      _lRaceTime = 0;
      _lPerfectCrossingTime = 0;
      _byDogState = GOINGIN;
      _ChangeDogNumber(0);
      _bFault = false;
      for (auto& lTime : _lDogEnterTimes)
      {
         lTime = 0;
      }
      for (auto& lTime : _lDogExitTimes)
      {
         lTime = 0;
      }
      for (auto& lTime : _lDogTimes)
      {
         lTime = 0;
      }
      for (auto& lTime : _lCrossingTimes)
      {
         lTime = 0;
      }
   }
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
   _iS1TriggerState = digitalRead(_iS1Pin);
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
   _iS2TriggerState = digitalRead(_iS2Pin);
}

double RaceHandlerClass::GetRaceTime()
{
   double dRaceTimeSeconds = 0;
   if (RaceState != STARTING)
   {
      dRaceTimeSeconds = _lRaceTime / 1000000.0;
   }

   return dRaceTimeSeconds;
}

double RaceHandlerClass::GetDogTime(int _iDogNumber)
{
   double dDogTimeSeconds = 0;

   //First check if we have final time for the requested dog number
   if (_lDogTimes[_iDogNumber] > 0)
   {
      dDogTimeSeconds = _lDogTimes[_iDogNumber] / 1000000.0;
   }
   //Then check if the requested dog is perhaps running (and coming back) so we can return the time so far
   else if (RaceState == RUNNING && iCurrentDog == _iDogNumber && _byDogState == COMINGBACK)
   {
      dDogTimeSeconds = (micros() - _lDogEnterTimes[_iDogNumber]) / 1000000.0;
   }
   dDogTimeSeconds -= (_lCrossingTimes[_iDogNumber] / 1000000.0);
   return dDogTimeSeconds;
}

String RaceHandlerClass::GetCrossingTime(int _iDognumber)
{
   double dCrossingTime = 0;
   char cCrossingTime[8];
   String strCrossingTime;
   dCrossingTime = _lCrossingTimes[_iDognumber] / 1000000.0;
   if (dCrossingTime < 0)
   {
      dCrossingTime = fabs(dCrossingTime);
      strCrossingTime = "-";
   }
   else
   {
      strCrossingTime = "+";
   }
   dtostrf(dCrossingTime, 7, 3, cCrossingTime);
   strCrossingTime += cCrossingTime;
   
   return strCrossingTime;
}

double RaceHandlerClass::GetTotalCrossingTime()
{
   long lTotalCrossingTime =
      _lCrossingTimes[0]
      + _lCrossingTimes[1]
      + _lCrossingTimes[2]
      + _lCrossingTimes[3];

   double dTotalCrossingTime = lTotalCrossingTime / 1000000.0;
   return dTotalCrossingTime;
}

String RaceHandlerClass::GetRaceStateString()
{
   String strRaceState;
   switch (RaceState)
   {
   case RaceHandlerClass::STOPPED:
      strRaceState = " STOP";
      break;
   case RaceHandlerClass::STARTING:
      strRaceState = " START";
      break;
   case RaceHandlerClass::RUNNING:
      strRaceState = "RUNNING";
      break;
   default:
      break;
   }

   return strRaceState;
}

RaceHandlerClass RaceHandler;

