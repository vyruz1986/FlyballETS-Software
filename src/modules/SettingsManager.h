// SettingsManager.h

#ifndef _SETTINGSMANAGER_h
#define _SETTINGSMANAGER_h

#include "Arduino.h"


class SettingsManagerClass
{
protected:
   bool _settings_save;

public:
   void loop();
   void init();

   String getSetting(const String& key, String defaultValue);
   String getSetting(const String& key);
   
   bool setSetting(const String& key, String value);

   void saveSettings();
   bool hasSetting(const String & key);
   void setDefaultSettings();
};

extern SettingsManagerClass SettingsManager;


#endif

