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

#ifndef _WIFIMANAGER_H
#define _WIFIMANAGER_H

#include "Arduino.h"
#include "WiFi.h"

#define WIFI_CHECK_INTERVAL 500

class WifiManager
{
public:
    void SetupWiFi();
    void WiFiLoop();
    void WiFiEvent(WiFiEvent_t event, system_event_info_t info);

private:
    IPAddress _IPGateway;
    IPAddress _IPSubnet;
    String _strAPName;
    String _strSTAName;

    unsigned long ulLastWifiCheck = 0;
};

#endif