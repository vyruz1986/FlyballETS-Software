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
#include "LCDController.h"

/// <summary>
///   Initialises SD card.
/// </summary>
void SDcardControllerClass::init()
{
   if (!SD_MMC.begin("/SDCARD", true))
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
         bSDCardDetected = true;
      }
      else if (cardType == CARD_SD)
      {
         Serial.println("SDSC");
         bSDCardDetected = true;
      }
      else if (cardType == CARD_SDHC)
      {
         Serial.println("SDHC");
         bSDCardDetected = true;
      }
      else
      {
         Serial.println("UNKNOWN");
      }
      if (bSDCardDetected)
      {
         LCDController.UpdateField(LCDController.SDcardState, "sd");
         Serial.printf("SD_MMC Card Size: %lluMB\n\n", SD_MMC.cardSize() / (1024 * 1024));
         Serial.printf("Free space: %lluMB\n\n", (SD_MMC.totalBytes() - SD_MMC.usedBytes()) / (1024 * 1024));
         if (!SD_MMC.exists("/SENSORS_DATA"))
         {
            createDir(SD_MMC, "/SENSORS_DATA");
         }
         //listDir(SD_MMC, "/", 1);
         
         // Check last tag value and increase it by one or create new tag.txt file with initial value 1
         File tagfile = SD_MMC.open("/tag.txt");
         if(!tagfile) {
            delay(50);
            writeFile(SD_MMC, "/tag.txt", "1\r\n");
            tagfile.close();
         }
         else
         {
            uint16_t oldTagValue = tagfile.parseInt();
            tagfile.close();
            iTagValue = oldTagValue + 1;
            sTagValue = String(iTagValue);
            ESP_LOGI(__FILE__, "Previous Tag value: %i. Updating tag.txt file...", oldTagValue);
            deleteFile(SD_MMC, "/tag.txt");
            String writeTagValue = sTagValue + "\r\n";
            writeFile(SD_MMC, "/tag.txt", writeTagValue.c_str());
            while (sTagValue.length() < 4)
            {
               sTagValue = "0" + sTagValue;
            }
            ESP_LOGI(__FILE__, "New tag.txt file value: %i. Tag string: %s", iTagValue, sTagValue);
         }
      }
   }
}

/// <summary>
///   Monitoring of SD card detect pin to trigger ESP reboot if card has been inserted or removed.
/// </summary>
void SDcardControllerClass::CheckSDcardSlot(uint8_t iSDdetectPin)
{
   _iSDdetectPinStatus = digitalRead(iSDdetectPin);
   if (bSDCardDetected && _iSDdetectPinStatus == HIGH)
   {
      Serial.println("\nSD Card plugged out - rebooting!\n");
      delay(500);
      ESP.restart();
   }
   if (!bSDCardDetected && _iSDdetectPinStatus == LOW)
   {
      Serial.println("\nSD Card plugged in - rebooting!\n");
      delay(500);
      ESP.restart();
   }
}

/// <summary>
///   File and directory operating metods.
/// </summary>
void SDcardControllerClass::listDir(fs::FS &fs, const char * dirname, uint8_t levels){
   Serial.printf("Listing directory: %s\n", dirname);

   File root = fs.open(dirname);
   if(!root){
      Serial.println("Failed to open directory");
      return;
   }
   if(!root.isDirectory()){
      Serial.println("Not a directory");
      return;
   }

   File file = root.openNextFile();
   while(file){
      if(file.isDirectory()){
         Serial.print("  DIR : ");
         Serial.println(file.name());
         if(levels){
               listDir(fs, file.name(), levels -1);
         }
      } else {
         Serial.print("  FILE: ");
         Serial.print(file.name());
         Serial.print("  SIZE: ");
         Serial.println(file.size());
      }
      file = root.openNextFile();
   }
}

void SDcardControllerClass::createDir(fs::FS &fs, const char * path){
   Serial.printf("Creating Dir: %s\n", path);
   if(fs.mkdir(path)){
      Serial.println("Dir created");
   } else {
      Serial.println("mkdir failed");
   }
}

void SDcardControllerClass::removeDir(fs::FS &fs, const char * path){
   Serial.printf("Removing Dir: %s\n", path);
   if(fs.rmdir(path)){
      Serial.println("Dir removed");
   } else {
      Serial.println("rmdir failed");
   }
}

void SDcardControllerClass::readFile(fs::FS &fs, const char * path){
   Serial.printf("Reading file: %s\n", path);

   File file = fs.open(path);
   if(!file){
      Serial.println("Failed to open file for reading");
      return;
   }

   Serial.print("Read from file: ");
   while(file.available()){
      Serial.write(file.read());
   }
}

void SDcardControllerClass::writeFile(fs::FS &fs, const char * path, const char * message){
   Serial.printf("Writing file: %s\n", path);

   File file = fs.open(path, FILE_WRITE);
   if(!file){
      Serial.println("Failed to open file for writing");
      return;
   }
   if(file.print(message)){
      Serial.println("File written");
   } else {
      Serial.println("Write failed");
   }
}

void SDcardControllerClass::appendFile(fs::FS &fs, const char * path, const char * message){
   Serial.printf("Appending to file: %s\n", path);

   File file = fs.open(path, FILE_APPEND);
   if(!file){
      Serial.println("Failed to open file for appending");
      return;
   }
   if(file.print(message)){
      Serial.println("Message appended");
   } else {
      Serial.println("Append failed");
   }
}

void SDcardControllerClass::renameFile(fs::FS &fs, const char * path1, const char * path2){
   Serial.printf("Renaming file %s to %s\n", path1, path2);
   if (fs.rename(path1, path2)) {
      Serial.println("File renamed");
   } else {
      Serial.println("Rename failed");
   }
}

void SDcardControllerClass::deleteFile(fs::FS &fs, const char * path){
   Serial.printf("Deleting file: %s\n", path);
   if(fs.remove(path)){
      Serial.println("File deleted");
   } else {
      Serial.println("Delete failed");
   }
}

void SDcardControllerClass::testFileIO(fs::FS &fs, const char * path){
   File file = fs.open(path);
   static uint8_t buf[512];
   size_t len = 0;
   uint32_t start = millis();
   uint32_t end = start;
   if(file){
      len = file.size();
      size_t flen = len;
      start = millis();
      while(len){
         size_t toRead = len;
         if(toRead > 512){
               toRead = 512;
         }
         file.read(buf, toRead);
         len -= toRead;
      }
      end = millis() - start;
      Serial.printf("%u bytes read for %u ms\n", flen, end);
      file.close();
   } else {
      Serial.println("Failed to open file for reading");
   }


   file = fs.open(path, FILE_WRITE);
   if(!file){
      Serial.println("Failed to open file for writing");
      return;
   }

   size_t i;
   start = millis();
   for(i=0; i<2048; i++){
      file.write(buf, 512);
   }
   end = millis() - start;
   Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
   file.close();
}

SDcardControllerClass SDcardController;