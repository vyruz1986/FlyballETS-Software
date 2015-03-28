#include "LCDController.h"
#include "RaceHandler.h"
#include "LightsController.h"
#include "BatterySensor.h"
#include "printf.h"
#include "global.h"
#include <LiquidCrystal.h>

/*List of pins and the ones used:
   - D0: Reserved for RX
   - D1: Reserved for TX
   - D2: S1 (handler side) photoelectric sensor
   - D3: S2 (box side) photoelectric sensor
   - D4: LCD Data0
   - D5: LCD Data1
   - D6: LCD Data2
   - D7: LCD Data3
   - D8: Lights 74HC595 clock pin
   - D9: Lights 74HC595 data pin
   - D10: LCD2 (line 3&4) enable pin
   - D11: LCD1 (line 1&2) enable pin
   - D12: LCD RS Pin
   - D13: Lights 74HC595 latch pin
   - A0: remote D5
   - A1: remote D2
   - A2: remote D1
   - A3: remote D0
   - A4: remote D3
   - A5: remote D4
   - A6: battery sensor pin
   - A7: <free>
*/

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

char cDogTime[8];
char cCrossingTime[7];
char cElapsedRaceTime[8];
char cTotalCrossingTime[8];

//Battery variables
int iBatterySensorPin = A6;
uint16_t fBatteryVoltage = 0;

//Initialise Lights stuff
long lLastSerialOutput = 0;

//remote control pins
int iRC0Pin = A3;
int iRC1Pin = A2;
int iRC2Pin = A1;
int iRC3Pin = A4;
int iRC4Pin = A5;
int iRC5Pin = A0;
//Array to hold last time button presses
unsigned long lLastRCPress[6] = {0, 0, 0, 0, 0, 0};

LiquidCrystal lcd(12, 11, 7, 6, 5, 4);  //declare two LCD's
LiquidCrystal lcd2(12, 10, 7, 6, 5, 4); // Ths is the second

void setup()
{
  Serial.begin(115200);
  printf_begin();

  pinMode(iS1Pin, INPUT);
  pinMode(iS2Pin, INPUT);
  attachInterrupt(1, Sensor2Wrapper, CHANGE);
  attachInterrupt(0, Sensor1Wrapper, CHANGE);
  BatterySensor.init(iBatterySensorPin);

  LightsController.init(13,8,9);
  RaceHandler.init(iS1Pin, iS2Pin);
  pinMode(iRC0Pin, INPUT);
  pinMode(iRC1Pin, INPUT);
  pinMode(iRC2Pin, INPUT);
  pinMode(iRC3Pin, INPUT);
  pinMode(iRC4Pin, INPUT);
  pinMode(iRC5Pin, INPUT);

  LCDController.init(&lcd, &lcd2);
}

