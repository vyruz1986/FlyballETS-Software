// Copyright (C) 2018 Alex Goris
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

#define Simulate true                     // Set to true to enable race simulation (see Simulator.h/.cpp)

#define WS281x                            //Comment out this line if you want to use the v1 setup which used a 74HC595 shift register to control lights
#define WS_METHOD NeoWs2813Method
#define EEPROM_SIZE           4096        // EEPROM size in bytes

#define WS_TICKET_BUFFER_SIZE 5           //Number of websocket tickets kept in memory
#define WS_TIMEOUT            1800000     // Timeout for secured websocket

#define APP_VER "05.00.00"