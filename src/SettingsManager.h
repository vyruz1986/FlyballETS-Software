// SettingsManager.h

#ifndef _SETTINGSMANAGER_h
#define _SETTINGSMANAGER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

class SettingsManagerClass
{
protected:
   boolean _settings_save;

public:
   void loop();
   void init();

   String getSetting(const String &key, String defaultValue);
   String getSetting(const String &key);

   void setSetting(const String &key, String value);

   void saveSettings();
   bool hasSetting(const String &key);
   void setDefaultSettings();
};

extern SettingsManagerClass SettingsManager;

#endif
