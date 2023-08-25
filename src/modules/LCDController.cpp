// file:	LCDController.cpp
//
// summary:	Implements the LCD controller class
// Copyright (C) 2019 Alex Goris
// This file is part of FlyballETS-Software
// FlyballETS-Software is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.If not, see <http://www.gnu.org/licenses/>

#include "config.h"
#include "LCDController.h"

/// <summary>
///   Initialises this object.
/// </summary>
///
/// <param name="Clcd1">   [in,out] Pointer to first LCD object. </param>
/// <param name="Clcd2">   [in,out] Pointer to second LCD object. </param>
void LCDControllerClass::init(LiquidCrystal *Clcd1, LiquidCrystal *Clcd2)
{
   _Clcd1 = Clcd1;
   _Clcd1->begin(40, 2);
   _Clcd1->clear();

   _Clcd2 = Clcd2;
   _Clcd2->begin(40, 2);
   _Clcd2->clear();

   // Put initial text on screen
   //                                  1         2         3
   // LCD layout:            0123456789012345678901234567890123456789
   _UpdateLCD(2, 0, String("   Flyball Electronic Training System   "), 40);
   _UpdateLCD(3, 0, String("            ver.          rev.S         "), 40);
   _UpdateLCD(3, 17, String(FW_VER), 8);
   vTaskDelay(1500);
   _UpdateLCD(1, 0, String("1:   0.000  +  0.000    |   WELCOME  000"), 40);
   _UpdateLCD(2, 0, String("2:   0.000  +  0.000    | Team:    0.000"), 40);
   _UpdateLCD(3, 0, String("3:   0.000  +  0.000    |   CT:    0.000"), 40);
   _UpdateLCD(4, 0, String("4:   0.000  +  0.000    |  100% W G sd >"), 40);

   _SlcdfieldFields[D1Time] = {true, 1, 3, 7, String("  0.000")};
   _SlcdfieldFields[D2Time] = {true, 2, 3, 7, String("  0.000")};
   _SlcdfieldFields[D3Time] = {true, 3, 3, 7, String("  0.000")};
   _SlcdfieldFields[D4Time] = {true, 4, 3, 7, String("  0.000")};
   _SlcdfieldFields[D1CrossTime] = {true, 1, 12, 8, String("        ")};
   _SlcdfieldFields[D2CrossTime] = {true, 2, 12, 8, String("        ")};
   _SlcdfieldFields[D3CrossTime] = {true, 3, 12, 8, String("        ")};
   _SlcdfieldFields[D4CrossTime] = {true, 4, 12, 8, String("        ")};
   _SlcdfieldFields[D1RerunInfo] = {true, 1, 22, 2, String("  ")};
   _SlcdfieldFields[D2RerunInfo] = {true, 2, 22, 2, String("  ")};
   _SlcdfieldFields[D3RerunInfo] = {true, 3, 22, 2, String("  ")};
   _SlcdfieldFields[D4RerunInfo] = {true, 4, 22, 2, String("  ")};
   _SlcdfieldFields[TeamTime] = {true, 2, 33, 7, String("  0.000")};
   _SlcdfieldFields[CleanTime] = {true, 3, 33, 7, String("  0.000")};
   _SlcdfieldFields[RaceState] = {true, 1, 28, 7, String(" READY ")};
   _SlcdfieldFields[RaceID] = {true, 1, 37, 3, String("  1")};
#if BatteryCalibration
   _SlcdfieldFields[BattLevel] = {true, 4, 27, 4, String("0000")};
#else
   _SlcdfieldFields[BattLevel] = {true, 4, 27, 3, String("UNK")};
#endif
   _SlcdfieldFields[WifiState] = {true, 4, 32, 1, String("W")};
   _SlcdfieldFields[GpsState] = {true, 4, 34, 1, String(" ")};
   _SlcdfieldFields[SDcardState] = {true, 4, 36, 2, String("  ")};
   _SlcdfieldFields[BoxDirection] = {true, 4, 39, 1, String(">")};
}