void loop()
{
   //Handle Race main processing
   RaceHandler.Main();

   //Handle lights main processing
   LightsController.Main();

   //Handle battery sensor main processing
   BatterySensor.CheckBatteryVoltage();

   //Handle LCD processing
   LCDController.Main();
   
   
   /*Update LCD Display fields*/
   //Update team time to display
   dtostrf(RaceHandler.GetRaceTime(), 7, 3, cElapsedRaceTime);
   LCDController.UpdateField(LCDController.TeamTime, cElapsedRaceTime);

   //Update battery percentage to display
   fBatteryVoltage = BatterySensor.GetBatteryVoltage();
   uint16_t iBatteryPercentage = BatterySensor.GetBatteryPercentage();
   LCDController.UpdateField(LCDController.BattLevel, String(iBatteryPercentage));

   //Update total crossing time
   dtostrf(RaceHandler.GetTotalCrossingTime(), 7, 3, cTotalCrossingTime);
   LCDController.UpdateField(LCDController.TotalCrossTime, cTotalCrossingTime);

   //Update race status to display
   LCDController.UpdateField(LCDController.RaceState, RaceHandler.GetRaceStateString());

   //Handle individual dog info
   dtostrf(RaceHandler.GetDogTime(0), 7, 3, cDogTime);
   LCDController.UpdateField(LCDController.D1Time, cDogTime);
   LCDController.UpdateField(LCDController.D1CrossTime, RaceHandler.GetCrossingTime(0));
   LCDController.UpdateField(LCDController.D1RerunInfo, RaceHandler.GetRerunInfo(0));

   dtostrf(RaceHandler.GetDogTime(1), 7, 3, cDogTime);
   LCDController.UpdateField(LCDController.D2Time, cDogTime);
   LCDController.UpdateField(LCDController.D2CrossTime, RaceHandler.GetCrossingTime(1));
   LCDController.UpdateField(LCDController.D2RerunInfo, RaceHandler.GetRerunInfo(1));

   dtostrf(RaceHandler.GetDogTime(2), 7, 3, cDogTime);
   LCDController.UpdateField(LCDController.D3Time, cDogTime);
   LCDController.UpdateField(LCDController.D3CrossTime, RaceHandler.GetCrossingTime(2));
   LCDController.UpdateField(LCDController.D3RerunInfo, RaceHandler.GetRerunInfo(2));

   dtostrf(RaceHandler.GetDogTime(3), 7, 3, cDogTime);
   LCDController.UpdateField(LCDController.D4Time, cDogTime);
   LCDController.UpdateField(LCDController.D4CrossTime, RaceHandler.GetCrossingTime(3));
   LCDController.UpdateField(LCDController.D4RerunInfo, RaceHandler.GetRerunInfo(3));

   if (RaceHandler.RaceState != RaceHandler.PreviousRaceState)
   {
      printf("%lu: Race State Changed: %i\r\n", millis(), RaceHandler.RaceState);
   }

   if (RaceHandler.iCurrentDog != RaceHandler.iPreviousDog)
   {
      dtostrf(RaceHandler.GetDogTime(RaceHandler.iPreviousDog), 7, 3, cDogTime);
      printf("%lu: Dog %i time: %s\r\n", millis(), RaceHandler.iPreviousDog, cDogTime);
      printf("%lu: Dog number changed to: %i\r\n", millis(), RaceHandler.iCurrentDog);
   }

   if ((millis() - lLastSerialOutput) > 5000)
   {
      //printf("%lu: ping! voltage is: %.2u, this is %i%%\r\n", millis(), fBatteryVoltage, iBatteryPercentage);
      //printf("%lu: Elapsed time: %s\r\n", millis(), cElapsedRaceTime);

      lLastSerialOutput = millis();
   }

   //Race start/stop button (remote D0 output)
   if (digitalRead(iRC0Pin) == HIGH
       && (millis() - lLastRCPress[0] > 2000))
   {
     lLastRCPress[0] = millis();
      if (RaceHandler.RaceState == RaceHandler.STOPPED //If race is stopped
         && RaceHandler.GetRaceTime() == 0)           //and timers are zero
      {
         //Then start the race
         bDEBUG ? printf("%lu: Starting light sequence!\r\n", millis()) : NULL;
         LightsController.InitiateStartSequence();
         RaceHandler.StartRace();
      }
      else //If race state is running or starting, we should stop it
      {
         RaceHandler.StopRace();
         LightsController.DeleteSchedules();
      }
   }

   //Race reset button (remote D1 output)
   if (digitalRead(iRC1Pin) == HIGH
      && RaceHandler.RaceState == RaceHandler.STOPPED   //Only allow reset when race is stopped first
      && (millis() - lLastRCPress[1] > 2000))
   {
      lLastRCPress[1] = millis();
      LightsController.ResetLights();
      RaceHandler.ResetRace();
   }

   //Dog0 fault RC button
   if (digitalRead(iRC2Pin) == HIGH
      && RaceHandler.RaceState == RaceHandler.RUNNING   //Only allow reset when race is stopped first
      && (millis() - lLastRCPress[2] > 2000))
   {
      lLastRCPress[2] = millis();
      //Toggle fault for dog
      RaceHandler.SetDogFault(0);
   }

   //Dog1 fault RC button
   if (digitalRead(iRC3Pin) == HIGH
      && RaceHandler.RaceState == RaceHandler.RUNNING   //Only allow reset when race is stopped first
      && (millis() - lLastRCPress[3] > 2000))
   {
      lLastRCPress[3] = millis();
      //Toggle fault for dog
      RaceHandler.SetDogFault(1);
   }
   //Dog2 fault RC button
   if (digitalRead(iRC4Pin) == HIGH
      && RaceHandler.RaceState == RaceHandler.RUNNING   //Only allow reset when race is stopped first
      && (millis() - lLastRCPress[4] > 2000))
   {
      lLastRCPress[4] = millis();
      //Toggle fault for dog
      RaceHandler.SetDogFault(2);
   }
   
   //Dog3 fault RC button
   if (digitalRead(iRC5Pin) == HIGH
      && RaceHandler.RaceState == RaceHandler.RUNNING   //Only allow reset when race is stopped first
      && (millis() - lLastRCPress[5] > 2000))
   {
      lLastRCPress[5] = millis();
      //Toggle fault for dog
      RaceHandler.SetDogFault(3);
   }

   //Cleanup
   //These variables are used to determine whether a value changed
   //They should be reset at the end of the loop since otherwise they stay always true
   RaceHandler.PreviousRaceState = RaceHandler.RaceState;
   RaceHandler.iPreviousDog = RaceHandler.iCurrentDog;
}

void Sensor2Wrapper()
{
   RaceHandler.TriggerSensor2();
}

void Sensor1Wrapper()
{
   RaceHandler.TriggerSensor1();
}
