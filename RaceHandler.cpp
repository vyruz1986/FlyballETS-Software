//  file:	RaceHandler.cpp
//
// summary:	Implements the race handler class
// Copyright (C) 2017 Alex Goris
// This file is part of FlyballETS-Software
// FlyballETS-Software is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.If not, see <http://www.gnu.org/licenses/>

#include "StreamPrint.h"
#include "LightsController.h"
#include "RaceHandler.h"
#include "global.h"

/// <summary>
///   Initialises this object andsets all counters to 0.
/// </summary>
///
/// <param name="iS1Pin">  Zero-based index of the S1 pin. </param>
/// <param name="iS2Pin">  Zero-based index of the S2 pin. </param>
void RaceHandlerClass::init(uint8_t iS1Pin, uint8_t iS2Pin)
{
   //Start in stopped state
   _ChangeRaceState(STOPPED);
   _iS1Pin = iS1Pin;
   _iS2Pin = iS2Pin;

   ResetRace();

   _iCurrentRaceId = 0;
}

/// <summary>
///   Changes race state, if byNewRaceState is different from current one.
/// </summary>
///
/// <param name="byNewRaceState">   New race state. </param>
void RaceHandlerClass::_ChangeRaceState(RaceStates byNewRaceState)
{
   //First check if the new state (this function could be called superfluously)
   if (RaceState != byNewRaceState)
   {
      PreviousRaceState = RaceState;
      RaceState = byNewRaceState;
   }
}

/// <summary>
///   Change dog state. If byNewDogState is different from current one.
/// </summary>
///
/// <param name="byNewDogState"> State of the new dog. </param>
void RaceHandlerClass::_ChangeDogState(_byDogStates byNewDogState)
{
   if (_byDogState != byNewDogState)
   {
      _byDogState = byNewDogState;
      if(bDEBUG) Serialprint("DogState: %i\r\n", byNewDogState);
   }
}

/// <summary>
///   Change dog number, if new dognumber is different from current one.
/// </summary>
///
/// <param name="iNewDogNumber"> Zero-based index of the new dog number. </param>
void RaceHandlerClass::_ChangeDogNumber(uint8_t iNewDogNumber)
{
   //Check if the dog really changed (this function could be called superfluously)
   if (iNewDogNumber != iCurrentDog)
   {
      iPreviousDog = iCurrentDog;
      iCurrentDog = iNewDogNumber;
      if(bDEBUG) Serialprint("Prev Dog: %i|ENT:%lu|EXIT:%lu|TOT:%lu\r\n", iPreviousDog, _lDogEnterTimes[iPreviousDog], _lDogExitTimes[iPreviousDog], _lDogTimes[iPreviousDog][_iDogRunCounters[iPreviousDog]]);

   }
}

