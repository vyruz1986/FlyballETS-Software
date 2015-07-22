#include <StreamPrint.h>
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

   if (!_QueueEmpty())   //If read index is not equal to write index, it means there is stuff in the buffer
   {
      //Check if the transition string up till now tells us the last dog cleared the gates
      String strLast2TransitionChars = _strTransition.substring(_strTransition.length() -2);
      if (_strTransition.length() == 0    //String might still be empty, in which case the dog was clear
         || (strLast2TransitionChars == "ab"
         || strLast2TransitionChars == "ba")) //Sensors going low in either direction indicate gates are clear
      {
         _bGatesClear = true;
         _strTransition = "";
      }
      else
      {
         _bGatesClear = false;
      }

      //Increase read index and get next record
      STriggerRecord STriggerRecord = _QueuePop();
      if (bDEBUG) Serialprint("S%i|T:%lu|St:%i\r\n", STriggerRecord.iSensorNumber, STriggerRecord.lTriggerTime, STriggerRecord.iSensorState);

      Serialprint("bGatesClear: %i\r\n", _bGatesClear);

      if (STriggerRecord.iSensorNumber == 1)
      {
         //Check what current state of race is
         if (RaceState != STOPPED
            && _bGatesClear
            && STriggerRecord.iSensorState == 1)
         {
            //Potential fault occured
            //Special handling for dog 0 when it's not yet in the lane
            if (iCurrentDog == 0
               && _byDogState == GOINGIN
               && STriggerRecord.lTriggerTime < _lPerfectCrossingTime)
            {
               //Dog 0 is too early!
               SetDogFault(iCurrentDog, ON);
               if (bDEBUG) Serialprint("F! D:%i!\r\n", iCurrentDog);
               _lCrossingTimes[iCurrentDog][_iDogRunCounters[iCurrentDog]] = STriggerRecord.lTriggerTime - _lPerfectCrossingTime;
               _lDogEnterTimes[iCurrentDog] = STriggerRecord.lTriggerTime;
               _ChangeDogState(COMINGBACK);
            }
            //Check if this is a next dog which is too early (we are expecting a dog to come back)
            else if (_byDogState == COMINGBACK)
            {
               //This dog is too early!
               //We assume previous dog came in at the more or less the same time
               //TODO: Since the previous dog didn't come in yet, we don't know what the perfect crossing time is
               //Therefor we can't display a negative crossing time on the display
               //So we don't set the CrossingTime for this dog (it stays +0.000s)
               _lDogExitTimes[iCurrentDog] = STriggerRecord.lTriggerTime;
               _lDogTimes[iCurrentDog][_iDogRunCounters[iCurrentDog]] = STriggerRecord.lTriggerTime - _lDogEnterTimes[iCurrentDog];

               //Handle next dog
               _ChangeDogNumber(iCurrentDog + 1);
               _lDogEnterTimes[iCurrentDog] = STriggerRecord.lTriggerTime;
               if (bDEBUG) Serialprint("F! D:%i!\r\n", iCurrentDog);
               SetDogFault(iCurrentDog, ON);
            }

            //Normal race handling (no faults)
            if (_byDogState == GOINGIN)
            {
               //Store crossing time only when dogstate was GOINGIN
               _lCrossingTimes[iCurrentDog][_iDogRunCounters[iCurrentDog]] = STriggerRecord.lTriggerTime - _lPerfectCrossingTime;
               _ChangeDogState(COMINGBACK);
               _lDogEnterTimes[iCurrentDog] = STriggerRecord.lTriggerTime;

               //If this dog is doing a rerun we have to turn the error light for this dog off
               if (_bRerunBusy)
               {
                  //remove fault for this dog
                  SetDogFault(iCurrentDog, OFF);
               }
            }
         }
      }

      if (STriggerRecord.iSensorNumber == 2
         && _byDogState == COMINGBACK) //Only check 2nd sensor if we're expecting a dog to come back
      {
         if (_bGatesClear
            && STriggerRecord.iSensorState == 1)
         {
            //Check if current dog has a fault
            //TODO: The current dog could also have a fault which is not caused by being too early (manually triggered fault).
            //We should store the fault type also so we can check if the dog was too early or not.
            if (iCurrentDog != 0                                                             //If dog is not 1st dog
                && _bDogFaults[iCurrentDog]                                                  //and current dog has fault
                && (STriggerRecord.lTriggerTime - _lDogEnterTimes[iCurrentDog]) < 2000000)   //And S2 is trigger less than 2s after current dog's enter time
                                                                                             //Then we know It's actually the previous dog who's still coming back (current dog was way too early).
            {
               //Current dog had a fault (was too early), so we need to modify the previous dog crossing time (we didn't know this before)
               //Update exit and total time of previous dog
               _lDogExitTimes[iCurrentDog - 1] = STriggerRecord.lTriggerTime;
               _lDogTimes[iCurrentDog - 1][_iDogRunCounters[iCurrentDog -1]] = _lDogExitTimes[iCurrentDog - 1] - _lDogEnterTimes[iCurrentDog - 1];

               //And update crossing time of this dog (who is in fault)
               _lCrossingTimes[iCurrentDog][_iDogRunCounters[iCurrentDog]] = _lDogEnterTimes[iCurrentDog] - _lDogExitTimes[iCurrentDog - 1];
            }
            else
            {
               //Normal handling for dog coming back
               _lDogExitTimes[iCurrentDog] = STriggerRecord.lTriggerTime;
               _lDogTimes[iCurrentDog][_iDogRunCounters[iCurrentDog]] = STriggerRecord.lTriggerTime - _lDogEnterTimes[iCurrentDog];
               //The time the dog came in is also the perfect crossing time
               _lPerfectCrossingTime = STriggerRecord.lTriggerTime;
               //Set race back to expect incoming dog
               _ChangeDogState(GOINGIN);

               if ((iCurrentDog == 3 && _bFault == false && _bRerunBusy == false) //If this is the 4th dog and there is no fault we have to stop the race
                  || (_bRerunBusy == true && _bFault == false))                //Or if the rerun sequence was started but no faults exist anymore
               {
                  _lRaceEndTime = STriggerRecord.lTriggerTime;
                  _lRaceTime = _lRaceEndTime - _lRaceStartTime;
                  _ChangeRaceState(STOPPED);
               }
               else if ((iCurrentDog == 3 && _bFault == true && _bRerunBusy == false)  //If current dog is dog 4 and a fault exists, we have to initiate rerun sequence
                  || _bRerunBusy == true)                                        //Or if rerun is busy (and faults still exist)
               {
                  //Dog 3 came in but there is a fault, we have to initiate the rerun sequence
                  _bRerunBusy = true;
                  for (int i = 0; i < 4; i++)
                  {
                     if (_bDogFaults[i])
                     {
                        //Set dognumber for first offending dog
                        _ChangeDogNumber(i);
                        //Reset timers for this dog
                        _lDogEnterTimes[i] = STriggerRecord.lTriggerTime;
                        _lDogExitTimes[i] = 0;
                        //Increase run counter for this dog
                        _iDogRunCounters[i]++;
                        break;
                     }
                  }
                  if (bDEBUG) Serialprint("RR! D:%i\r\n", millis(), iCurrentDog);
               }
               else
               {
                  //And increase dog number
                  _ChangeDogNumber(iCurrentDog + 1);
                  //Store next dog enter time
                  _lDogEnterTimes[iCurrentDog] = STriggerRecord.lTriggerTime;
               }
            }
         }
      }

      //Add trigger record to transition string
      _AddToTransitionString(STriggerRecord);

      //Check if more than 1 dog passed
      if (_strTransition.length() > 3)  //If transistion string is at least 4 characters long
      {
         //Transition string is 4 characters or longer
         //So we can check what happened
         if (_strTransition != "ABab"     //Dog going to box
            && _strTransition != "BAba")  //Dog coming back
         {
            //Transition string indicates more than 1 dog passed
            //We increase the dog number and set perfect corssing time for new dog
            _ChangeDogNumber(iCurrentDog + 1);
            _lCrossingTimes[iCurrentDog][_iDogRunCounters[iCurrentDog]] = 0;
            _ChangeDogState(COMINGBACK);
            _lDogEnterTimes[iCurrentDog] = _lDogExitTimes[iCurrentDog - 1];
         }
      }
   }

   //Update racetime
   if (RaceState == RUNNING)
   {
      _lRaceTime = micros() - _lRaceStartTime;
   }

   //Check for faults, loop through array of dogs checking for faults
   _bFault = false;
   for (auto bFault : _bDogFaults)
   {
      if (bFault)
      {
         //At least one dog with fault is found, set general fault value to true
         _bFault = true;
         break;
      }
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
      _bRerunBusy = false;
      _iQueueReadIndex = 0;
      _iQueueWriteIndex = 0;
      _strTransition = "";
      
      for (auto& bFault : _bDogFaults)
      {
         bFault = false;
      }
      for (auto& lTime : _lDogEnterTimes)
      {
         lTime = 0;
      }
      for (auto& lTime : _lDogExitTimes)
      {
         lTime = 0;
      }
      for (auto& Dog : _lDogTimes)
      {
         for (auto& lTime : Dog)
         {
            lTime = 0;
         }
      }
      for (auto& Dog : _lCrossingTimes)
      {
         for (auto& lTime : Dog)
         {
            lTime = 0;
         }
      }
      for (auto& iCounter : _iDogRunCounters)
      {
         iCounter = 0;
      }
      for (auto& lTime : _lLastDogTimeReturnTimeStamp)
      {
         lTime = 0;
      }
      for (auto& iCounter : _iLastReturnedRunNumber)
      {
         iCounter = 0;
      }
   }
}
void RaceHandlerClass::SetDogFault(int iDogNumber, DogFaults State)
{
   bool bFault;
   //Check if we have to toggle
   if (State == TOGGLE)
   {
      bFault = !_bDogFaults[iDogNumber];
   }
   else
   {
      bFault = State;
   }

   //Set fault to specified value for relevant dog
   _bDogFaults[iDogNumber] = bFault;

   //If fault is true, set light to on and set general value fault variable to true
   if (bFault)
   {
      LightsController.ToggleFaultLight(iDogNumber, LightsController.ON);
      _bFault = true;
   }
   else
   {
      //If fault is false, turn off fault light for this dog
      LightsController.ToggleFaultLight(iDogNumber, LightsController.OFF);
   }
}

