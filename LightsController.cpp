// 
// 
// 

#include "LightsController.h"
#include "RaceHandler.h"
#include "global.h"

void LightsControllerClass::init(int iLatchPin, int iClockPin, int iDataPin)
{
   _iLatchPin = iLatchPin;
   _iClockPin = iClockPin;
   _iDataPin = iDataPin;

   pinMode(_iLatchPin, OUTPUT);
   pinMode(_iClockPin, OUTPUT);
   pinMode(_iDataPin, OUTPUT);
}

void LightsControllerClass::HandleLightStates()
{
   HandleStartSequence();
   if (_byCurrentLightsState != _byNewLightsState)
   {
      bDEBUG ? printf("%lu: New light states: %i\r\n", millis(), _byNewLightsState) : NULL;
      _byCurrentLightsState = _byNewLightsState;
      digitalWrite(_iLatchPin, LOW);
      shiftOut(_iDataPin, _iClockPin, MSBFIRST, _byCurrentLightsState);
      digitalWrite(_iLatchPin, HIGH);
   }
}

void LightsControllerClass::HandleStartSequence()
{
   //This function takes care of the starting lights sequence
   //First check if the overall state of this calls is 'STARTING'
   if (byOverallState == STARTING)
   {
      //The class is in the 'STARTING' state, check if the lights have been programmed yet
      if (!_bStartSequenceStarted)
      {
         //Start sequence is not yet started, we need to schedule the lights on/off times
         
         //Set schedule for RED light
         _lLightsOnSchedule[1] = millis(); //Turn on NOW
         _lLightsOutSchedule[1] = millis() + 1000; //keep on for 1 second

         //Set schedule for YELLOW1 light
         _lLightsOnSchedule[2] = millis() + 1000; //Turn on after 1 second
         _lLightsOutSchedule[2] = millis() + 2000; //Turn off after 2 seconds

         //Set schedule for YELLOW2 light
         _lLightsOnSchedule[4] = millis() + 2000; //Turn on after 2 seconds
         _lLightsOutSchedule[4] = millis() + 3000; //Turn off after 2 seconds

         //Set schedule for GREEN light
         _lLightsOnSchedule[5] = millis() + 3000; //Turn on after 3 seconds
         _lLightsOutSchedule[5] = millis() + 4000; //Turn off after 4 seconds

         _bStartSequenceStarted = true;
      }
      //Check if we have to toggle any lights
      bool bStartSequenceBusy = false;
      for (int i = 0; i < 6; i++)
      {
         if (millis() > _lLightsOnSchedule[i]
            && _lLightsOnSchedule[i] != 0)
         {
            ToggleLightState(_byLightsArray[i], ON);
            _lLightsOnSchedule[i] = 0; //Delete schedule
         }
         if (millis() > _lLightsOutSchedule[i]
            && _lLightsOutSchedule[i] != 0)
         {
            ToggleLightState(_byLightsArray[i], OFF);
            _lLightsOutSchedule[i] = 0; //Delete schedule
         }
         if (_lLightsOnSchedule[i] > 0
            || _lLightsOutSchedule[i] > 0)
         {
            bStartSequenceBusy = true;
         }
      }
      //Check if we should start the timer (GREEN light on)
      if (CheckLightState(GREEN) == ON
         && RaceHandler.RaceState == RaceHandler.STARTING)
      {
         bDEBUG ? printf("%lu: GREEN light is ON!\r\n", millis()) : NULL;
         RaceHandler.StartTimers();
      }
      if (!bStartSequenceBusy)
      {
         _bStartSequenceStarted = false;
         byOverallState = STARTED;
      }
   }
}

void LightsControllerClass::InitiateStartSequence()
{
   byOverallState = STARTING;
}

void LightsControllerClass::ToggleLightState(Lights byLight, LightStates byLightState)
{
   bool byCurrentLightState = CheckLightState(byLight);
   if (byLightState == TOGGLE)
   {
      //We have to toggle the lights
      if (byCurrentLightState == ON)
      {
         byLightState = OFF;
      }
      else
      {
         byLightState = ON;
      }
   }
   if (byCurrentLightState != byLightState)
   {
      
      if (byLightState == ON)
      {
         _byNewLightsState = _byNewLightsState + byLight;
      }
      else
      {
         _byNewLightsState = _byNewLightsState - byLight;
      }
   }
}

LightsControllerClass::LightStates LightsControllerClass::CheckLightState(Lights byLight)
{
   //TODO: this function keeps returning true when it should not...
   bDEBUG ? printf("%lu: CheckLightState: %c\r\n", millis(), byLight && _byNewLightsState) : NULL;
   if (byLight && _byNewLightsState)
   {
      return ON;
   }
   else
   {
      return OFF;
   }
}

LightsControllerClass LightsController;

