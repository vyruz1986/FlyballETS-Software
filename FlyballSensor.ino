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
//LiquidCrystal lcd(RS,RW,Enable1,Enable2, data3,data2,data1,data0);
LiquidCrystal lcd(12, 11, 7, 6, 5, 4);  //declare two LCD's
LiquidCrystal lcd2(12, 10, 7, 6, 5, 4); // Ths is the second


uint16_t fBatteryVoltage = 0;

//Initialise Lights stuff
long lLastSerialOutput = 0;

//int senspin = 3;

//Pin connected to the Start/Stop button
int iPushButtonPin = A1;

void setup()
{
  Serial.begin(115200);
  printf_begin();
  //pinMode(senspin, INPUT);
  pinMode(ledPin, OUTPUT);         // sets the digital pin as output
  lcd.begin(40,2);
  lcd.clear();
  attachInterrupt(1, Sensor2Wrapper, RISING);
  attachInterrupt(0, Sensor1Wrapper, RISING);
  BatterySensor.init(A0);

  LightsController.init(13,12,11);
  RaceHandler.init();
  pinMode(iPushButtonPin, INPUT_PULLUP);
}

void loop()
{
   //Handle Race main processing
   RaceHandler.Main();

   //Handle lights main processing
   LightsController.HandleLightStates();

   //Handle battery sensor main processing
   BatterySensor.CheckBatteryVoltage();
   
   
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
      fBatteryVoltage = BatterySensor.GetBatteryVoltage();
      //printf("%lu: ping! voltage is: %.2u\r\n", millis(), fBatteryVoltage);
      
      lLastSerialOutput = millis();
   }

   if (digitalRead(iPushButtonPin) == LOW
      || millis() > 5000 && LightsController.byOverallState == LightsController.STOPPED)  //TODO:start after 5 seconds since we don't have a pushbutton yet :-(
   {
      bDEBUG ? printf("%lu: Starting light sequence!\r\n", millis()) : NULL;
      LightsController.InitiateStartSequence();
      RaceHandler.StartRace();
   }
   

   //Update LCD if longer than lLCDInterval ago
   if(millis() - lPreviousLCDUpdate > lLCDInterval)
   {
      fUpdateLCD(0,strLCDLine1);
      fUpdateLCD(1,FormattedTime);
      lPreviousLCDUpdate = millis();
   }

   //Cleanup
   //These variables are used to determine whether a value changed
   //They should be reset at the end of the loop since otherwise they stay always true
   RaceHandler.PreviousRaceState = RaceHandler.RaceState;
   RaceHandler.iPreviousDog = RaceHandler.iCurrentDog;
}

void fUpdateLCD(int iLine, String strMessage)
{
  //Check how long strMessage is:
  int iMessageLength = strMessage.length();
  if(iMessageLength > 16)
  {
    //Message is too long, make it scroll!
    int iExtraChars = iMessageLength - 15;
    for (int i=0; i < iExtraChars; i++)
    {
      String strMessageSubString = strMessage.substring(i,i+16);
      lcd.setCursor(INT0,iLine);
      lcd.print(strMessageSubString);
    }
    return;
  }
  else if(iMessageLength < 16)
  {
    //Message is too short, we need to pad it
    //First find missing characters
    int iMissingChars = 16 - iMessageLength;
    for (int i = 0; i < iMissingChars; i++)
    {
      strMessage = String(strMessage + " ");
    }
  }
  lcd.setCursor(0,iLine);
  lcd.print(strMessage);
}

String fGetFormattedTime(long lMilliTime)
{
  elapsedTime =   lMilliTime;
  FormattedTime = String((int)(elapsedTime / 1000L));
  FormattedTime = String(FormattedTime + ".");
  // use modulo operator to get fractional part of time 
  fractional = (int)(elapsedTime % 1000L);
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
