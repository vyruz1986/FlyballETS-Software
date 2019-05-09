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

#include "SlaveHandler.h"
//static const char TAG[] = __FILE__;

//Wifi stuff
IPAddress IPGateway;
IPAddress IPSubnet;
String strAPName;
String strSTAName;

unsigned long ulLastWifiCheck = 0;
#define WIFI_CHECK_INTERVAL 500

void SetupWiFi() {
   WiFi.onEvent(WiFiEvent);
   uint8_t iOpMode = SettingsManager.getSetting("OperationMode").toInt();
   strAPName = SettingsManager.getSetting("APName");
   String strAPPass = SettingsManager.getSetting("APPass");
   ESP_LOGI(TAG, "Starting in mode %i", iOpMode);
   ESP_LOGI(TAG, "Name: %s, pass: %s\r\n", strAPName.c_str(), strAPPass.c_str());
   if (iOpMode == SystemModes::MASTER) {
      IPGateway = IPAddress(192, 168, 20, 1);
      IPSubnet = IPAddress(255, 255, 255, 0);
      if (!WiFi.mode(WIFI_MODE_AP)
         || !WiFi.softAP(strAPName.c_str(), strAPPass.c_str())) {
         ESP_LOGW(TAG, "[WiFi]: Error initializing softAP with name %s!", strAPName.c_str());
      }
   }
   else if (iOpMode == SystemModes::SLAVE) {
      strSTAName = strAPName;
      strAPName += "_SLV";
      IPGateway = IPAddress(192, 168, 4, 1);
      IPSubnet = IPAddress(255, 255, 255, 0);
      if(!WiFi.mode(WIFI_MODE_APSTA)
         || !WiFi.softAP(strAPName.c_str(), strAPPass.c_str())) {
         ESP_LOGW(TAG, "[WiFi]: Error initializing softAP with name %s!", strAPName.c_str());
      }
      WiFi.begin(strSTAName.c_str(), strAPPass.c_str());
   }
   else {
      ESP_LOGE(TAG, "[WiFi]: Got unknown mode, no idea how I should start...");
   }
}

void WiFiLoop() {
   if (millis() - ulLastWifiCheck > WIFI_CHECK_INTERVAL) {
      ulLastWifiCheck = millis();
   }
}

void WiFiEvent(WiFiEvent_t event) {
   switch (event) {
   case SYSTEM_EVENT_AP_START:
      ESP_LOGI(TAG, "Trying to configure IP: %s, SM: %s", IPGateway.toString().c_str(), IPSubnet.toString().c_str());
      if (!WiFi.softAPConfig(IPGateway, IPGateway, IPSubnet)) {
         ESP_LOGE(TAG, "[WiFi]: AP Config failed!");
      }
      else {
         ESP_LOGI(TAG, "[WiFi]: AP Started with name %s, IP: %s", strAPName.c_str(), WiFi.softAPIP().toString().c_str());
      }
      if (WiFi.softAPIP() != IPGateway)
      {
         ESP_LOGE(TAG, "I am not running on the correct IP (%s instead of %s), rebooting!", WiFi.softAPIP().toString().c_str(), IPGateway.toString().c_str());
         ESP.restart();
      }
      break;
   case SYSTEM_EVENT_AP_STOP:
      ESP_LOGI(TAG, "[WiFi]: AP Stopped");
      break;

   case SYSTEM_EVENT_STA_GOT_IP:
      ESP_LOGI(TAG, "[WiFi]: STA got IP %s", WiFi.localIP().toString().c_str());
      break;

   case SYSTEM_EVENT_STA_DISCONNECTED:
   {
      SlaveHandler.resetConnection();
      ESP_LOGI(TAG, "Disconnected from AP (event %i)", SYSTEM_EVENT_STA_DISCONNECTED);
      String strAPName = SettingsManager.getSetting("APName");
      String strAPPass = SettingsManager.getSetting("APPass");
      WiFi.begin(strAPName.c_str(), strAPPass.c_str());
      ESP_LOGW(TAG, "[WiFi]: Connecting to AP...");
      break;
   }

   case SYSTEM_EVENT_STA_CONNECTED:
   {
      ESP_LOGI(TAG, "Connected to AP %s (event %i)", WiFi.SSID().c_str(), SYSTEM_EVENT_STA_CONNECTED);
      break;
   }

   default:
      ESP_LOGD(TAG, "Wifi event %i\r\n", event);
      break;
   }
}