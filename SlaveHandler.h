// SlaveHandler.h

#ifndef _SLAVEHANDLER_h
#define _SLAVEHANDLER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif
#include <WebSocketsClient.h>
#define CONNECT_CHECK 1000

class SlaveHandlerClass
{
protected:
   IPAddress _RemoteIP;
   bool _bConnected;
   bool _bIAmSlave;
   bool _bSlaveConfigured;
   bool _bWSConnectionStarted;
   unsigned long _ulLastConnectCheck;
   WebSocketsClient _wsClient;

   void _ConnectRemote();
   void _WsEvent(WStype_t type, uint8_t * payload, size_t length);
   void _SetDisconnected();
   void _AnnounceSlaveIfApplicable();

public:
   void init();
   void loop();
   void setRemoteIp(IPAddress ipSlaveIP);
   void removeSlave();
   bool slavePresent();
};

extern SlaveHandlerClass SlaveHandler;

#endif

