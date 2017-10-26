#include "SettingsManager.h"
#include "Embedis.h"
#include <EEPROM.h>
#include <SPIFFS.h>
#include <StreamString.h>
#include "Debug.h"
#include "config.h"

void SettingsManagerClass::loop()
{
   if (_settings_save) {
      Debug.DebugSend(LOG_DEBUG, "[SETTINGS] Saving\r\n");
      EEPROM.commit();
      _settings_save = false;
   }
}

void SettingsManagerClass::init()
{
   EEPROM.begin(EEPROM_SIZE);
   _settings_save = false;

   Embedis::dictionary(F("EEPROM"),
      SPI_FLASH_SEC_SIZE,
      [](size_t pos) -> char { return EEPROM.read(pos); },
      [](size_t pos, char value) { EEPROM.write(pos, value); },
      []() {}
   );
}

String SettingsManagerClass::getSetting(const String& key, String defaultValue)
{
   String value;
   if (!Embedis::get(key, value)) value = defaultValue;
   return value;
}

String SettingsManagerClass::getSetting(const String& key)
{
   return getSetting(key, "");
}

//template<typename T> bool SettingsManagerClass::setSetting(const String& key, T value)
bool SettingsManagerClass::setSetting(const String& key, String value)
{
   return Embedis::set(key, String(value));
}

void SettingsManagerClass::saveSettings()
{
   _settings_save = true;
}

SettingsManagerClass SettingsManager;
