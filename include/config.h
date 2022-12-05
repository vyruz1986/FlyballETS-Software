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
// along with this program. If not, see <http://www.gnu.org/licenses/>

#ifdef ESP32
#define GET_MICROS esp_timer_get_time()
#else
#define GET_MICROS micros()
#endif

#ifndef GLOBALVAR_H
#define GLOBALVAR_H

#define FW_VER "1.10.7"          // Flyball ETS firmware version


#define Simulate true            // Set to true to enable race simulation (see Simulator.h/.cpp)
#define NumSimulatedRaces 48     // Number of prepeared simulated races. Sererial interface command to change interface: e.g. "race 1"
#define TRIGGER_QUEUE_LENGTH 60  // Number of triggers in the queue

#define WiFiON                   // If defined all WiFi features are on: OTA, Web server. Please be carefull. Keep remote receiver board (antenna) away from ESP32 to avoid interferences.
//#define WebUIonSDcard           // When defined webserver data will be loaded from SC card (MMC 1 bit mode) and not taken compiled into fimrware package. Precondition: SDcard defined too.
#define BatteryCalibration false // after setting to true LCD will display analog read value from battery pin (range 0-4095). This is handfull for battery volate curve definition (dPinVoltage)

#define LIGHTSCHAINS 1           // Numer of WS281x lights chains. 1 - one chain of 5 pixels/lights, 2 - two chains --> 10 pixels/lights, etc.
#define WS_METHOD NeoEsp32Rmt0800KbpsMethod

#define EEPROM_SIZE 4096         // EEPROM size in bytes
#define SPI_FLASH_SEC_SIZE 4096  // Flash Sector Size declaration for ESP32 as it seems to become removed from embedded libraries

#define WS_TICKET_BUFFER_SIZE 8  // Number of websocket tickets kept in memory
#define WS_TIMEOUT 1800000       // Timeout for secured websocket in miliseconds

#define APP_VER "1.0.4"          // WebUI version

#endif