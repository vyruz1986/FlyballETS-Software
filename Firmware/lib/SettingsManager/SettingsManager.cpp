#include <SettingsManager.h>
#include "Embedis.h"
#include <EEPROM.h>
#include <SPIFFS.h>
#include <config.h>

void SettingsManagerClass::loop()
{
   if (_settings_save)
   {
      ESP_LOGD(__FILE__, "[SETTINGS] Saving");
      Serial.printf("Writing settings\r\n");
      EEPROM.commit();
      _settings_save = false;
   }
}

void SettingsManagerClass::init()
{
   EEPROM.begin(SPI_FLASH_SEC_SIZE);
   _settings_save = false;

   Embedis::dictionary("EEPROM",
                       SPI_FLASH_SEC_SIZE,
                       [](size_t pos) -> char { return EEPROM.read(pos); },
                       [](size_t pos, char value) { EEPROM.write(pos, value); },
                       []() { EEPROM.commit(); });

   setDefaultSettings();
}

String SettingsManagerClass::getSetting(const String &key, String defaultValue)
{
   String value;
   if (!Embedis::get(key, value))
      value = defaultValue;
   ESP_LOGD(__FILE__, "Returning value %s for setting %s", value.c_str(), key.c_str());
   return value;
}

String SettingsManagerClass::getSetting(const String &key)
{
   String strReturnValue = getSetting(key, "");
   return strReturnValue;
}

void SettingsManagerClass::setSetting(const String &key, String value)
{
   ESP_LOGD(__FILE__, "Setting value %s for key %s", value.c_str(), key.c_str());
   Embedis::set(key, String(value));
   saveSettings();
}

void SettingsManagerClass::saveSettings()
{
   _settings_save = true;
}

bool SettingsManagerClass::hasSetting(const String &key)
{
   return getSetting(key).length() != 0;
}

void SettingsManagerClass::setDefaultSettings()
{
   bool shouldSave = false;
   if (!hasSetting("AdminPass"))
   {
      setSetting("AdminPass", "FlyballETS.1234");
      shouldSave = true;
   }

   if (!hasSetting("APName"))
   {
      setSetting("APName", "FlyballETS");
      shouldSave = true;
   }

   if (!hasSetting("APPass"))
   {
      setSetting("APPass", "FlyballETS.1234");
      shouldSave = true;
   }

   if (!hasSetting("RunDirectionInverted"))
   {
      setSetting("RunDirectionInverted", "0");
      shouldSave = true;
   }

   if (!hasSetting("OperationMode"))
   {
      setSetting("OperationMode", "0");
      shouldSave = true;
   }
   if (shouldSave)
      saveSettings();
}

SettingsManagerClass SettingsManager;
