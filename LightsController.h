// LightsController.h

#ifndef _LIGHTSCONTROLLER_h
#define _LIGHTSCONTROLLER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

class LightsControllerClass
{
protected:

public:
	void init(int iLatchPin, int iClockPin, int iDataPin);

   //Overal state of this class
   enum OverallStates{
      STOPPED,
      STARTING,
      STARTED
   };
   OverallStates byOverallState = STOPPED;

   //Decimal values of lights connected to 74HC595
   enum Lights {
      WHITE = 2,
      RED = 4,
      YELLOW1 = 8,
      BLUE = 16,
      YELLOW2 = 32,
      GREEN = 64
   };

   enum LightStates {
      OFF,
      ON,
      TOGGLE
   };
   LightStates CheckLightState(Lights byLight);
   void HandleLightStates();
   void HandleStartSequence();
   void InitiateStartSequence();
   void ToggleLightState(Lights byLight, LightStates byLightState = TOGGLE);
   void ResetLights();
   void DeleteSchedules();
   
private:
   //Pin connected to ST_CP of 74HC595
   int _iLatchPin = 12;
   //Pin connected to SH_CP of 74HC595
   int _iClockPin = 13;
   //Pin connected to DS of 74HC595
   int _iDataPin = 11;

   //This byte contains the combined states of all ligths at any given time
   byte _byCurrentLightsState = 0;
   byte _byNewLightsState = 0;

   bool _bStartSequenceStarted = 0;

   long _lLightsOnSchedule[6];
   long _lLightsOutSchedule[6];

   Lights _byLightsArray[6] = {
      WHITE,
      RED,
      YELLOW1,
      BLUE,
      YELLOW2,
      GREEN
   };

};

extern LightsControllerClass LightsController;

#endif

