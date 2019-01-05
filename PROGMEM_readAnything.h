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
#include <Arduino.h>  // for type definitions

template <typename T> void PROGMEM_readAnything(const T * sce, T& dest)
{
   memcpy_P(&dest, sce, sizeof(T));
}

template <typename T> T PROGMEM_getAnything(const T * sce)
{
   static T temp;
   memcpy_P(&temp, sce, sizeof(T));
   return temp;
}