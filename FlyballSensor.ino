#include "RaceHandler.h"
#include "LightsController.h"
#include "BatterySensor.h"
#include <StopWatch.h>
#include <LiquidCrystal.h>



/* StopWatch
 * Paul Badger 2008
 * Demonstrates using millis(), pullup resistors, 
 * making two things happen at once, printing fractions
 *
 * Physical setup: momentary switch connected to pin 4, other side connected to ground
 * LED with series resistor between pin 13 and ground
 */


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

StopWatch SWRace(StopWatch::MICROS);
StopWatch SWDog1(StopWatch::MICROS);
StopWatch SWDog2(StopWatch::MICROS);
StopWatch SWDog3(StopWatch::MICROS);
StopWatch SWDog4(StopWatch::MICROS);


float fBatteryVoltage = 0;

//Initialise Lights stuff
long lLastSerialOutput = 0;

//int senspin = 3;

int iPushButtonPin = A1;

void setup()
{
  Serial.begin(115200);
  //pinMode(senspin, INPUT);
  pinMode(ledPin, OUTPUT);         // sets the digital pin as output
  lcd.begin(40,2);
  lcd.clear();
  attachInterrupt(1,RaceHandler.TriggerSensor2, RISING);
  //attachInterrupt(0, fToggleStopWatch, LOW);
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
   
   if (millis() - lLastSerialOutput > 500)
   {
      fBatteryVoltage = BatterySensor.GetBatteryVoltage();
      //Serial.print(millis()); Serial.print(": Voltage: "); Serial.println(fBatteryVoltage);
      lLastSerialOutput = millis();
      //Lights.ToggleLightState(Lights.BLUE);
   }

   if (digitalRead(iPushButtonPin) == LOW)
   {
      LightsController.InitiateStartSequence();
      RaceHandler.StartRace();
   }
   
   switch (RaceHandler.RaceState)
   {
      case RaceHandler.STOPPED:
         strLCDLine1 = "Stopped";
         break;

      case RaceHandler.STARTING:
         strLCDLine1 = "Starting";
         break;

      case RaceHandler.GOINGIN:
      case RaceHandler.COMINGBACK:
         strLCDLine1 = "Running";
         break;
   }

   //Update LCD if longer than lLCDInterval ago
   if(millis() - lPreviousLCDUpdate > lLCDInterval)
   {
      fUpdateLCD(0,strLCDLine1);
      fUpdateLCD(1,FormattedTime);
      lPreviousLCDUpdate = millis();
   }
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

void fToggleStopWatch()
{
  if((millis() - lLastTriggeredMillis) > 200)
  {
    lLastTriggeredMillis = millis();
    //first get stopwatch state
    if(sw_millis.isRunning())
    {
      //stopwatch is running, so we must stop it
      sw_millis.stop();
    }
    else
    {
      //stopwatch is stopped, so we must start it
      //First reset it
      sw_millis.reset();
      //then start
      sw_millis.start();
    }
  }
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
      
