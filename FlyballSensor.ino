#include "LCDController.h"
#include "RaceHandler.h"
#include "LightsController.h"
#include "BatterySensor.h"
#include "printf.h"
#include "global.h"
#include <LiquidCrystal.h>

#define ledPin  5                  // LED connected to digital pin 13

int value = LOW;                    // previous value of the LED
int blinking;                       // condition for blinking - timer is timing
long interval = 100;                // blink interval - change to suit
long lLCDInterval = 250;
long lPreviousLCDUpdate = 0;
long previousMillis = 0;            // variable to store last time LED was updated
long startTime ;                    // start time for stop watch
long elapsedTime ;                  // elapsed time for stop watch
int fractional;                     // variable used to store fractional part of time
float ElapsedTimeMillis;
int DecimalTime;
String FormattedTime;
long lLastTriggeredMillis = 0;
String strLCDLine1;
int iS1Pin = 2;
int iS2Pin = 3;



uint16_t fBatteryVoltage = 0;

//Initialise Lights stuff
long lLastSerialOutput = 0;

//int senspin = 3;

//Pin connected to the Start/Stop button
int iPushButtonPin = A1;

//LiquidCrystal lcd(RS,RW,Enable1,Enable2, data3,data2,data1,data0);
LiquidCrystal lcd(12, 11, 7, 6, 5, 4);  //declare two LCD's
LiquidCrystal lcd2(12, 10, 7, 6, 5, 4); // Ths is the second

void setup()
{
  Serial.begin(115200);
  printf_begin();
  //pinMode(senspin, INPUT);
  pinMode(ledPin, OUTPUT);         // sets the digital pin as output

  pinMode(iS1Pin, INPUT);
  pinMode(iS2Pin, INPUT);
  attachInterrupt(1, Sensor2Wrapper, CHANGE);
  attachInterrupt(0, Sensor1Wrapper, CHANGE);
  BatterySensor.init(A0);

  LightsController.init(13,8,9);
  RaceHandler.init(iS1Pin, iS2Pin);
  pinMode(iPushButtonPin, INPUT_PULLUP);

  LCDController.init(&lcd, &lcd2);
}

void loop()
{
   //Handle Race main processing
   RaceHandler.Main();

   //Handle lights main processing
   LightsController.HandleLightStates();

   //Handle battery sensor main processing
   BatterySensor.CheckBatteryVoltage();

   //Handle LCD processing
   LCDController.Main();
   
   //Update time to display
   double dElapsedRaceTime = RaceHandler.GetRaceTime();
   char cElapsedRaceTime[8];
   dtostrf(dElapsedRaceTime, 7, 3, cElapsedRaceTime);
   LCDController.UpdateField(LCDController.TeamTime, String(cElapsedRaceTime));

   //Update battery percentage to display
   fBatteryVoltage = BatterySensor.GetBatteryVoltage();
   uint16_t iBatteryPercentage = BatterySensor.GetBatteryPercentage();
   LCDController.UpdateField(LCDController.BattLevel, String(iBatteryPercentage));

   if (RaceHandler.RaceState == RaceHandler.RUNNING)
   {
      if (RaceHandler.GetDogTime(0) > 0)
      {
         char cDogTime[8];
         dtostrf(RaceHandler.GetDogTime(0), 7, 3, cDogTime);
         LCDController.UpdateField(LCDController.D1Time, cDogTime);
      }

      if (RaceHandler.GetDogTime(1) > 0)
      {
         char cDogTime[8];
         dtostrf(RaceHandler.GetDogTime(1), 7, 3, cDogTime);
         LCDController.UpdateField(LCDController.D2Time, cDogTime);
      }

      if (RaceHandler.GetDogTime(2) > 0)
      {
         char cDogTime[8];
         dtostrf(RaceHandler.GetDogTime(2), 7, 3, cDogTime);
         LCDController.UpdateField(LCDController.D3Time, cDogTime);
      }

      if (RaceHandler.GetDogTime(3) > 0)
      {
         char cDogTime[8];
         dtostrf(RaceHandler.GetDogTime(3), 7, 3, cDogTime);
         LCDController.UpdateField(LCDController.D4Time, cDogTime);
      }
      
   }
   
   if (RaceHandler.RaceState != RaceHandler.PreviousRaceState)
   {
      printf("%lu: Race State Changed: %i\r\n", millis(), RaceHandler.RaceState);
   }

   if (RaceHandler.iCurrentDog != RaceHandler.iPreviousDog)
   {
      printf("%lu: Dog %i time: %lu\r\n", millis(), RaceHandler.iPreviousDog, RaceHandler.lDogTimes[RaceHandler.iPreviousDog]);
      printf("%lu: Dog number changed to: %i\r\n", millis(), RaceHandler.iCurrentDog);
   }

   if ((millis() - lLastSerialOutput) > 5000)
   {
      printf("%lu: ping! voltage is: %.2u, this is %i%%\r\n", millis(), fBatteryVoltage, iBatteryPercentage);
      printf("%lu: Elapsed time: %s\r\n", millis(), cElapsedRaceTime);
      for (int i = 0; i < 4; i++)
      {
         printf("%lu: Dog %i time: %lu \r\n", millis(), i, RaceHandler.lDogTimes[i]);
      }

      lLastSerialOutput = millis();
   }

   if (digitalRead(iPushButtonPin) == LOW
      || millis() > 5000 && RaceHandler.RaceState == RaceHandler.STOPPED)  //TODO:start after 5 seconds since we don't have a pushbutton yet :-(
   {
      bDEBUG ? printf("%lu: Starting light sequence!\r\n", millis()) : NULL;

      //TODO: Disabled lightsequence for now since this causes interference with the LCD display
      LightsController.InitiateStartSequence();
      RaceHandler.StartRace();
   }
   //Cleanup
   //These variables are used to determine whether a value changed
   //They should be reset at the end of the loop since otherwise they stay always true
   RaceHandler.PreviousRaceState = RaceHandler.RaceState;
   RaceHandler.iPreviousDog = RaceHandler.iCurrentDog;
}


String fGetFormattedTime(long lMicroTime)
{
  FormattedTime = String((int)(lMicroTime / 1000000L));
  FormattedTime = String(FormattedTime + ",");
  // use modulo operator to get fractional part of time 
  fractional = (int)(lMicroTime % 1000L);
  FormattedTime = String(FormattedTime + fractional);
  // pad in leading zeros - wouldn't it be nice if 
  // Arduino language had a flag for this? :)
  if (fractional == 0)
  {
    FormattedTime = String(FormattedTime + "000");
  }
  else if (fractional < 10)    // if fractional < 10 the 0 is ignored giving a wrong time, so add the zeros
  {
    FormattedTime = String(FormattedTime + "00");
  }
  else if (fractional < 100)
  {
    FormattedTime = String(FormattedTime + "00");
  }
  
  return FormattedTime;
}

void Sensor2Wrapper()
{
   RaceHandler.TriggerSensor2();
}

void Sensor1Wrapper()
{
   RaceHandler.TriggerSensor1();
}
