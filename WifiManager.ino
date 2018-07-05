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

//Wifi stuff
//WiFiMulti wm;
IPAddress IPGateway;
IPAddress IPNetwork;
IPAddress IPSubnet;

unsigned long ulLastWifiCheck = 0;
#define WIFI_CHECK_INTERVAL 500

void SetupWiFi() {
   WiFi.onEvent(WiFiEvent);
   uint8_t iOpMode = SettingsManager.getSetting("OperationMode").toInt();
   String strAPName = SettingsManager.getSetting("APName");
   String strAPPass = SettingsManager.getSetting("APPass");
   syslog.logf_P("[WiFi]: Starting in mode %i", iOpMode);
   if (iOpMode == SystemModes::MASTER) {
      WiFi.mode(WIFI_MODE_AP);
      IPGateway = IPAddress(192, 168, 20, 1);
      IPNetwork = IPAddress(192, 168, 20, 0);
      IPSubnet = IPAddress(255, 255, 255, 0);
      WiFi.softAPConfig(IPGateway, IPGateway, IPSubnet);
      if (!WiFi.softAP(strAPName.c_str(), strAPPass.c_str())) {
         syslog.logf_P(LOG_ALERT, "[WiFi]: Error initializing softAP with name %s!", strAPName.c_str());
      }
   }
   else if (iOpMode == SystemModes::SLAVE) {
      WiFi.mode(WIFI_MODE_APSTA);
      IPGateway = IPAddress(192, 168, 4, 1);
      IPNetwork = IPAddress(192, 168, 4, 0);
      IPSubnet = IPAddress(255, 255, 255, 0);
      WiFi.softAPConfig(IPGateway, IPGateway, IPSubnet);
      if(!WiFi.softAP(String(strAPName + "_SLV").c_str(), strAPPass.c_str())) {
         syslog.logf_P(LOG_ALERT, "[WiFi]: Error initializing softAP with name %s!", strAPName.c_str());
      }
      WiFi.begin(strAPName.c_str(), strAPPass.c_str());
   }
   else {
      syslog.logf_P(LOG_ERR, "[WiFi]: Got unknown mode, no idea how I should start...");
   }
}

void WiFiLoop() {
   if (millis() - ulLastWifiCheck > WIFI_CHECK_INTERVAL) {
      ulLastWifiCheck = millis();
      if (WiFi.getMode() == WIFI_MODE_APSTA) {
         if (WiFi.status() != WL_CONNECTED) {
            syslog.logf_P(LOG_WARNING, "[WiFi]: Connecting to AP...");
         }
      }
   }
}

void WiFiEvent(WiFiEvent_t event) {
   
   switch (event) {
   case SYSTEM_EVENT_AP_START:
      syslog.logf_P("[WiFi]: AP Started with name %s, IP: %s", WiFi.SSID().c_str(), WiFi.softAPIP().toString().c_str());
      if (!WiFi.softAPConfig(IPGateway, IPGateway, IPSubnet)) {
         syslog.logf_P(LOG_ERR, "[WiFi]: AP Config failed!");
      }
      if (WiFi.softAPIP() != IPGateway)
      {
         syslog.logf_P(LOG_ERR, "I am not running on the correct IP (%s instead of %s), rebooting!", WiFi.softAPIP().toString().c_str(), IPGateway.toString().c_str());
         ESP.restart();
      }
      break;
   case SYSTEM_EVENT_AP_STOP:
      syslog.logf_P("[WiFi]: AP Stopped");
      break;

   case SYSTEM_EVENT_STA_GOT_IP:
      syslog.logf_P("[WiFi]: STA got IP %s", WiFi.localIP().toString().c_str());
      break;

   default:
      Serial.printf("Wifi event %i\r\n", event);
      break;
   }
}