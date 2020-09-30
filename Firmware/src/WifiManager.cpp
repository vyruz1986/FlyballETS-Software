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

#include "WifiManager.h"
#include <SlaveHandler.h>
#include <SettingsManager.h>
#include "enums.h"
#include "WiFi.h"

void WifiManager::SetupWiFi()
{
   //WiFi.onEvent(std::bind(&WifiManager::WiFiEvent, this, std::placeholders::_1));
   WiFi.onEvent(
       [this](WiFiEvent_t event, system_event_info_t info) {
          this->WiFiEvent(event, info);
       });
   uint8_t iOpMode = SettingsManager.getSetting("OperationMode").toInt();
   _strAPName = SettingsManager.getSetting("APName");
   String strAPPass = SettingsManager.getSetting("APPass");
   ESP_LOGI(__FILE__, "Starting in mode %i", iOpMode);
   ESP_LOGI(__FILE__, "Name: %s, pass: %s\r\n", _strAPName.c_str(), strAPPass.c_str());
   if (iOpMode == SystemModes::MASTER)
   {
      _IPGateway = IPAddress(192, 168, 20, 1);
      _IPSubnet = IPAddress(255, 255, 255, 0);
      if (!WiFi.mode(WIFI_MODE_AP) ||
          !WiFi.softAPConfig(_IPGateway, _IPGateway, _IPSubnet) ||
          !WiFi.softAP(_strAPName.c_str(), strAPPass.c_str()))
      {
         ESP_LOGW(__FILE__, "[WiFi]: Error initializing softAP with name %s!", _strAPName.c_str());
      }
   }
   else if (iOpMode == SystemModes::SLAVE)
   {
      _strSTAName = _strAPName;
      _strAPName += "_SLV";
      _IPGateway = IPAddress(192, 168, 4, 1);
      _IPSubnet = IPAddress(255, 255, 255, 0);
      if (!WiFi.mode(WIFI_MODE_APSTA) || !WiFi.softAP(_strAPName.c_str(), strAPPass.c_str()))
      {
         ESP_LOGW(__FILE__, "[WiFi]: Error initializing softAP with name %s!", _strAPName.c_str());
      }
      WiFi.begin(_strSTAName.c_str(), strAPPass.c_str());
   }
   else
   {
      ESP_LOGE(__FILE__, "[WiFi]: Got unknown mode, no idea how I should start...");
   }
}

void WifiManager::WiFiLoop()
{
   if (millis() - ulLastWifiCheck > WIFI_CHECK_INTERVAL)
   {
      ulLastWifiCheck = millis();
      ESP_LOGV(__FILE__, "Wifi Status: %u, Wifi.localIP: %S, SoftAPIP: %s, SoftAPStationNum: %i",
               WiFi.status(),
               WiFi.localIP().toString().c_str(),
               WiFi.softAPIP().toString().c_str(),
               WiFi.softAPgetStationNum());
   }
}

void WifiManager::WiFiEvent(WiFiEvent_t event, system_event_info_t info)
{
   switch (event)
   {
   case SYSTEM_EVENT_AP_START:
      ESP_LOGI(__FILE__, "Trying to configure IP: %s, SM: %s", _IPGateway.toString().c_str(), _IPSubnet.toString().c_str());
      if (!WiFi.softAPConfig(_IPGateway, _IPGateway, _IPSubnet))
      {
         ESP_LOGE(__FILE__, "[WiFi]: AP Config failed!");
      }
      else
      {
         ESP_LOGI(__FILE__, "[WiFi]: AP Started with name %s, IP: %s", _strAPName.c_str(), WiFi.softAPIP().toString().c_str());
      }
      if (WiFi.softAPIP() != _IPGateway)
      {
         ESP_LOGE(__FILE__, "I am not running on the correct IP (%s instead of %s), rebooting!", WiFi.softAPIP().toString().c_str(), _IPGateway.toString().c_str());
         ESP.restart();
      }
      break;
   case SYSTEM_EVENT_AP_STOP:
      ESP_LOGI(__FILE__, "[WiFi]: AP Stopped");
      break;

   case SYSTEM_EVENT_STA_GOT_IP:
      ESP_LOGI(__FILE__, "[WiFi]: STA got IP %s", WiFi.localIP().toString().c_str());
      break;

   case SYSTEM_EVENT_STA_DISCONNECTED:
   {
      SlaveHandler.resetConnection();
      ESP_LOGI(__FILE__, "Disconnected from AP (event %i)", SYSTEM_EVENT_STA_DISCONNECTED);
      String _strAPName = SettingsManager.getSetting("APName");
      String strAPPass = SettingsManager.getSetting("APPass");
      WiFi.begin(_strAPName.c_str(), strAPPass.c_str());
      ESP_LOGW(__FILE__, "[WiFi]: Connecting to AP...");
      break;
   }

   case SYSTEM_EVENT_STA_CONNECTED:
   {
      ESP_LOGI(__FILE__, "Connected to AP %s (event %i)", WiFi.SSID().c_str(), SYSTEM_EVENT_STA_CONNECTED);
      break;
   }

   default:
      ESP_LOGD(__FILE__, "Wifi event %i\r\n", event);
      break;
   }
}