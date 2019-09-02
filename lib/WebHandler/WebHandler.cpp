//
//
//

#include "WebHandler.h"

void WebHandlerClass::_WsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
   //FIXME: The next line works but introduces an unnecessary String. The 2nd line does not work, I don't know why...
   boolean isAdmin = String("/wsa").equals(server->url());
   //boolean isAdmin = (server->url() == "/wsa");

   if (type == WS_EVT_CONNECT)
   {
      if (client->remoteIP().toString().equals("192.168.20.1"))
      {
         //This client is the master!
         _MasterStatus.client = _ws->client(client->id());
         _MasterStatus.ClientID = client->id();
         _MasterStatus.ip = client->remoteIP();
         _MasterStatus.Configured = true;
         _MasterStatus.LastReply = millis();
         //_MasterStatus.LastCheck = millis();
      }
      ESP_LOGI(__FILE__, "Client %i connected to %s!", client->id(), server->url());

      //Should we check authentication?
      if (isAdmin)
      {
         if (!_wsAuth(client))
         {
            client->close(401, "Unauthorized");
            return;
         }
      }

      //TODO: Would be nicer if we only send this to the specific client who just connected instead of all clients
      _SendRaceData(); //Make sure we always broadcast racedata when new client connects
      client->ping();
   }
   else if (type == WS_EVT_DISCONNECT)
   {
      if (client->id() == _MasterStatus.ClientID)
      {
         ESP_LOGI(__FILE__, "Disconnecting master due to disconnect event from ws\r\n");
         _DisconnectMaster();
      }
      if (_bIsConsumerArray[client->id()])
      {
         _iNumOfConsumers--;
         _bIsConsumerArray[client->id()] = false;
      }

      ESP_LOGI(__FILE__, "Client %u disconnected!\n", client->id());
   }
   else if (type == WS_EVT_ERROR)
   {
      ESP_LOGI(__FILE__, "ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
   }
   else if (type == WS_EVT_PONG)
   {
      if (client->id() == _MasterStatus.ClientID)
      {
         _MasterStatus.LastReply = millis();
         ESP_LOGD(__FILE__, "Pong received from master (%lu)!\r\n", millis());
      }
      ESP_LOGI(__FILE__, "ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
   }
   else if (type == WS_EVT_DATA)
   {
      AwsFrameInfo *info = (AwsFrameInfo *)arg;
      String msg = "";
      if (info->final && info->index == 0 && info->len == len)
      {
         //the whole message is in a single frame and we got all of it's data
         ESP_LOGI(__FILE__, "ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);

         if (info->opcode == WS_TEXT)
         {
            for (size_t i = 0; i < info->len; i++)
            {
               msg += (char)data[i];
            }
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
         ESP_LOGI(__FILE__, "%s\n", msg.c_str());
      }
      else
      {
         //message is comprised of multiple frames or the frame is split into multiple packets
         if (info->index == 0)
         {
            if (info->num == 0)
               ESP_LOGI(__FILE__, "ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
            ESP_LOGI(__FILE__, "ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
         }

         ESP_LOGI(__FILE__, "ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT) ? "text" : "binary", info->index, info->index + len);

         if (info->opcode == WS_TEXT)
         {
            for (size_t i = 0; i < info->len; i++)
            {
               msg += (char)data[i];
            }
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
         ESP_LOGI(__FILE__, "%s\n", msg.c_str());

         if ((info->index + len) == info->len)
         {
            ESP_LOGI(__FILE__, "ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
            if (info->final)
            {
               ESP_LOGI(__FILE__, "ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
            }
         }
      }

      // Parse JSON input
      //DynamicJsonBuffer jsonBufferRequest;
      StaticJsonDocument<bsActionScheduleStartRace> jsonRequestDoc;
      DeserializationError error = deserializeJson(jsonRequestDoc, msg);
      JsonObject request = jsonRequestDoc.as<JsonObject>();
      if (error)
      {
         ESP_LOGE(__FILE__, "Error parsing JSON: %s", error.c_str());
         //wsSend_P(client->id(), PSTR("{\"message\": 3}"));
         client->text("{\"error\":\"Invalid JSON received\"}");
         return;
      }

      const size_t bufferSize = JSON_ARRAY_SIZE(50) + 50 * JSON_OBJECT_SIZE(3);
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
         ESP_LOGI(__FILE__, "Got valid JSON but unknown message!");
         JsonResponseRoot["error"] = "Got valid JSON but unknown message!";
      }

      size_t len = measureJson(jsonResponseDoc);
      AsyncWebSocketMessageBuffer *wsBuffer = _ws->makeBuffer(len);
      if (wsBuffer)
      {
         serializeJson(jsonResponseDoc, (char *)wsBuffer->get(), len + 1);
         client->text(wsBuffer);
      }
   }
}

void WebHandlerClass::init(int webPort)
{
   snprintf_P(_last_modified, sizeof(_last_modified), PSTR("%s %s GMT"), __DATE__, __TIME__);
   _bSlavePresent = false;
   _server = new AsyncWebServer(webPort);
   _ws = new AsyncWebSocket("/ws");
   _wsa = new AsyncWebSocket("/wsa");

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

   // Serve home (basic authentication protection)
   _server->on("/index.html", HTTP_GET, std::bind(&WebHandlerClass::_onHome, this, std::placeholders::_1));

   //Authentication handler
   _server->on("/auth", HTTP_GET, std::bind(&WebHandlerClass::_onAuth, this, std::placeholders::_1));

   _server->begin();

   _lLastRaceDataBroadcast = 0;
   _lRaceDataBroadcastInterval = 750;

   _lLastSystemDataBroadcast = 0;
   _lSystemDataBroadcastInterval = 2000;

   _SystemData.CPU0ResetReason = rtc_get_reset_reason(0);
   _SystemData.CPU1ResetReason = rtc_get_reset_reason(1);

   for (boolean &bIsConsumer : _bIsConsumerArray)
   {
      bIsConsumer = false;
   }

   _iNumOfConsumers = 0;
}

void WebHandlerClass::loop()
{
   //When race is starting or running, or time > 0 (stopped but not reset)
   if ((RaceHandler.RaceState != RaceHandler.STOPPED || RaceHandler.GetRaceTime() > 0))
   {
      //Send race data each 200ms
      if (millis() - _lLastRaceDataBroadcast > _lRaceDataBroadcastInterval)
      {
         _SendRaceData();
         _lLastRaceDataBroadcast = millis();
      }
   }
   if (millis() - _lLastSystemDataBroadcast > _lSystemDataBroadcastInterval)
   {
      _lLastSystemDataBroadcast = millis();
      _GetSystemData();
      _SendSystemData();

      ESP_LOGD(__FILE__, "Have %i clients, %i consumers\r\n", _ws->count(), _iNumOfConsumers);
   }

   _CheckMasterStatus();
}

void WebHandlerClass::SendLightsData(stLightsState LightStates)
{
   StaticJsonDocument<bsLightsData> JsonDoc;
   JsonObject JsonRoot = JsonDoc.to<JsonObject>();

   JsonArray JsonLightsData = JsonRoot.createNestedArray("LightsData");
   copyArray(LightStates.State, JsonLightsData);

   size_t len = measureJson(JsonDoc);
   AsyncWebSocketMessageBuffer *wsBuffer = _ws->makeBuffer(len);
   if (wsBuffer)
   {
      serializeJson(JsonDoc, (char *)wsBuffer->get(), len + 1);
      for (uint8_t i = 0; i < _ws->count(); i++)
      {
         if (_bIsConsumerArray[i])
         {
            _ws->text(i, (char *)wsBuffer);
         }
      }
   }
}

boolean WebHandlerClass::_DoAction(JsonObject ActionObj, String *ReturnError, AsyncWebSocketClient *Client)
{
   String ActionType = ActionObj["actionType"];
   if (ActionType == "StartRace")
   {
      if (RaceHandler.RaceState != RaceHandler.STOPPED)
      {
         //ReturnError = String("Race was not stopped, stop it first!");
         return false;
      }
      else if (RaceHandler.GetRaceTime() != 0)
      {
         //ReturnError = "Race was not reset, reset race first!";
         return false;
      }
      else
      {
         if (_bSlavePresent)
         {
            unsigned long ulStartTime = GPSHandler.GetEpochTime() + 2;

            StaticJsonDocument<bsActionScheduleStartRace> jsonDoc;

            JsonObject root = jsonDoc.to<JsonObject>();

            JsonObject action = root.createNestedObject("action");
            action["actionType"] = "ScheduleStartRace";
            JsonObject action_actionData = action.createNestedObject("actionData");
            action_actionData["startTime"] = ulStartTime;

            String strScheduleStartMessage;
            serializeJson(root, strScheduleStartMessage);
            SlaveHandler.sendToSlave(strScheduleStartMessage);

            long lMillisToStart = GPSHandler.GetMillisToEpochSecond(ulStartTime);
            if (lMillisToStart < 0)
            {
               return false;
            }
            RaceHandler.StartRace(lMillisToStart + millis());
         }
         else
         {
            RaceHandler.StartRace();
         }
         return true;
      }
   }
   else if (ActionType == "StopRace")
   {
      if (RaceHandler.RaceState == RaceHandler.STOPPED)
      {
         //ReturnError = "Race was already stopped!";
         return false;
      }
      else
      {
         RaceHandler.StopRace();
         LightsController.DeleteSchedules();
         if (_bSlavePresent)
         {
            SlaveHandler.sendToSlave("{ \"action\": {\"actionType\": \"StopRace\"}}");
         }
         return true;
      }
   }
   else if (ActionType == "ResetRace")
   {
      if (RaceHandler.RaceState != RaceHandler.STOPPED)
      {
         //ReturnError = "Race was not stopped, first stop it before resetting!";
         return false;
      }
      else if (RaceHandler._lRaceStartTime == 0)
      {
         //ReturnError = "Race was already reset!";
         return false;
      }
      else
      {
         LightsController.ResetLights();
         RaceHandler.ResetRace();
         if (_bSlavePresent)
         {
            SlaveHandler.sendToSlave("{ \"action\": {\"actionType\": \"ResetRace\"}}");
         }
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
      boolean bFaultState = ActionObj["actionData"]["faultState"];
      RaceHandler.SetDogFault(iDogNum, (bFaultState ? RaceHandler.ON : RaceHandler.OFF));
      return true;
   }
   else if (ActionType == "ScheduleStartRace") //ScheduleStartRace action is used to schedule a race start on 2 lanes. This command would typically be sent by the master to the slave
   {
      //Check if race is stopped. Instead of returning error if it isn't, we just stop and reset it.
      if (RaceHandler.RaceState != RaceHandler.STOPPED)
      {
         RaceHandler.StopRace(micros());
         RaceHandler.ResetRace();
         LightsController.ResetLights();
      }

      String StartTime = ActionObj["actionData"]["startTime"];

      unsigned long lStartEpochTime = StartTime.toInt();
      ESP_LOGD(__FILE__, "StartTime S: %s, UL: %lu\r\n", StartTime.c_str(), lStartEpochTime);
      long lMillisToStart = GPSHandler.GetMillisToEpochSecond(lStartEpochTime);

      ESP_LOGI(__FILE__, "Received request to schedule race to start at %lu s which is in %ld ms", lStartEpochTime, lMillisToStart);

      if (lMillisToStart < 0)
      {
         //ReturnError = "Requested starttime is in the past!";
         ESP_LOGI(__FILE__, "Race schedule received for the past (%ld ms)!", lMillisToStart);
         return false;
      }

      RaceHandler.StartRace(lMillisToStart + millis());
      return true;
   }
   else if (ActionType == "AnnounceSlave")
   {
      ESP_LOGI(__FILE__, "We have a slave with IP %s", Client->remoteIP().toString().c_str());
      SlaveHandler.configureSlave(Client->remoteIP());
      _bSlavePresent = true;
      return true;
   }

   else if (ActionType == "AnnounceConsumer")
   {
      ESP_LOGI(__FILE__, "We have a consumer with ID %i and IP %s", Client->id(), Client->remoteIP().toString().c_str());
      if (!_bIsConsumerArray[Client->id()])
      {
         _iNumOfConsumers++;
      }
      _bIsConsumerArray[Client->id()] = true;
      _SendRaceData(RaceHandler._iCurrentRaceId, Client->id());
      return true;
   }
   else
   {
      //ReturnError = "Unknown action received!";
      return false;
   }
}

boolean WebHandlerClass::_GetRaceDataJsonString(uint iRaceId, String &strJsonString)
{
   StaticJsonDocument<bsRaceData> JsonDoc;
   JsonObject JsonRoot = JsonDoc.to<JsonObject>();
   JsonObject JsonRaceData = JsonRoot.createNestedObject("RaceData");
   stRaceData RequestedRaceData = RaceHandler.GetRaceData();
   JsonRaceData["id"] = RequestedRaceData.Id;
   JsonRaceData["startTime"] = RequestedRaceData.StartTime;
   JsonRaceData["endTime"] = RequestedRaceData.EndTime;
   JsonRaceData["elapsedTime"] = RequestedRaceData.ElapsedTime;
   JsonRaceData["totalCrossingTime"] = RequestedRaceData.TotalCrossingTime;
   JsonRaceData["raceState"] = RequestedRaceData.RaceState;

   JsonArray JsonDogDataArray = JsonRaceData.createNestedArray("dogData");
   for (uint8_t i = 0; i < 4; i++)
   {
      JsonObject JsonDogData = JsonDogDataArray.createNestedObject();
      JsonDogData["dogNumber"] = RequestedRaceData.DogData[i].DogNumber;
      JsonArray JsonDogDataTimingArray = JsonDogData.createNestedArray("timing");
      for (uint8_t i2 = 0; i2 < 4; i2++)
      {
         JsonObject DogTiming = JsonDogDataTimingArray.createNestedObject();
         DogTiming["time"] = RequestedRaceData.DogData[i].Timing[i2].Time;
         DogTiming["crossingTime"] = RequestedRaceData.DogData[i].Timing[i2].CrossingTime;
      }
      JsonDogData["fault"] = RequestedRaceData.DogData[i].Fault;
      JsonDogData["running"] = RequestedRaceData.DogData[i].Running;
   }

   serializeJson(JsonRaceData, strJsonString);

   return true;
}

void WebHandlerClass::_SendRaceData(uint iRaceId, int8_t iClientId)
{
   if (_iNumOfConsumers == 0)
   {
      return;
   }
   StaticJsonDocument<bsRaceDataArray> JsonDoc;
   JsonObject JsonRoot = JsonDoc.to<JsonObject>();
   JsonArray jsonRaceData = JsonRoot.createNestedArray("RaceData");

   StaticJsonDocument<bsRaceData> jsonMasterRaceDataDoc;
   JsonObject jsonMasterRaceData = jsonMasterRaceDataDoc.to<JsonObject>();

   StaticJsonDocument<bsRaceData> jsonSlaveRaceDataDoc;
   JsonObject jsonSlaveRaceData = jsonSlaveRaceDataDoc.to<JsonObject>();

   stRaceData RequestedRaceData = RaceHandler.GetRaceData();
   jsonMasterRaceData["id"] = RequestedRaceData.Id;
   jsonMasterRaceData["startTime"] = RequestedRaceData.StartTime;
   jsonMasterRaceData["endTime"] = RequestedRaceData.EndTime;
   jsonMasterRaceData["elapsedTime"] = RequestedRaceData.ElapsedTime;
   jsonMasterRaceData["totalCrossingTime"] = RequestedRaceData.TotalCrossingTime;
   jsonMasterRaceData["raceState"] = RequestedRaceData.RaceState;

   JsonArray JsonDogDataArray = jsonMasterRaceData.createNestedArray("dogData");
   for (uint8_t i = 0; i < 4; i++)
   {
      JsonObject JsonDogData = JsonDogDataArray.createNestedObject();
      JsonDogData["dogNumber"] = RequestedRaceData.DogData[i].DogNumber;
      JsonArray JsonDogDataTimingArray = JsonDogData.createNestedArray("timing");
      for (uint8_t i2 = 0; i2 < 4; i2++)
      {
         JsonObject DogTiming = JsonDogDataTimingArray.createNestedObject();
         DogTiming["time"] = RequestedRaceData.DogData[i].Timing[i2].Time;
         DogTiming["crossingTime"] = RequestedRaceData.DogData[i].Timing[i2].CrossingTime;
      }
      JsonDogData["fault"] = RequestedRaceData.DogData[i].Fault;
      JsonDogData["running"] = RequestedRaceData.DogData[i].Running;
   }

   jsonRaceData.add(jsonMasterRaceData);
   ESP_LOGD(__FILE__, "[WEBHANDLER]: Collected own racedata, length: %i\r\n", measureJson(jsonRaceData));

   if (_bSlavePresent)
   {
      ESP_LOGD(__FILE__, "[WEBHANDLER]: Requesting slave racedata...\r\n");
#define USE_STRING

#ifdef USE_STRING
      String strJsonSlaveRaceData = SlaveHandler.getSlaveRaceData();
      ESP_LOGD(__FILE__, "Slave racedata: %s\r\n", strJsonSlaveRaceData.c_str());
      DeserializationError error = deserializeJson(jsonSlaveRaceDataDoc, strJsonSlaveRaceData);
      //char * strJsonSlaveRaceData = SlaveHandler.getSlaveRaceData2();
      //ESP_LOGD(__FILE__, "Slave racedata: %s, length: %i\r\n", strJsonSlaveRaceData, strlen(strJsonSlaveRaceData));
      //DeserializationError error = deserializeJson(jsonSlaveRaceDataDoc, strJsonSlaveRaceData, strlen(strJsonSlaveRaceData));
      if (error)
      {
         ESP_LOGD(__FILE__, "Error parsing json data from slave: %s\r\n", error.c_str());
      }
      jsonSlaveRaceData = jsonSlaveRaceDataDoc.as<JsonObject>();
#else
      JsonObject jsonSlaveRaceData = SlaveHandler.getSlaveRaceData1();

#endif // USE_STRING

      String strJsonRaceDataTest;
      serializeJson(jsonSlaveRaceData, strJsonRaceDataTest);
      ESP_LOGD(__FILE__, "Got json slave racedata: %s, length: %i\r\n", strJsonRaceDataTest.c_str(), measureJson(jsonSlaveRaceData));

      if (measureJson(jsonSlaveRaceData) > 2)
      {
         jsonRaceData.add(jsonSlaveRaceData);
      }
      else
      {
         ESP_LOGI(__FILE__, "Got invalid slave racedata (length: %i)", measureJson(jsonSlaveRaceData));
      }
   }

   /*
   size_t len = measureJson(JsonDoc);
   AsyncWebSocketMessageBuffer * wsBuffer = _ws->makeBuffer(len);
   if (wsBuffer)
   {
      serializeJson(JsonDoc, (char *)wsBuffer->get(), len + 1);
      if (iClientId == -1) {
         for (uint8_t i = 0; i < _ws->count(); i++) {
            if (_bIsConsumerArray[i]) {
               _ws->text(i, (char*)wsBuffer);
            }
         }
      }
      else {
         _ws->text(iClientId, wsBuffer);
      }
   } 
   */
   String strJsonRaceData;
   serializeJson(JsonDoc, strJsonRaceData);
   ESP_LOGD(__FILE__, "Sending back RD array: %s\r\n", strJsonRaceData.c_str());
   if (iClientId == -1)
   {

      uint8_t iId = 0;
      for (auto &isConsumer : _bIsConsumerArray)
      {
         if (isConsumer)
         {
            ESP_LOGD(__FILE__, "Getting client obj for id %i\r\n", iId);
            AsyncWebSocketClient *client = _ws->client(iId);
            if (client && client->status() == WS_CONNECTED)
            {
               ESP_LOGD(__FILE__, "Sending to client %i\r\n", iId);
               client->text(strJsonRaceData);
            }
         }
         iId++;
      }
   }
   else
   {
      ESP_LOGD(__FILE__, "Sending to client %i\r\n", iClientId);
      _ws->text(iClientId, strJsonRaceData);
   }
}

boolean WebHandlerClass::_ProcessConfig(JsonArray newConfig, String *ReturnError)
{
   bool save = false;
   bool changed = false;
   ESP_LOGD(__FILE__, "config is %i big\r\n", newConfig.size());
   for (unsigned int i = 0; i < newConfig.size(); i++)
   {
      String key = newConfig[i]["name"];
      String value = newConfig[i]["value"];

      if (value != SettingsManager.getSetting(key))
      {
         ESP_LOGD(__FILE__, "[WEBHANDLER] Storing %s = %s", key.c_str(), value.c_str());
         SettingsManager.setSetting(key, value);
         save = changed = true;
      }
   }

   if (save)
   {
      SettingsManager.saveSettings();
      //Schedule system reboot to activate new settings in 5s
      SystemManager.scheduleReboot(millis() + 5000);
   }

   return true;
}

boolean WebHandlerClass::_GetData(String dataType, JsonObject Data)
{
   if (dataType == "config")
   {
      Data["APName"] = SettingsManager.getSetting("APName");
      Data["APPass"] = SettingsManager.getSetting("APPass");
      Data["UserPass"] = SettingsManager.getSetting("UserPass");
      Data["AdminPass"] = SettingsManager.getSetting("AdminPass");
   }
   else if (dataType == "triggerQueue")
   {
      JsonArray triggerQueue = Data.createNestedArray("triggerQueue");

      for (auto &trigger : RaceHandler._STriggerQueue)
      {
         JsonObject triggerObj = triggerQueue.createNestedObject();
         triggerObj["sensorNum"] = trigger.iSensorNumber;
         triggerObj["triggerTime"] = trigger.lTriggerTime - RaceHandler._lRaceStartTime;
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

void WebHandlerClass::_GetSystemData()
{
   _SystemData.FreeHeap = esp_get_free_heap_size();
   _SystemData.Uptime = millis();
   _SystemData.NumClients = _ws->count();
   _SystemData.UTCSystemTime = GPSHandler.GetUTCTimestamp();
   _SystemData.BatteryPercentage = BatterySensor.GetBatteryPercentage();
}

void WebHandlerClass::_SendSystemData(int8_t iClientId)
{
   if (_iNumOfConsumers == 0)
   {
      return;
   }
   const size_t bufferSize = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(7) + 170;
   StaticJsonDocument<bufferSize> JsonDoc;
   JsonObject JsonRoot = JsonDoc.to<JsonObject>();

   JsonObject JsonSystemData = JsonRoot.createNestedObject("SystemData");
   JsonSystemData["uptime"] = _SystemData.Uptime;
   JsonSystemData["freeHeap"] = _SystemData.FreeHeap;
   JsonSystemData["CPU0ResetReason"] = (int)_SystemData.CPU0ResetReason;
   JsonSystemData["CPU1ResetReason"] = (int)_SystemData.CPU1ResetReason;
   JsonSystemData["numClients"] = _SystemData.NumClients;
   JsonSystemData["systemTimestamp"] = _SystemData.UTCSystemTime;
   JsonSystemData["batteryPercentage"] = _SystemData.BatteryPercentage;

   String strJsonSystemData;
   serializeJson(JsonDoc, strJsonSystemData);

   if (iClientId == -1)
   {

      uint8_t iId = 0;
      for (auto &isConsumer : _bIsConsumerArray)
      {
         if (isConsumer)
         {
            ESP_LOGD(__FILE__, "Getting client obj for id %i\r\n", iId);
            AsyncWebSocketClient *client = _ws->client(iId);
            if (client && client->status() == WS_CONNECTED)
            {
               ESP_LOGD(__FILE__, "Sending to client %i\r\n", iId);
               client->text(strJsonSystemData);
            }
         }
         iId++;
      }
   }
   else
   {
      ESP_LOGD(__FILE__, "Sending to client %i\r\n", iClientId);
      _ws->text(iClientId, strJsonSystemData);
   }
   ESP_LOGD(__FILE__, "Sent sysdata at %lu\r\n", millis());
}

void WebHandlerClass::_onAuth(AsyncWebServerRequest *request)
{
   if (!_authenticate(request))
      return request->requestAuthentication("", false);
   IPAddress ip = request->client()->remoteIP();
   unsigned long now = millis();
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
   boolean bAuthResult = request->authenticate("Admin", httpPassword);
   if (!bAuthResult)
   {
      ESP_LOGE(__FILE__, "Admin user failed to login!");
   }
   return bAuthResult;
}

bool WebHandlerClass::_wsAuth(AsyncWebSocketClient *client)
{

   IPAddress ip = client->remoteIP();
   unsigned long now = millis();
   unsigned short index = 0;

   //TODO: Here be dragons, this way of 'authenticating' is all but secure
   //We are just storing the client's IP and an expiration timestamp of now + 30 mins
   //So if someone logs in, then disconnects from the WiFi, and then someone else connects
   //this new user will/could have the same IP as the previous user, and will be authenticated :(

   for (index = 0; index < WS_TICKET_BUFFER_SIZE; index++)
   {
      ESP_LOGI(__FILE__, "Checking ticket: %i, ip: %s, time: %lu", index, _ticket[index].ip.toString().c_str(), _ticket[index].timestamp);
      if ((_ticket[index].ip == ip) && (now - _ticket[index].timestamp < WS_TIMEOUT))
         break;
   }

   if (index == WS_TICKET_BUFFER_SIZE)
   {
      ESP_LOGI(__FILE__, "[WEBSOCKET] Validation check failed\n");
      client->text("{\"success\": false, \"error\": \"You shall not pass!!!! Please authenticate first :-)\", \"authenticated\": false}");
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
      AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html_gz, index_html_gz_len);
      response->addHeader("Content-Encoding", "gzip");
      response->addHeader("Last-Modified", _last_modified);
      request->send(response);
   }
}

bool WebHandlerClass::MasterConnected()
{
   return _bSlavePresent;
}

void WebHandlerClass::_CheckMasterStatus()
{
   if (!_MasterStatus.Configured)
   {
      return;
   }
   if (millis() - _MasterStatus.LastCheck > 1200)
   {
      ESP_LOGD(__FILE__, "Checking slave, if any...\r\n");
      _MasterStatus.LastCheck = millis();
      ESP_LOGD(__FILE__, "Slave wsClient->Status: %i\r\n", _MasterStatus.client->status());
      if (_MasterStatus.client->status() != WS_CONNECTED)
      {
         ESP_LOGD(__FILE__, "ws client status says master is disconnected (actual status: %i)\r\n", _MasterStatus.client->status());
         _DisconnectMaster();
      }
      else if (millis() - _MasterStatus.LastReply > 3600)
      {
         ESP_LOGD(__FILE__, "Disconnecting master due to timeout (last pong: %lu, now: %lu)\r\n", _MasterStatus.LastReply, millis());
         _DisconnectMaster();
      }
      else
      {
         ESP_LOGD(__FILE__, "I am pinging the master at %lu\r\n", millis());
         _MasterStatus.client->ping();
      }
   }
}

void WebHandlerClass::_DisconnectMaster()
{
   _MasterStatus.Configured = false;
   _ws->close(_MasterStatus.ClientID);
   ESP_LOGI(__FILE__, "Master disconnected!");
}

WebHandlerClass WebHandler;