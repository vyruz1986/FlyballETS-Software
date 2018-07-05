// 
// 
// 

#include "SystemManager.h"

void SystemManagerClass::init()
{
   _ulScheduledRebootTS = 0;
}

void SystemManagerClass::loop()
{
   if (_ulScheduledRebootTS > 0
      && millis() > _ulScheduledRebootTS) {
      ESP.restart();
   }
}

void SystemManagerClass::scheduleReboot(unsigned long ulRebootTs) {
   if (ulRebootTs > millis()) {
      _ulScheduledRebootTS = ulRebootTs;
   }
}

SystemManagerClass SystemManager;