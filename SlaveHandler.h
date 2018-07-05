// SlaveHandler.h

#ifndef _SLAVEHANDLER_h
#define _SLAVEHANDLER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#define CONNECT_RETRY 1000

class SlaveHandlerClass
{
protected:
   IPAddress _RemoteIP;
   bool _bConnected;
   bool _bIAmSlave;
   bool _bSlaveConfigured;
   unsigned long _ulLastConnectAttempt;

   void _ConnectRemote();


public:
   void init();
   void loop();
   void setRemoteIp(IPAddress ipSlaveIP);
   void removeSlave();
   bool slavePresent();
};

extern SlaveHandlerClass SlaveHandler;

#endif

