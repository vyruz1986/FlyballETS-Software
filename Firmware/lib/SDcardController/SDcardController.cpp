// file:	SDCard.cpp
//
// summary:	Implements the SD Card controller class
// Copyright (C) 2021 Simon Giemza
// This file is part of FlyballETS-Software by simonttp78 that is forked from FlyballETS-Software by Alex Gore 
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

#include "SDcardController.h"

/// <summary>
///   Initialises SD card.
/// </summary>
void SDcardControllerClass::init()
{
   if (!SD_MMC.begin("/sdcard", true))
   {
      Serial.println("Card Mount Failed");
      return;
   }
   else
   {
      uint8_t cardType = SD_MMC.cardType();

      if (cardType == CARD_NONE)
      {
         Serial.println("No SD_MMC card attached");
         return;
      }
      Serial.print("\nSD_MMC Card Type: ");
      if (cardType == CARD_MMC)
      {
         Serial.println("MMC");
         _bSDCardDetected = true;
      }
      else if (cardType == CARD_SD)
      {
         Serial.println("SDSC");
         _bSDCardDetected = true;
      }
      else if (cardType == CARD_SDHC)
      {
         Serial.println("SDHC");
         _bSDCardDetected = true;
      }
      else
      {
         Serial.println("UNKNOWN");
      }
      uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
      Serial.printf("SD_MMC Card Size: %lluMB\n\n", cardSize);
   }
}

/// <summary>
///   Monitoring of SD card detect pin to trigger ESP reboot if card has been inserted or removed.
/// </summary>
void SDcardControllerClass::CheckSDcardSlot(uint8_t iSDdetectPin)
{
   _iSDdetectPinStatus = digitalRead(iSDdetectPin);
   if (_bSDCardDetected && _iSDdetectPinStatus == HIGH)
   {
      Serial.println("\nSD Card plugged out - rebooting!\n");
      delay(500);
      ESP.restart();
   }
   if (!_bSDCardDetected && _iSDdetectPinStatus == LOW)
   {
      Serial.println("\nSD Card plugged in - rebooting!\n");
      delay(500);
      ESP.restart();
   }
}

SDcardControllerClass SDcardController;