//
//
//
#include "WebHandler.h"
#include <AsyncElegantOTA.h>

void WebHandlerClass::_WsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
   bool isAdmin = String("/wsa").equals(server->url());

   if (type == WS_EVT_CONNECT)
   {
      ESP_LOGI(__FILE__, "Client %i connected to %s!", client->id(), server->url());

      //Access via /wsa. Checking if clinet/consumer has already athenticated
      if (isAdmin)
      {
         if (!_wsAuth(client))
         {
            client->close(401, "Unauthorized");
            return;
         }
      }
      client->ping();
   }
   else if (type == WS_EVT_DISCONNECT)
   {
      if (_bIsConsumerArray[client->id()])
      {
         _iNumOfConsumers--;
         _bIsConsumerArray[client->id()] = false;
      }

      ESP_LOGI(__FILE__, "Client %u disconnected!\n", client->id());
   }
   else if (type == WS_EVT_ERROR)
   {
      ESP_LOGE(__FILE__, "ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
   }
   else if (type == WS_EVT_PONG)
   {
      ESP_LOGD(__FILE__, "ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
   }
   else if (type == WS_EVT_DATA)
   {
      AwsFrameInfo *info = (AwsFrameInfo *)arg;
      String msg = "";
      if (info->final && info->index == 0 && info->len == len)
      {
         //the whole message is in a single frame and we got all of it's data
         //ESP_LOGD(__FILE__, "ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);

         if (info->opcode == WS_TEXT)
         {
            data[len] = 0;
            msg = (char*)data;
         }
         else
         {
            char buff[3];
            for (size_t i = 0; i < info->len; i++)
            {
               sprintf(buff, "%02x ", (uint8_t)data[i]);
               msg += buff;
            }
         }
         ESP_LOGD(__FILE__, "%s\n", msg.c_str());
      }
      else
      {
         //message is comprised of multiple frames or the frame is split into multiple packets
         if (info->index == 0)
         {
            if (info->num == 0)
               ESP_LOGD(__FILE__, "ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
            ESP_LOGD(__FILE__, "ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
         }

         ESP_LOGD(__FILE__, "ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT) ? "text" : "binary", info->index, info->index + len);

         if (info->opcode == WS_TEXT)
         {
            data[len] = 0;
            msg = (char*)data;
         }
         else
         {
            char buff[3];
            for (size_t i = 0; i < info->len; i++)
            {
               sprintf(buff, "%02x ", (uint8_t)data[i]);
               msg += buff;
            }
         }
         ESP_LOGD(__FILE__, "%s\n", msg.c_str());

         if ((info->index + len) == info->len)
         {
            ESP_LOGD(__FILE__, "ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
            if (info->final)
               ESP_LOGD(__FILE__, "ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
         }
      }

      // Parse JSON input
      StaticJsonDocument<512> jsonRequestDoc;
      DeserializationError error = deserializeJson(jsonRequestDoc, msg);
      JsonObject request = jsonRequestDoc.as<JsonObject>();
      if (error)
      {
         ESP_LOGE(__FILE__, "Error parsing JSON: %s", error.c_str());
         client->text("{\"error\":\"Invalid JSON received\"}");
         return;
      }

      _lWebSocketReceivedTime = GET_MICROS / 1000;
      //const size_t bufferSize = JSON_ARRAY_SIZE(50) + 50 * JSON_OBJECT_SIZE(3);
      const size_t bufferSize = 384;
      StaticJsonDocument<bufferSize> jsonResponseDoc;
      JsonObject JsonResponseRoot = jsonResponseDoc.to<JsonObject>();

      if (request.containsKey("action"))
      {
         JsonObject ActionResult = JsonResponseRoot.createNestedObject("ActionResult");
         String errorText;
         bool result = _DoAction(request["action"].as<JsonObject>(), &errorText, client);
         ActionResult["success"] = result;
         ActionResult["error"] = errorText;
      }
      else if (request.containsKey("config"))
      {
         JsonObject ConfigResult = JsonResponseRoot.createNestedObject("configResult");
         String errorText;
         JsonArray config = request["config"].as<JsonArray>();
         //We allow setting config only over admin websocket
         bool result;
         if (isAdmin)
         {
            result = _ProcessConfig(config, &errorText);
         }
         else
         {
            result = false;
            errorText = "Unsupported message on this socket!";
         }
         ConfigResult["success"] = result;
         ConfigResult["error"] = errorText;
      }
      else if (request.containsKey("getData"))
      {
         String dataName = request["getData"];
         JsonObject DataResult = JsonResponseRoot.createNestedObject("dataResult");
         JsonObject DataObject = DataResult.createNestedObject(dataName + "Data");
         bool result;
         if (dataName == "config" && !isAdmin)
         {
            result = false;
            DataResult["error"] = "You can't get this datatype on this socket!";
         }
         else
         {
            result = _GetData(dataName, DataObject);
         }
         DataResult["success"] = result;
      }
      else
      {
         ESP_LOGW(__FILE__, "Got valid JSON but unknown message!");
         JsonResponseRoot["error"] = "Got valid JSON but unknown message!";
      }

      size_t len = measureJson(jsonResponseDoc);
      AsyncWebSocketMessageBuffer *wsBuffer = _ws->makeBuffer(len);
      _lLastBroadcast = GET_MICROS / 1000;
      if (wsBuffer)
      {
         serializeJson(jsonResponseDoc, (char *)wsBuffer->get(), len + 1);
         //ESP_LOGD(__FILE__, "wsBuffer to send: %s", (char *)wsBuffer->get());
         client->text(wsBuffer);
      }
   }
}

void WebHandlerClass::init(int webPort)
{
   snprintf_P(_last_modified, sizeof(_last_modified), PSTR("%s %s GMT"), __DATE__, __TIME__);
   _server = new AsyncWebServer(webPort);
   _ws = new AsyncWebSocket("/ws");
   _wsa = new AsyncWebSocket("/wsa");

   _lWebSocketReceivedTime = 0;

   _ws->onEvent(std::bind(&WebHandlerClass::_WsEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
   _wsa->onEvent(std::bind(&WebHandlerClass::_WsEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));

   _server->addHandler(_ws);
   _server->addHandler(_wsa);

   _server->onNotFound([](AsyncWebServerRequest *request) {
      ESP_LOGE(__FILE__, "Not found: %s!", request->url().c_str());
      request->send(404);
   });

   // Rewrites
   _server->rewrite("/", "/index.html");
   #ifdef WebUIonSDcard
   _server->serveStatic("/", SD_MMC, "/");
   #endif

   // Serve home (basic authentication protection)
   _server->on("/index.html", HTTP_GET, std::bind(&WebHandlerClass::_onHome, this, std::placeholders::_1));

   //Authentication handler
   _server->on("/auth", HTTP_GET, std::bind(&WebHandlerClass::_onAuth, this, std::placeholders::_1));

   //Favicon handler
   _server->on("/favicon.ico", HTTP_GET, std::bind(&WebHandlerClass::_onFavicon, this, std::placeholders::_1));

   String password = SettingsManager.getSetting("AdminPass");
   char httpPassword[password.length() + 1];
   password.toCharArray(httpPassword, password.length() + 1);
   AsyncElegantOTA.begin(_server, "Admin", httpPassword);    // Start ElegantOTA

   _server->begin();

   _lLastRaceDataBroadcast = 0;
   _lRaceDataBroadcastInterval = 750;

   _lLastSystemDataBroadcast = 0;
   _lSystemDataBroadcastInterval = 3500;

   _lLastPingBroadcast = 0;
   _lPingBroadcastInterval = 30000;

   _lLastBroadcast = 0;

   _SystemData.PwrOnTag = SDcardController.iTagValue;

   for (bool &bIsConsumer : _bIsConsumerArray)
   {
      bIsConsumer = false;
   }

   _iNumOfConsumers = 0;
}

void WebHandlerClass::loop()
{
   unsigned long lCurrentUpTime = GET_MICROS / 1000;
   //ESP_LOGD(__FILE__, "_bSendRaceData: %i, _bUpdateLights: %i, since LastBroadcast: %ul, since WS received: %ul, \r\n", _bSendRaceData, _bUpdateLights, (lCurrentUpTime - _lLastBroadcast), (lCurrentUpTime - _lWebSocketReceivedTime));
   if ((lCurrentUpTime - _lLastBroadcast > 100) && (lCurrentUpTime - _lWebSocketReceivedTime > 50))
   {
      if (_bUpdateLights)
         _SendLightsData();
      else if (_bSendRaceData
               || ((RaceHandler.RaceState == RaceHandler.RUNNING || (RaceHandler.RaceState == RaceHandler.STOPPED && (GET_MICROS - RaceHandler._llRaceEndTime) / 1000 < 1500))
                     && (lCurrentUpTime - _lLastRaceDataBroadcast > _lRaceDataBroadcastInterval)))
         _SendRaceData(RaceHandler.iCurrentRaceId, -1);
      else if (RaceHandler.RaceState == RaceHandler.RESET || (RaceHandler.RaceState == RaceHandler.STOPPED && (GET_MICROS - RaceHandler._llRaceEndTime) / 1000 > 1500))
      {
         if (lCurrentUpTime - _lLastSystemDataBroadcast > _lSystemDataBroadcastInterval)
            _SendSystemData();
         if (lCurrentUpTime - _lLastPingBroadcast > _lPingBroadcastInterval)
         {
            _ws->cleanupClients();
            ESP_LOGD(__FILE__, "Have %i clients, %i consumers\r\n", _ws->count(), _iNumOfConsumers);
            //_ws->pingAll();
            //_lLastBroadcast = GET_MICROS / 1000;
            _lLastPingBroadcast = GET_MICROS / 1000;
         }
      }
   }
}

void WebHandlerClass::_SendLightsData()
{
   stLightsState LightStates = LightsController.GetLightsState();
   StaticJsonDocument<96> jsonLightsDoc;
   JsonObject JsonRoot = jsonLightsDoc.to<JsonObject>();

   JsonArray JsonLightsData = JsonRoot.createNestedArray("LightsData");
   copyArray(LightStates.State, JsonLightsData);

   size_t len = measureJson(jsonLightsDoc);
   AsyncWebSocketMessageBuffer *wsBuffer = _ws->makeBuffer(len);
   if (wsBuffer)
   {
      serializeJson(jsonLightsDoc, (char *)wsBuffer->get(), len + 1);
      //ESP_LOGD(__FILE__, "LightsData wsBuffer to send: %s. No of ws clients is: %i", (char *)wsBuffer->get(), _ws->count());
      _ws->textAll(wsBuffer);
      _lLastBroadcast = GET_MICROS / 1000;
      _bUpdateLights = false;
      /*uint8_t iId = 0;
      for (auto &isConsumer : _bIsConsumerArray)
      {
         if (isConsumer)
         {
            AsyncWebSocketClient *client = _ws->client(iId);
            if (client && client->status() == WS_CONNECTED)
            {
               //ESP_LOGD(__FILE__, "Ligts update to client %i\r\n", iId);
               client->text(wsBuffer);
            }
         }
         iId++;
      }*/
   }
}

bool WebHandlerClass::_DoAction(JsonObject ActionObj, String *ReturnError, AsyncWebSocketClient *Client)
{
   String ActionType = ActionObj["actionType"];
   if (ActionType == "UpdateRace")
   {
      if (RaceHandler.RaceState == RaceHandler.STOPPED || RaceHandler.RaceState == RaceHandler.RESET)
      {
         _bSendRaceData = true;
         _bUpdateLights = true;
         return true;
      }
      else
      {
         return false;
      }
   }
   if (ActionType == "StartRace")
   {
      if (RaceHandler.RaceState != RaceHandler.RESET)
      {
         //ReturnError = String("Race was not reset, stop and reset it first!");
         return false;
      }
      else
      {
         if (LightsController.bModeNAFA)
            LightsController.WarningStartSequence();
         else
            LightsController.InitiateStartSequence();
         return true;
      }
   }
   else if (ActionType == "StopRace")
   {
      if (RaceHandler.RaceState == RaceHandler.STOPPED || RaceHandler.RaceState == RaceHandler.RESET)
      {
         //ReturnError = "Race was already stopped!";
         _bSendRaceData = true;
         _bUpdateLights = true;
         return false;
      }
      else
      {
         LightsController.DeleteSchedules();
         RaceHandler.StopRace();
         return true;
      }
   }
   else if (ActionType == "ResetRace")
   {
      if (RaceHandler.RaceState != RaceHandler.STOPPED)
      {
         //ReturnError = "Race was not stopped, or already in RESET state.";
         _bSendRaceData = true;
         _bUpdateLights = true;
         return false;
      }
      else
      {
         RaceHandler.ResetRace();
         LightsController.ResetLights();
         return true;
      }
   }
   else if (ActionType == "SetDogFault")
   {
      if (!ActionObj.containsKey("actionData"))
      {
         //ReturnError = "No actionData found!";
         return false;
      }
      uint8_t iDogNum = ActionObj["actionData"]["dogNumber"];
      //bool bFaultState = ActionObj["actionData"]["faultState"];
      //RaceHandler.SetDogFault(iDogNum, (bFaultState ? RaceHandler.ON : RaceHandler.OFF));
      RaceHandler.SetDogFault(iDogNum);
      return true;
   }
   else if (ActionType == "AnnounceConsumer")
   {
      ESP_LOGD(__FILE__, "We have a consumer with ID %i and IP %s", Client->id(), Client->remoteIP().toString().c_str());
      if (!_bIsConsumerArray[Client->id()])
      {
         _iNumOfConsumers++;
      }
      _bIsConsumerArray[Client->id()] = true;
      _bSendRaceData = true;
      _bUpdateLights = true;
      return true;
   }
   else if (ActionType == "SetDogs4")
   {
      if (RaceHandler.RaceState != RaceHandler.RESET)
      {
         _bSendRaceData = true;
         _bUpdateLights = true;
         return false;
      }
      else
      {
         RaceHandler.SetNumberOfDogs(4);
         return true;
      }
   }
   else if (ActionType == "SetDogs3")
   {
      if (RaceHandler.RaceState != RaceHandler.RESET)
      {
         _bSendRaceData = true;
         _bUpdateLights = true;
         return false;
      }
      else
      {
         RaceHandler.SetNumberOfDogs(3);
         return true;
      }
   }
   else if (ActionType == "SetDogs2")
   {
      if (RaceHandler.RaceState != RaceHandler.RESET)
      {
         _bSendRaceData = true;
         _bUpdateLights = true;
         return false;
      }
      else
      {
         RaceHandler.SetNumberOfDogs(2);
         return true;
      }
   }
   else if (ActionType == "SetDogs1")
   {
      if (RaceHandler.RaceState != RaceHandler.RESET)
      {
         _bSendRaceData = true;
         _bUpdateLights = true;
         return false;
      }
      else
      {
         RaceHandler.SetNumberOfDogs(1);
         return true;
      }
   }
   else if (ActionType == "SetRerunsOff")
   {
      if (!ActionObj.containsKey("actionData") || (RaceHandler.RaceState != RaceHandler.RESET))
      {
         //ReturnError = "No actionData found!";
         return false;
      }
      bool _bRerunsOff = ActionObj["actionData"]["rerunsOff"];
      //ESP_LOGD(__FILE__, "Received rerunsOff: %o", _bRerunsOff);
      if (_bRerunsOff)
         RaceHandler.ToggleRerunsOffOn(1);
      else
         RaceHandler.ToggleRerunsOffOn(0);
      return true;
   }
   else
   {
      //ReturnError = "Unknown action received!";
      ESP_LOGD(__FILE__, "Unknown action received: %s", ActionType.c_str());
      return false;
   }
}

void WebHandlerClass::_SendRaceData(int iRaceId, int8_t iClientId)
{
   if (_iNumOfConsumers == 0)
   {
      return;
   }
   else
   {
      StaticJsonDocument<bsRaceData> JsonRaceDataDoc;
      JsonObject JsonRoot = JsonRaceDataDoc.to<JsonObject>();
      JsonObject JsonRaceData = JsonRoot.createNestedObject("RaceData");

      stRaceData RequestedRaceData = RaceHandler.GetRaceData();
      JsonRaceData["id"] = RequestedRaceData.Id;
      JsonRaceData["startTime"] = RequestedRaceData.StartTime;
      JsonRaceData["endTime"] = RequestedRaceData.EndTime;
      JsonRaceData["elapsedTime"] = RequestedRaceData.ElapsedTime;
      JsonRaceData["NetTime"] = RequestedRaceData.NetTime;
      JsonRaceData["raceState"] = RequestedRaceData.RaceState;
      JsonRaceData["racingDogs"] = RequestedRaceData.RacingDogs;
      JsonRaceData["rerunsOff"] = RequestedRaceData.RerunsOff;

      JsonArray JsonDogDataArray = JsonRaceData.createNestedArray("dogData");
      for (uint8_t i = 0; i < RaceHandler.iNumberOfRacingDogs; i++)
      {
         JsonObject JsonDogData = JsonDogDataArray.createNestedObject();
         JsonDogData["dogNumber"] = RequestedRaceData.DogData[i].DogNumber;
         JsonArray JsonDogDataTimingArray = JsonDogData.createNestedArray("timing");
         char cForJson[9];
         for (uint8_t i2 = 0; i2 <= RaceHandler.iDogRunCounters[i]; i2++)
         {
            JsonObject DogTiming = JsonDogDataTimingArray.createNestedObject();
            RequestedRaceData.DogData[i].Timing[i2].Time.toCharArray(cForJson, 9);
            DogTiming["time"] = cForJson;
            RequestedRaceData.DogData[i].Timing[i2].CrossingTime.toCharArray(cForJson, 9);
            DogTiming["crossingTime"] = cForJson;
         }
         JsonDogData["fault"] = RequestedRaceData.DogData[i].Fault;
         JsonDogData["running"] = RequestedRaceData.DogData[i].Running;
      }

      size_t len = measureJson(JsonRaceDataDoc);
      AsyncWebSocketMessageBuffer *wsBuffer = _ws->makeBuffer(len);
      if (wsBuffer)
      {
         serializeJson(JsonRaceDataDoc, (char *)wsBuffer->get(), len + 1);
         //ESP_LOGD(__FILE__, "RaceData wsBuffer to send: %s", (char *)wsBuffer->get());
         if (iClientId == -1)
         {
            _ws->textAll(wsBuffer);
            /*uint8_t iId = 0;
            for (auto &isConsumer : _bIsConsumerArray)
            {
               if (isConsumer)
               {
                  //ESP_LOGD(__FILE__, "Getting client obj for id %i\r\n", iId);
                  AsyncWebSocketClient *client = _ws->client(iId);
                  if (client && client->status() == WS_CONNECTED)
                  {
                     //ESP_LOGD(__FILE__, "Generic update. Sending to client %i\r\n", iId);
                     client->text(wsBuffer);
                  }
               }
               iId++;
            }*/
         }
         else
         {
            //ESP_LOGD(__FILE__, "Specific update. Sending to client %i\r\n", iClientId);
            AsyncWebSocketClient *client = _ws->client(iClientId);
            client->text(wsBuffer);
         }
         _lLastRaceDataBroadcast = _lLastBroadcast = GET_MICROS / 1000;
         _bSendRaceData = false;
      }
      
   }
}

bool WebHandlerClass::_ProcessConfig(JsonArray newConfig, String *ReturnError)
{
   bool save = false;
   ESP_LOGD(__FILE__, "Config has %i elements\r\n", newConfig.size());
   for (unsigned int i = 0; i < newConfig.size(); i++)
   {
      String key = newConfig[i]["name"];
      String value = newConfig[i]["value"];

      if (value != SettingsManager.getSetting(key))
      {
         ESP_LOGD(__FILE__, "[WEBHANDLER] Storing %s = %s", key.c_str(), value.c_str());
         SettingsManager.setSetting(key, value);
         save = true;
      }
   }

   if (save)
   {
      SettingsManager.saveSettings();
      //Schedule system reboot to activate new settings in 5s
      //SystemManager.scheduleReboot(GET_MICROS / 1000 + 5000);
   }

   return true;
}

bool WebHandlerClass::_GetData(String dataType, JsonObject Data)
{
   if (dataType == "config")
   {
      Data["APName"] = SettingsManager.getSetting("APName");
      Data["APPass"] = SettingsManager.getSetting("APPass");
      Data["AdminPass"] = SettingsManager.getSetting("AdminPass");
      Data["RunDirectionInverted"] = SettingsManager.getSetting("RunDirectionInverted");
      Data["StartingSequenceNAFA"] = SettingsManager.getSetting("StartingSequenceNAFA");
      Data["LaserOnTimer"] = SettingsManager.getSetting("LaserOnTimer");
      //ESP_LOGD(__FILE__, "AdminPass: %s, RunDirection: %s", SettingsManager.getSetting("AdminPass").c_str(), SettingsManager.getSetting("RunDirectionInverted"));
   }
   else if (dataType == "triggerQueue")
   {
      JsonArray triggerQueue = Data.createNestedArray("triggerQueue");

      for (auto &trigger : RaceHandler._OutputTriggerQueue)
      {
         JsonObject triggerObj = triggerQueue.createNestedObject();
         triggerObj["sensorNum"] = trigger.iSensorNumber;
         triggerObj["triggerTime"] = trigger.llTriggerTime - RaceHandler.llRaceStartTime;
         triggerObj["state"] = trigger.iSensorState;
      }
   }
   else
   {
      Data["error"] = "Unknown datatype (" + dataType + ") requested!";
      return false;
   }

   return true;
}

void WebHandlerClass::_SendSystemData(int8_t iClientId)
{
   if (_iNumOfConsumers == 0)
   {
      return;
   }
   else
   {
      _SystemData.FreeHeap = esp_get_free_heap_size();
      _SystemData.RaceID = RaceHandler.iCurrentRaceId + 1;
      _SystemData.Uptime = GET_MICROS / 1000;
      _SystemData.NumClients = _ws->count();
      _SystemData.LocalSystemTime =  (char*) GPSHandler.GetUtcDateAndTime();
      _SystemData.BatteryPercentage = BatterySensor.GetBatteryPercentage();
      
      StaticJsonDocument<192> JsonSystemDataDoc;
      JsonObject JsonRoot = JsonSystemDataDoc.to<JsonObject>();

      JsonObject JsonSystemData = JsonRoot.createNestedObject("SystemData");
      JsonSystemData["uptime"] = _SystemData.Uptime;
      JsonSystemData["freeHeap"] = _SystemData.FreeHeap;
      JsonSystemData["PwrOnTag"] = _SystemData.PwrOnTag;
      JsonSystemData["RaceID"] = _SystemData.RaceID;
      JsonSystemData["numClients"] = _SystemData.NumClients;
      JsonSystemData["systemTimestamp"] = _SystemData.LocalSystemTime;
      JsonSystemData["batteryPercentage"] = _SystemData.BatteryPercentage;

      size_t len = measureJson(JsonSystemDataDoc);   
      AsyncWebSocketMessageBuffer *wsBuffer = _ws->makeBuffer(len);
      if (wsBuffer)
      {
         serializeJson(JsonSystemDataDoc, (char *)wsBuffer->get(), len + 1);
         if (iClientId == -1)
         {
            _ws->textAll(wsBuffer);
            /*uint8_t iId = 0;
            for (auto &isConsumer : _bIsConsumerArray)
            {
               if (isConsumer)
               {
                  //ESP_LOGD(__FILE__, "Getting client obj for id %i\r\n", iId);
                  AsyncWebSocketClient *client = _ws->client(iId);
                  if (client && client->status() == WS_CONNECTED)
                  {
                     //ESP_LOGD(__FILE__, "Generic update. Sending to client %i\r\n", iId);
                     client->text(wsBuffer);
                  }
               }
               iId++;
            }*/
         }
         else
         {
            //ESP_LOGD(__FILE__, "Specific update. Sending to client %i\r\n", iClientId);
            AsyncWebSocketClient *client = _ws->client(iClientId);
            client->text(wsBuffer);
         }
         _lLastSystemDataBroadcast = _lLastBroadcast = GET_MICROS / 1000;
         //ESP_LOGD(__FILE__, "Sent sysdata at %lu\r\n", GET_MICROS / 1000);
      }
   }
}

void WebHandlerClass::_onAuth(AsyncWebServerRequest *request)
{
   if (!_authenticate(request))
      return request->requestAuthentication("", false);
   IPAddress ip = request->client()->remoteIP();
   unsigned long now = GET_MICROS / 1000;
   unsigned short index;
   for (index = 0; index < WS_TICKET_BUFFER_SIZE; index++)
   {
      if (_ticket[index].ip == ip)
         break;
      if (_ticket[index].timestamp == 0)
         break;
      if (now - _ticket[index].timestamp > WS_TIMEOUT)
         break;
   }
   if (index == WS_TICKET_BUFFER_SIZE)
   {
      request->send(429);
   }
   else
   {
      _ticket[index].ip = ip;
      _ticket[index].timestamp = now;
      request->send(204);
   }
}

bool WebHandlerClass::_authenticate(AsyncWebServerRequest *request)
{
   String password = SettingsManager.getSetting("AdminPass");
   char httpPassword[password.length() + 1];
   password.toCharArray(httpPassword, password.length() + 1);
   bool bAuthResult = request->authenticate("Admin", httpPassword);
   if (!bAuthResult)
   {
      ESP_LOGE(__FILE__, "[WEBHANDLER] Admin user failed to login!");
   }
   return bAuthResult;
}

bool WebHandlerClass::_wsAuth(AsyncWebSocketClient *client)
{
   IPAddress ip = client->remoteIP();
   unsigned short index = 0;

   //TODO: Here be dragons, this way of 'authenticating' is all but secure
   //We are just storing the client's IP and an expiration timestamp of now + 30 mins
   //So if someone logs in, then disconnects from the WiFi, and then someone else connects
   //this new user will/could have the same IP as the previous user, and will be authenticated :(

   for (index = 0; index < WS_TICKET_BUFFER_SIZE; index++)
   {
      ESP_LOGD(__FILE__, "Checking ticket: %i, ip: %s, time: %ul", index, _ticket[index].ip.toString().c_str(), _ticket[index].timestamp);
      if ((_ticket[index].ip == ip) && (GET_MICROS / 1000 - _ticket[index].timestamp < WS_TIMEOUT))
         break;
   }

   if (index == WS_TICKET_BUFFER_SIZE)
   {
      ESP_LOGE(__FILE__, "[WEBSOCKET] Validation check failed\n");
      client->text("{\"success\": false, \"error\": \"You shall not pass!!!! Please authenticate first :-)\", \"authenticated\": false}");
      _lLastBroadcast = GET_MICROS / 1000;
      return false;
   }

   return true;
}

void WebHandlerClass::_onHome(AsyncWebServerRequest *request)
{

   //if (!_authenticate(request)) return request->requestAuthentication(getSetting("hostname").c_str());
   if (request->header("If-Modified-Since").equals(_last_modified))
   {
      request->send(304);
   }
   else
   {
      #ifndef WebUIonSDcard
      AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html_gz, index_html_gz_len);
      response->addHeader("Content-Encoding", "gzip");
      #else
      AsyncWebServerResponse *response = request->beginResponse(SD_MMC, "/index.htm", "text/html");
      #endif
      response->addHeader("Last-Modified", _last_modified);
      request->send(response);
   }
}


void WebHandlerClass::_onFavicon(AsyncWebServerRequest *request)
{
   #ifndef WebUIonSDcard
   AsyncWebServerResponse *response = request->beginResponse_P(200, "image/png", index_html_gz, index_html_gz_len);
   response->addHeader("Content-Encoding", "gzip");
   #else
   AsyncWebServerResponse *response = request->beginResponse(SD_MMC, "/favicon.ico", "image/png");
   #endif
   request->send(response);
}
WebHandlerClass WebHandler;
