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
#include <FS.h>
#include <SD_MMC.h>

class SDcardControllerClass
{
protected:
public:
    void init();
    void CheckSDcardSlot(uint8_t iSDdetectPin);

private:
    boolean _bSDCardDetected = false;
    uint8_t _iSDdetectPinStatus;
};

extern SDcardControllerClass SDcardController;

#endif