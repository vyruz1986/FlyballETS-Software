// WebHandler.h

#ifndef _WEBHANDLER_h
#define _WEBHANDLER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "SettingsManager.h"
#include <Hash.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "Debug.h"
#include "RaceHandler.h"
#include "Structs.h"
#include "LightsController.h"


class WebHandlerClass
{
   friend class RaceHandlerClass;
protected:
   AsyncWebServer *_server;
   AsyncWebSocket *_ws;
   AsyncWebSocket *_wsa;
   void _WsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
   boolean _DoAction(String action, String * ReturnError);
   void _SendRaceData(uint iRaceId = RaceHandler._iCurrentRaceId);

   boolean _ProcessConfig(JsonArray& newConfig, String * ReturnError);

   boolean _GetData(String dataType, JsonObject& ReturnError);

   stSystemData _GetSystemData();

   void _SendSystemData();
   
   unsigned long _lLastRaceDataBroadcast;
   unsigned long _lRaceDataBroadcastInterval;
   unsigned long _lLastSystemDataBroadcast;
   unsigned long _lSystemDataBroadcastInterval;

   stSystemData _SystemData;

public:
   void init(int webPort);
   void loop();
};

extern WebHandlerClass WebHandler;

#endif