/// <summary>
///   Main entry-point for this application, this function should be called in every main loop
///   cycle. It will check whether the last time we updated the LCD screen is more than the given
///   timeout, and if yes, it will update the LCD screen with the latest data.
/// </summary>
void LCDControllerClass::Main()
{
   // This is the main loop which handles LCD updates
   if (((millis() - _ulLastLCDUpdate) > iLCDUpdateInterval) && !bExecuteLCDUpdate)
      bExecuteLCDUpdate = true;

   if (bExecuteLCDUpdate)
   {
      // Handle LCD data updates
      _HandleLCDUpdates();
      uint8_t iLoopCounter = 0;
      for (const SLCDField &lcdField : _SlcdfieldFields)
      {
         //log_v("lcdField: %i, UpdateFlag: %i", iLoopCounter, lcdField.bUpdateFlag);
         // Trigger update only for fields with UpdateFlag set
         if (lcdField.bUpdateFlag)
         {
            _UpdateLCD(lcdField.iLine, lcdField.iStartingPosition, lcdField.strText, lcdField.iFieldLength);
            _SlcdfieldFields[iLoopCounter].bUpdateFlag = false;
         }
         iLoopCounter++;
      }
      _ulLastLCDUpdate = millis();
      bExecuteLCDUpdate = false;
   }
}

/// <summary>
///   Updates a given pre-defined field on the LCD, with the new value.
/// </summary>
///
/// <param name="lcdfieldField"> The lcdfield identifier for which field should be updated </param>
/// <param name="strNewValue">   The new value. </param>
void LCDControllerClass::UpdateField(LCDFields lcdfieldField, String strNewValue)
{
   if (_SlcdfieldFields[lcdfieldField].iFieldLength < strNewValue.length())
   {
      // The new value will not fit into the new field!
      log_e("[LCD Controller] Field (%i) has %i characters. Received string '%s' has %i characters",
            lcdfieldField, _SlcdfieldFields[lcdfieldField].iFieldLength, strNewValue.c_str(), strNewValue.length());
      return;
   }
   _SlcdfieldFields[lcdfieldField].strText = strNewValue;
   _SlcdfieldFields[lcdfieldField].bUpdateFlag = true;
}

/// <summary>
///   Updates the LCD. This function will update the correct portion of the LCD, based on which line and position we want to update.
/// </summary>
///
/// <param name="iLine">         Zero-based index of the line (1-4). </param>
/// <param name="iPosition">     Zero-based index of the starting position of the text which should be put on the screen. </param>
/// <param name="strText">       The text which should be put at the given position. </param>
/// <param name="iFieldLength">  Length of the field, if the given text is longer than this value, the text will be made scrolling within the given field length. </param>
void LCDControllerClass::_UpdateLCD(int iLine, int iPosition, String strText, int iFieldLength)
{
   LiquidCrystal *CActiveLCD = 0;
   if (iLine > 2)
   {
      // DisplayLine is higher than 2, this means we need to update the 2nd LCD
      CActiveLCD = _Clcd2;
      iLine = iLine - 2; // Convert line number to correct line number for 2nd display
   }
   else
   {
      CActiveLCD = _Clcd1;
   }
   /* Since this function is user friendly and uses diplay lines 1-4, we have to convert this number
      to a real display line (0-1) by substracting 1 again
   */
   iLine = iLine - 1;

   // Check how long strMessage is:
   int iMessageLength = strText.length();
   if (iMessageLength > iFieldLength)
   {
      // Message is too long, make it scroll!
      int iExtraChars = iMessageLength - (iFieldLength - 1);
      for (int i = 0; i < iExtraChars; i++)
      {
         String strMessageSubString = strText.substring(i, i + iFieldLength);
         CActiveLCD->setCursor(iPosition, iLine);
         CActiveLCD->print(strMessageSubString);
      }
      return;
   }
   else if (iMessageLength < iFieldLength)
   {
      // Message is too short, we need to pad it
      // First find missing characters
      int iMissingChars = iFieldLength - iMessageLength;
      for (int i = 0; i < iMissingChars; i++)
      {
         strText = String(strText + " ");
      }
   }
   CActiveLCD->setCursor(iPosition, iLine);
   CActiveLCD->print(strText);
}

