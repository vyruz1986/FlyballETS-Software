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
#include "SDcardController.h"

/// <summary>
///   Initialises this object andsets all counters to 0.
/// </summary>
///
/// <param name="iS1Pin">  Zero-based index of the S1 pin. </param>
/// <param name="iS2Pin">  Zero-based index of the S2 pin. </param>
void RaceHandlerClass::init(uint8_t iS1Pin, uint8_t iS2Pin)
{
   // Start in ready/reset state
   _iS1Pin = iS1Pin;
   _iS2Pin = iS2Pin;
   if (SettingsManager.getSetting("RunDirectionInverted").equals("1"))
   {
      bRunDirectionInverted = true;
      LCDController.UpdateField(LCDController.BoxDirection, "<");
      LCDController.bExecuteLCDUpdate = true;
      log_i("Run direction from settings: inverted");
   }
   else
      log_i("Run direction from settings: normal");

   if (SettingsManager.getSetting("Accuracy3digits").equals("1"))
   {
      _bAccuracy3digits = true;
      log_i("Accuracy from settings: 3 digits");
   }
   else
      log_i("Accuracy from settings: 2 digits");
   LCDController.bUpdateAccuracyOnLCD = true;
   LCDController.bExecuteLCDUpdate = true;
}

/// <summary>
///   Main entry-point for this application. This function should be called once every main loop.
///   It will check if any new interrupts were saved from the sensors, and handle them if this
///   the case. All timing related data and also fault handling of the dogs is done in this
///   function.
/// </summary>
void RaceHandlerClass::Main()
{
   // Check if race state should be changed to RUNNING
   if (RaceState == STARTING && MICROS >= llRaceStartTime)
   {
      _ChangeRaceState(RUNNING);
      // log_d("GREEN light is ON!");
   }

   // Update racetime
   if (RaceState == RUNNING)
   {
      if (MICROS > llRaceStartTime)
         llRaceTime = MICROS - llRaceStartTime;
      // If race last for 10 minutes race will be stopped
      if (llRaceTime > 600000000)
      {
         log_w("Race TIMEOUT!!!");
         StopRace(llRaceTime);
      }
   }

   // Trigger filterring of sensors interrupts if new records available and 12ms waiting time passed
   while ((_iInputQueueReadIndex != _iInputQueueWriteIndex) && ((MICROS - _InputTriggerQueue[_iInputQueueWriteIndex - 1].llTriggerTime) > 12000))
   {
      log_v("IQRI:%d | IQWI:%d | Delta:%lld | RaceTime:%lld", _iInputQueueReadIndex, _iInputQueueWriteIndex, MICROS - _InputTriggerQueue[_iInputQueueWriteIndex - 1].llTriggerTime, MICROS - llRaceStartTime);
      _QueueFilter();
   }

   if (bExecuteStopRace)
   {
      StopRace();
      bExecuteStopRace = false;
   }

   if (bExecuteResetRace)
   {
      ResetRace();
      bExecuteResetRace = false;
   }

   if (bExecuteStartRaceTimer)
   {
      StartRaceTimer();
      bExecuteStartRaceTimer = false;
   }

   if (!_QueueEmpty()) // If queue is not empty, we have work to do
   {
      // Get next record from queue
      STriggerRecord STriggerRecord = _QueuePop();
      // If the transition string is not empty and it wasn't updated for 350ms then it was noise and we have to clear it.
      // Forst first entering dog filtering is 750ms to cover scenario from simulated race 39.
      if (_strTransition.length() != 0 && ((MICROS - _llLastTransitionStringUpdate) > 350000 && (iCurrentDog != 0 || (iCurrentDog == 0 && _bRerunBusy)) //
                                           || (MICROS - _llLastTransitionStringUpdate) > 750000 && iCurrentDog == 0 && !_bRerunBusy))
      {
         if (_byDogState == GOINGIN)
         {
            // If dog state is GOINGIN, but we have noise of S2 line, then next dog had to go invisibly by the gate (simulated race 25 & 18-41)
            if (_strTransition.substring(_strTransition.length() - 1) == "b" && _strPreviousTransitionFirstLetter == "B")
            {
               _ChangeDogState(COMINGBACK);
               _bS1StillSafe = false;
               _llDogEnterTimes[iCurrentDog] = _llLastDogExitTime;
               log_d("Seems next dog entered gate already as S2 state 'b' detected. S1 is not safe anymore.");
            }
            // If dog state is GOINGIN we could have false detection of entering dog so we need to set S1StillSafe flag
            else
            {
               _bS1StillSafe = true;
               log_d("False entering dog detected. S1 is again safe.");
            }
         }
         else
         {
            // Coming back dog caused S1 sensor noise after gate clear detection (simulated race 21 & 37)
            if (_strTransition.substring(_strTransition.length() - 1) == "a" && _strPreviousTransitionFirstLetter == "B")
            {
               _ChangeDogState(GOINGIN);
               _bS1StillSafe = true;
               log_d("Seems next dog stil didn't enter gate as S1 state 'a' detected.");
            }
            else
            {
               _bS1StillSafe = false;
               log_d("Noise caused by entering dog detected. S1 is not safe anymore.");
            }
         }
         _strTransition = "";
         _bSensorNoise = false;
         _bGatesClear = true;
         log_d("Reset transition strings.");
         log_d("Gate: CLEAR.");
      }

      // If we have potential negative cross detected on S2, but S1 wasn't crossed for 100ms since then it had to be sensor noise and we need to reset flag
      if (_bPotentialNegativeCrossDetected && (MICROS - _llS2CrossedUnsafeGetMicrosTime) > 100000)
      {
         _bPotentialNegativeCrossDetected = false;
         _strTransition = "";
         _bGatesClear = true;
         log_d("Potential negative cross flag reset as S1 not crossed for 100ms");
      }

      log_d("S%i | TT:%lld | T:%lld | St:%i", STriggerRecord.iSensorNumber, STriggerRecord.llTriggerTime, STriggerRecord.llTriggerTime - llRaceStartTime, STriggerRecord.iSensorState);

      // Calculate what our next dog will be
      uint8_t iNextDogChanged = iNextDog;
      if (_bFault && iCurrentDog == (iNumberOfRacingDogs - 1) && iNextDog < 5)
      {
         // If reruns are turned off set next do to dummy value 5
         if (bRerunsOff)
            iNextDog = 5;
         else
         {
            // In case last dog before reruns is running and fault flag is active we have to check which the next offending dog starting from dog 1
            for (uint8_t i = 0; i < iNumberOfRacingDogs; i++)
            {
               if (_bDogFaults[i] || _bDogManualFaults[i])
               {
                  // Set dognumber to next/first offending dog
                  iNextDog = i;
                  break;
               }
            }
         }
      }
      else if (_bFault && _bRerunBusy && !bRerunsOff)
      {
         // In case we have re-runs in progress we have to check which the next offending dog with proper order
         for (uint8_t i = (iCurrentDog + 1); i < iNumberOfRacingDogs; i++)
         {
            if (_bDogFaults[i] || _bDogManualFaults[i])
            {
               // Set dognumber to next offending dog
               iNextDog = i;
               _bNextDogFound = true;
               break;
            }
         }
         if (!_bNextDogFound)
         {
            for (uint8_t i = 0; (iCurrentDog + 1); i++)
            {
               if (_bDogFaults[i] || _bDogManualFaults[i])
               {
                  // Set dognumber to next offending dog
                  iNextDog = i;
                  _bNextDogFound = false;
                  break;
               }
            }
         }
         _bNextDogFound = false;
      }
      else if ((!bRerunsOff && !_bFault && iCurrentDog == (iNumberOfRacingDogs - 1)) || (!_bFault && _bRerunBusy))
         iNextDog = iCurrentDog;
      else if (iNextDog != iCurrentDog + 1 && iNextDog < 5)
         // First runs of every dog, just increase number
         iNextDog = iCurrentDog + 1;
      if (iNextDogChanged != iNextDog)
         log_d("Next Dog is %i.", iNextDog + 1);

      // Handle sensor 1 events (handlers side) with gates CLEAR
      if (STriggerRecord.iSensorNumber == 1 && STriggerRecord.iSensorState == 1 && _bGatesClear && iCurrentDog < 5) // Only if gates are clear and S1 sensor is HIGH (A)
      {
         // Update race elapsed time
         _llRaceElapsedTime = STriggerRecord.llTriggerTime - llRaceStartTime;
         // Special handling for first dog during race start (excluding possible re-run)
         if (_byDogState == GOINGIN && iCurrentDog == 0 && !_bRerunBusy)
         {
            _llDogEnterTimes[iCurrentDog] = STriggerRecord.llTriggerTime;
            _llCrossingTimes[iCurrentDog][iDogRunCounters[iCurrentDog]] = _llDogEnterTimes[iCurrentDog] - llRaceStartTime;
            //_ChangeDogState(COMINGBACK); //Moved to Tstring section
            // False start handling
            if (STriggerRecord.llTriggerTime < llRaceStartTime)
            {
               SetDogFault(iCurrentDog, ON);
               log_i("Dog 1 FALSE START!");
            }
         }
         // Normal race handling (positive cross)
         else if (_byDogState == GOINGIN && (iCurrentDog != 0 || (iCurrentDog == 0 && _bRerunBusy)) && _bS1StillSafe)
         {
            // If this is not re-run and last string was BAba and next dog didn't enter yet, we might have fast next dog that is already running
            // and next dog entered gate to early (fault). This is fix for simulated race 40 (74-15).
            // Period between 3.5s and 5.5s is covered and scenario when last dog is running excluded. Fix for simulated race 45 (83-30).
            // log_d("bRerunBusy: %i, _bLastStringBAba: %i, TfromLastDogExit: %lld, iCurrentDog: %i, iNextDog: %i.", _bRerunBusy, _bLastStringBAba, (STriggerRecord.llTriggerTime - _llLastDogExitTime), iCurrentDog + 1, iNextDog + 1);
            if (!_bRerunBusy && _bLastStringBAba && (STriggerRecord.llTriggerTime - _llLastDogExitTime) > 3500000 //
                && (STriggerRecord.llTriggerTime - _llLastDogExitTime) < 5500000 && iCurrentDog != iNextDog)
            {
               // Calculte times for running invisible dog
               SetDogFault(iNextDog, ON);
               _llDogEnterTimes[iCurrentDog] = _llLastDogExitTime;
               _llDogExitTimes[iCurrentDog] = STriggerRecord.llTriggerTime;
               _llLastDogExitTime = _llDogExitTimes[iCurrentDog];
               _llDogTimes[iCurrentDog][iDogRunCounters[iCurrentDog]] = _llDogExitTimes[iCurrentDog] - _llDogEnterTimes[iCurrentDog];
               // Set crossing time to zero (ok)
               _llCrossingTimes[iCurrentDog][iDogRunCounters[iCurrentDog]] = 0;
               if (_bDogManualFaults[iCurrentDog]) // If dog has manual fault we assume it's due to he missed gate while entering
                  _bDogMissedGateGoingin[iCurrentDog][iDogRunCounters[iCurrentDog]] = true;
               else // This is true invisible dog case so treated as big OK cross
                  _bDogBigOK[iCurrentDog][iDogRunCounters[iCurrentDog]] = true;
               _ChangeDogNumber(iNextDog);
               log_d("S1 crossed after 3.5-5.5s and last Tstring was BAba. Invisible dog %i is running and next dog enters with fault.", iPreviousDog + 1);
            }
            else
            {
               // If this dog is doing a rerun we have to turn the error light for this dog off
               if (_bRerunBusy)
                  SetDogFault(iCurrentDog, OFF);
               log_d("Dog %i going in crossed S1 safely. Clear fault if rerun.", iCurrentDog + 1);
            }
            _llDogEnterTimes[iCurrentDog] = STriggerRecord.llTriggerTime;
            _llCrossingTimes[iCurrentDog][iDogRunCounters[iCurrentDog]] = _llDogEnterTimes[iCurrentDog] - _llLastDogExitTime;
            _bS1StillSafe = false;
            //_ChangeDogState(COMINGBACK); //Moved to Tstring section
         }
         // Check if this is a next dog which is too early (we were expecting a dog to come back)
         else if (_byDogState == COMINGBACK && !_bS1StillSafe && (STriggerRecord.llTriggerTime - _llDogEnterTimes[iCurrentDog]) > 2000000 // Filter out S1 HIGH signals that are < 2 seconds after dog enter time
                  && (iCurrentDog != iNextDog))                                                                                           // Exclude scenario if next dog is equal current dog as this can't be comming back dog.
         {
            // Set fault light for next dog.
            SetDogFault(iNextDog, ON);
            // Change state to going in to resist from possible S1 sensor noise and hitting condition again. Setting to COMINGBACK will be done in Tstring section.
            _ChangeDogState(GOINGIN);
            // We assume no negative cross (if that would be the case it will be updated later) so current dog exit time is same as next dog enter time
            _llDogExitTimes[iCurrentDog] = STriggerRecord.llTriggerTime;
            _llLastDogExitTime = _llDogExitTimes[iCurrentDog];
            _llDogTimes[iCurrentDog][iDogRunCounters[iCurrentDog]] = _llLastDogExitTime - _llDogEnterTimes[iCurrentDog];
            // Handle next dog
            _llDogEnterTimes[iNextDog] = STriggerRecord.llTriggerTime;
            log_d("Dog %i FAULT as coming back dog was expected.", iNextDog + 1);
            if ((iCurrentDog == (iNumberOfRacingDogs - 1) && _bFault && !_bRerunBusy && !bRerunsOff) // If current dog is last dog before reruns and a fault exists, we have to initiate rerun sequence
                || _bRerunBusy)                                                                      // or if rerun is busy (and faults still exist)
            {
               // Last dog came in but there is a fault, we have to initiate the rerun sequence
               _bRerunBusy = true;
               // Reset timers for this dog
               _llDogExitTimes[iNextDog] = 0;
               // Increase run counter for this dog
               iDogRunCounters[iNextDog]++;
               log_d("Re-run for dog %i", iNextDog + 1);
            }
            // In case reruns are turned off and current dog is last racing dog
            else if (iNextDog == 5)
            {
               StopRace(STriggerRecord.llTriggerTime);
               log_i("Reruns off. No more dogs was expected. Race stopped.");
            }
            _ChangeDogNumber(iNextDog);
         }
         // Special case after false detection of "ok crossing" --> S1 activated above 100ms after "ok crossing" detection or re-run with next dog = current dog
         else if (_byDogState == COMINGBACK && _bDogSmallok[iCurrentDog][iDogRunCounters[iCurrentDog]] && !_bS1StillSafe &&
                  (((STriggerRecord.llTriggerTime - _llDogEnterTimes[iCurrentDog]) > 100000 && (STriggerRecord.llTriggerTime - _llDogEnterTimes[iCurrentDog]) < 2000000) // filtering changed to < 2s fix for 79-7
                   || (_bRerunBusy && iCurrentDog == iNextDog)))
         {
            _bDogSmallok[iCurrentDog][iDogRunCounters[iCurrentDog]] = false;
            _llDogEnterTimes[iCurrentDog] = STriggerRecord.llTriggerTime;
            _llCrossingTimes[iCurrentDog][iDogRunCounters[iCurrentDog]] = _llDogEnterTimes[iCurrentDog] - _llLastDogExitTime;
            log_d("False 'ok crossing' detected. Recalculate dog %i times.", iCurrentDog + 1);
         }
         // else
         //    log_d("Unexpected S1 crossing while gate CLEAR. CurrentDog: %i, NextDog: %i, S1StillSafe: %i, RerunBusy: %i", iCurrentDog + 1, iNextDog + 1, _bS1StillSafe, _bRerunBusy);
      }

      ////Handle sensor 1 events (handlers side) with gates state DOG IN
      if (STriggerRecord.iSensorNumber == 1 && STriggerRecord.iSensorState == 1 && !_bGatesClear && iCurrentDog < 5) // Only if gates are busy (dog in) and S1 sensor is HIGH (A)
      {
         // If negative cross detected and S1 crossed above 6ms from S2 crossing
         if (_bPotentialNegativeCrossDetected && ((STriggerRecord.llTriggerTime - _llS2CrossedUnsafeTriggerTime) > 6000))
         {
            _llRaceElapsedTime = STriggerRecord.llTriggerTime - llRaceStartTime;
            _bPotentialNegativeCrossDetected = false;
            _bNegativeCrossDetected = true;
            if (_bDogMissedGateComingback[iPreviousDog][iDogRunCounters[iPreviousDog]]) // Clear flag if set as previous dog is comming back so he didn't missed the gate
               _bDogMissedGateComingback[iPreviousDog][iDogRunCounters[iPreviousDog]] = false;
            // If current dog has no fault it has to be invisible dog coming back and it's next dog who did negative cross
            if (!_bDogFaults[iCurrentDog])
            {
               _llDogEnterTimes[iNextDog] = _llDogEnterTimes[iCurrentDog];
               SetDogFault(iNextDog, ON);
               _llDogEnterTimes[iCurrentDog] = _llLastDogExitTime;
               _llCrossingTimes[iCurrentDog][iDogRunCounters[iCurrentDog]] = 0;
               if (_bDogManualFaults[iCurrentDog]) // If dog has manual fault we assume it's due to he missed gate while entering
                  _bDogMissedGateGoingin[iCurrentDog][iDogRunCounters[iCurrentDog]] = true;
               else // This is true invisible dog case so treated as big OK cross
                  _bDogBigOK[iCurrentDog][iDogRunCounters[iCurrentDog]] = true;
               if (_bRerunBusy)
                  iDogRunCounters[iNextDog]++; // Increase run counter for next dog
               log_d("Invisible dog %i came back! Dog times updated. OK or Perfect crossing.", iNextDog + 1);
               _ChangeDogNumber(iNextDog);
            }
            // And previous dog time (who just crossed S1)
            _llDogExitTimes[iPreviousDog] = STriggerRecord.llTriggerTime;
            _llDogTimes[iPreviousDog][iDogRunCounters[iPreviousDog]] = _llDogExitTimes[iPreviousDog] - _llDogEnterTimes[iPreviousDog];
            // Update crossing (negative) and dog time of current dog (the one with fault)
            _llCrossingTimes[iCurrentDog][iDogRunCounters[iCurrentDog]] = _llDogEnterTimes[iCurrentDog] - _llDogExitTimes[iPreviousDog];
            // Check if previous dog has fake time flag active and clear it
            if (_bDogFakeTime[iPreviousDog][iDogRunCounters[iPreviousDog]])
            {
               _bDogFakeTime[iPreviousDog][iDogRunCounters[iPreviousDog]] = false;
               log_d("Fake time flag for dog %i cleared.", iPreviousDog + 1);
            }
            _llRaceElapsedTime = STriggerRecord.llTriggerTime - llRaceStartTime;
            //_bNegativeCrossDetected = false; // moved to Tstring section and used as "if" condition in BAba scenario
            //
            log_d("Calculate negative cross time for dog %i and update times for previous dog %i.", iCurrentDog + 1, iPreviousDog + 1);
            log_d("Dod %i updated crossing time [ms]: %lld", iCurrentDog + 1, (_llCrossingTimes[iCurrentDog][iDogRunCounters[iCurrentDog]] + 500) / 1000);
            log_d("Dog %i updated time [ms]: %lld", iPreviousDog + 1, ((_llDogTimes[iPreviousDog][iDogRunCounters[iPreviousDog]] + 500) / 1000));
         }
         // Negative cross detected, but S1 was crossed below 6ms from S2 crossing. This can't be a dog and it has to be sensor noise  (fix for simulated race 36 & 37)
         else if (_bPotentialNegativeCrossDetected && ((STriggerRecord.llTriggerTime - _llS2CrossedUnsafeTriggerTime) <= 6000))
         {
            _bSensorNoise = true;
            log_d("S2/S1 sensors noise.");
         }
         else if (_bS1isSafe) // If S2 crossed before S1 (ok or positive cross scenarios)
         {
            // Normal handling for dog coming back or dog going in after S2 crossed safely
            _llRaceElapsedTime = STriggerRecord.llTriggerTime - llRaceStartTime;
            _llDogExitTimes[iCurrentDog] = STriggerRecord.llTriggerTime;
            _llLastDogExitTime = _llDogExitTimes[iCurrentDog];
            _llDogTimes[iCurrentDog][iDogRunCounters[iCurrentDog]] = _llDogExitTimes[iCurrentDog] - _llDogEnterTimes[iCurrentDog];
            _bS1isSafe = false;
            log_d("S1 line crossed while being safe. Calculate dog %i time.", iCurrentDog + 1);
            if (_llDogExitTimes[iCurrentDog] - _llS2CrossedSafeTime < 5000) // If S1 (safe) crossing time is below 5ms after S2 crossing means S1 was crossed
                                                                            // by going in dog and we have true PERFECT crossing
            {
               _bDogPerfectCross[iNextDog][iDogRunCounters[iNextDog]] = true;
               log_d("PERFECT cross below 5ms detected for dog %i.", iNextDog + 1);
            }
            if ((iCurrentDog == (iNumberOfRacingDogs - 1) && !_bFault && !_bRerunBusy) // If this is the last dog and there are no faults (reruns) we have to stop the race
                || (_bRerunBusy && !_bFault)                                           // or if the rerun sequence was started but no faults exist anymore
                || iNextDog == 5)                                                      // or reruns are turned off and this is last dog
               StopRace(STriggerRecord.llTriggerTime);
            else if ((iCurrentDog == (iNumberOfRacingDogs - 1) && _bFault && !_bRerunBusy) // If current dog is last dog and a fault(s) exists, we have to initiate rerun sequence
                     || _bRerunBusy)                                                       // Or if rerun is busy (and faults still exist)
            {
               // Last dog came back but there is a fault, we have to initiate the rerun sequence
               _bRerunBusy = true;
               // Reset timers for this dog
               _llDogEnterTimes[iNextDog] = STriggerRecord.llTriggerTime;
               _llDogExitTimes[iNextDog] = 0;
               // Increase run counter for this dog
               iDogRunCounters[iNextDog]++;
               log_d("Re-run for dog %i", iNextDog + 1);
            }
            _ChangeDogNumber(iNextDog);
         }
         // else
         //    log_d("Unexpected S1 crossing while gate DOG(s). CurrentDog: %i, NextDog: %i, S1StillSafe: %i, RerunBusy: %i", iCurrentDog + 1, iNextDog + 1, _bS1StillSafe, _bRerunBusy);
      }

      // Handle sensor 2 (box side)
      if (STriggerRecord.iSensorNumber == 2 && STriggerRecord.iSensorState == 1 && _bGatesClear && iCurrentDog < 5) // Only if gates are clear S2 sensor is HIGH (B)
      {
         if (_byDogState == GOINGIN && (STriggerRecord.llTriggerTime - _llLastDogExitTime) > 3000000)
         {
            /* Gates were clear, we weren't expecting a dog back, but one came back.
            This means we missed the dog going in,
            most likely due to perfect crossing were next dog was faster than previous dog (Tstring --> BAba),
            and thus passed through sensors unseen */
            // Set enter time for this dog to exit time of previous dog
            _llDogEnterTimes[iCurrentDog] = _llLastDogExitTime;
            // Set crossing time to zero (ok)
            _llCrossingTimes[iCurrentDog][iDogRunCounters[iCurrentDog]] = 0;
            if (_bDogManualFaults[iCurrentDog]) // If dog has manual fault we assume it's due to he missed gate while entering
               _bDogMissedGateGoingin[iCurrentDog][iDogRunCounters[iCurrentDog]] = true;
            else // This is true invisible dog case so treated as big OK cross
               _bDogBigOK[iCurrentDog][iDogRunCounters[iCurrentDog]] = true;
            _bS1isSafe = true;
            _bS1StillSafe = true;
            _llS2CrossedSafeTime = STriggerRecord.llTriggerTime;
            _ChangeDogState(COMINGBACK);
            log_d("Invisible dog %i came back!. Update enter time. OK or Perfect crossing.", iCurrentDog + 1);
         }
         else if (_byDogState == COMINGBACK)
         {
            // S2 is triggered less than 1.5s after current dog's enter time what means we have potential early (negative) cross
            // unless this is first dog
            if ((STriggerRecord.llTriggerTime - _llDogEnterTimes[iCurrentDog]) < 1500000)
            {
               if (iCurrentDog != 0 || (iCurrentDog == 0 && _bRerunBusy))
               {
                  _bPotentialNegativeCrossDetected = true;
                  _llS2CrossedUnsafeTriggerTime = STriggerRecord.llTriggerTime;
                  _llS2CrossedUnsafeGetMicrosTime = MICROS; // fix for simulated race 35
                  log_d("Dog %i potential negative cross detected.", iCurrentDog + 1);
               }
               else
               {
                  _bSensorNoise = true;
                  log_d("Entering first dog caused S2 sensor noise.");
               }
            }
            else // S2 was triggered after 2s since current dog entry time
            {
               _bS1isSafe = true;
               _bS1StillSafe = true;
               _llS2CrossedSafeTime = STriggerRecord.llTriggerTime;
               log_d("Coming back dog %i crossed S2 line. S1 is safe.", iCurrentDog + 1);
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

      // Add trigger record to transition string
      _AddToTransitionString(STriggerRecord);

      // Check if the transition string up till now tells us the gates are clear
      String strLast2TransitionChars = _strTransition.substring(_strTransition.length() - 2);
      if (_strTransition.length() == 0                                           // String might still be empty, in which case the gates were clear
          || strLast2TransitionChars == "ab" || strLast2TransitionChars == "ba") // Sensors going low in either direction indicate gates are clear
      {
         // Print the transition string up til now for debugging purposes
         log_d("Tstring: %s", _strTransition.c_str());
         // The gates are clear, set flag
         _bGatesClear = true;
         log_d("Gate: CLEAR.");

         // Only check transition string when gates are clear
         // TODO: If transistion string is 3 or longer but actually more events are coming related to same transition, these are not considered.
         if (_strTransition.length() > 3) // And if transistion string is at least 4 characters long
         {
            // Clear last string ABab flag if it was true
            if (_bLastStringBAba) // fix for simulated race 40
               _bLastStringBAba = false;
            // Transition string is 4 characters or longer
            // So we can check what happened
            if (_bSensorNoise) // fix for simulated race 36
            {
               log_d("Sensor noise was detected. Ignore Tstring");
               _bSensorNoise = false;
            }
            else if (_strTransition == "ABab") // Dog going to box
            {
               // Change dog state to coming back
               _ChangeDogState(COMINGBACK);
               _strPreviousTransitionFirstLetter = "A";
               log_d("New dog state: COMINGBACK. ABab.");
               if (_bDogManualFaults[iPreviousDog]) // If previous dog has manual fault we assume it's because he missed gate while coming back
                                                    // If however this will be negative cross scenario flag will be deactivated in S2 sensor section
               {
                  _bDogMissedGateComingback[iPreviousDog][iDogRunCounters[iPreviousDog]] = true;
               }
            }
            else if (_strTransition == "BAba" && !_bNegativeCrossDetected && RaceState != STOPPED) // Typical dog coming back case
            {
               // Normal handling, change dog state to GOING IN
               _ChangeDogState(GOINGIN);
               _strPreviousTransitionFirstLetter = "B";
               _bLastStringBAba = true;
               log_d("New dog state: GOINGING.");
            }
            else if (RaceState == STOPPED) // Last returning dog
            {
               log_d("Last dog came back.");
               bIgnoreSensors = true;
            }
            else // So called "uncertain Tstring"
            {
               // If Transition string indicated something else and we expect returning dog, it means 2 dogs must have passed
               String strFirstTransitionChar = _strTransition.substring(0, 1);
               if (_byDogState == COMINGBACK && strFirstTransitionChar == "B")
               {
                  _strPreviousTransitionFirstLetter = "B";
                  // We increase the dog number
                  //_ChangeDogNumber(iNextDog);
                  if (_bNegativeCrossDetected && RaceState != STOPPED) // Dog comming back after negative cross
                  {
                     _bNegativeCrossDetected = false;
                     log_d("Dog state still COMINGBACK. Dog coming back after negative cross of next dog.");
                  }
                  // If this is Re-run and dog had fault active we need to turn it OFF if this is perfect crossing case (string starts with B) during re-run
                  else
                  {
                     if ((_bRerunBusy && _bFault))
                        SetDogFault(iCurrentDog, OFF);
                     _bS1StillSafe = false;
                     _llCrossingTimes[iCurrentDog][iDogRunCounters[iCurrentDog]] = 0;
                     _llDogEnterTimes[iCurrentDog] = _llDogExitTimes[iPreviousDog];
                     if (_strTransition == "BAab" || _strTransition == "BAba" || _strTransition == "BbABab" || _strTransition == "BbABba") // Big OK cross
                     {
                        _bDogBigOK[iCurrentDog][iDogRunCounters[iCurrentDog]] = true;
                        log_d("Unmeasurable 'OK' crossing for dog %i.", iCurrentDog + 1);
                     }
                     else
                     {
                        _bDogSmallok[iCurrentDog][iDogRunCounters[iCurrentDog]] = true;
                        log_d("Unmeasurable 'ok' crossing for dog %i.", iCurrentDog + 1);
                     }
                  }
               }
               else if (_byDogState == COMINGBACK && strFirstTransitionChar == "A")
               {
                  _strPreviousTransitionFirstLetter = "A";
                  log_d("Dog %i fault. Tstring starting with A.", iCurrentDog + 1);
               }
               else
               // It has to be dog going in + sensors noise
               {
                  _ChangeDogState(COMINGBACK);
                  _strPreviousTransitionFirstLetter = _strTransition.substring(0, 1);
                  log_d("New dog %i state: COMINGBACK. Uncertain.", iCurrentDog + 1);
               }
            }
         }
         _strTransition = "";
      }
      else if (_bGatesClear)
      {
         _bGatesClear = false;
         log_d("Gate: DOG(s)");
      }
   }

   // Check for faults, loop through array of dogs checking for faults
   _bFault = false;
   for (auto bFault : _bDogFaults)
   {
      if (bFault)
      {
         // At least one dog with fault has been found, set general fault value to true
         _bFault = true;
         _bNoValidCleanTime = true;
         break;
      }
   }
   for (auto bManualFault : _bDogManualFaults)
   {
      if (bManualFault)
      {
         // At least one dog with manual fault has been found, set general fault value to true
         _bFault = true;
         _bNoValidCleanTime = true;
         break;
      }
   }

   if (!bIgnoreSensors && RaceState == STOPPED && (MICROS - _llRaceEndTime) > 400000)
      bIgnoreSensors = true;

   if (bIgnoreSensors && !_bRaceSummaryPrinted)
   {
      _PrintRaceSummary();
      _bRaceSummaryPrinted = true;
   }
}

/// <summary>
///   Changes race state, if byNewRaceState is different from current one.
/// </summary>
///
/// <param name="byNewRaceState">   New race state. </param>
void RaceHandlerClass::_ChangeRaceState(RaceStates byNewRaceState)
{
   RaceState = byNewRaceState;
   switch (RaceState)
   {
   case RaceHandlerClass::RESET:
      strRaceState = " READY ";
      break;
   case RaceHandlerClass::STARTING:
      strRaceState = " START ";
      break;
   case RaceHandlerClass::RUNNING:
      strRaceState = "RUNNING";
      break;
   case RaceHandlerClass::STOPPED:
      strRaceState = " STOP  ";
      break;
   default:
      break;
   }
   log_i("RS: %s", strRaceState);
   LCDController.UpdateField(LCDController.RaceState, strRaceState);
}

/// <summary>
///   Change dog state. If byNewDogState is different from current one.
/// </summary>
///
/// <param name="byNewDogState"> State of the new dog. </param>
void RaceHandlerClass::_ChangeDogState(_byDogStates byNewDogState)
{
   if (_byDogState != byNewDogState)
      _byDogState = byNewDogState;
   // log_d("DogState: %i", byNewDogState);
}

/// <summary>
///   Change dog number, if new dognumber is different from current one.
/// </summary>
///
/// <param name="iNewDogNumber"> Zero-based index of the new dog number. </param>
void RaceHandlerClass::_ChangeDogNumber(uint8_t iNewDogNumber)
{
   // Check if the dog really changed (this function could be called superfluously)
   if (iNewDogNumber != iCurrentDog)
   {
      iPreviousDog = iCurrentDog;
      iCurrentDog = iNewDogNumber;
      log_d("Dog:%i|ENT:%lld|EXIT:%lld|TOT:%lld", iPreviousDog + 1, _llDogEnterTimes[iPreviousDog], _llLastDogExitTime, _llDogTimes[iPreviousDog][iDogRunCounters[iPreviousDog]]);
      if (RaceState == RUNNING)
      {
         log_i("Dog %i: %s | CR: %s", iPreviousDog + 1, GetDogTime(iPreviousDog, -2), GetCrossingTime(iPreviousDog, -2).c_str());
         log_d("Running dog: %i.", iCurrentDog + 1);
      }
   }
}

/// <summary>
///   Initiates race start timer and sets the status of the race to STARTING.
///   Should be called when RED light comes ON during start sequence.
/// </summary>
void RaceHandlerClass::StartRaceTimer()
{
   llRaceStartTime = MICROS + 3000000;
   _ChangeRaceState(STARTING);
   log_i("STARTING! Tag: %i, Race ID: %i.", SDcardController.iTagValue, iCurrentRaceId + 1);
   cRaceStartTimestamp = GPSHandler.GetLocalTimestamp();
   log_i("Timestamp: %s", cRaceStartTimestamp);
      LCDController.iLCDUpdateIntervalMultiplier = 5;
#ifdef WiFiON
   // Send updated racedata to all web clients
   WebHandler.bSendRaceData = true;
#endif
}

/// <summary>
///   Stops a race.
/// </summary>
void RaceHandlerClass::StopRace()
{
   this->StopRace(MICROS);
}

/// <summary>
///   Stops a race.
/// </summary>
/// <param name="StopTime">   The time in microseconds at which the race stopped. </param>
void RaceHandlerClass::StopRace(long long llStopTime)
{
   if (RaceState == STOPPED || RaceState == RESET)
      return;
   else
   {
      // Race is running, so we have to record the EndTime
      _llRaceEndTime = llStopTime;
      if (RaceState == RUNNING)
         llRaceTime = _llRaceEndTime - llRaceStartTime;
      else
         llRaceTime = 0;
      _ChangeRaceState(STOPPED);
      LCDController.iLCDUpdateIntervalMultiplier = 1;
   #ifdef WiFiON
      // Send updated racedata to any web clients
      WebHandler.bSendRaceData = true;
   #endif
   }
}

/// <summary>
///   Resets the race, this function should be called to reset all timers to 0 and prepare the
///   software for starting a next race.
/// </summary>
void RaceHandlerClass::ResetRace()
{
   if (RaceState != STOPPED)
      return;
   else
   {
      iCurrentDog = 0;
      iNextDog = 1;
      iPreviousDog = 0;
      llRaceStartTime = MICROS;
      _llRaceEndTime = MICROS;
      llRaceTime = 0;
      _llRaceElapsedTime = 0;
      _llLastDogExitTime = 0;
      _llS2CrossedSafeTime = 0;
      _llS2CrossedUnsafeTriggerTime = 0;
      _llS2CrossedUnsafeGetMicrosTime = 0;
      _byDogState = GOINGIN;
      _ChangeDogNumber(0);
   #if Simulate
      Simulator.bExecuteSimRaceReset = true;
   #endif
      _bFault = false;
      _bRerunBusy = false;
      _iOutputQueueReadIndex = 0;
      _iInputQueueReadIndex = 0;
      _iOutputQueueWriteIndex = 0;
      _iInputQueueWriteIndex = 0;
      _strTransition = "";
      _strPreviousTransitionFirstLetter = "";
      _bGatesClear = true;
      _bS1isSafe = false;
      _bS1StillSafe = false;
      _bNextDogFound = false;
      _bNegativeCrossDetected = false;
      _bPotentialNegativeCrossDetected = false;
      _bSensorNoise = false;
      _bLastStringBAba = false;
      _bNoValidCleanTime = false;
      for (auto &bFault : _bDogFaults)
         bFault = false;
      for (auto &bManualFault : _bDogManualFaults)
         bManualFault = false;
      for (auto &Dog : _bDogPerfectCross)
      {
         for (auto &bDogPerfectCross : Dog)
            bDogPerfectCross = false;
      }
      for (auto &Dog : _bDogBigOK)
      {
         for (auto &bDogBigOK : Dog)
            bDogBigOK = false;
      }
      for (auto &Dog : _bDogSmallok)
      {
         for (auto &bDogSmallok : Dog)
            bDogSmallok = false;
      }
      for (auto &Dog : _bDogFakeTime)
      {
         for (auto &bDogFakeTime : Dog)
            bDogFakeTime = false;
      }
      for (auto &Dog : _bDogMissedGateGoingin)
      {
         for (auto &bDogMissedGateGoingin : Dog)
            bDogMissedGateGoingin = false;
      }
      for (auto &Dog : _bDogMissedGateComingback)
      {
         for (auto &bDogMissedGateComingback : Dog)
            bDogMissedGateComingback = false;
      }
      for (auto &llTime : _llDogEnterTimes)
         llTime = 0;
      for (auto &llTime : _llDogExitTimes)
         llTime = 0;
      for (auto &Dog : _llDogTimes)
      {
         for (auto &llTime : Dog)
            llTime = 0;
      }
      for (auto &Dog : _llCrossingTimes)
      {
         for (auto &llTime : Dog)
            llTime = 0;
      }
      for (auto &iCounter : iDogRunCounters)
         iCounter = 0;
      for (auto &llTime : _llLastDogTimeReturnTimeStamp)
         llTime = 0;
      for (auto &iCounter : _iLastReturnedRunNumber)
         iCounter = 0;
      _ChangeRaceState(RESET);
      bIgnoreSensors = false;
      _bRaceSummaryPrinted = false;

      if (iCurrentRaceId == 999)
         iCurrentRaceId = 0;
      else
         iCurrentRaceId++;
      String _sCurrentRaceId = String(iCurrentRaceId + 1);
      while (_sCurrentRaceId.length() < 3)
         _sCurrentRaceId = " " + _sCurrentRaceId;
      LCDController.UpdateField(LCDController.RaceID, _sCurrentRaceId);
      log_i("Reset Race: DONE");
#ifdef WiFiON
      // Send updated racedata to any web clients
      WebHandler.bSendRaceData = true;
#endif
   }
}

void RaceHandlerClass::_PrintRaceSummary()
{
   log_v("RaceState: %i, MICROS: %lld, _llRaceEndTime: %lld, Delta: %lld, _bRaceSummaryPrinted: %i", RaceState, MICROS, _llRaceEndTime, MICROS - _llRaceEndTime, _bRaceSummaryPrinted);
   // Race has been stopped 0.5 second ago: print race summary to console
   for (uint8_t i = 0; i < iNumberOfRacingDogs; i++)
   {
      // log_d("Dog %i -> %i run(s).", i + 1, iDogRunCounters[i] + 1);
      for (uint8_t i2 = 0; i2 < (iDogRunCounters[i] + 1); i2++)
         log_i("Dog %i: %s | CR: %s", i + 1, GetStoredDogTimes(i, i2), TransformCrossingTime(i, i2));
   }
   log_i(" Team: %s", GetRaceTime());
   log_i("   CT: %s", GetCleanTime());
   if (SDcardController.bSDCardDetected)
      SDcardController.SaveRaceDataToFile();
#if !Simulate
   if (CORE_DEBUG_LEVEL >= ESP_LOG_DEBUG)
      _PrintRaceTriggerRecords();
   if (SDcardController.bSDCardDetected)
      _PrintRaceTriggerRecordsToFile();
#endif
}

/// <summary>
///   After race is ended/stopped print trigger records to console
/// </summary>
void RaceHandlerClass::_PrintRaceTriggerRecords()
{
   uint8_t iRecordToPrintIndex = 0;
   while (iRecordToPrintIndex < _iInputQueueWriteIndex)
   {
      STriggerRecord RecordToPrint = _InputTriggerQueue[iRecordToPrintIndex];
      printf("{%i, %lld, %i},\r\n", RecordToPrint.iSensorNumber, RecordToPrint.llTriggerTime - llRaceStartTime, RecordToPrint.iSensorState);
      iRecordToPrintIndex++;
   }
   while (iRecordToPrintIndex < TRIGGER_QUEUE_LENGTH)
   {
      printf("{0, 0, 0},\r\n");
      iRecordToPrintIndex++;
   }
}

/// <summary>
///   If SD card is present, after race is ended/stopped print trigger records to file
/// </summary>
void RaceHandlerClass::_PrintRaceTriggerRecordsToFile()
{
   File rawSensorsReadingFile;
   String rawSensorsReadingFileName = "/SENSORS_DATA/" + SDcardController.sTagValue + "_SensorsData" + ".txt";
   if (iCurrentRaceId == 0)
   {
      SDcardController.writeFile(SD_MMC, rawSensorsReadingFileName.c_str(),
                                 "ID; Time [us]; state\n");
   }
   rawSensorsReadingFile = SD_MMC.open(rawSensorsReadingFileName.c_str(), FILE_APPEND);
   if (rawSensorsReadingFile)
   {
      rawSensorsReadingFile.print("Race ID: ");
      rawSensorsReadingFile.println(RaceHandler.iCurrentRaceId + 1);
      uint8_t iRecordToPrintIndex = 0;
      while (iRecordToPrintIndex < _iInputQueueWriteIndex)
      {
         STriggerRecord RecordToPrint = _InputTriggerQueue[iRecordToPrintIndex];
         rawSensorsReadingFile.print("{");
         rawSensorsReadingFile.print(RecordToPrint.iSensorNumber);
         rawSensorsReadingFile.print(", ");
         rawSensorsReadingFile.print(RecordToPrint.llTriggerTime - llRaceStartTime);
         rawSensorsReadingFile.print(", ");
         rawSensorsReadingFile.print(RecordToPrint.iSensorState);
         rawSensorsReadingFile.println("},");
         iRecordToPrintIndex++;
      }
      while (iRecordToPrintIndex < TRIGGER_QUEUE_LENGTH)
      {
         rawSensorsReadingFile.println("{0, 0, 0},");
         iRecordToPrintIndex++;
      }
      rawSensorsReadingFile.close();
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
   // Don't process any faults when race is not running
   if (RaceState == STOPPED || RaceState == RESET)
      return;
   bool bFault;
   // Check if we have to toggle. Assumed only manual faults use TOGGLE option
   if (State == TOGGLE)
   {
      bFault = !_bDogManualFaults[iDogNumber];
      _bDogManualFaults[iDogNumber] = bFault;
   }
   else
   {
      bFault = State;
      // Set fault to specified value for relevant dog (automatic faults)
      _bDogFaults[iDogNumber] = bFault;
      // If crossing fault detected of next dog dog then set fake time flag for current dog
      if (bFault && (iDogNumber > 0 || (iDogNumber == 0 && iCurrentDog > 0)))
         _bDogFakeTime[iCurrentDog][iDogRunCounters[iCurrentDog]] = true;
   }

   if (bFault)
   {
      // If fault is true, set light to on and set general value fault variable to true
      LightsController.ToggleFaultLight(iDogNumber, LightsController.ON);
      _bFault = true;
      _bNoValidCleanTime = true;
      log_i("Dog %i fault ON", iDogNumber + 1);
   }
   else
   {
      // If fault is false, turn off fault light for this dog and reset also manual fault state
      _bDogManualFaults[iDogNumber] = bFault;
      LightsController.ToggleFaultLight(iDogNumber, LightsController.OFF);
      log_i("Dog %i fault OFF", iDogNumber + 1);
   }
}

/// <summary>
///   ISR function for sensor 1, this function will record the sensor number, microseconds and
///   state (HIGH/LOW) of the sensor in the interrupt queue.
/// </summary>
void RaceHandlerClass::TriggerSensor1()
{
   if (bIgnoreSensors)
      return;
   else if (RaceState == RESET)
   {
      if (digitalRead(_iS1Pin) == 1)
         LightsController.bExecuteRaceReadyFaultON = true;
      else
         LightsController.bExecuteRaceReadyFaultOFF = true;
   }
   else
      _QueuePush({bRunDirectionInverted ? 2 : 1, MICROS, digitalRead(_iS1Pin)});
}

/// <summary>
///   ISR function for sensor 2, this function will record the sensor number, microseconds and
///   state (HIGH/LOW) of the sensor in the interrupt queue.
/// </summary>
void RaceHandlerClass::TriggerSensor2()
{
   if (bIgnoreSensors)
      return;
   else if (RaceState == RESET)
   {
      if (digitalRead(_iS2Pin) == 1)
         LightsController.bExecuteRaceReadyFaultON = true;
      else
         LightsController.bExecuteRaceReadyFaultOFF = true;
   }
   else
      _QueuePush({bRunDirectionInverted ? 1 : 2, MICROS, digitalRead(_iS2Pin)});
}

/// <summary>
///   Gets race time. Time since start if race is still running, final time if race is finished.
/// </summary>
///
/// <returns>
///   The race time in seconds with milisecond accuracy rounded up or down.
/// </returns>
String RaceHandlerClass::GetRaceTime()
{
   char cRaceTimeSeconds[8];
   String strRaceTimeSeconds;
   double dRaceTimeSeconds;
   if (!_bAccuracy3digits)
   {
      dRaceTimeSeconds = ((long long)(llRaceTime + 5000) / 10000) / 100.0;
      dtostrf(dRaceTimeSeconds, 7, 2, cRaceTimeSeconds);
   }
   else
   {
      dRaceTimeSeconds = ((long long)(llRaceTime + 500) / 1000) / 1000.0;
      dtostrf(dRaceTimeSeconds, 7, 3, cRaceTimeSeconds);
   }
   strRaceTimeSeconds = cRaceTimeSeconds;
   return strRaceTimeSeconds;
}

/// <summary>
///   Supporting alternate function of multiple dog results.
/// </summary>
///
/// <param name="iDogNumber"> Zero-based index of the dog number. </param>
/// <param name="iRunNumber"> Zero-based index of the run number. If -1 is passed, this function
///                           will alternate between each run we have for the dog, passing a new
///                           run every 2 seconds. If -2 is passed, the last run number we have
///                           for this dog will be passed. </param>
///
/// <returns>
///   Transformed iRunNumber.
/// </returns>
int8_t RaceHandlerClass::SelectRunNumber(uint8_t iDogNumber, int8_t iRunNumber)
{
   if (iDogRunCounters[iDogNumber] > 0)
   {
      // We have multiple times for this dog.
      // if run number is -1 (unspecified), we have to cycle throug them
      if (iRunNumber == -1)
      {
         auto &ulLastReturnedTimeStamp = _llLastDogTimeReturnTimeStamp[iDogNumber];
         iRunNumber = _iLastReturnedRunNumber[iDogNumber];
         if ((millis() - ulLastReturnedTimeStamp) > 2000)
         {
            if (iRunNumber == iDogRunCounters[iDogNumber])
               iRunNumber = 0;
            else
               iRunNumber++;
            ulLastReturnedTimeStamp = millis();
         }
         _iLastReturnedRunNumber[iDogNumber] = iRunNumber;
      }
      else if (iRunNumber == -2)
         // if RunNumber is -2 it means we should return the last one
         iRunNumber = iDogRunCounters[iDogNumber];
   }
   else if (iRunNumber < 0)
      iRunNumber = 0;
   return iRunNumber;
}

/// <summary>
///   Gets dog time, time since dog entered if dog is still running, final time if dog is already
///   back in. Keep in mind each dog can have multiple runs (reruns for faults).
/// </summary>
///
/// <param name="iDogNumber"> Zero-based index of the dog number. </param>
/// <param name="iRunNumber"> Zero-based index of the run number. If -1 is passed, this function
///                           will alternate between each run we have for the dog, passing a new
///                           run every 2 seconds. If -2 is passed, the last run number we have
///                           for this dog will be passed. </param>
///
/// <returns>
///   String with following options:
///   - dog time if available
///   - 0.000 or 0.00 if no dog time available
///   - partial time if dog is still running
///   - " run in" if manual fault active and "invisible dog" detected (assumed gate was missed while entering)
///   - "outside" if manual fault active and ABab string detected for next dog (assumed gate was missed while exiting)
/// </returns>

String RaceHandlerClass::GetDogTime(uint8_t iDogNumber, int8_t iRunNumber)
{
   char cDogTime[8];
   String strDogTime;
   double dDogTime;
   unsigned long ulDogTimeMillis = 0;
   iRunNumber = SelectRunNumber(iDogNumber, iRunNumber);
   // First check if we have final time for the requested dog number
   if (_llDogTimes[iDogNumber][iRunNumber] > 0)
      ulDogTimeMillis = (_llDogTimes[iDogNumber][iRunNumber] + 500) / 1000;
   // Then check if the requested dog is perhaps running (and coming back) so we can return the time so far
   // and if requested run number is lower then number of times dog has run
   else if ((RaceState == RUNNING && iCurrentDog == iDogNumber && _byDogState == COMINGBACK) && iRunNumber <= iDogRunCounters[iDogNumber] //
            && _llDogEnterTimes[iDogNumber] != 0)
      ulDogTimeMillis = (MICROS - _llDogEnterTimes[iDogNumber]) / 1000;
   // If next dog didn't enter yet (e.g. positive cross) just show zero
   else if (_llDogTimes[iDogNumber][iRunNumber] == 0)
      ulDogTimeMillis = 0;
   
   if (!_bAccuracy3digits)
   {
      dDogTime = ((unsigned long)(ulDogTimeMillis + 5) / 10) / 100.0;
      dtostrf(dDogTime, 7, 2, cDogTime);
   }
   else
   {
      dDogTime = ulDogTimeMillis / 1000.0;
      dtostrf(dDogTime, 7, 3, cDogTime);
   }
   
   if (_bDogMissedGateGoingin[iDogNumber][iRunNumber])
      strDogTime = " run in";
   else if (_bDogMissedGateComingback[iDogNumber][iRunNumber])
      strDogTime = "outside";
   else
   {
      strDogTime = cDogTime;
      if (_bDogFakeTime[iDogNumber][iRunNumber])
         strDogTime[0] = '#';
   }
   return strDogTime;
}

/// <summary>
///   Gets stored dog time for specific run.
/// </summary>
///
/// <param name="iDogNumber"> Zero-based index of the dog number. </param>
/// <param name="iRunNumber"> Zero-based index of the run number. </param>
///
/// <returns>
///   The dog time in seconds with miliseconds accuracy and rounded up or down.
/// </returns>
String RaceHandlerClass::GetStoredDogTimes(uint8_t iDogNumber, int8_t iRunNumber)
{
   char cDogTime[8];
   String strDogTime;
   double dDogTime = 0;
   if (!_bAccuracy3digits)
   {
      dDogTime = ((long long)(_llDogTimes[iDogNumber][iRunNumber] + 5000) / 10000) / 100.0;
      dtostrf(dDogTime, 7, 2, cDogTime);
   }
   else
   {
      dDogTime = ((long long)(_llDogTimes[iDogNumber][iRunNumber] + 500) / 1000) / 1000.0;
      dtostrf(dDogTime, 7, 3, cDogTime);
   }
   if (_bDogMissedGateGoingin[iDogNumber][iRunNumber])
      strDogTime = " run in";
   else if (_bDogMissedGateComingback[iDogNumber][iRunNumber])
      strDogTime = "outside";
   else if (dDogTime == 0)
      strDogTime = "";
   else
   {
      strDogTime = cDogTime;
      if (_bDogFakeTime[iDogNumber][iRunNumber])
         strDogTime[0] = '#';
   }

   return strDogTime;
}

/// <summary>
///   Gets dogs crossing time. Keep in mind each dog can have multiple runs (reruns for faults).
/// </summary>
///
/// <param name="iDogNumber"> Zero-based index of the dog number. </param>
/// <param name="iRunNumber"> Zero-based index of the run number. If -1 is passed, this function
///                           will alternate between each run we have for the dog, passing a new
///                           run every 2 seconds. If -2 is passed, the last run number we have
///                           for this dog will be passed. </param>
///
/// <returns>
///   The dog time in seconds with miliseconds accuracy rounded up or down.
/// </returns>
String RaceHandlerClass::GetCrossingTime(uint8_t iDogNumber, int8_t iRunNumber)
{
   String strCrossingTime;
   iRunNumber = SelectRunNumber(iDogNumber, iRunNumber);
   strCrossingTime = TransformCrossingTime(iDogNumber, iRunNumber);
   return strCrossingTime;
}

/// <summary>
///   Transform crossing time to string.
/// </summary>
///
/// <param name="iDogNumber"> Zero-based index of the dog number. </param>
/// <param name="iRunNumber"> Zero-based index of the run number. </param>
/// <param name="bToFile"> used for results printing to file. </param>
///
/// <returns>
///   String with following options:
///   * positive cross value rounded up or down
///   * ok - good un-measurable cross
///   * Perfect - perfect cross with S1 crossed below 5ms after S2 have been crossed
///   * OK - goon un-measurable cross with string BAab or invisible dog run in secnario (BAba)
///   * negative cross value - measurable fault rounded up or down
///   * fault - un-measurable fault
///   * "empty" - if no crossing time available / possible
/// </returns>
String RaceHandlerClass::TransformCrossingTime(uint8_t iDogNumber, int8_t iRunNumber, bool bToFile)
{
   double dCrossingTime;
   char cCrossingTime[8];
   String strCrossingTime;
   if (((MICROS - _llDogEnterTimes[iDogNumber] > 300000) || (iRunNumber < iDogRunCounters[iDogNumber])) && _llCrossingTimes[iDogNumber][iRunNumber] < 0 && _llDogEnterTimes[iDogNumber] != 0)
   {
      if ((iDogNumber == 0 && iRunNumber == 0 && _llCrossingTimes[0][0] > -9500) && !_bAccuracy3digits && !bToFile) // If this is first dog false start below 9.5ms
                                                                                                                    // use "ms" accuracy even if 2digits accuracy has been set
      {
         dCrossingTime = ((long long)(_llCrossingTimes[iDogNumber][iRunNumber] - 500) / 1000);
         dCrossingTime = fabs(dCrossingTime);
         dtostrf(dCrossingTime, 3, 0, cCrossingTime);
         strCrossingTime = "-";
         strCrossingTime += cCrossingTime;
         strCrossingTime += " ms";
      }
      else if (_bAccuracy3digits || ((iDogNumber == 0 && iRunNumber == 0 && _llCrossingTimes[0][0] > -9500) && !_bAccuracy3digits && bToFile))
      {
         dCrossingTime = ((long long)(_llCrossingTimes[iDogNumber][iRunNumber] - 500) / 1000) / 1000.0;
         dCrossingTime = fabs(dCrossingTime);
         dtostrf(dCrossingTime, 7, 3, cCrossingTime);
         strCrossingTime = "-";
         strCrossingTime += cCrossingTime;
      }
      else
      {
         dCrossingTime = ((long long)(_llCrossingTimes[iDogNumber][iRunNumber] - 5000) / 10000) / 100.0;
         dCrossingTime = fabs(dCrossingTime);
         dtostrf(dCrossingTime, 6, 2, cCrossingTime);
         strCrossingTime = "-";
         strCrossingTime += cCrossingTime;
      }
   }
   else if (((MICROS - _llDogEnterTimes[iDogNumber] > 300000) || (iRunNumber < iDogRunCounters[iDogNumber])) && _llCrossingTimes[iDogNumber][iRunNumber] > 0 && _llDogEnterTimes[iDogNumber] != 0)
   {
      if ((iDogNumber == 0 && iRunNumber == 0 && _llCrossingTimes[0][0] < 9500) && !_bAccuracy3digits && !bToFile) // If this is first dog entry time (start) below 9.5ms
                                                                                                                   // use "ms" accuracy even if 2digits accuracy has been set
      {
         dCrossingTime = ((long long)(_llCrossingTimes[iDogNumber][iRunNumber] + 500) / 1000);
         dtostrf(dCrossingTime, 3, 0, cCrossingTime);
         strCrossingTime = "+";
         strCrossingTime += cCrossingTime;
         strCrossingTime += " ms";
      }
      else if (_bAccuracy3digits || ((iDogNumber == 0 && iRunNumber == 0 && _llCrossingTimes[0][0] < 9500) && !_bAccuracy3digits && bToFile))
      {
         dCrossingTime = ((long long)(_llCrossingTimes[iDogNumber][iRunNumber] + 500) / 1000) / 1000.0;
         dtostrf(dCrossingTime, 7, 3, cCrossingTime);
         strCrossingTime = "+";
         strCrossingTime += cCrossingTime;
      }
      else
      {
         dCrossingTime = ((long long)(_llCrossingTimes[iDogNumber][iRunNumber] + 5000) / 10000) / 100.0;
         dtostrf(dCrossingTime, 6, 2, cCrossingTime);
         strCrossingTime = "+";
         strCrossingTime += cCrossingTime;
      }
   }
   else if (_bDogMissedGateGoingin[iDogNumber][iRunNumber])
      strCrossingTime = " ";
   else if (_bDogPerfectCross[iDogNumber][iRunNumber])
   {
      if (_bAccuracy3digits)
         strCrossingTime = " Perfect";
      else
         strCrossingTime = "Perfect";
   }
   else if (_bDogBigOK[iDogNumber][iRunNumber])
   {
      if (_bAccuracy3digits)
         strCrossingTime = "      OK";
      else
         strCrossingTime = "     OK";
   }
   // We have dog time (crossing time is zero)
   else if (_llDogTimes[iDogNumber][iRunNumber] > 0)
   {
      if ((iDogRunCounters[iDogNumber] > 0 && iDogRunCounters[iDogNumber] != iRunNumber) || (iDogRunCounters[iDogNumber] == 0 && (_bDogFaults[iDogNumber] || _bDogManualFaults[iDogNumber])))
      {
         if (_bAccuracy3digits)
            strCrossingTime = "   fault";
         else
            strCrossingTime = "  fault";
      }
      else
      {
         if (_bAccuracy3digits)
            strCrossingTime = "      ok";
         else
            strCrossingTime = "     ok";
      }
   }
   // If dog is still running
   else if ((RaceState == RUNNING && iCurrentDog == iDogNumber && _byDogState == COMINGBACK) && iRunNumber <= iDogRunCounters[iDogNumber] //
            && _llDogEnterTimes[iDogNumber] != 0 && (MICROS - _llDogEnterTimes[iDogNumber]) > 300000)
   {
      if (_bDogFaults[iDogNumber] || _bDogManualFaults[iDogNumber])
      {
         if (_bAccuracy3digits)
            strCrossingTime = "   fault";
         else
            strCrossingTime = "  fault";
      }
      else
      {
         if (_bAccuracy3digits)
            strCrossingTime = "      ok";
         else
            strCrossingTime = "     ok";
      }
   }
   else
      strCrossingTime = " ";
   return strCrossingTime;
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
   if (bRerunsOff)
      strRerunInfo = "*X";
   else if (iDogRunCounters[iDogNumber] > 0)
   {
      strRerunInfo = "*";
      strRerunInfo += (iRunNumber + 1);
   }
   return strRerunInfo;
}

/// <summary>
///   Gets team Clean Time in seconds.
///   In case of heat would have a fault(s) Clean Time will not be displayed as it has no meaning.
///   In such situation "n/a" will be shown.
///   Please mark that "Cleat Time" term is often use in the meaning of Clean Time Breakout.
///   Please refer to FCI Regulations for Flyball Competition section 1.03 point (h).
/// </summary>
String RaceHandlerClass::GetCleanTime()
{
   String strCleanTime;
   if (!_bNoValidCleanTime)
   {
      long long llTotalNetTime = 0;
      for (auto &Dog : _llDogTimes)
      {
         for (auto &llNetTime : Dog)
         {
            if (llNetTime > 0)
               llTotalNetTime += llNetTime;
         }
      }
      char cCleanTime[8];
      double dCleanTime;
      if (!_bAccuracy3digits)
      {
         dCleanTime = ((long long)(llTotalNetTime + 5000) / 10000) / 100.0;
         dtostrf(dCleanTime, 7, 2, cCleanTime);
      }
      else
      {
         dCleanTime = ((long long)(llTotalNetTime + 500) / 1000) / 1000.0;
         dtostrf(dCleanTime, 7, 3, cCleanTime);
      }
      strCleanTime = cCleanTime;
   }
   else
      strCleanTime = "    n/a";
   return strCleanTime;
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
   stRaceData RequestedRaceData;
   // We need to return data for the current dace
   RequestedRaceData.Id = iCurrentRaceId + 1;
   RequestedRaceData.StartTime = llRaceStartTime / 1000;
   RequestedRaceData.EndTime = _llRaceEndTime / 1000;
   RequestedRaceData.ElapsedTime = GetRaceTime();
   RequestedRaceData.CleanTime = GetCleanTime();
   RequestedRaceData.raceState = RaceState;
   RequestedRaceData.RacingDogs = iNumberOfRacingDogs;
   RequestedRaceData.RerunsOff = bRerunsOff;

   // Get Dog info
   for (uint8_t i = 0; i < iNumberOfRacingDogs; i++)
   {
      RequestedRaceData.DogData[i].DogNumber = i;

      for (uint8_t i2 = 0; i2 <= iDogRunCounters[i]; i2++)
      {
         RequestedRaceData.DogData[i].Timing[i2].Time = GetDogTime(i, i2);
         RequestedRaceData.DogData[i].Timing[i2].CrossingTime = GetCrossingTime(i, i2);
      }
      RequestedRaceData.DogData[i].Fault = (_bDogFaults[i] || _bDogManualFaults[i]);
      RequestedRaceData.DogData[i].Running = (iCurrentDog == i);
   }
   return RequestedRaceData;
}

/// <summary>
///   Toggles the direction the system expects dogs to run in
/// </summary>
void RaceHandlerClass::ToggleRunDirection()
{
   if (RaceState == STOPPED || RaceState == RESET)
   {
      bRunDirectionInverted = !bRunDirectionInverted;
      SettingsManager.setSetting("RunDirectionInverted", String(bRunDirectionInverted));
      if (bRunDirectionInverted)
      {
         LCDController.UpdateField(LCDController.BoxDirection, "<");
         LCDController.bExecuteLCDUpdate = true;
         log_i("Run direction changed to: inverted");
      }
      else
      {
         LCDController.UpdateField(LCDController.BoxDirection, ">");
         LCDController.bExecuteLCDUpdate = true;
         log_i("Run direction changed to: normal");
      }
   }
   else
      return;
}

/// <summary>
///   Toggles displayed results accuracy between 2 and 3 digits
/// </summary>
void RaceHandlerClass::ToggleAccuracy()
{
   _bAccuracy3digits = !_bAccuracy3digits;
   SettingsManager.setSetting("Accuracy3digits", String(_bAccuracy3digits));
   LCDController.bUpdateAccuracyOnLCD = true;
   if (_bAccuracy3digits)
      log_i("Accuracy switched to 3 digits");
   else
      log_i("Accuracy switched to 2 digits");
#ifdef WiFiON
   WebHandler.bSendRaceData = true;
#endif
}

/// <summary>
///   Toggles status or reruns off/on
/// </summary>
void RaceHandlerClass::ToggleRerunsOffOn(uint8_t _iState)
{
   if (RaceState == STOPPED || RaceState == RESET)
   {
      if (_iState == 2)
         bRerunsOff = !bRerunsOff;
      else if (_iState == 1)
         bRerunsOff = true;
      else if (_iState == 0)
         bRerunsOff = false;

      if (bRerunsOff)
         log_i("Reruns turned off.");
      else
         log_i("Reruns turned on.");

#ifdef WiFiON
      WebHandler.bSendRaceData = true;
#endif
   }
   else
      return;
}

/// <summary>
///   Set number of racing dogs
/// </summary>
void RaceHandlerClass::SetNumberOfDogs(uint8_t _iNumberOfRacingDogs)
{
   iNumberOfRacingDogs = _iNumberOfRacingDogs;
   LCDController.UpdateNumberOfDogsOnLCD(iNumberOfRacingDogs);
#ifdef WiFiON
   WebHandler.bSendRaceData = true;
#endif
   log_i("Number of dogs set to: %i.", iNumberOfRacingDogs);
}

/// <summary>
///   Pushes an Sensor interrupt trigger record to the back of the input interrupts buffer.
/// </summary>
///
/// <param name="_InterruptTrigger">   The interrupt trigger record. </param>
void RaceHandlerClass::_QueuePush(RaceHandlerClass::STriggerRecord _InterruptTrigger)
{
   // Add record to queue
   _InputTriggerQueue[_iInputQueueWriteIndex] = _InterruptTrigger;

   // Write index has to be increased, check it we should wrap-around
   if (_iInputQueueWriteIndex == TRIGGER_QUEUE_LENGTH - 1) //(sizeof(_OutputTriggerQueue) / sizeof(*_OutputTriggerQueue) - 1))
      // Write index has reached end of array, start at 0 again
      _iInputQueueWriteIndex = 0;
   else
      // End of array not yet reached, increase index by 1
      _iInputQueueWriteIndex++;
}

/// <summary>
///   Filter input interrupt record(s) from the front of the interrupts buffer
///   and push filtered records to output interrupts records queue
/// </summary>
void RaceHandlerClass::_QueueFilter()
{
   
   STriggerRecord _PreviousRecord = _InputTriggerQueue[_iInputQueueReadIndex - 1]; //fix for Ultra 15-22
   STriggerRecord _CurrentRecord = _InputTriggerQueue[_iInputQueueReadIndex];
   STriggerRecord _NextRecord = _InputTriggerQueue[_iInputQueueReadIndex + 1];

   // If there are 2 records from the same sensors line and delta time is below 6ms ignore both
   if ((_iInputQueueReadIndex <= _iInputQueueWriteIndex - 2) && (_CurrentRecord.llTriggerTime - _PreviousRecord.llTriggerTime <= 200000)//
      && (_CurrentRecord.iSensorNumber == _NextRecord.iSensorNumber && _NextRecord.llTriggerTime - _CurrentRecord.llTriggerTime <= 6000))
   {
      // log_d("Next record %lld - Current record %lld = %lld < 6ms.", _NextRecord.llTriggerTime, _CurrentRecord.llTriggerTime, _NextRecord.llTriggerTime - _CurrentRecord.llTriggerTime);
      log_d("S%i | TT:%lld | T:%lld | St:%i | IGNORED-1", _CurrentRecord.iSensorNumber, _CurrentRecord.llTriggerTime,
            _CurrentRecord.llTriggerTime - llRaceStartTime, _CurrentRecord.iSensorState);
      log_d("S%i | TT:%lld | T:%lld | St:%i | IGNORED-2", _NextRecord.iSensorNumber, _NextRecord.llTriggerTime,
            _NextRecord.llTriggerTime - llRaceStartTime, _NextRecord.iSensorState);

      // Input Read index has to be increased, check it we should wrap-around
      if (_iInputQueueReadIndex == TRIGGER_QUEUE_LENGTH - 2)
         // Input Read index has reached end of array, start at 0 again
         _iInputQueueReadIndex = 0;
      else
         // End of array not yet reached, increase index by 2
         _iInputQueueReadIndex = _iInputQueueReadIndex + 2;
   }
   else
   {
      // Only one record available or next record is for different line or next record is for same line, but delta is above 6ms. Push record to Output Queue.
      // log_d("One record in the Input Queue. Push S%i record %lld to Output Queue.", _CurrentRecord.iSensorNumber, _CurrentRecord.llTriggerTime);
      // This function copy current record to common interrupt queue
      _OutputTriggerQueue[_iOutputQueueWriteIndex] = _InputTriggerQueue[_iInputQueueReadIndex];

      // Input Read index has to be increased, check it we should wrap-around
      if (_iInputQueueReadIndex == TRIGGER_QUEUE_LENGTH - 1)
         // Input Read index has reached end of array, start at 0 again
         _iInputQueueReadIndex = 0;
      else
         // End of array not yet reached, increase index by 1
         _iInputQueueReadIndex++;

      // Output Write index has to be increased, check it we should wrap-around
      if (_iOutputQueueWriteIndex == TRIGGER_QUEUE_LENGTH - 1)
         // Output Write index has reached end of array, start at 0 again
         _iOutputQueueWriteIndex = 0;
      else
         // End of array not yet reached, increase index by 1
         _iOutputQueueWriteIndex++;
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
   // This function returns the next record of the interrupt queue
   STriggerRecord NextRecord = _OutputTriggerQueue[_iOutputQueueReadIndex];

   // Read index has to be increased, check it we should wrap-around
   if (_iOutputQueueReadIndex == TRIGGER_QUEUE_LENGTH - 1) //(sizeof(_OutputTriggerQueue) / sizeof(*_OutputTriggerQueue) - 1))

      // Read index has reached end of array, start at 0 again
      _iOutputQueueReadIndex = 0;
   else
      // End of array not yet reached, increase index by 1
      _iOutputQueueReadIndex++;
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
   // This function checks if queue is empty.
   // This is determined by comparing the read and write index.
   // If they are equal, it means we have cought up reading and the queue is 'empty' (the array is not really emmpty...)
   if (_iOutputQueueReadIndex == _iOutputQueueWriteIndex)
      return true;
   else
      return false;
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
   // The transition string consists of lower and upper case A and B characters.
   // A indicates the handlers side, B indicates the boxes side
   // Uppercase indicates a high signal (dog broke beam), lowercase indicates a low signal (dog left beam)

   _llLastTransitionStringUpdate = MICROS;

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

   // We assumged the signal was high, if it is not we should make the character lowercase
   if (_InterruptTrigger.iSensorState == LOW)
      cTemp = tolower(cTemp);
   // Add the character to the transition string
   _strTransition += cTemp;

   // Filtering for unwanted sensor jitter - DEACTIVATED DUE TO RACE 25
   // Filter consecutive alternating changes out
   if (_strTransition.endsWith("AaA"))
   {
      _strTransition.replace("AaA", "A");
      log_d("Tstring AaA replaced with A");
   }
   if (_strTransition.endsWith("aAa"))
   {
      _strTransition.replace("aAa", "a");
      log_d("Tstring aAa replaced with a");
   }
   if (_strTransition.endsWith("BbB"))
   {
      _strTransition.replace("BbB", "B");
      log_d("Tstring BbB replaced with B");
   }
   if (_strTransition.endsWith("bBb"))
   {
      _strTransition.replace("bBb", "b");
      log_d("Tstring bBb replaced with b");
   }
}

RaceHandlerClass RaceHandler;
