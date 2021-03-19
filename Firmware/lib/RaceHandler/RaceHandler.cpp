//  file:	RaceHandler.cpp
//
// summary:	Implements the race handler class
// Copyright (C) 2019 Alex Goris
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

#include "LightsController.h"
#include "LCDController.h"
#include "RaceHandler.h"
#include "SettingsManager.h"
#include "config.h"
#include "WebHandler.h"

/// <summary>
///   Initialises this object andsets all counters to 0.
/// </summary>
///
/// <param name="iS1Pin">  Zero-based index of the S1 pin. </param>
/// <param name="iS2Pin">  Zero-based index of the S2 pin. </param>
void RaceHandlerClass::init(uint8_t iS1Pin, uint8_t iS2Pin)
{
   //Start in ready/reset state
   _iS1Pin = iS1Pin;
   _iS2Pin = iS2Pin;
   ResetRace();
   _iCurrentRaceId = 0;
   Serial.printf("Run Direction from settings: %s", SettingsManager.getSetting("RunDirectionInverted").c_str());
   
   if (SettingsManager.getSetting("RunDirectionInverted").equals("1"))
   {
      ToggleRunDirection();
   }
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
      WebHandler._SendRaceData();
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
      //ESP_LOGD(__FILE__, "DogState: %i", byNewDogState);
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
      ESP_LOGD(__FILE__, "Dog: %i | ENT:%lld | EXIT:%lld | TOT:%lld", iPreviousDog+1, _llDogEnterTimes[iPreviousDog], _llDogExitTimes[iPreviousDog], _llDogTimes[iPreviousDog][_iDogRunCounters[iPreviousDog]]);
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
   //Trigger filterring of sensors interrupts in new records available
   while (_iInputQueueReadIndex != _iInputQueueWriteIndex)
   {
      ESP_LOGD(__FILE__, "%lld | IQRI:%d | IQWI:%d", GET_MICROS, _iInputQueueReadIndex, _iInputQueueWriteIndex); 
      _QueueFilter();
      
   }

   //Don't handle anything if race is stopped or reset
   if (RaceState == STOPPED || RaceState == RESET)
   {
      while (!_QueueEmpty())
      {
         STriggerRecord STempRecord = _QueuePop();
         ESP_LOGD(__FILE__, "S%i | TT:%lld | T:%lld | St:%i", STempRecord.iSensorNumber, STempRecord.llTriggerTime, STempRecord.llTriggerTime - _llRaceStartTime, STempRecord.iSensorState);
      }
      return;
   }

   if (!_QueueEmpty()) //If queue is not empty, we have work to do
   {
      //Get next record from queue
      STriggerRecord STriggerRecord = _QueuePop();
      //If the transition string is not empty and is was not updated for 2 seconds then we have to clear it.
      if (_strTransition.length() != 0 && (GET_MICROS - _llLastTransitionStringUpdate) > 2000000)
      {
         _strTransition = "";
         ESP_LOGD(__FILE__, "Reset transition strings as not updated since 2s.");
      }
      if (_strTransition.length() == 0)
      {
         _bGatesClear = true;
         _bSafeCross = false;
         ESP_LOGD(__FILE__, "Gate: CLEAR as no transition strings");
      }

      ESP_LOGD(__FILE__, "S%i | TT:%lld | T:%lld | St:%i", STriggerRecord.iSensorNumber, STriggerRecord.llTriggerTime, STriggerRecord.llTriggerTime - _llRaceStartTime, STriggerRecord.iSensorState);


      //Calculate what our next dog will be
      if ((_bFault && _bRerunBusy) || (_bFault && iCurrentDog == 3))
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


      //Handle sensor 1 events (handlers side) with gates CLEAR 
      if (STriggerRecord.iSensorNumber == 1 && _bGatesClear //Only if gates are clear
          && STriggerRecord.iSensorState == 1)              //And act on HIGH events (beam broken)
      {
         //First check if we don't have a fault situation
         //Special fault handling for dog 0 when it's not yet in the lane
         if (iCurrentDog == 0 && _byDogState == GOINGIN && STriggerRecord.llTriggerTime < _llPerfectCrossingTime)
         {
            //Dog 0 is too early!
            SetDogFault(iCurrentDog, ON);
            ESP_LOGD(__FILE__, "Fault! Dog: 1! false start");
            _llCrossingTimes[iCurrentDog][_iDogRunCounters[iCurrentDog]] = STriggerRecord.llTriggerTime - _llPerfectCrossingTime;
            _llDogEnterTimes[iCurrentDog] = STriggerRecord.llTriggerTime;
         }
         //Check if this is a next dog which is too early (we are expecting a dog to come back)
         else if (_byDogState == COMINGBACK && (STriggerRecord.llTriggerTime - _llDogEnterTimes[iCurrentDog]) > 1500000) //Filter out S1 HIGH signals that are < 1.5 seconds after dog enter time
         {
            //This dog is too early!
            //We don't increase the dog number at this point. The transition string further down in the code will determine whether current dog came in or not.
            //Set fault light for next dog.
            SetDogFault(iNextDog, ON);

            //For now we assume dogs crossed more or less at the same time.
            //It is very unlikely that a next dog clears the sensors before the previous dog crosses them (this would be a veeery early crossing).
            _llDogExitTimes[iCurrentDog] = STriggerRecord.llTriggerTime;
            _llDogTimes[iCurrentDog][_iDogRunCounters[iCurrentDog]] = STriggerRecord.llTriggerTime - _llDogEnterTimes[iCurrentDog];

            //Handle next dog
            _llDogEnterTimes[iNextDog] = STriggerRecord.llTriggerTime;
            ESP_LOGD(__FILE__, "Fault! Dog: %i! as comingback dog expected and filter 1.5 second expired", iNextDog+1);
         }

         //Normal race handling (no faults)
         if (_byDogState == GOINGIN)
         {
            //Store crossing time
            _llCrossingTimes[iCurrentDog][_iDogRunCounters[iCurrentDog]] = STriggerRecord.llTriggerTime - _llPerfectCrossingTime;
            ESP_LOGD(__FILE__, "Dog %i going in crossed S1 safely. Calculate crossing time. Clear fault if rerun.", iCurrentDog+1);

            //If this dog is doing a rerun we have to turn the error light for this dog off
            if (_bRerunBusy)
            {
               //remove fault for this dog
               SetDogFault(iCurrentDog, OFF);
            }
         }
      }

      ////Handle sensor 1 events (handlers side) with gates state DOG IN
      if (STriggerRecord.iSensorNumber == 1 && !_bGatesClear && _bNegativeCross //Only if gates are busy (dog in) and we have negative cross detected on S2
          && STriggerRecord.iSensorState == 1)              //And only if sensor is HIGH
         {
            //Update crossing time (negative) of previous dog (who is in fault)
            _llCrossingTimes[iCurrentDog][_iDogRunCounters[iCurrentDog]] = _llDogEnterTimes[iCurrentDog] - STriggerRecord.llTriggerTime;
            _bNegativeCross = false;
            ESP_LOGD(__FILE__, "Calculate negative cross time for %i dog", iCurrentDog+1);
         }
      
      if (STriggerRecord.iSensorNumber == 1 && !_bGatesClear && _bSafeCross //Only if gates are busy (dog in) and we have safe cross active (S2 crossed while gates clear)
          && STriggerRecord.iSensorState == 1)              //And only if sensor is HIGH
      {
         //Normal handling for dog coming back or dog going after S2 crossed safely
         _bSafeCross = false;
         _llDogExitTimes[iCurrentDog] = STriggerRecord.llTriggerTime;
         _llDogTimes[iCurrentDog][_iDogRunCounters[iCurrentDog]] = STriggerRecord.llTriggerTime - _llDogEnterTimes[iCurrentDog];
         //The time the dog came in is also the perfect crossing time
         _llPerfectCrossingTime = STriggerRecord.llTriggerTime;
         ESP_LOGI(__FILE__, "S1 line crossed while being safe. Calculate dog %i time. Start counting crossing time for dog %i.", iCurrentDog+1, iNextDog+1);
         if (_llPerfectCrossingTime - _llS2PerfectCrossingTime < 10000) // If S1 (safe) crossing time is below 10ms after S2 crossing means S1 was crossed by going in dog and we have true perfect crossing --> big OK case
         {
            _bDogPerfectCross = true;
            ESP_LOGI(__FILE__, "Perfect cross below 10ms detected for dog %i.", iNextDog+1);
         } 


         if ((iCurrentDog == 3 && _bFault == false && _bRerunBusy == false) //If this is the 4th dog and there is no fault we have to stop the race
               || (_bRerunBusy == true && _bFault == false))                  //Or if the rerun sequence was started but no faults exist anymore
         {
            StopRace(STriggerRecord.llTriggerTime);
            ESP_LOGD(__FILE__, "Last Dog: %i | ENT:%lld | EXIT:%lld | TOT:%lld", iCurrentDog+1, _llDogEnterTimes[iCurrentDog], _llDogExitTimes[iCurrentDog], _llDogTimes[iCurrentDog][_iDogRunCounters[iCurrentDog]]);
         }
         else if ((iCurrentDog == 3 && _bFault == true && _bRerunBusy == false) //If current dog is dog 4 and a fault exists, we have to initiate rerun sequence
                  || _bRerunBusy == true)                                       //Or if rerun is busy (and faults still exist)
         {
            //Dog 3 came in but there is a fault, we have to initiate the rerun sequence
            _bRerunBusy = true;
            //Reset timers for this dog
            _llDogEnterTimes[iNextDog] = STriggerRecord.llTriggerTime;
            _llDogExitTimes[iNextDog] = 0;
            //Increase run counter for this dog
            _iDogRunCounters[iNextDog]++;
            ESP_LOGI(__FILE__, "Re-run for dog %i", iNextDog+1);
         }
         else
         {
            //Store next dog enter time
            _llDogEnterTimes[iNextDog] = STriggerRecord.llTriggerTime;   
         }
      }


      //Handle sensor 2 (box side)
      if (STriggerRecord.iSensorNumber == 2 && _bGatesClear //Only if gates are clear
          && STriggerRecord.iSensorState == 1)              //And only if sensor is HIGH
      {
         if ((_byDogState == GOINGIN))
         {
            /* Gates were clear, we weren't expecting a dog back, but one came back.
            This means we missed the dog going in,
            most likely due to perfect crossing were next dog was faster than previous dog,
            and thus passed through sensors unseen */
            //Set enter time for this dog to exit time of previous dog
            _llDogEnterTimes[iCurrentDog] = _llDogExitTimes[iPreviousDog];
            ESP_LOGD(__FILE__, "Invisible dog came back!. Update his enter time.");
         }

         //Check if current dog has a fault
         //TODO: The current dog could also have a fault which is not caused by being too early (manually triggered fault).
         //We should store the fault type also so we can check if the dog was too early or not.
         if (iCurrentDog != 0                                                           //If dog is not 1st dog
             && _bDogFaults[iCurrentDog]                                                //and current dog has fault
             && (STriggerRecord.llTriggerTime - _llDogEnterTimes[iCurrentDog]) < 2000000) //And S2 is trigger less than 2s after current dog's enter time
                                                                                        //Then we know It's actually the previous dog who's still coming back (current dog was way too early).
         {
            //Update crossing time (negative) of previous dog (who is in fault)
            _bNegativeCross = true;
            //_llCrossingTimes[iCurrentDog][_iDogRunCounters[iCurrentDog]] = _llDogEnterTimes[iCurrentDog] - STriggerRecord.llTriggerTime; / moved to S1
            ESP_LOGD(__FILE__, "Huge crossing fault detected! Prepare to calculate negative cross time.");
         }
         else if ((STriggerRecord.llTriggerTime - _llDogEnterTimes[iCurrentDog]) > 2000000) //Filter out S2 HIGH signals that are < 2 seconds after dog enter time
         {
            _bSafeCross = true;
            _llS2PerfectCrossingTime = STriggerRecord.llTriggerTime;
            ESP_LOGD(__FILE__, "Dog %i coming back crossed S2 line. S1 is safe.", iCurrentDog+1);
         
         /* OLD CODE with S2 used as end line 
         //Normal handling for dog coming back
            _llDogExitTimes[iCurrentDog] = STriggerRecord.llTriggerTime; //SIMON: not true, this need to be handle by S1
            _llDogTimes[iCurrentDog][_iDogRunCounters[iCurrentDog]] = STriggerRecord.llTriggerTime - _llDogEnterTimes[iCurrentDog]; //SIMON: not true, this need to be handle by S1
            //The time the dog came in is also the perfect crossing time
            _llPerfectCrossingTime = STriggerRecord.llTriggerTime;

            if ((iCurrentDog == 3 && _bFault == false && _bRerunBusy == false) //If this is the 4th dog and there is no fault we have to stop the race
                || (_bRerunBusy == true && _bFault == false))                  //Or if the rerun sequence was started but no faults exist anymore
            {
               StopRace(STriggerRecord.llTriggerTime);
               ESP_LOGD(__FILE__, "Last Dog: %i | ENT:%lld | EXIT:%lld | TOT:%lld", iCurrentDog+1, _llDogEnterTimes[iCurrentDog], _llDogExitTimes[iCurrentDog], _llDogTimes[iCurrentDog][_iDogRunCounters[iCurrentDog]]);
            }
            else if ((iCurrentDog == 3 && _bFault == true && _bRerunBusy == false) //If current dog is dog 4 and a fault exists, we have to initiate rerun sequence
                     || _bRerunBusy == true)                                       //Or if rerun is busy (and faults still exist)
            {
               //Dog 3 came in but there is a fault, we have to initiate the rerun sequence
               _bRerunBusy = true;
               //Reset timers for this dog
               _llDogEnterTimes[iNextDog] = STriggerRecord.llTriggerTime;
               _llDogExitTimes[iNextDog] = 0;
               //Increase run counter for this dog
               _iDogRunCounters[iNextDog]++;
               ESP_LOGI(__FILE__, "Re-run for dog %i", iNextDog+1);
            }
            else
            {
               //Store next dog enter time
               _llDogEnterTimes[iNextDog] = STriggerRecord.llTriggerTime;
            }
         */
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
      if (_strTransition.length() == 0                                             //String might still be empty, in which case the gates were clear
          || (strLast2TransitionChars == "ab" || strLast2TransitionChars == "ba")) //Sensors going low in either direction indicate gates are clear
      {
         //The gates are clear, set boolean
         _bGatesClear = true;
         ESP_LOGD(__FILE__, "Gate: CLEAR as ab/ba detected");

         //Print the transition string up til now for debugging purposes
         ESP_LOGD(__FILE__, "Tstring: %s", _strTransition.c_str());

         //Only check transition string when gates are clear
         //TODO: If transistion string is 3 or longer but actually more events are coming related to same transition, these are not considered.
         if (_strTransition.length() > 3) //And if transistion string is at least 4 characters long
         {
            //Transition string is 4 characters or longer
            //So we can check what happened
            if (_strTransition == "ABab") //Dog going to box
            {
               //Change dog state to coming back
               _ChangeDogState(COMINGBACK);
               ESP_LOGD(__FILE__, "New dog state: COMINGBACK. String was ABab.");
            }
            else if (_strTransition == "BAba") //Dog coming back
            {
               //Normal handling, change dog state to GOING IN
               _ChangeDogState(GOINGIN);
               ESP_LOGD(__FILE__, "New dog state: GOINGING. String was BAba");
               //Set next dog active
               _ChangeDogNumber(iNextDog);
               
            }
            else if (_strTransition == "BbAa")
            {
               //Transistion string BbAa indicates small object has passed through sensors
               //Most likely dog spat ball
               ESP_LOGI(__FILE__, "Spat ball detected?!");
               SetDogFault(iCurrentDog, ON);
            }
            else 
            {
               //If Transition string indicated something else and we expect coming back dog, it means 2 dogs must have passed
               if (_byDogState == COMINGBACK)
               {
                  //Transition string indicates more than 1 dog passed
                  //We increase the dog number
                  _ChangeDogNumber(iNextDog);

                  //If this is Re-run and dog had fault active we need to turn it OFF if this is perfect crossing case during re-run
                  if ((_bRerunBusy == true && _bFault == true))
                  {
                     SetDogFault(iCurrentDog, OFF);
                     ESP_LOGD(__FILE__, "Clear dog %i fault. String unlcear", iCurrentDog+1);
                  }

                  // and set perfect crossing time for new dog
                  _ChangeDogState(COMINGBACK);
                  ESP_LOGD(__FILE__, "Perfect crossing (string unclear) for dog %i. New state: COMINGBACK.", iCurrentDog+1);
                  _llCrossingTimes[iCurrentDog][_iDogRunCounters[iCurrentDog]] = 0;
                  _llDogEnterTimes[iCurrentDog] = _llDogExitTimes[iPreviousDog];
                  if (_bDogPerfectCross) // if this dog crossingn time was below 10ms, flag for this dog Big OK and clear dog perfect cross
                  {
                     _bDogBigOK[iCurrentDog][_iDogRunCounters[iCurrentDog]] = true;
                     _bDogPerfectCross = false;
                     ESP_LOGD(__FILE__, "Big OK stored for dog %i.", iCurrentDog+1);
                  }
               }
               else
               //It has to be dog going in + sensors noise
               {
                  _ChangeDogState(COMINGBACK);
                  ESP_LOGD(__FILE__, "New dog %i state: COMINGBACK with sensors noise", iCurrentDog+1);
               }
               
            }
         }
         _strTransition = "";
      }
      else
      {
         _bGatesClear = false;
         ESP_LOGD(__FILE__, "Gate: DOG(s)");
      }
   }

   //Update racetime
   if (RaceState == RUNNING)
   {
      if (GET_MICROS > _llRaceStartTime)
      {
         _llRaceTime = GET_MICROS - _llRaceStartTime;
      }
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
   _llRaceStartTime = GET_MICROS + 3000000;
   _llPerfectCrossingTime = _llRaceStartTime;
   _llDogEnterTimes[0] = _llRaceStartTime;
}

/// <summary>
///   Stops a race.
/// </summary>
void RaceHandlerClass::StopRace()
{
   this->StopRace(GET_MICROS);
}

/// <summary>
///   Stops a race.
/// </summary>
/// <param name="StopTime">   The time in microseconds at which the race stopped. </param>
void RaceHandlerClass::StopRace(long long llStopTime)
{
   if (RaceState == RUNNING)
   {
      //Race is running, so we have to record the EndTime
      _llRaceEndTime = llStopTime;
      _llRaceTime = _llRaceEndTime - _llRaceStartTime;
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
      uint8_t iRecordToPrintIndex = 0;
      while (iRecordToPrintIndex < _iInputQueueWriteIndex)
      {
         STriggerRecord RecordToPrint = _InputTriggerQueue[iRecordToPrintIndex];
         printf("{%i, %lld, %i},\n", RecordToPrint.iSensorNumber, RecordToPrint.llTriggerTime - _llRaceStartTime, RecordToPrint.iSensorState);
         iRecordToPrintIndex++;
      }

      iCurrentDog = 0;
      iPreviousDog = 0;
      _llRaceStartTime = 0;
      _llRaceEndTime = 0;
      _llRaceTime = 0;
      _llPerfectCrossingTime = 0;
      _byDogState = GOINGIN;
      _ChangeDogNumber(0);
      _bFault = false;
      _bRerunBusy = false;
      _iOutputQueueReadIndex = 0;
      _iInputQueueReadIndex = 0;
      _iOutputQueueWriteIndex = 0;
      _iInputQueueWriteIndex = 0;
      _strTransition = "";
      _bGatesClear = true;
      _bSafeCross = false;
      _bDogPerfectCross = false;

      for (auto &bFault : _bDogFaults)
      {
         bFault = false;
      }
      for (auto &llTime : _llDogEnterTimes)
      {
         llTime = 0;
      }
      for (auto &llTime : _llDogExitTimes)
      {
         llTime = 0;
      }
      for (auto &Dog : _llDogTimes)
      {
         for (auto &llTime : Dog)
         {
            llTime = 0;
         }
      }
      for (auto &Dog : _llCrossingTimes)
      {
         for (auto &llTime : Dog)
         {
            llTime = 0;
         }
      }
      for (auto &Dog : _bDogBigOK)
      {
         for (auto &llTime : Dog)
         {
            llTime = 0;
         }
      }
      for (auto &iCounter : _iDogRunCounters)
      {
         iCounter = 0;
      }
      for (auto &llTime : _llLastDogTimeReturnTimeStamp)
      {
         llTime = 0;
      }
      for (auto &iCounter : _iLastReturnedRunNumber)
      {
         iCounter = 0;
      }
      _ChangeRaceState(RESET);
   }

   if (_iCurrentRaceId == NUM_HISTORIC_RACE_RECORDS)
   {
      _iCurrentRaceId = 0;
   }
   else
   {
      _iCurrentRaceId++;
   }
   ESP_LOGD(__FILE__, "Reset Race: DONE");
   //Send updated racedata to any web clients
   WebHandler._SendRaceData();
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
   if (RaceState == STOPPED || RaceState == RESET)
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
      ESP_LOGI(__FILE__, "Dog%i fault ON", iDogNumber+1);
   }
   else
   {
      //If fault is false, turn off fault light for this dog
      LightsController.ToggleFaultLight(iDogNumber, LightsController.OFF);
      ESP_LOGI(__FILE__, "Dog%i fault OFF", iDogNumber+1);
   }
}

/// <summary>
///   ISR function for sensor 1, this function will record the sensor number, microseconds and
///   state (HIGH/LOW) of the sensor in the interrupt queue.
/// </summary>
void RaceHandlerClass::TriggerSensor1()
{
   if (RaceState == STOPPED && GET_MICROS > (_llRaceEndTime + 1500000))
   {
      return;
   }
   else if (RaceState == RESET)
   {
      if (digitalRead(_iS1Pin) == 1)
      {
         LightsController.ReaceReadyFault(LightsController.ON);
      }
      else
      {
         LightsController.ReaceReadyFault(LightsController.OFF);
      }
   }
   else
   _QueuePush({_bRunDirectionInverted ? 2 : 1, GET_MICROS, digitalRead(_iS1Pin)});
}

/// <summary>
///   ISR function for sensor 2, this function will record the sensor number, microseconds and
///   state (HIGH/LOW) of the sensor in the interrupt queue.
/// </summary>
void RaceHandlerClass::TriggerSensor2()
{
   if (RaceState == STOPPED && GET_MICROS > (_llRaceEndTime + 1500000))
   {
      return;
   }
   else if (RaceState == RESET)
   {
      if (digitalRead(_iS2Pin) == 1)
      {
         LightsController.ReaceReadyFault(LightsController.ON);
      }
      else
      {
         LightsController.ReaceReadyFault(LightsController.OFF);
      }
   }
   else
   _QueuePush({_bRunDirectionInverted ? 1 : 2, GET_MICROS, digitalRead(_iS2Pin)});
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
      dRaceTimeSeconds = _llRaceTime / 1000000.0;
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
   double dDogTime;
   long long llDogTimeMillis = GetDogTimeMillis(iDogNumber, iRunNumber);
   dDogTime = llDogTimeMillis / 1000.0;

   return dDogTime;
}
unsigned long RaceHandlerClass::GetDogTimeMillis(uint8_t iDogNumber, int8_t iRunNumber)
{
   long long llDogTimeMillis = 0;
   if (_iDogRunCounters[iDogNumber] > 0)
   {
      //We have multiple times for this dog.
      //if run number is -1 (unspecified), we have to cycle throug them
      if (iRunNumber == -1)
      {
         auto &llLastReturnedTimeStamp = _llLastDogTimeReturnTimeStamp[iDogNumber];
         iRunNumber = _iLastReturnedRunNumber[iDogNumber];
         if ((GET_MICROS / 1000 - llLastReturnedTimeStamp) > 2000)
         {
            if (iRunNumber == _iDogRunCounters[iDogNumber])
            {
               iRunNumber = 0;
            }
            else
            {
               iRunNumber++;
            }
            llLastReturnedTimeStamp = GET_MICROS / 1000;
         }
         _iLastReturnedRunNumber[iDogNumber] = iRunNumber;
      }
      else if (iRunNumber == -2)
      {
         //if RunNumber is -2 it means we should return the last one
         iRunNumber = _iDogRunCounters[iDogNumber];
      }
   }
   else if (iRunNumber < 0)
   {
      iRunNumber = 0;
   }

   //First check if we have final time for the requested dog number
   if (_llDogTimes[iDogNumber][iRunNumber] > 0)
   {
      llDogTimeMillis = _llDogTimes[iDogNumber][iRunNumber] / 1000;
   }
   //Then check if the requested dog is perhaps running (and coming back) so we can return the time so far
   else if ((RaceState == RUNNING && iCurrentDog == iDogNumber && _byDogState == COMINGBACK) && iRunNumber <= _iDogRunCounters[iDogNumber]) //And if requested run number is lower then number of times dog has run
   {
      llDogTimeMillis = (GET_MICROS - _llDogEnterTimes[iDogNumber]) / 1000;
   }

   //Fixes issue 7 (https://github.com/vyruz1986/FlyballETS-Software/issues/7)
   //Only deduct crossing time if it is positive
   if (_llCrossingTimes[iDogNumber][iRunNumber] > 0 && llDogTimeMillis > (_llCrossingTimes[iDogNumber][iRunNumber] / 1000))
   {
      llDogTimeMillis -= (_llCrossingTimes[iDogNumber][iRunNumber] / 1000);
   }
   return llDogTimeMillis;
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

   long long llCrossingTimeMillis = 0;
   if (_iDogRunCounters[iDogNumber] > 0)
   {
      //We have multiple times for this dog.
      //if run number is -1 (unspecified), we have to cycle throug them
      if (iRunNumber == -1)
      {
         auto &llLastReturnedTimeStamp = _llLastDogTimeReturnTimeStamp[iDogNumber];
         iRunNumber = _iLastReturnedRunNumber[iDogNumber];
         if ((GET_MICROS / 1000 - llLastReturnedTimeStamp) > 2000)
         {
            if (iRunNumber == _iDogRunCounters[iDogNumber])
            {
               iRunNumber = 0;
            }
            else
            {
               iRunNumber++;
            }
            llLastReturnedTimeStamp = GET_MICROS / 1000;
         }
         _iLastReturnedRunNumber[iDogNumber] = iRunNumber;
      }
      else if (iRunNumber == -2)
      {
         //if RunNumber is -2 it means we should return the last one
         iRunNumber = _iDogRunCounters[iDogNumber];
      }
   }
   else if (iRunNumber < 0)
   {
      iRunNumber = 0;
   }

   llCrossingTimeMillis = _llCrossingTimes[iDogNumber][iRunNumber] / 1000;

   dCrossingTime = llCrossingTimeMillis / 1000.0;

   if (dCrossingTime < 0)
   {
      dCrossingTime = fabs(dCrossingTime);
      strCrossingTime = "-";
      dtostrf(dCrossingTime, 7, 3, cCrossingTime);
      strCrossingTime += cCrossingTime;
   }
   else if (dCrossingTime > 0)
   {
      strCrossingTime = "+";
      dtostrf(dCrossingTime, 7, 3, cCrossingTime);
      strCrossingTime += cCrossingTime;
   }
   else if (_bDogBigOK[iDogNumber][iRunNumber] == 1)
   {
      strCrossingTime = "      OK";
   }
   else
   {
      if (_llDogTimes[iDogNumber][iRunNumber] > 0)
      {
         if ((_iDogRunCounters[iDogNumber] > 0 && _iDogRunCounters[iDogNumber] != iRunNumber) || (_iDogRunCounters[iDogNumber] == 0 && _bDogFaults[iDogNumber] == true))
         {
            strCrossingTime = "   fault";
         }
         else
         {
            strCrossingTime = "      ok";
         }
      }
      else if ((RaceState == RUNNING && iCurrentDog == iDogNumber && _byDogState == COMINGBACK) && iRunNumber <= _iDogRunCounters[iDogNumber] && _bDogFaults[iDogNumber] == true) //If still running and fault active
      {
         strCrossingTime = "   fault";
      }
      else if ((RaceState == RUNNING && iCurrentDog == iDogNumber && _byDogState == COMINGBACK) && iRunNumber <= _iDogRunCounters[iDogNumber] && _bDogFaults[iDogNumber] == false) //If still running and no active fault
      {
         strCrossingTime = "      ok";
      }
      else
      {
         strCrossingTime = " ";
      }
   }

   return strCrossingTime;
}
long RaceHandlerClass::GetCrossingTimeMillis(uint8_t iDogNumber, int8_t iRunNumber)
{
   long long llCrossingTime = 0;
   if (_iDogRunCounters[iDogNumber] > 0)
   {
      //We have multiple times for this dog.
      //if run number is -1 (unspecified), we have to cycle throug them
      if (iRunNumber == -1)
      {
         auto &llLastReturnedTimeStamp = _llLastDogTimeReturnTimeStamp[iDogNumber];
         iRunNumber = _iLastReturnedRunNumber[iDogNumber];
         if ((GET_MICROS / 1000 - llLastReturnedTimeStamp) > 2000)
         {
            if (iRunNumber == _iDogRunCounters[iDogNumber])
            {
               iRunNumber = 0;
            }
            else
            {
               iRunNumber++;
            }
            llLastReturnedTimeStamp = GET_MICROS / 1000;
         }
         _iLastReturnedRunNumber[iDogNumber] = iRunNumber;
      }
      else if (iRunNumber == -2)
      {
         //if RunNumber is -2 it means we should return the last one
         iRunNumber = _iDogRunCounters[iDogNumber];
      }
   }
   else if (iRunNumber < 0)
   {
      iRunNumber = 0;
   }

   llCrossingTime = _llCrossingTimes[iDogNumber][iRunNumber] / 1000;

   return llCrossingTime;
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
///   Gets total crossing time (milliseconds) without consideration negative crosses or false start as those are inluded in Race Time.
///   Result will be use to calculate team Net Time that is equal to Clean Time in case of clean heat (no single fault during the heat).
/// </summary>
long RaceHandlerClass::GetTotalCrossingTimeMillis()
{
   long long llTotalCrossingTime = 0;

   for (auto &Dog : _llCrossingTimes)
   {
      for (auto &llTime : Dog)
      {
         if (llTime > 0)
         {
            llTotalCrossingTime += llTime;
         }
      }
   }
   return llTotalCrossingTime / 1000;
}

/// <summary>
///   Gets team Net Time in seconds (see also GetTotalCrossingTimeMillis description above).
///   In case of heat without faults (clean heat) Net Time == Clean Time.
///   Please mark that "Cleat Time" term is often use in the meaning of Cleat Time Breakout.
///   Please refer to FCI Regulations for Flyball Competition section 1.03 point (h).
/// </summary>
double RaceHandlerClass::GetNetTime()
{
   //double dNetTime = GetRaceTime() - (GetTotalCrossingTimeMillis() / 1000.0);
      long long llTotalNetTime = 0;

   for (auto &Dog : _llDogTimes)
   {
      for (auto &llNetTime : Dog)
      {
         if (llNetTime > 0)
         {
            llTotalNetTime += llNetTime;
         }
      }
   }
   double dNetTime = llTotalNetTime / 1000000.0 - GetTotalCrossingTimeMillis() / 1000.0;
   return dNetTime;
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
      strRaceState = " STOP  ";
      break;
   case RaceHandlerClass::STARTING:
      strRaceState = " START ";
      break;
   case RaceHandlerClass::RUNNING:
      strRaceState = "RUNNING";
      break;
   case RaceHandlerClass::RESET:
      strRaceState = " READY ";
      break;
   default:
      break;
   }

   return strRaceState;
}

/// <summary>
///   Gets race data for the current race
/// </summary>
///
/// <returns>
///   The race data struct
/// </returns>
stRaceData RaceHandlerClass::GetRaceData()
{
   return GetRaceData(_iCurrentRaceId);
}

/// <summary>
///   Gets race data for given race ID
/// </summary>

/// <param name="iRaceId">The ID for the race you want the data for</param>
///
/// <returns>
///  Race data struct
/// </returns>
stRaceData RaceHandlerClass::GetRaceData(uint iRaceId)
{
   stRaceData RequestedRaceData;

   if (iRaceId == _iCurrentRaceId)
   {
      //We need to return data for the current dace
      RequestedRaceData.Id = _iCurrentRaceId;
      RequestedRaceData.StartTime = _llRaceStartTime / 1000;
      RequestedRaceData.EndTime = _llRaceEndTime / 1000;
      RequestedRaceData.ElapsedTime = _llRaceTime / 1000;
      //Serial.printf("Elapsed1: %lld - %lld = %lld\r\n", micros(), _llRaceStartTime, _llRaceTime);
      //Serial.printf("Elapsed2: %lld - %lld = %lld\r\n", GET_MICROS, _llRaceStartTime, _llRaceTime);
      RequestedRaceData.TotalCrossingTime = this->GetTotalCrossingTimeMillis();
      RequestedRaceData.RaceState = RaceState;

      //Get Dog info
      for (uint8_t i = 0; i < 4; i++)
      {
         RequestedRaceData.DogData[i].DogNumber = i;

         for (uint8_t i2 = 0; i2 < 4; i2++)
         {
            RequestedRaceData.DogData[i].Timing[i2].Time = GetDogTimeMillis(i, i2);
            RequestedRaceData.DogData[i].Timing[i2].CrossingTime = GetCrossingTimeMillis(i, i2);
         }
         RequestedRaceData.DogData[i].Fault = _bDogFaults[i];
         RequestedRaceData.DogData[i].Running = (iCurrentDog == i);
      }
   }
   else
   {
      RequestedRaceData = _HistoricRaceData[iRaceId];
   }
   return RequestedRaceData;
}

/// <summary>
///   Toggles the direction the system expects dogs to run in
/// </summary>
void RaceHandlerClass::ToggleRunDirection()
{
   _bRunDirectionInverted = !_bRunDirectionInverted;
   SettingsManager.setSetting("RunDirectionInverted", String(_bRunDirectionInverted));
   if (_bRunDirectionInverted)
   {
      LCDController.UpdateField(LCDController.BoxDirection, "<--");
   }
   else
   {
      LCDController.UpdateField(LCDController.BoxDirection, "-->");
   }
}

/// <summary>
///   Returns the run direction
/// </summary>
/// <returns>
///   false means normal direction (box to right)
///   true means inverted direction (box to left)
/// </returns>
boolean RaceHandlerClass::GetRunDirection()
{
   return _bRunDirectionInverted;
}

/// <summary>
///   Pushes an Sensor interrupt trigger record to the back of the input interrupts buffer.
/// </summary>
///
/// <param name="_InterruptTrigger">   The interrupt trigger record. </param>
void RaceHandlerClass::_QueuePush(RaceHandlerClass::STriggerRecord _InterruptTrigger)
{
   //Add record to queue
   _InputTriggerQueue[_iInputQueueWriteIndex] = _InterruptTrigger;

   //Write index has to be increased, check it we should wrap-around
   if (_iInputQueueWriteIndex == TRIGGER_QUEUE_LENGTH - 1) //(sizeof(_OutputTriggerQueue) / sizeof(*_OutputTriggerQueue) - 1))
   {
      //Write index has reached end of array, start at 0 again
      _iInputQueueWriteIndex = 0;
   }
   else
   {
      //End of array not yet reached, increase index by 1
      _iInputQueueWriteIndex++;
   }
}

/// <summary>
///   Filter input interrupt record(s) from the front of the interrupts buffer
///   and push filtered records to output interrupts records queue
/// </summary>
void RaceHandlerClass::_QueueFilter()
{
   STriggerRecord _CurrentRecord = _InputTriggerQueue[_iInputQueueReadIndex];
   
   if (_iInputQueueReadIndex <= _iInputQueueWriteIndex - 2)
   {
      STriggerRecord _NextRecord = _InputTriggerQueue[_iInputQueueReadIndex+1];

      // If 2 records are from the same sensors line and delta time is below 4ms ignore both
      if (_CurrentRecord.iSensorNumber == _NextRecord.iSensorNumber && _NextRecord.llTriggerTime - _CurrentRecord.llTriggerTime <= 4000)
      {
         ESP_LOGD(__FILE__, "Next record %lld - Current record %lld = %lld < 4ms (4000) ", _NextRecord.llTriggerTime, _CurrentRecord.llTriggerTime, _NextRecord.llTriggerTime - _CurrentRecord.llTriggerTime); 
         ESP_LOGD(__FILE__, "S%i | TT:%lld | T:%lld | St:%i | IGNORED", _CurrentRecord.iSensorNumber, _CurrentRecord.llTriggerTime,
         _CurrentRecord.llTriggerTime - _llRaceStartTime, _CurrentRecord.iSensorState);
         ESP_LOGD(__FILE__, "S%i | TT:%lld | T:%lld | St:%i | IGNORED", _NextRecord.iSensorNumber, _NextRecord.llTriggerTime,
         _NextRecord.llTriggerTime - _llRaceStartTime, _NextRecord.iSensorState);
         
         //Input Read index has to be increased, check it we should wrap-around
         if (_iInputQueueReadIndex == TRIGGER_QUEUE_LENGTH - 2)
         {
            //Input Read index has reached end of array, start at 0 again
            _iInputQueueReadIndex = 0;
         }
         else
         {
            //End of array not yet reached, increase index by 2
            _iInputQueueReadIndex = _iInputQueueReadIndex + 2;
         }
      }
      else
      {
         //Next record is for different sensor line or delta time is higher than 4ms. Push Current record Output Queue.
         ESP_LOGD(__FILE__, "Next record > 4ms or for different sensors line. Push Current S%i record %lld to Output Queue.", _CurrentRecord.iSensorNumber, _CurrentRecord.llTriggerTime); 
         //This function copy current record to common interrupt queue
         _OutputTriggerQueue[_iOutputQueueWriteIndex] = _InputTriggerQueue[_iInputQueueReadIndex];
         
         //Input Read index has to be increased, check it we should wrap-around
         if (_iInputQueueReadIndex == TRIGGER_QUEUE_LENGTH - 1)
         {
            //Input Read index has reached end of array, start at 0 again
            _iInputQueueReadIndex = 0;
         }
         else
         {
            //End of array not yet reached, increase index by 1
            _iInputQueueReadIndex++;
         }   

         //Output Write index has to be increased, check it we should wrap-around
         if (_iOutputQueueWriteIndex == TRIGGER_QUEUE_LENGTH - 1)
         {
            //Output Write index has reached end of array, start at 0 again
            _iOutputQueueWriteIndex = 0;
         }
         else
         {
            //End of array not yet reached, increase index by 1
            _iOutputQueueWriteIndex++;
         }
      }
      
   }
   else
   {
      //Only one record available in the Input Queue. Push it to Output Queue.
      ESP_LOGD(__FILE__, "One record in the Input Queue. Push S%i record %lld to Output Queue.", _CurrentRecord.iSensorNumber, _CurrentRecord.llTriggerTime); 
      //This function copy current record to common interrupt queue
      _OutputTriggerQueue[_iOutputQueueWriteIndex] = _InputTriggerQueue[_iInputQueueReadIndex];
      
      //Input Read index has to be increased, check it we should wrap-around
      if (_iInputQueueReadIndex == TRIGGER_QUEUE_LENGTH - 1)
      {
         //Input Read index has reached end of array, start at 0 again
         _iInputQueueReadIndex = 0;
      }
      else
      {
         //End of array not yet reached, increase index by 1
         _iInputQueueReadIndex++;
      }   

      //Output Write index has to be increased, check it we should wrap-around
      if (_iOutputQueueWriteIndex == TRIGGER_QUEUE_LENGTH - 1)
      {
         //Output Write index has reached end of array, start at 0 again
         _iOutputQueueWriteIndex = 0;
      }
      else
      {
         //End of array not yet reached, increase index by 1
         _iOutputQueueWriteIndex++;
      }
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
   STriggerRecord NextRecord = _OutputTriggerQueue[_iOutputQueueReadIndex];

   //Read index has to be increased, check it we should wrap-around
   if (_iOutputQueueReadIndex == TRIGGER_QUEUE_LENGTH - 1) //(sizeof(_OutputTriggerQueue) / sizeof(*_OutputTriggerQueue) - 1))
   {
      //Read index has reached end of array, start at 0 again
      _iOutputQueueReadIndex = 0;
   }
   else
   {
      //End of array not yet reached, increase index by 1
      _iOutputQueueReadIndex++;
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
   if (_iOutputQueueReadIndex == _iOutputQueueWriteIndex)
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

   _llLastTransitionStringUpdate = GET_MICROS;

   char cTemp;
   switch (_InterruptTrigger.iSensorNumber)
   {
   case 1:
      cTemp = 'A';
      break;

   case 2:
      cTemp = 'B';
      break;
   default:
      cTemp = 'X';
      break;
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
