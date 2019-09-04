// SystemManager.h

#ifndef _SYSTEMMANAGER_h
#define _SYSTEMMANAGER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

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
