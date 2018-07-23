// WebHandler.h

#ifndef _WEBHANDLER_h
#define _WEBHANDLER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "SettingsManager.h"
//#include <Hash.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
//#include <FS.h>
//#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "RaceHandler.h"
#include "Structs.h"
#include "LightsController.h"
#include "syslog.h"


class WebHandlerClass
{
   friend class RaceHandlerClass;
protected:
   AsyncWebServer *_server;
   AsyncWebSocket *_ws;
   AsyncWebSocket *_wsa;
   void _WsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
   boolean _DoAction(JsonObject ActionObj, String * ReturnError, AsyncWebSocketClient * Client);
   boolean _GetRaceDataJsonString(uint iRaceId, String &strJsonString);
   void _SendRaceData(uint iRaceId = RaceHandler._iCurrentRaceId, int8_t iClientId = -1);

   boolean _ProcessConfig(JsonArray newConfig, String * ReturnError);

   boolean _GetData(String dataType, JsonObject ReturnError);

   stSystemData _GetSystemData();
   void _SendSystemData(int8_t iClientId = -1);
   void _onAuth(AsyncWebServerRequest * request);
   bool _authenticate(AsyncWebServerRequest * request);
   bool _wsAuth(AsyncWebSocketClient * client);

   void _onHome(AsyncWebServerRequest * request);
   void _CheckMasterStatus();
   void _DisconnectMaster();

   unsigned long _lLastRaceDataBroadcast;
   unsigned long _lRaceDataBroadcastInterval;
   unsigned long _lLastSystemDataBroadcast;
   unsigned long _lSystemDataBroadcastInterval;
   stSystemData _SystemData;
   char _last_modified[50];
   bool _bSlavePresent;

   typedef struct
   {
      bool Configured;
      IPAddress ip;
      AsyncWebSocketClient * client;
      uint8_t ClientID;
      unsigned long LastCheck;
      unsigned long LastReply;
   } stMasterStatus;
   stMasterStatus _MasterStatus;
   
   typedef struct
   {
      IPAddress ip;
      unsigned long timestamp = 0;
   } ws_ticket_t;
   ws_ticket_t _ticket[WS_TICKET_BUFFER_SIZE];

   boolean _bIsConsumerArray[10];
   uint8_t _iNumOfConsumers;

public:
   void init(int webPort);
   void loop();
   void SendLightsData(stLightsState LightStates);
   bool MasterConnected();
};

extern WebHandlerClass WebHandler;

#endif

