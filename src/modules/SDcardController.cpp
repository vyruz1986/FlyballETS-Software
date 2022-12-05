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
#include "config.h"

/// <summary>
///   Initialises SD card.
/// </summary>
void SDcardControllerClass::init()
{
   if (SettingsManager.getSetting("CommaInCsv").equals("1"))
   {
      _bCommaInCsv = true;
      log_i("CSV decimal separator from settings: comma ','");
   }
   else
      log_i("CSV decimal separator from settings: period '.'");

   if (!SD_MMC.begin("/SDCARD", true, false, SDMMC_FREQ_DEFAULT))
   {
      log_e("Card Mount Failed");
      return;
   }
   else
   {
      uint8_t cardType = SD_MMC.cardType();
      if (cardType == CARD_NONE)
      {
         log_e("No SD_MMC card attached");
         return;
      }
      if (cardType == CARD_MMC)
      {
         log_i("SD_MMC Card Type: MMC");
         bSDCardDetected = true;
      }
      else if (cardType == CARD_SD)
      {
         log_i("SD_MMC Card Type: SDSC");
         bSDCardDetected = true;
      }
      else if (cardType == CARD_SDHC)
      {
         log_i("SD_MMC Card Type: SDHC");
         bSDCardDetected = true;
      }
      else
         log_i("UNKNOWN");

      if (bSDCardDetected)
      {
         LCDController.UpdateField(LCDController.SDcardState, "sd");
         log_i("SD_MMC Card Size: %lluMB", SD_MMC.cardSize() / (1024 * 1024));
         log_i("Free space: %lluMB", (SD_MMC.totalBytes() - SD_MMC.usedBytes()) / (1024 * 1024));
         if (!SD_MMC.exists("/SENSORS_DATA"))
         {
            createDir(SD_MMC, "/SENSORS_DATA");
         }
         // listDir(SD_MMC, "/", 1);

         // Check last tag value and increase it by one or create new tag.txt file with initial value 1
         File tagfile = SD_MMC.open("/tag.txt");
         if (!tagfile)
         {
            delay(50);
            writeFile(SD_MMC, "/tag.txt", "0\r\n");
            tagfile.close();
            iTagValue = 1;
            sTagValue = "0001";
            log_i("New tag.txt file created.");
         }
         else
         {
            uint16_t iOldTagValue = tagfile.parseInt();
            tagfile.close();
            iTagValue = iOldTagValue + 1;
            sTagValue = String(iTagValue);
            while (sTagValue.length() < 4)
            {
               sTagValue = "0" + sTagValue;
            }
            log_i("New tag value: %i. Tag string: %s", iTagValue, sTagValue);
         }
      }
   }
}