/// <summary>
///   Main entry-point for this application. This function should be called once every main loop.
///   It will check if any new interrupts were saved from the sensors, and handle them if this
///   the case. All timing related data and also fault handling of the dogs is done in this
///   function.
/// </summary>
void RaceHandlerClass::Main()
{
   //Don't handle anything if race is stopped
   if (RaceState == STOPPED)
   {
      //If we are debugging we should dump the remainder of the interrupt queue, even if the race is stopped
      if (bDEBUG)
      {
         while (!_QueueEmpty())
         {
            STriggerRecord STempRecord = _QueuePop();
            if (bDEBUG) Serialprint("S%i|T:%li|St:%i\r\n", STempRecord.iSensorNumber, STempRecord.lTriggerTime - _lRaceStartTime, STempRecord.iSensorState);
         }
      }
      return;
   }

   if (!_QueueEmpty())   //If queue is not empty, we have work to do
   {
      //Get next record from queue
      STriggerRecord STriggerRecord = _QueuePop();
      //If the transition string is not empty and is was not updated for 2 seconds then we have to clear it.
      if (_strTransition.length() != 0
         && (micros() - _lLastTransitionStringUpdate) > 2000000)
      {
         _strTransition = "";
      }
      if (_strTransition.length() == 0)
      {
         _bGatesClear = true;
      }

      if (bDEBUG) Serialprint("S%i|T:%li|St:%i\r\n", STriggerRecord.iSensorNumber, STriggerRecord.lTriggerTime - _lRaceStartTime, STriggerRecord.iSensorState);
      if (bDEBUG) Serialprint("bGatesClear: %i\r\n", _bGatesClear);

      //Calculate what our next dog will be
      if ((_bFault && _bRerunBusy)
          ||(_bFault && iCurrentDog == 3))
      {
         //In case we are doing a rerun, we have to check which the next offending dog is
         for (uint8_t i = 0; i < 4; i++)
         {
            if (_bDogFaults[i])
            {
               //Set dognumber to next/first offending dog
               iNextDog = i;
               break;
            }
         }
      }
      else
      {
         //Not the last dog, and no rerun busy, just increase number
         iNextDog = iCurrentDog + 1;
      }
      
      //Handle sensor 1 events (handlers side)
      if (STriggerRecord.iSensorNumber == 1
         &&_bGatesClear                         //Only if gates are clear
         && STriggerRecord.iSensorState == 1)   //And act on HIGH events (beam broken)
      {
         //First check if we don't have a fault situation
         //Special fault handling for dog 0 when it's not yet in the lane
         if (iCurrentDog == 0
            && _byDogState == GOINGIN
            && STriggerRecord.lTriggerTime < _lPerfectCrossingTime)
         {
            //Dog 0 is too early!
            SetDogFault(iCurrentDog, ON);
            if (bDEBUG) Serialprint("F! D:%i!\r\n", iCurrentDog);
            _lCrossingTimes[iCurrentDog][_iDogRunCounters[iCurrentDog]] = STriggerRecord.lTriggerTime - _lPerfectCrossingTime;
            _lDogEnterTimes[iCurrentDog] = STriggerRecord.lTriggerTime;
         }
         //Check if this is a next dog which is too early (we are expecting a dog to come back)
         else if (_byDogState == COMINGBACK)
         {
            //This dog is too early!
            //We don't increase the dog number at this point. The transition string further down in the code will determine whether current dog came in or not.
            //Set fault light for next dog.
            SetDogFault(iNextDog, ON);

            //For now we assume dogs crossed more or less at the same time.
            //It is very unlikely that a next dog clears the sensors before the previous dog crosses them (this would be a veeery early crossing).
            _lDogExitTimes[iCurrentDog] = STriggerRecord.lTriggerTime;
            _lDogTimes[iCurrentDog][_iDogRunCounters[iCurrentDog]] = STriggerRecord.lTriggerTime - _lDogEnterTimes[iCurrentDog];

            //Handle next dog
            _lDogEnterTimes[iNextDog] = STriggerRecord.lTriggerTime;
            if (bDEBUG) Serialprint("F! D:%i!\r\n", iNextDog);
         }

         //Normal race handling (no faults)
         if (_byDogState == GOINGIN)
         {
            //Store crossing time
            _lCrossingTimes[iCurrentDog][_iDogRunCounters[iCurrentDog]] = STriggerRecord.lTriggerTime - _lPerfectCrossingTime;

            //If this dog is doing a rerun we have to turn the error light for this dog off
            if (_bRerunBusy)
            {
               //remove fault for this dog
               SetDogFault(iCurrentDog, OFF);
            }
         }
      }
      
      //Handle sensor 2 (box side)
      if (STriggerRecord.iSensorNumber == 2
         &&_bGatesClear                         //Only if gates are clear
         && STriggerRecord.iSensorState == 1)   //And only if sensor is HIGH
      {
         if (_byDogState != COMINGBACK)
         {
            /* Gates were clear, we weren't expecting a dog back, but one came back.
            This means we missed the dog going in,
            most likely due to perfect crossing were next dog was faster than previous dog,
            and thus passed through sensors unseen */
            //Set enter time for this dog to exit time of previous dog
            _lDogEnterTimes[iCurrentDog] = _lDogExitTimes[iPreviousDog];
            if (bDEBUG) Serialprint("Invisible dog came back!\r\n");
         }

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
            _lDogExitTimes[iPreviousDog] = STriggerRecord.lTriggerTime;
            _lDogTimes[iPreviousDog][_iDogRunCounters[iPreviousDog]] = _lDogExitTimes[iPreviousDog] - _lDogEnterTimes[iPreviousDog];

            //And update crossing time of this dog (who is in fault)
            _lCrossingTimes[iCurrentDog][_iDogRunCounters[iCurrentDog]] = _lDogEnterTimes[iCurrentDog] - _lDogExitTimes[iPreviousDog];
         }
         else if ((STriggerRecord.lTriggerTime - _lDogEnterTimes[iCurrentDog]) > 2000000) //Filter out S2 HIGH signals that are < 2 seconds after dog enter time
         {
            //Normal handling for dog coming back
            _lDogExitTimes[iCurrentDog] = STriggerRecord.lTriggerTime;
            _lDogTimes[iCurrentDog][_iDogRunCounters[iCurrentDog]] = STriggerRecord.lTriggerTime - _lDogEnterTimes[iCurrentDog];
            //The time the dog came in is also the perfect crossing time
            _lPerfectCrossingTime = STriggerRecord.lTriggerTime;

            if ((iCurrentDog == 3 && _bFault == false && _bRerunBusy == false) //If this is the 4th dog and there is no fault we have to stop the race
               || (_bRerunBusy == true && _bFault == false))                //Or if the rerun sequence was started but no faults exist anymore
            {
               StopRace(STriggerRecord.lTriggerTime);
               if (bDEBUG) Serialprint("Last Dog: %i|ENT:%lu|EXIT:%lu|TOT:%lu\r\n", iCurrentDog, _lDogEnterTimes[iCurrentDog], _lDogExitTimes[iCurrentDog], _lDogTimes[iCurrentDog][_iDogRunCounters[iCurrentDog]]);
            }
            else if ((iCurrentDog == 3 && _bFault == true && _bRerunBusy == false)  //If current dog is dog 4 and a fault exists, we have to initiate rerun sequence
               || _bRerunBusy == true)                                        //Or if rerun is busy (and faults still exist)
            {
               //Dog 3 came in but there is a fault, we have to initiate the rerun sequence
               _bRerunBusy = true;
               //Reset timers for this dog
               _lDogEnterTimes[iNextDog] = STriggerRecord.lTriggerTime;
               _lDogExitTimes[iNextDog] = 0;
               //Increase run counter for this dog
               _iDogRunCounters[iNextDog]++;
               Serialprint("RR%i\r\n", iNextDog);
            }
            else
            {
               //Store next dog enter time
               _lDogEnterTimes[iNextDog] = STriggerRecord.lTriggerTime;
            }
         }
      }
      
      /***********************************
       * The code below handles what we call the 'transition string'
       * It is an algorithm which saves all sensor events in sequence, until it recognizes a pattern.
       * We have 2 sensor columns: the handler side column, and the box side column
       * To indicate the handler side column, we use the letter A
       * To indicate the box side column, we use the letter B
       * A HIGH sensor reading (beam broken), will be represented by an upper case character
       * A LOW sensor reading (beam not broken), will be represented by a lower case character
       * e.g.: A --> handler side HIGH reading, b --> box side LOW reading, etc...
       * 
       * We chain these characters up to get our 'transition string'
       * Then we check this string to determine what happened
       * For example:
       * 'ABab'
       *    --> Handler side HIGH, box side HIGH, handler side LOW, box side LOW
       *    --> This tells us ONE dog passed the gates in the direction of the box
       * 'BAba'
       *    --> Box side HIGH, handler side HIGH, box side LOW, handler side LOW
       *    --> This tells us ONE dog passed the gates in the direction of the handler
       * 'BAab'
       *    --> Box side HIGH, handler side HIGH, handler side LOW, box side LOW
       *    --> This tells us TWO dogs crossed the gates simultaneously, and the dog going to the box was the last to leave the gates
      ***********************************/
      
      //Add trigger record to transition string
      _AddToTransitionString(STriggerRecord);

      //Check if the transition string up till now tells us the gates are clear
      String strLast2TransitionChars = _strTransition.substring(_strTransition.length() - 2);
      if (_strTransition.length() == 0    //String might still be empty, in which case the gates were clear
         || (strLast2TransitionChars == "ab"
         || strLast2TransitionChars == "ba")) //Sensors going low in either direction indicate gates are clear
      {
         //The gates are clear, set boolean
         _bGatesClear = true;
         
         //Print the transition string up til now for debugging purposes
         if (bDEBUG) Serialprint("Tstring: %s\r\n", _strTransition.c_str());
         
         //Only check transition string when gates are clear
         //TODO: If transistion string is 3 or longer but actually more events are coming related to same transition, these are not considered.
         if (_strTransition.length() > 3)  //And if transistion string is at least 4 characters long
         {
            //Transition string is 4 characters or longer
            //So we can check what happened
            if (_strTransition == "ABab")     //Dog going to box
            {
               //Change dog state to coming back
               _ChangeDogState(COMINGBACK);
            }
            else if (_strTransition == "BAba")  //Dog coming back
            {
               //Normal handling, change dog state to GOING IN
               _ChangeDogState(GOINGIN);
               //Set next dog active
               _ChangeDogNumber(iNextDog);
            }
            else if (_strTransition == "BbAa")
            {
               //Transistion string BbAa indicates small object has passed through sensors
               //Most likely dog spat ball
               Serialprint("Spat ball detected?!\r\n");
               SetDogFault(iCurrentDog, ON);

            }
            else  //Transition string indicated something other than dog coming back or dog going in, it means 2 dogs must have passed
            {
               //Transition string indicates more than 1 dog passed
               //We increase the dog number
               _ChangeDogNumber(iNextDog);

               //First check if no error was set for next dog (too early)
               if (_bDogFaults[iCurrentDog])
               {
                  //This dog was too early, but since have a simultaneous crossing we don't know the crossing time.
                  //Set it to 0 for now
               }

               // and set perfect crossing time for new dog
               _ChangeDogState(COMINGBACK);
               _lCrossingTimes[iCurrentDog][_iDogRunCounters[iCurrentDog]] = 0;
               _lDogEnterTimes[iCurrentDog] = _lDogExitTimes[iPreviousDog];
            }
         }
         _strTransition = "";
      }
      else
      {
         _bGatesClear = false;
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

/// <summary>
///   Starts the timers. Should be called once GREEN light comes ON.
/// </summary>
void RaceHandlerClass::StartTimers()
{
   _ChangeRaceState(RUNNING);
}

/// <summary>
///   Sets the status of the race to STARTING, should be called at same time when start light
///   sequence is called.
/// </summary>
void RaceHandlerClass::StartRace()
{
   _ChangeRaceState(STARTING);
   _lRaceStartTime = micros() + 3000000;
   _lPerfectCrossingTime = _lRaceStartTime;
   _lDogEnterTimes[0] = _lRaceStartTime;
}

/// <summary>
///   Stops a race.
/// </summary>
///
/// <param name="StopTime">   The time in microseconds at which the race stopped. </param>
void RaceHandlerClass::StopRace(unsigned long StopTime)
{
   if (RaceState == RUNNING)
   {
      //Race is running, so we have to record the EndTime
      _lRaceEndTime = StopTime;
      _lRaceTime = _lRaceEndTime - _lRaceStartTime;
   }
   _ChangeRaceState(STOPPED);

   _HistoricRaceData[_iCurrentRaceId] = GetRaceData(_iCurrentRaceId);
}

/// <summary>
///   Resets the race, this function should be called to reset all timers to 0 and prepare the
///   software for starting a next race.
/// </summary>
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
      _bGatesClear = true;
      
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

   if (_iCurrentRaceId == NUM_HISTORIC_RACE_RECORDS)
   {
      _iCurrentRaceId = 0;
   }
   else
   {
      _iCurrentRaceId++;
   }
}

/// <summary>
///   Sets dog fault for given dog number to given state.
/// </summary>
///
/// <param name="iDogNumber"> Zero-based index of the dog number. </param>
/// <param name="State">      The state. </param>
void RaceHandlerClass::SetDogFault(uint8_t iDogNumber, DogFaults State)
{
   //Don't process any faults when race is not running
   if (RaceState == STOPPED)
   {
      return;
   }
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
      Serialprint("D%iF1\r\n", iDogNumber);
   }
   else
   {
      //If fault is false, turn off fault light for this dog
      LightsController.ToggleFaultLight(iDogNumber, LightsController.OFF);
      Serialprint("D%iF0\r\n", iDogNumber);
   }
}

