// WebHandler.h
#ifndef _WEBHANDLER_h
#define _WEBHANDLER_h

#include "config.h"
#include "global.h"
#include "Arduino.h"
#include "SettingsManager.h"
#include <Hash.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ESPAsyncWebServer.h>
#include "SDcardController.h"
#include <ArduinoJson.h>
#include "RaceHandler.h"
#include "Structs.h"
#include "LightsController.h"
#include "BatterySensor.h"
#include <AsyncTCP.h>
#include "GPSHandler.h"
#include <rom/rtc.h>
#ifndef WebUIonSDcard
#include "index.html.gz.h"
#endif

class WebHandlerClass
{
   friend class RaceHandlerClass;

protected:
   AsyncWebServer *_server;
   AsyncWebSocket *_ws;
   AsyncWebSocket *_wsa;
   void _WsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
   bool _DoAction(JsonObject ActionObj, String *ReturnError, AsyncWebSocketClient *Client);
   void _SendRaceData(int iRaceId, int8_t iClientId);
   void _SendLightsData(int8_t iClientId = -1);

   bool _ProcessConfig(JsonArray newConfig, String *ReturnError);

   bool _GetData(String dataType, JsonObject ReturnError);

   void _SendSystemData(int8_t iClientId = -1);
   void _onAuth(AsyncWebServerRequest *request);
   bool _authenticate(AsyncWebServerRequest *request);
   bool _wsAuth(AsyncWebSocketClient *client);
   void _onHome(AsyncWebServerRequest *request);
   void _onFavicon(AsyncWebServerRequest *request);

   unsigned long _lLastRaceDataBroadcast;
   const uint16_t _iRaceDataBroadcastInterval = 750;
   unsigned long _lLastSystemDataBroadcast;
   const uint16_t _iSystemDataBroadcastInterval = 3500;
   unsigned long _lLastPingBroadcast;
   const uint16_t _iPingBroadcastInterval = 30000;
   unsigned long _lWebSocketReceivedTime;
   unsigned long _lLastBroadcast;
   stSystemData _SystemData;
   char _last_modified[50];

   typedef struct
   {
      IPAddress ip;
      unsigned long timestamp = 0;
   } ws_ticket_t;
   ws_ticket_t _ticket[WS_TICKET_BUFFER_SIZE];

   bool _bIsConsumerArray[255];
   uint8_t _iNumOfConsumers;

public:
   void init(int webPort);
   void loop();
   void disconnectWsClient(IPAddress ipDisconnectedIP);
   bool bUpdateLights = false;
   bool bSendRaceData = false;
   bool bUpdateRaceData = false;
   bool bUpdateTimerWebUIdata = false;
   enum RaceDataFields
   {
      D1Time,      // 0
      D2Time,      // 1
      D3Time,      // 2
      D4Time,      // 3
      D1CrossTime, // 4
      D2CrossTime, // 5
      D3CrossTime, // 6
      D4CrossTime, // 7
      D1FaultInfo, // 8
      D2FaultInfo, // 9
      D3FaultInfo, // 10
      D4FaultInfo, // 11
      D1Running,   // 12
      D2Running,   // 13
      D3Running,   // 14
      D4Running,   // 15
      elapsedTime, // 16
      cleanTime,   // 17
      raceState,   // 18
      id,          // 19
      racingDogs,  // 20
      rerunsOff    // 21
   };
   bool bUpdateThisRaceDataField[22];
};

extern WebHandlerClass WebHandler;

#endif
