// WebHandler.h

#ifndef _WEBHANDLER_h
#define _WEBHANDLER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Hash.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "Debug.h"
#include "RaceHandler.h"


class WebHandlerClass
{
protected:
   AsyncWebServer *_server;
   AsyncWebSocket *_ws;
   void _WsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
   void wsSend_P(PGM_P payload);
   void wsSend_P(uint32_t client_id, PGM_P payload);
   boolean DoAction(String action, String * ReturnError);

public:
   void init(int webPort);
   void loop();
};

extern WebHandlerClass WebHandler;

#endif