void LCDControllerClass::_HandleLCDUpdates()
{
   // Update team clean time
   if (bUpdateThisLCDField[CleanTime] || bUpdateTimerLCDdata)
   {
      String sReadCleanTime = RaceHandler.GetCleanTime();
      UpdateField(CleanTime, sReadCleanTime);
      bUpdateThisLCDField[CleanTime] = false;
      log_v("LCD CleanTime updated with string '%s'", sReadCleanTime.c_str());
   }
   // Update team time
   if (bUpdateThisLCDField[TeamTime] || bUpdateTimerLCDdata)
   {
      String sReadRaceTime = RaceHandler.GetRaceTime();
      UpdateField(TeamTime, sReadRaceTime);
      bUpdateThisLCDField[TeamTime] = false;
      log_v("LCD TeamTime updated with string '%s'", sReadRaceTime.c_str());
   }
  // Update dogs times, crossing/entry times and re-run info
   for (int i = 0; i < RaceHandler.iNumberOfRacingDogs; i++)
   {
      int iRunNumber;
      if (RaceHandler.iDogRunCounters[i] > 0 && !RaceHandler.bRerunsOff)
         iRunNumber = RaceHandler.SelectRunNumber(i);
      else
         iRunNumber = 0;
      
      if (bUpdateThisLCDField[i] || bUpdateTimerLCDdata || (RaceHandler.iDogRunCounters[i] > 0 && !RaceHandler.bRerunsOff))
      {
         String sReadDogTime = RaceHandler.GetDogTime(i, iRunNumber);
         UpdateField(LCDFields(i), sReadDogTime);
         bUpdateThisLCDField[i] = false;
         log_v("LCD Dog %i time updated with string '%s'. Run number: %i. Dog counters: %i", i + 1, sReadDogTime.c_str(), iRunNumber, RaceHandler.iDogRunCounters[i]);
      }
      if (bUpdateThisLCDField[i + 4] || bUpdateTimerLCDdata || (RaceHandler.iDogRunCounters[i] > 0 && !RaceHandler.bRerunsOff))
      {
         String sReadCrossingTime = RaceHandler.GetCrossingTime(i, iRunNumber);
         UpdateField(LCDFields(i + 4), sReadCrossingTime);
         bUpdateThisLCDField[i + 4] = false;
         log_v("LCD Dog %i Crossing time updated with string '%s'. Run number: %i. Dog counters: %i", i + 1, sReadCrossingTime.c_str(), iRunNumber, RaceHandler.iDogRunCounters[i]);
      }
      if (bUpdateThisLCDField[i + 8] || bUpdateTimerLCDdata || (RaceHandler.iDogRunCounters[i] > 0 && !RaceHandler.bRerunsOff))
      {
         String sReadRerunInfo = RaceHandler.GetRerunInfo(i, iRunNumber);
         UpdateField(LCDFields(i + 8), sReadRerunInfo);
         bUpdateThisLCDField[i + 8] = false;
         log_v("LCD Dog %i Re-run Info updated with string '%s'. Run number: %i. Dog counters: %i", i + 1, sReadRerunInfo.c_str(), iRunNumber, RaceHandler.iDogRunCounters[i]);
      }
   }
   bUpdateTimerLCDdata = false;

   if (bUpdateNonTimerLCDdata)
   {
      bUpdateNonTimerLCDdata = false;
      for (uint8_t i = 14; i < 21; i++)
         _SlcdfieldFields[i].bUpdateFlag = true;
   }

   // Update battery percentage
   if ((millis() < 2000 || ((millis() - llLastBatteryLCDupdate) > 30000)) //
      && (RaceHandler.RaceState == RaceHandler.STOPPED || RaceHandler.RaceState == RaceHandler.RESET))
   {
      uint16_t iBatteryVoltage = BatterySensor.GetBatteryVoltage();
      uint16_t iBatteryPercentage = BatterySensor.GetBatteryPercentage();
      String sBatteryPercentage;
      if (iBatteryPercentage == 9999)
      {
         sBatteryPercentage = "!!!";
         UpdateField(BattLevel, sBatteryPercentage);
         LightsController.bExecuteResetLights = true;
         vTaskDelay(3000);
         esp_deep_sleep_start();
      }
      else if (iBatteryPercentage == 9911)
         sBatteryPercentage = "USB";
      else if (iBatteryPercentage == 0)
         sBatteryPercentage = "LOW";
      else
         sBatteryPercentage = String(iBatteryPercentage);

      while (sBatteryPercentage.length() < 3)
         sBatteryPercentage = " " + sBatteryPercentage;
      UpdateField(BattLevel, sBatteryPercentage);
      // log_d("Battery: analog: %i ,voltage: %i, level: %i%%", BatterySensor.GetLastAnalogRead(), iBatteryVoltage, iBatteryPercentage);
      llLastBatteryLCDupdate = millis();
   }
}

