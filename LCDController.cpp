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
   _UpdateLCD(1, 0, String("D1: 000,000s CR: 00,000s|STOP   B:    0%"), 40);
   _UpdateLCD(2, 0, String("D2: 000,000s CR: 00,000s| Team: 000,000s"), 40);
   _UpdateLCD(3, 0, String("D3: 000,000s CR: 00,000s|   CR: 000,000s"), 40);
   _UpdateLCD(4, 0, String("D4: 000,000s CR: 00,000s|"), 40);

   _lcdfieldFields[D1Time] = { 1, 7, 4, String("000,000") };
   _lcdfieldFields[D2Time] = { 2, 7, 4, String("000,000") };
   _lcdfieldFields[D3Time] = { 3, 7, 4, String("000,000") };
   _lcdfieldFields[D4Time] = { 4, 7, 4, String("000,000") };
   _lcdfieldFields[D1CrossTime] = { 1, 6, 17, String("00,000") };
   _lcdfieldFields[D2CrossTime] = { 2, 6, 17, String("00,000") };
   _lcdfieldFields[D3CrossTime] = { 3, 6, 17, String("00,000") };
   _lcdfieldFields[D4CrossTime] = { 4, 6, 17, String("00,000") };
   _lcdfieldFields[BattLevel] = { 1, 3, 36, String("0") };
   _lcdfieldFields[RaceState] = { 1, 5, 25, String("STOP") };
   _lcdfieldFields[TeamTime] = { 2, 7, 32, String("000,000") };
   _lcdfieldFields[TotalCrossTime] = { 3, 7, 32, String("000,000") };
}

void LCDControllerClass::Main()
{
   //This is the main loop which handles LCD updates
   if ((millis() - _lLastLCDUpdate) > _iLCDUpdateInterval)
   {  
      
      for (const SLCDField& lcdField : _lcdfieldFields)
      {
         _UpdateLCD(lcdField.iLine, lcdField.iStartingPosition, lcdField.strText, lcdField.iFieldLength);
      }
      
      _lLastLCDUpdate = millis();
   }


}

void LCDControllerClass::_UpdateLCD(int _iLine, int _iPosition, String _strText, int _iFieldLength)
{
   //LiquidCrystal* CActiveLCD;
   LiquidCrystal *CActiveLCD = _Clcd1;
   if (_iLine > 2)
   {
      //DisplayLine is higher than 2, this means we need to update the 2nd LCD
      //LiquidCrystal** CActiveLCD = &_Clcd2;
      _iLine = _iLine - 2; //Convert line number to correct line number for 2nd display
   }
   else
   {
      //LiquidCrystal** CActiveLCD = &_Clcd1;
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
      int iExtraChars = iMessageLength - 15;
      for (int i = 0; i < iExtraChars; i++)
      {
         String strMessageSubString = _strText.substring(i, i + 16);
         CActiveLCD->setCursor(_iPosition, _iLine);
         CActiveLCD->print(strMessageSubString);
      }
      return;
   }
   else if (iMessageLength < _iFieldLength)
   {
      //Message is too short, we need to pad it
      //First find missing characters
      int iMissingChars = 16 - iMessageLength;
      for (int i = 0; i < iMissingChars; i++)
      {
         _strText = String(_strText + " ");
      }
   }
   CActiveLCD->setCursor(_iPosition, _iLine);
   CActiveLCD->print(_strText);
}

LCDControllerClass LCDController;