void RaceHandlerClass::TriggerSensor1()
{
   if (RaceState == STOPPED)
   {
      return;
   }
   _QueuePush({ 1, micros(), digitalRead(_iS1Pin) });
}

void RaceHandlerClass::TriggerSensor2()
{
   if (RaceState == STOPPED)
   {
      return;
   }
   _QueuePush({ 2, micros(), digitalRead(_iS2Pin) });
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

double RaceHandlerClass::GetDogTime(int iDogNumber)
{
   double dDogTimeSeconds = 0;

   uint8_t& iRunNumber = _iLastReturnedRunNumber[iDogNumber];
   auto& lLastReturnedTimeStamp = _lLastDogTimeReturnTimeStamp[iDogNumber];
   if (_iDogRunCounters[iDogNumber] > 0)
   {
      //We have multiple crossing times for this dog, so we must cycle through them
      if ((millis() - lLastReturnedTimeStamp) > 2000)
      {
         if (iRunNumber == _iDogRunCounters[iDogNumber])
         {
            iRunNumber = 0;
         }
         else
         {
            iRunNumber++;
         }
         lLastReturnedTimeStamp = millis();
      }
   }

   //First check if we have final time for the requested dog number
   if (_lDogTimes[iDogNumber][iRunNumber] > 0)
   {
      dDogTimeSeconds = _lDogTimes[iDogNumber][iRunNumber] / 1000000.0;
   }
   //Then check if the requested dog is perhaps running (and coming back) so we can return the time so far
   else if (RaceState == RUNNING && iCurrentDog == iDogNumber && _byDogState == COMINGBACK)
   {
      dDogTimeSeconds = (micros() - _lDogEnterTimes[iDogNumber]) / 1000000.0;
   }
   dDogTimeSeconds -= (_lCrossingTimes[iDogNumber][iRunNumber] / 1000000.0);
   return dDogTimeSeconds;
}

String RaceHandlerClass::GetCrossingTime(int iDogNumber)
{
   double dCrossingTime = 0;
   char cCrossingTime[8];
   String strCrossingTime;

   uint8_t& iRunNumber = _iLastReturnedRunNumber[iDogNumber];
   auto& lLastReturnedTimeStamp = _lLastDogTimeReturnTimeStamp[iDogNumber];
   if (_iDogRunCounters[iDogNumber] > 0)
   {
      //We have multiple crossing times for this dog, so we must cycle through them
      if ((millis() - lLastReturnedTimeStamp) > 2000)
      {
         if (iRunNumber == _iDogRunCounters[iDogNumber])
         {
            iRunNumber = 0;
         }
         else
         {
            iRunNumber++;
         }
         lLastReturnedTimeStamp = millis();
      }
   }
   dCrossingTime = _lCrossingTimes[iDogNumber][iRunNumber] / 1000000.0;
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

String RaceHandlerClass::GetRerunInfo(int iDogNumber)
{
   String strRerunInfo = "  ";
   
   uint8_t iRunNumber = _iLastReturnedRunNumber[iDogNumber];
   if (_iDogRunCounters[iDogNumber] > 0)
   {
      strRerunInfo = "*";
      strRerunInfo += (iRunNumber + 1);
   }
   return strRerunInfo;
}

double RaceHandlerClass::GetTotalCrossingTime()
{
   long lTotalCrossingTime = 0;

   for (auto& Dog : _lCrossingTimes)
   {
      for (auto& lTime : Dog)
      {
         lTotalCrossingTime += lTime;
      }
   }
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

void RaceHandlerClass::_QueuePush(RaceHandlerClass::STriggerRecord _InterruptTrigger)
{
   //Add record to queue
   _STriggerQueue[_iQueueWriteIndex] = _InterruptTrigger;

   //Write index has to be increased, check it we should wrap-around
   if (_iQueueWriteIndex == 9)//(sizeof(_STriggerQueue) / sizeof(*_STriggerQueue) - 1))
   {
      //Write index has reached end of array, start at 0 again
      _iQueueWriteIndex = 0;
   }
   else
   {
      //End of array not yet reached, increase index by 1
      _iQueueWriteIndex++;
   }
}

RaceHandlerClass::STriggerRecord RaceHandlerClass::_QueuePop()
{
   //This function returns the next record of the interrupt queue
   STriggerRecord NextRecord = _STriggerQueue[_iQueueReadIndex];

   //Read index has to be increased, check it we should wrap-around
   if (_iQueueReadIndex == 9)//(sizeof(_STriggerQueue) / sizeof(*_STriggerQueue) - 1))
   {
      //Write index has reached end of array, start at 0 again
      _iQueueReadIndex = 0;
   }
   else
   {
      //End of array not yet reached, increase index by 1
      _iQueueReadIndex++;
   }
   return NextRecord;
}

bool RaceHandlerClass::_QueueEmpty()
{
   //This function checks if queue is empty.
   //This is determined by comparing the read and write index.
   //If they are equal, it means we have cought up reading and the queue is 'empty' (the array is not really emmpty...)
   if (_iQueueReadIndex == _iQueueWriteIndex)
   {
      return true;
   }
   else
   {
      return false;
   }
}

void RaceHandlerClass::_AddToTransitionString(STriggerRecord _InterruptTrigger)
{
   //The transition string consists of lower and upper case A and B characters.
   //A indicates the handlers side, B indicates the boxes side
   //Uppercase indicates a high signal (dog broke beam), lowercase indicates a low signal (dog left beam)
   char cTemp;
   switch (_InterruptTrigger.iSensorNumber)
   {
      case 1:
         cTemp = 'A';
         break;

      case 2:
         cTemp = 'B';
   }
   
   //We assumged the signal was high, if it is not we should make the character lowercase
   if (_InterruptTrigger.iSensorState == LOW)
   {
      cTemp = tolower(cTemp);
   }

   //Filtering for unwanted sensor jitter
   //If this letter in this case is already in the string we don't want it
   if (_strTransition.indexOf(cTemp) == -1)
   {
      //Finally add the character to the transition string
      _strTransition += cTemp;
   }

   Serialprint("Tstring: %s\r\n", _strTransition.c_str());
}

RaceHandlerClass RaceHandler;