void LCDControllerClass::reInit()
{
   _UpdateLCD(1, 0, String("                                        "), 40);
   _UpdateLCD(2, 0, String("                 mode:                  "), 40);
   _UpdateLCD(3, 0, String("                                        "), 40);
   _UpdateLCD(4, 0, String("                                        "), 40);
   if (!LightsController.bModeNAFA)
   {
      _UpdateLCD(3, 0, String("                  FCI                   "), 40);
      vTaskDelay(500);
   }
   else
   {
      _UpdateLCD(3, 0, String("                 NAFA                   "), 40);
      vTaskDelay(500);
   }
   _UpdateLCD(1, 0, String("1:                      |               "), 40);
   _UpdateLCD(2, 0, String("2:                      | Team:         "), 40);
   _UpdateLCD(3, 0, String("3:                      |   CT:         "), 40);
   _UpdateLCD(4, 0, String("4:                      |     %         "), 40);
   bUpdateNonTimerLCDdata = true;
   bUpdateTimerLCDdata = true;
   bExecuteLCDUpdate = true;
}

void LCDControllerClass::UpdateNumberOfDogsOnLCD(uint8_t iNumberOfDogs)
{
   if (iNumberOfDogs == 3)
   {
      _UpdateLCD(1, 0, String("1:                      |               "), 40);
      _UpdateLCD(2, 0, String("2:                      | Team:         "), 40);
      _UpdateLCD(3, 0, String("3:                      |   CT:         "), 40);
      _UpdateLCD(4, 0, String("                        |     %         "), 40);
      _SlcdfieldFields[D4Time].strText = "       ";
      _SlcdfieldFields[D4RerunInfo].strText = "  ";
   }
   else if (iNumberOfDogs == 2)
   {
      _UpdateLCD(1, 0, String("1:                      |               "), 40);
      _UpdateLCD(2, 0, String("2:                      | Team:         "), 40);
      _UpdateLCD(3, 0, String("                        |   CT:         "), 40);
      _UpdateLCD(4, 0, String("                        |     %         "), 40);
      _SlcdfieldFields[D3Time].strText = "       ";
      _SlcdfieldFields[D4Time].strText = "       ";
      _SlcdfieldFields[D3RerunInfo].strText = "  ";
      _SlcdfieldFields[D4RerunInfo].strText = "  ";
   }
   else if (iNumberOfDogs == 1)
   {
      _UpdateLCD(1, 0, String("1:                      |               "), 40);
      _UpdateLCD(2, 0, String("                        | Team:         "), 40);
      _UpdateLCD(3, 0, String("                        |   CT:         "), 40);
      _UpdateLCD(4, 0, String("                        |     %         "), 40);
      _SlcdfieldFields[D2Time].strText = "       ";
      _SlcdfieldFields[D3Time].strText = "       ";
      _SlcdfieldFields[D4Time].strText = "       ";
      _SlcdfieldFields[D2RerunInfo].strText = "  ";
      _SlcdfieldFields[D3RerunInfo].strText = "  ";
      _SlcdfieldFields[D4RerunInfo].strText = "  ";
   }
   else
   {
      _UpdateLCD(1, 0, String("1:                      |               "), 40);
      _UpdateLCD(2, 0, String("2:                      | Team:         "), 40);
      _UpdateLCD(3, 0, String("3:                      |   CT:         "), 40);
      _UpdateLCD(4, 0, String("4:                      |     %         "), 40);
   }
   bUpdateNonTimerLCDdata = true;
   bUpdateTimerLCDdata = true;
   bExecuteLCDUpdate = true;
}

void LCDControllerClass::FirmwareUpdateInit()
{
   _UpdateLCD(1, 0, String("                                        "), 40);
   _UpdateLCD(2, 0, String("            Firmware update             "), 40);
   _UpdateLCD(3, 0, String("                                        "), 40);
   _UpdateLCD(4, 0, String("               Progress:     0%         "), 40);
}

void LCDControllerClass::FirmwareUpdateProgress(String strNewValue)
{
   _SlcdfieldFields[BattLevel].strText = strNewValue;
   _UpdateLCD(_SlcdfieldFields[BattLevel].iLine, _SlcdfieldFields[BattLevel].iStartingPosition, _SlcdfieldFields[BattLevel].strText, _SlcdfieldFields[BattLevel].iFieldLength);
}

void LCDControllerClass::FirmwareUpdateSuccess()
{
   _UpdateLCD(4, 0, String("              OTA Success               "), 40);
}

void LCDControllerClass::FirmwareUpdateError()
{
   _UpdateLCD(4, 0, String("              OTA Error                 "), 40);
}

/// <summary>
///   The LCD controller.
/// </summary>
LCDControllerClass LCDController;