/// <summary>
///   ISR function for sensor 1, this function will record the sensor number, microseconds and
///   state (HIGH/LOW) of the sensor in the interrupt queue.
/// </summary>
void RaceHandlerClass::TriggerSensor1()
{
   if (RaceState == STOPPED)
   {
      return;
   }
   _QueuePush({ 1, micros(), digitalRead(_iS1Pin) });
}

/// <summary>
///   ISR function for sensor 2, this function will record the sensor number, microseconds and
///   state (HIGH/LOW) of the sensor in the interrupt queue.
/// </summary>
void RaceHandlerClass::TriggerSensor2()
{
   if (RaceState == STOPPED)
   {
      return;
   }
   _QueuePush({ 2, micros(), digitalRead(_iS2Pin) });
}

/// <summary>
///   Gets race time. Time since start if race is still running, final time if race is finished.
/// </summary>
///
/// <returns>
///   The race time in seconds with 2 decimals places.
/// </returns>
double RaceHandlerClass::GetRaceTime()
{
   double dRaceTimeSeconds = 0;
   if (RaceState != STARTING)
   {
      dRaceTimeSeconds = _lRaceTime / 1000000.0;
   }

   return dRaceTimeSeconds;
}

/// <summary>
///   Gets dog time, time since dog entered if dog is still running, final time if dog is already
///   back in. Keep in mind each dog can have multiple runs (reruns for faultS).
/// </summary>
///
/// <param name="iDogNumber"> Zero-based index of the dog number. </param>
/// <param name="iRunNumber"> Zero-based index of the run number. If -1 is passed, this function
///                           will alternate between each run we have for the dog, passing a new
///                           run every 2 seconds. If -2 is passed, the last run number we have
///                           for this dog will be passed. </param>
///
/// <returns>
///   The dog time in seconds with 2 decimals.
/// </returns>
double RaceHandlerClass::GetDogTime(uint8_t iDogNumber, int8_t iRunNumber)
{
   double dDogTimeMillis = GetDogTimeMillis(iDogNumber, iRunNumber) / 1000.0;
}
unsigned long RaceHandlerClass::GetDogTimeMillis(uint8_t iDogNumber, int8_t iRunNumber)
{
   unsigned long ulDogTimeMillis = 0;
   if (_iDogRunCounters[iDogNumber] > 0)
   {
      //We have multiple times for this dog.
      //if run number is -1 (unspecified), we have to cycle throug them
      if (iRunNumber == -1)
      {
         auto& lLastReturnedTimeStamp = _lLastDogTimeReturnTimeStamp[iDogNumber];
         iRunNumber = _iLastReturnedRunNumber[iDogNumber];
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
         _iLastReturnedRunNumber[iDogNumber] = iRunNumber;
      }
      else if (iRunNumber == -2)
      {
         //if RunNumber is -2 it means we should return the last one
         iRunNumber = _iDogRunCounters[iDogNumber];
      }
   }
   else
   {
      iRunNumber = 0;
   }

   //First check if we have final time for the requested dog number
   if (_lDogTimes[iDogNumber][iRunNumber] > 0)
   {
      ulDogTimeMillis = _lDogTimes[iDogNumber][iRunNumber] / 1000;
   }
   //Then check if the requested dog is perhaps running (and coming back) so we can return the time so far
   else if (RaceState == RUNNING && iCurrentDog == iDogNumber && _byDogState == COMINGBACK)
   {
      ulDogTimeMillis = (micros() - _lDogEnterTimes[iDogNumber]) / 1000;
   }

   //Fixes issue 7 (https://github.com/vyruz1986/FlyballETS-Software/issues/7)
   //Only deduct crossing time if it is positive
   if (_lCrossingTimes[iDogNumber][iRunNumber] > 0)
   {
      ulDogTimeMillis -= (_lCrossingTimes[iDogNumber][iRunNumber] / 1000);
   }
   return ulDogTimeMillis;
}

