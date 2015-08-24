#include "StreamPrint.h"
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

   digitalWrite(_iLatchPin, LOW);
   shiftOut(_iDataPin, _iClockPin, MSBFIRST, 0);
   digitalWrite(_iLatchPin, HIGH);
}

void LightsControllerClass::Main()
{
   HandleStartSequence();

   //Check if we have to toggle any lights
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
   }

   if (_byCurrentLightsState != _byNewLightsState)
   {
      if (bDEBUG) Serialprint("%lu: New light states: %i\r\n", millis(), _byNewLightsState);
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
      //Check if the start sequence is busy
      bool bStartSequenceBusy = false;
      for (int i = 0; i < 6; i++)
      {
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
         RaceHandler.StartTimers();
         if (bDEBUG) Serialprint("%lu: GREEN light is ON!\r\n", millis());
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

void LightsControllerClass::ResetLights()
{
   byOverallState = STOPPED;
   
   //Set all lights off
   _byNewLightsState = 0;
   DeleteSchedules();
}

void LightsControllerClass::DeleteSchedules()
{
   //Delete any set schedules
   for (int i = 0; i < 6; i++)
   {
      _lLightsOnSchedule[i] = 0; //Delete schedule
      _lLightsOutSchedule[i] = 0; //Delete schedule
   }
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

void LightsControllerClass::ToggleFaultLight(uint8_t DogNumber, LightStates byLightState)
{
   //Get error light for dog number from array
   Lights byLight = _byDogErrorLigths[DogNumber];
   if (byLightState == ON)
   {
      //If a fault lamp is turned on we have to light the white light for 1 sec
      //Set schedule for WHITE light
      _lLightsOnSchedule[0] = millis(); //Turn on NOW
      _lLightsOutSchedule[0] = millis() + 1000; //keep on for 1 second
   }
   ToggleLightState(byLight, byLightState);
   if (bDEBUG) Serialprint("Fault light for dog %i: %i\r\n", DogNumber, byLightState);
}

LightsControllerClass::LightStates LightsControllerClass::CheckLightState(Lights byLight)
{
   if ((byLight & _byNewLightsState) == byLight)
   {
      return ON;
   }
   else
   {
      return OFF;
   }
}

LightsControllerClass LightsController;

