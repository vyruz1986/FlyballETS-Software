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

#include <ArduinoJson.h>
#pragma once

const size_t bsRaceData = 5 * JSON_ARRAY_SIZE(4) + JSON_OBJECT_SIZE(1) + 16 * JSON_OBJECT_SIZE(2) + 4 * JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(7) + 870;
const size_t bsRaceDataArray = JSON_ARRAY_SIZE(2) + 10 * JSON_ARRAY_SIZE(4) + JSON_OBJECT_SIZE(1) + 32 * JSON_OBJECT_SIZE(2) + 8 * JSON_OBJECT_SIZE(4) + 2 * JSON_OBJECT_SIZE(7) + 1720;