/// <summary>
///   Saving race data to file. Function called after race has been stopped.
/// </summary>
void SDcardControllerClass::SaveRaceDataToFile()
{
   File raceDataFile;
   String sDate = GPSHandler.GetDate();
   if (RaceHandler.iCurrentRaceId == 0)
   {
      UpdateTagFile();
      raceDataFileName = "/" + sTagValue + "_ETS_" + sDate + ".csv";
      writeFile(SD_MMC, raceDataFileName.c_str(),
                "sep=;\nTag;Race ID;Date;Race timestamp;Number of racing dogs;Re-runs turned OFF?;Dog 1 time;Dog 1 starting;Dog 1 re-run time;Dog 1 re-run crossing;Dog 1 2nd re-run time;Dog 1 2nd re-run crossing;Dog 2 time;Dog 2 crossing;Dog 2 re-run time;Dog 2 re-run crossing;Dog 2 2nd re-run time;Dog 2 2nd re-run crossing;Dog 3 time;Dog 3 crossing;Dog 3 re-run time;Dog 3 re-run crossing;Dog 3 2nd re-run time;Dog 3 2nd re-run crossing;Dog 4 time;Dog 4 crossing;Dog 4 re-run time;Dog 4 re-run crossing;Dog 4 2nd re-run time;Dog 4 2nd re-run crossing;Team time; Clean time;Comments\n");
   }
   raceDataFile = SD_MMC.open(raceDataFileName.c_str(), FILE_APPEND);
   if (raceDataFile)
   {
      raceDataFile.print(iTagValue);
      raceDataFile.print(";");
      raceDataFile.print(RaceHandler.iCurrentRaceId + 1);
      raceDataFile.print(";");
      raceDataFile.print(sDate);
      raceDataFile.print(";");
      raceDataFile.print(RaceHandler.cRaceStartTimestamp);
      raceDataFile.print(";");
      raceDataFile.print(RaceHandler.iNumberOfRacingDogs);
      raceDataFile.print(";");
      if (RaceHandler.bRerunsOff)
         raceDataFile.print("yes");
      else
         raceDataFile.print("no");
      raceDataFile.print(";");
      if (!_bCommaInCsv)
      {
         for (uint8_t i = 0; i < 4; i++)
         {
            for (uint8_t i2 = 0; i2 < 3; i2++)
            {
               raceDataFile.print(RaceHandler.GetStoredDogTimes(i, i2));
               raceDataFile.print(";");
               raceDataFile.print(RaceHandler.TransformCrossingTime(i, i2, true));
               raceDataFile.print(";");
            }
         }
         raceDataFile.print(RaceHandler.GetRaceTime());
         raceDataFile.print(";");
         raceDataFile.print(RaceHandler.GetCleanTime());
      }
      else
      {
         String sConvert;
         for (uint8_t i = 0; i < 4; i++)
         {
            for (uint8_t i2 = 0; i2 < 3; i2++)
            {
               sConvert = RaceHandler.GetStoredDogTimes(i, i2);
               sConvert.replace(".", ",");
               raceDataFile.print(sConvert);
               raceDataFile.print(";");
               sConvert = RaceHandler.TransformCrossingTime(i, i2, true);
               sConvert.replace(".", ",");
               raceDataFile.print(sConvert);
               raceDataFile.print(";");
            }
         }
         sConvert = RaceHandler.GetRaceTime();
         sConvert.replace(".", ",");
         raceDataFile.print(sConvert);
         sConvert = RaceHandler.GetCleanTime();
         sConvert.replace(".", ",");
         raceDataFile.print(";");
         raceDataFile.print(sConvert);
      }
      raceDataFile.println(";");
      raceDataFile.close();
   }
}

/// <summary>
///   Updating tag file. Function should be call while writing first race data after turning on ETS
/// </summary>
void SDcardControllerClass::UpdateTagFile()
{
   deleteFile(SD_MMC, "/tag.txt");
   String writeTagValue = String(iTagValue) + "\r\n";
   writeFile(SD_MMC, "/tag.txt", writeTagValue.c_str());
   log_i("Tag.txt file updated with value %i.", iTagValue);
}

/// <summary>
///   Monitoring of SD card detect pin to trigger ESP reboot if card has been inserted or removed.
/// </summary>
void SDcardControllerClass::CheckSDcardSlot(uint8_t iSDdetectPin)
{
   _iSDdetectPinStatus = digitalRead(iSDdetectPin);
   if (bSDCardDetected && _iSDdetectPinStatus == HIGH)
   {
      log_w("SD Card plugged out - rebooting!");
      delay(500);
      ESP.restart();
   }
   if (!bSDCardDetected && _iSDdetectPinStatus == LOW)
   {
      log_w("SD Card plugged in - rebooting!");
      delay(500);
      ESP.restart();
   }
}

/// <summary>
///   Toggles decimal separator between ',' and '.'
/// </summary>
void SDcardControllerClass::ToggleDecimalSeparator()
{
   _bCommaInCsv = !_bCommaInCsv;
   SettingsManager.setSetting("CommaInCsv", String(_bCommaInCsv));
   if (_bCommaInCsv)
      log_i("Decimal separator set to: comma ','");
   else
      log_i("Decimal separator set to: period '.'");
#ifdef WiFiON
   WebHandler.bSendRaceData = true;
#endif
}

