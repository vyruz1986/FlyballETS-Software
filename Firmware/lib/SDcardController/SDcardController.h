// SDCard.h
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

#ifndef _SDCARDCONTROLLER_h
#define _SDCARDCONTROLLER_h

#include "Arduino.h"
#include "GPSHandler.h"
#include "RaceHandler.h"
#include "LCDController.h"
#include <FS.h>
#include <SD_MMC.h>

class SDcardControllerClass
{
protected:

public:
    bool bSDCardDetected = false;
    String sTagValue;
    uint16_t iTagValue;
    void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
    void createDir(fs::FS &fs, const char * path);
    void removeDir(fs::FS &fs, const char * path);
    void readFile(fs::FS &fs, const char * path);
    void writeFile(fs::FS &fs, const char * path, const char * message);
    void appendFile(fs::FS &fs, const char * path, const char * message);
    void renameFile(fs::FS &fs, const char * path1, const char * path2);
    void deleteFile(fs::FS &fs, const char * path);
    void init();
    void UpdateTagFile();
    void CheckSDcardSlot(uint8_t iSDdetectPin);
    void SaveRaceDataToFile();

private:
    uint8_t _iSDdetectPinStatus;
    String raceDataFileName;
    void testFileIO(fs::FS &fs, const char * path);

};

extern SDcardControllerClass SDcardController;

#endif