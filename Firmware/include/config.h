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

#ifdef ESP32
#define GET_MICROS esp_timer_get_time()
#elif
#define GET_MICROS micros()
#endif

#ifndef GLOBALVAR_H
#define GLOBALVAR_H

#define Simulate false    // Set to true to enable race simulation (see Simulator.h/.cpp)
#define JTAG false          // when set to true you need converter board with pins remappig and jtag + programing port. It deactite featuers: LSR BTN+LED, battery sensor, switch button
#define NumSimulatedRaces 3 // number of prepeared simulated races. Sererial interface command to chane interface: e.g. RACE 1
#define WiFiActivation false // change to true to activate WiFi AP. Please keep in mind inteferences between WiFi and remote contol board
#define BatteryCalibration false // after setting to true LCD will display analog read value from battery pin (range 0-4095). This is handfull for battery volate curve definition (dPinVoltage)

#define LIGHTSCHAINS 1 // Numer of WS281x lights chains. 1 - one chain of 5 pixels/lights, 2 - two chains --> 10 pixels/lights, etc.

#define WS281x //Comment out this line if you want to use the v1 setup which used a 74HC595 shift register to control lights
#define WS_METHOD NeoWs2813Method
#define EEPROM_SIZE 4096        // EEPROM size in bytes
#define SPI_FLASH_SEC_SIZE 4096 // Flash Sector Size declaration for ESP32 as it seems to become removed from embedded libraries

#define WS_TICKET_BUFFER_SIZE 5 //Number of websocket tickets kept in memory
#define WS_TIMEOUT 1800000      // Timeout for secured websocket

#define APP_VER "5.0.0"

#endif