/// <summary>
///   File and directory operating metods.
/// </summary>
void SDcardControllerClass::listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
   log_i("Listing directory: %s", dirname);

   File root = fs.open(dirname);
   if (!root)
   {
      log_e("Failed to open directory");
      return;
   }
   if (!root.isDirectory())
   {
      log_e("Not a directory");
      return;
   }

   File file = root.openNextFile();
   while (file)
   {
      if (file.isDirectory())
      {
         Serial.print("  DIR : ");
         Serial.println(file.name());
         if (levels)
         {
            listDir(fs, file.name(), levels - 1);
         }
      }
      else
      {
         Serial.print("  FILE: ");
         Serial.print(file.name());
         Serial.print("  SIZE: ");
         Serial.println(file.size());
      }
      file = root.openNextFile();
   }
}

void SDcardControllerClass::createDir(fs::FS &fs, const char *path)
{
   log_i("Creating Dir: %s", path);
   if (fs.mkdir(path))
      log_i("Dir created");
   else
      log_e("mkdir failed");
}

void SDcardControllerClass::removeDir(fs::FS &fs, const char *path)
{
   log_i("Removing Dir: %s", path);
   if (fs.rmdir(path))
      log_i("Dir removed");
   else
      log_e("rmdir failed");
}

void SDcardControllerClass::readFile(fs::FS &fs, const char *path)
{
   log_i("Reading file: %s", path);

   File file = fs.open(path);
   if (!file)
   {
      log_e("Failed to open file for reading");
      return;
   }

   log_i("Read from file: ");
   while (file.available())
      Serial.write(file.read());
}

void SDcardControllerClass::writeFile(fs::FS &fs, const char *path, const char *message)
{
   log_i("Writing file: %s", path);

   File file = fs.open(path, FILE_WRITE);
   if (!file)
   {
      log_e("Failed to open file for writing");
      return;
   }
   if (file.print(message))
      log_i("File written");
   else
      log_e("Write failed");
}

void SDcardControllerClass::appendFile(fs::FS &fs, const char *path, const char *message)
{
   log_i("Appending to file: %s", path);

   File file = fs.open(path, FILE_APPEND);
   if (!file)
   {
      log_e("Failed to open file for appending");
      return;
   }
   if (file.print(message))
      log_i("Message appended");
   else
      log_e("Append failed");
}

void SDcardControllerClass::renameFile(fs::FS &fs, const char *path1, const char *path2)
{
   log_i("Renaming file %s to %s", path1, path2);
   if (fs.rename(path1, path2))
      log_i("File renamed");
   else
      log_e("Rename failed");
}

void SDcardControllerClass::deleteFile(fs::FS &fs, const char *path)
{
   log_i("Deleting file: %s", path);
   if (fs.remove(path))
      log_i("File deleted");
   else
      log_e("Delete failed");
}

void SDcardControllerClass::testFileIO(fs::FS &fs, const char *path)
{
   File file = fs.open(path);
   static uint8_t buf[512];
   size_t len = 0;
   uint64_t start = millis();
   uint64_t end = start;
   if (file)
   {
      len = file.size();
      size_t flen = len;
      start = millis();
      while (len)
      {
         size_t toRead = len;
         if (toRead > 512)
            toRead = 512;
         file.read(buf, toRead);
         len -= toRead;
      }
      end = millis() - start;
      log_i("%u bytes read for %u ms", flen, end);
      file.close();
   }
   else
      log_e("Failed to open file for reading");

   file = fs.open(path, FILE_WRITE);
   if (!file)
   {
      log_e("Failed to open file for writing");
      return;
   }

   size_t i;
   start = millis();
   for (i = 0; i < 2048; i++)
      file.write(buf, 512);
   end = millis() - start;
   log_i("%u bytes written for %u ms", 2048 * 512, end);
   file.close();
}

SDcardControllerClass SDcardController;