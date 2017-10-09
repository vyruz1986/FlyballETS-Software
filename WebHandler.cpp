// 
// 
// 

#include "WebHandler.h"
#include <Hash.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "Debug.h"

void WebHandlerClass::_WsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t * data, size_t len)
{
   if (type == WS_EVT_CONNECT) {
      Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
      //client->printf("Hello Client %u :)", client->id());
      client->ping();
   }
   else if (type == WS_EVT_DISCONNECT) {
      Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
   }
   else if (type == WS_EVT_ERROR) {
      Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
   }
   else if (type == WS_EVT_PONG) {
      Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char*)data : "");
   }
   else if (type == WS_EVT_DATA) {
      AwsFrameInfo * info = (AwsFrameInfo*)arg;
      String msg = "";
      if (info->final && info->index == 0 && info->len == len) {
         //the whole message is in a single frame and we got all of it's data
         Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);

         if (info->opcode == WS_TEXT) {
            for (size_t i = 0; i < info->len; i++) {
               msg += (char)data[i];
            }
         }
         else {
            char buff[3];
            for (size_t i = 0; i < info->len; i++) {
               sprintf(buff, "%02x ", (uint8_t)data[i]);
               msg += buff;
            }
         }
         Serial.printf("%s\n", msg.c_str());
      }
      else {
         //message is comprised of multiple frames or the frame is split into multiple packets
         if (info->index == 0) {
            if (info->num == 0)
               Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
            Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
         }

         Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT) ? "text" : "binary", info->index, info->index + len);

         if (info->opcode == WS_TEXT) {
            for (size_t i = 0; i < info->len; i++) {
               msg += (char)data[i];
            }
         }
         else {
            char buff[3];
            for (size_t i = 0; i < info->len; i++) {
               sprintf(buff, "%02x ", (uint8_t)data[i]);
               msg += buff;
            }
         }
         Serial.printf("%s\n", msg.c_str());

         if ((info->index + len) == info->len) {
            Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
            if (info->final) {
               Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
            }
         }
      }
      
      // Parse JSON input
      DynamicJsonBuffer jsonBufferRequest;
      JsonObject& request = jsonBufferRequest.parseObject(msg);
      if (!request.success()) {
         Debug.DebugSend(LOG_ERR, "Error parsing JSON!");
         //wsSend_P(client->id(), PSTR("{\"message\": 3}"));
         client->text("{\"error\":\"Invalid JSON received\"}");
         return;
      }

      if (request.containsKey("action")) {
         DynamicJsonBuffer jsonBufferResponse;
         JsonObject& JsonRoot = jsonBufferResponse.createObject();
         JsonObject& ActionResult = JsonRoot.createNestedObject("ActionResult");
         String errorText;
         bool result = _DoAction(request["action"], &errorText);
         ActionResult["success"] = result;
         ActionResult["error"] = errorText;

         String jsonResponse;
         JsonRoot.printTo(jsonResponse);
         client->text(jsonResponse);
      }
   }
}

void WebHandlerClass::init(int webPort)
{
   _server = new AsyncWebServer(webPort);
   _ws = new AsyncWebSocket("/ws");
   _ws->onEvent(std::bind(&WebHandlerClass::_WsEvent, this
      , std::placeholders::_1
      , std::placeholders::_2
      , std::placeholders::_3
      , std::placeholders::_4
      , std::placeholders::_5
      , std::placeholders::_6));

   _server->addHandler(_ws);

   _server->onNotFound([](AsyncWebServerRequest *request) {
      Serial.printf("Not found: %s!\r\n", request->url().c_str());
      request->send(404);
   });

   SPIFFS.begin(true);
   _server->serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

   _server->begin();

   _lLastRaceDataBroadcast = 0;
   _lRaceDataBroadcastInterval = 500;
}

void WebHandlerClass::loop()
{
   if (millis() - _lLastRaceDataBroadcast > _lRaceDataBroadcastInterval)
   {
      _SendRaceData();
      _lLastRaceDataBroadcast = millis();
   }
}

void WebHandlerClass::_wsSend_P(PGM_P payload) {
   if (_ws->count() > 0) {
      char buffer[strlen_P(payload)];
      strcpy_P(buffer, payload);
      _ws->textAll(buffer);
   }
}

void WebHandlerClass::_wsSend_P(uint32_t client_id, PGM_P payload) {
   char buffer[strlen_P(payload)];
   strcpy_P(buffer, payload);
   _ws->text(client_id, buffer);
}

boolean WebHandlerClass::_DoAction(String action, String * ReturnError) {
   if (action == "StartRace") {
      if (RaceHandler.RaceState != RaceHandler.STOPPED) {
         //ReturnError = String("Race was not stopped, stop it first!");
         return false;
      }
      else
      {
         LightsController.InitiateStartSequence();
         RaceHandler.StartRace();
         return true;
      }
   }
   else if (action == "StopRace")
   {
      if (RaceHandler.RaceState == RaceHandler.STOPPED) {
         //ReturnError = "Race was already stopped!";
         return false;
      }
      else
      {
         RaceHandler.StopRace(micros());
         LightsController.DeleteSchedules();
         return true;
      }
   }
   else if (action == "ResetRace")
   {
      if (RaceHandler.RaceState != RaceHandler.STOPPED) {
         //ReturnError = "Race was not stopped, first stop it before resetting!";
         return false;
      }
      else if (RaceHandler._lRaceStartTime == 0) {
         //ReturnError = "Race was already reset!";
         return false;
      }
      else
      {
         LightsController.ResetLights();
         RaceHandler.ResetRace();
         return true;
      }
   }
}

void WebHandlerClass::_SendRaceData(uint iRaceId)
{
   DynamicJsonBuffer JsonBuffer;
   JsonObject& JsonRoot = JsonBuffer.createObject();
   
   if (!JsonRoot.success())
   {
      Debug.DebugSend(LOG_ERR, "Error parsing JSON!");
      //wsSend_P(client->id(), PSTR("{\"message\": 3}"));
      _ws->textAll("{\"error\": \"Error parsing JSON from RaceData!\"}");
      return;
   }
   else
   {
      JsonObject& JsonRaceData = JsonRoot.createNestedObject("RaceData");
      RaceData RequestedRaceData = RaceHandler.GetRaceData();
      JsonRaceData["Id"] = RequestedRaceData.Id;
      JsonRaceData["StartTime"] = RequestedRaceData.StartTime;
      JsonRaceData["EndTime"] = RequestedRaceData.EndTime;
      JsonRaceData["ElapsedTime"] = RequestedRaceData.ElapsedTime;
      JsonRaceData["RaceState"] = RequestedRaceData.RaceState;
      String JsonString;
      JsonRoot.printTo(JsonString);
      Serial.printf("json: %s\r\n", JsonString.c_str());
      _ws->textAll(JsonString);
   }
}


WebHandlerClass WebHandler;