// SlaveHandler.h

#ifndef _SLAVEHANDLER_h
#define _SLAVEHANDLER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

class SlaveHandlerClass
{
protected:
   IPAddress _SlaveIP;
   bool _bSlaveConnected;

public:
   void init();
   void loop();
   void setSlaveIp(IPAddress ipSlaveIP);
   void removeSlave();
   bool slavePresent();
};

extern SlaveHandlerClass SlaveHandler;

#endif