/// <summary>
///   Gets dogs crossing time. Keep in mind each dog can have multiple runs (reruns for faultS).
/// </summary>
///
/// <param name="iDogNumber"> Zero-based index of the dog number. </param>
/// <param name="iRunNumber"> Zero-based index of the run number. If -1 is passed, this function
///                           will alternate between each run we have for the dog, passing a new
///                           run every 2 seconds. If -2 is passed, the last run number we have
///                           for this dog will be passed. </param>
///
/// <returns>
///   The dog time in seconds with 2 decimals.
/// </returns>
String RaceHandlerClass::GetCrossingTime(uint8_t iDogNumber, int8_t iRunNumber)
{
   double dCrossingTime = 0;
   char cCrossingTime[8];
   String strCrossingTime;
   dCrossingTime = GetCrossingTimeMillis(iDogNumber, iRunNumber) / 1000.0;
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
unsigned long RaceHandlerClass::GetCrossingTimeMillis(uint8_t iDogNumber, int8_t iRunNumber)
{
   unsigned long ulCrossingTime = 0;
   if (_iDogRunCounters[iDogNumber] > 0)
   {
      //We have multiple times for this dog.
      //if run number is -1 (unspecified), we have to cycle throug them
      if (iRunNumber == -1)
      {
         auto& lLastReturnedTimeStamp = _lLastDogTimeReturnTimeStamp[iDogNumber];
         iRunNumber = _iLastReturnedRunNumber[iDogNumber];
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
         _iLastReturnedRunNumber[iDogNumber] = iRunNumber;
      }
      else if (iRunNumber == -2)
      {
         //if RunNumber is -2 it means we should return the last one
         iRunNumber = _iDogRunCounters[iDogNumber];
      }
   }
   else
   {
      iRunNumber = 0;
   }

   ulCrossingTime = _lCrossingTimes[iDogNumber][iRunNumber] / 1000;
   
   return ulCrossingTime;
}

/// <summary>
///   Gets rerun information to put on LCD. If a dog has done more than 1 run, before the dogs
///   time we will display and * (asterisk) followed by the run number for which we are showing
///   the time. If a dog has not done a rerun, the space before the dogs time is empty.
/// </summary>
///
/// <param name="iDogNumber"> Zero-based index of the dog number. </param>
///
/// <returns>
///   The rerun information. * (asterisk) followed by run number if there is more than 1 run for
///   this dog. Empty string if the dog did only do 1 run.
/// </returns>
String RaceHandlerClass::GetRerunInfo(uint8_t iDogNumber)
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

/// <summary>
///   Gets total crossing time. This will return the total crossing time of all dogs (and reruns
///   if applicable). It allows the user to easily calculate the theoretical best time of the
///   team by subtracting this number from the total team time.
/// </summary>
///
/// <returns>
///   The total crossing time in seconds with 2 decimals.
/// </returns>
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

/// <summary>
///   Gets race state string. Internally the software uses a (enumerated) byte to keep the race
///   state, however on the display we have to display english text. This function returns the
///   correct english text for the current race state.
/// </summary>
///
/// <returns>
///   The race state string.
/// </returns>
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

stRaceData RaceHandlerClass::GetRaceData()
{
   return GetRaceData(_iCurrentRaceId);
}

stRaceData RaceHandlerClass::GetRaceData(uint iRaceId)
{
   stRaceData RequestedRaceData;
   
   if (iRaceId == _iCurrentRaceId)
   {
      //We need to return data for the current dace
      RequestedRaceData.Id = _iCurrentRaceId;
      RequestedRaceData.StartTime = _lRaceStartTime / 1000;
      RequestedRaceData.EndTime = _lRaceEndTime / 1000;
      RequestedRaceData.ElapsedTime = _lRaceTime / 1000;
      RequestedRaceData.RaceState = RaceState;
      
      //Get Dog info
      for (uint8_t i = 0; i < 4; i++)
      {
         RequestedRaceData.DogData[i].DogNumber = i;
         //TODO: Multiple runs should be supported somehow
         RequestedRaceData.DogData[i].Time = GetDogTimeMillis(i, -2);
         RequestedRaceData.DogData[i].CrossingTime = GetCrossingTimeMillis(i, -2);
         RequestedRaceData.DogData[i].Fault = _bDogFaults[i];
         RequestedRaceData.DogData[i].Running = (iCurrentDog == i);
      }
   }
   else {
      RequestedRaceData = _HistoricRaceData[iRaceId];
   }
   return RequestedRaceData;
}

/// <summary>
///   Pushes an interrupt trigger record to the back of the interrupt buffer.
/// </summary>
///
/// <param name="_InterruptTrigger">   The interrupt trigger record. </param>
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

/// <summary>
///   Pops one interrupt record from the front of the interrupt buffer.
/// </summary>
///
/// <returns>
///   A RaceHandlerClass::STriggerRecord from the front of the interrupt buffer.
/// </returns>
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

/// <summary>
///   Determines if the interrupt buffer queue is empty.
/// </summary>
///
/// <returns>
///   true if it is empty, false if it is not.
/// </returns>
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

/// <summary>
///   Adds an interrupt record to the transition string. This function will automatically
///   determine which character (upper or lowercase A or B) should be added to the string. Note
///   that some filtering of consecutive LOW-HIGH-LOW and HIGH-LOW-HIGH signals is done also in
///   this function.
/// </summary>
///
/// <param name="_InterruptTrigger">   The interrupt trigger record. </param>
void RaceHandlerClass::_AddToTransitionString(STriggerRecord _InterruptTrigger)
{
   //The transition string consists of lower and upper case A and B characters.
   //A indicates the handlers side, B indicates the boxes side
   //Uppercase indicates a high signal (dog broke beam), lowercase indicates a low signal (dog left beam)

   _lLastTransitionStringUpdate = micros();

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
   //Add the character to the transition string
   _strTransition += cTemp;

   //Filtering for unwanted sensor jitter
   //Filter consecutive alternating changes out
   _strTransition.replace("AaA", "A");
   _strTransition.replace("aAa", "a");
   _strTransition.replace("BbB", "B");
   _strTransition.replace("bBb", "b");
}

/// <summary>
///   The race handler class.
/// </summary>
RaceHandlerClass RaceHandler;

