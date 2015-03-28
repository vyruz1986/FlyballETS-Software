// LCDController.h

#ifndef _LCDCONTROLLER_h
#define _LCDCONTROLLER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif
#include <LiquidCrystal.h>
class LCDControllerClass
{
 protected:


 public:
	void init(LiquidCrystal* Clcd1, LiquidCrystal* Clcd2);
   void Main();
   enum LCDFields {
      D1Time,
      D1RerunInfo,
      D2Time,
      D2RerunInfo,
      D3Time,
      D3RerunInfo,
      D4Time,
      D4RerunInfo,
      D1CrossTime,
      D2CrossTime,
      D3CrossTime,
      D4CrossTime,
      RaceState,
      BattLevel,
      TeamTime,
      TotalCrossTime
   };

   void UpdateField(LCDFields lcdfieldField, String strNewValue);

private:
   void _UpdateLCD(int _iLine, int _iPosition, String _strText, int _iFieldLength);
   LiquidCrystal* _Clcd1;
   LiquidCrystal* _Clcd2;
   unsigned long _lLastLCDUpdate = 0;
   unsigned int _iLCDUpdateInterval = 500; //500ms update interval

   struct SLCDField
   {
      int iLine;
      int iStartingPosition;
      int iFieldLength;
      String strText;
   }_lcdfieldFields[16];

};

extern LCDControllerClass LCDController;

#endif

