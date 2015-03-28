// 
// 
// 

#include "LCDController.h"
#include <LiquidCrystal.h>

void LCDControllerClass::init(LiquidCrystal* Clcd1, LiquidCrystal* Clcd2)
{
   _Clcd1 = Clcd1;
   _Clcd1->begin(40, 2);
   _Clcd1->clear();

   _Clcd2 = Clcd2;
   _Clcd2->begin(40, 2);
   _Clcd2->clear();

   //Put initial text on screen
   //                                 1         2         3
   //LCD layout:            0123456789012345678901234567890123456789
   _UpdateLCD(1, 0, String("1:   0.000s +  0.000s   | STOP   B:   0%"), 40);
   _UpdateLCD(2, 0, String("2:   0.000s +  0.000s   | Team:   0.000s"), 40);
   _UpdateLCD(3, 0, String("3:   0.000s +  0.000s   |   CR:   0.000s"), 40);
   _UpdateLCD(4, 0, String("4:   0.000s +  0.000s   |"), 40);

   _lcdfieldFields[D1Time] = { 1, 3, 7, String("  0.000") };
   _lcdfieldFields[D1RerunInfo] = { 1, 22, 2, String("  ") };
   _lcdfieldFields[D2Time] = { 2, 3, 7, String("  0.000") };
   _lcdfieldFields[D2RerunInfo] = { 2, 22, 2, String("  ") };
   _lcdfieldFields[D3Time] = { 3, 3, 7, String("  0.000") };
   _lcdfieldFields[D3RerunInfo] = { 3, 22, 2, String("  ") };
   _lcdfieldFields[D4Time] = { 4, 3, 7, String("  0.000") };
   _lcdfieldFields[D4RerunInfo] = { 4, 22, 2, String("  ") };
   _lcdfieldFields[D1CrossTime] = { 1, 12, 8, String("+  0.000") };
   _lcdfieldFields[D2CrossTime] = { 2, 12, 8, String("+  0.000") };
   _lcdfieldFields[D3CrossTime] = { 3, 12, 8, String("+  0.000") };
   _lcdfieldFields[D4CrossTime] = { 4, 12, 8, String("+  0.000") };
   _lcdfieldFields[BattLevel] = { 1, 36, 3, String("  0") };
   _lcdfieldFields[RaceState] = { 1, 25, 7, String(" STOP") };
   _lcdfieldFields[TeamTime] = { 2, 32, 7, String("  0.000") };
   _lcdfieldFields[TotalCrossTime] = { 3, 32, 7, String("  0.000") };
}

void LCDControllerClass::Main()
{
   //This is the main loop which handles LCD updates
   if ((millis() - _lLastLCDUpdate) > _iLCDUpdateInterval)
   {
      //Remark: The for statement below is a range-for loop, this is new in C++11,
      //which is not (yet) enabled by default in Arduino (as of Arduino IDE 1.6.1).
      //See this link on how to enable C++11: http://stackoverflow.com/questions/16224746/how-to-use-c11-to-program-the-arduino
      //If you do not enable C++11 than the statement below will fail to compile.
      for (const SLCDField& lcdField : _lcdfieldFields)
      {
         _UpdateLCD(lcdField.iLine, lcdField.iStartingPosition, lcdField.strText, lcdField.iFieldLength);
      }
      
      _lLastLCDUpdate = millis();
   }
}

void LCDControllerClass::UpdateField(LCDFields lcdfieldField, String strNewValue)
{
   _lcdfieldFields[lcdfieldField].strText = strNewValue;
}

void LCDControllerClass::_UpdateLCD(int _iLine, int _iPosition, String _strText, int _iFieldLength)
{
   LiquidCrystal* CActiveLCD = 0;
   //LiquidCrystal *CActiveLCD = _Clcd1;
   if (_iLine > 2)
   {
      //DisplayLine is higher than 2, this means we need to update the 2nd LCD
      CActiveLCD = _Clcd2;
      _iLine = _iLine - 2; //Convert line number to correct line number for 2nd display
   }
   else
   {
      CActiveLCD = _Clcd1;
   }
   /* Since this function is user friendly and uses diplay lines 1-4, we have to convert this number
      to a real display line (0-1) by substracting 1 again
   */
   _iLine = _iLine - 1;

   //Check how long strMessage is:
   int iMessageLength = _strText.length();
   if (iMessageLength > _iFieldLength)
   {
      //Message is too long, make it scroll!
      int iExtraChars = iMessageLength - (_iFieldLength -1);
      for (int i = 0; i < iExtraChars; i++)
      {
         String strMessageSubString = _strText.substring(i, i + _iFieldLength);
         CActiveLCD->setCursor(_iPosition, _iLine);
         CActiveLCD->print(strMessageSubString);
      }
      return;
   }
   else if (iMessageLength < _iFieldLength)
   {
      //Message is too short, we need to pad it
      //First find missing characters
      int iMissingChars = _iFieldLength - iMessageLength;
      for (int i = 0; i < iMissingChars; i++)
      {
         _strText = String(_strText + " ");
      }
   }
   CActiveLCD->setCursor(_iPosition, _iLine);
   CActiveLCD->print(_strText);
}

LCDControllerClass LCDController;