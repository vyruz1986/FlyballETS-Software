// SystemManager.h

#ifndef _SYSTEMMANAGER_h
#define _SYSTEMMANAGER_h

#include "Arduino.h"

#include <SettingsManager.h>
#include <SlaveHandler.h>
#include <WebHandler.h>
#include <enums.h>

class SystemManagerClass
{
protected:
   unsigned long _ulScheduledRebootTS;
   uint8_t _iOpMode;

public:
   void init();
   void loop();
   void scheduleReboot(unsigned long ulRebootTs);
   bool CheckMasterSlaveConnection();
};

extern SystemManagerClass SystemManager;

#endif
