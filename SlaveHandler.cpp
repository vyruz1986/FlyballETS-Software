// 
// 
// 

#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include "SlaveHandler.h"
#include "SettingsManager.h"
#include "enums.h"
#include "global.h"
#include "syslog.h"

void SlaveHandlerClass::init()
{
   this->_bConnected = false;
   this->_bIAmSlave = false;
   this->_bWSConnectionStarted = false;
   this->_bSlaveAnnounced = false;
   this->_bConsumerAnnounced = false;
   this->_bSlaveConfigured = false;
   if (SettingsManager.getSetting("OperationMode").toInt() == SystemModes::SLAVE) {
      this->_bIAmSlave = true;
      this->_RemoteIP = IPAddress(192, 168, 20, 1);
   }

   this->_wsClient.onEvent(std::bind(&SlaveHandlerClass::_WsEvent, this
      , std::placeholders::_1
      , std::placeholders::_2
      , std::placeholders::_3));

   this->_wsClient.setReconnectInterval(CONNECT_CHECK);
   syslog.logf_P("SlaveHandler initialized!");

   _jsonRaceData = _jdocRaceData.to<JsonObject>();
}
void SlaveHandlerClass::loop()
{
   if (millis() - this->_ulLastConnectCheck > CONNECT_CHECK) {
      this->_ulLastConnectCheck = millis();
      Serial.printf("[SLAVEHANDLER] C: %i, CN: %i, CS: %i, S: %i, SC: %i, JsonRDLength: %i\r\n", _bConnected, _ConnectionNeeded(), _bWSConnectionStarted, _bIAmSlave, _bSlaveConfigured, measureJson(_jsonRaceData));
      if (!_bConnected
         && this->_ConnectionNeeded()
         && !_bWSConnectionStarted)
      {
         this->_ConnectRemote();
      }

      if (this->_bConnected && !this->_ConnectionNeeded()) {
         this->_WsCloseConnection();
      }
   }

   if (this->_bConnected || this->_ConnectionNeeded()) {
      //Serial.print(".");
      this->_wsClient.loop();
   }

   _TestConnection();
}

void SlaveHandlerClass::configureSlave(IPAddress ipSlaveIP)
{
   this->_RemoteIP = ipSlaveIP;
   this->_bSlaveConfigured = true;
   this->_bWSConnectionStarted = false;
   this->_SetDisconnected();
}

void SlaveHandlerClass::removeSlave()
{
   this->_RemoteIP = IPAddress(0, 0, 0, 0);
   this->_bSlaveConfigured = false;
   this->_bWSConnectionStarted = false;
}

bool SlaveHandlerClass::slavePresent()
{
   return (_bConnected && !_bIAmSlave);
}

void SlaveHandlerClass::_ConnectRemote()
{
   this->_wsClient.begin(this->_RemoteIP.toString(), 80, "/ws");
   this->_bWSConnectionStarted = true;
   Serial.printf("[WSc] Initiated connection...\r\n");
}

void SlaveHandlerClass::_WsEvent(WStype_t type, uint8_t * payload, size_t length)
{
   switch (type) {
   case WStype_DISCONNECTED:
      this->_SetDisconnected();
      break;

   case WStype_CONNECTED:
      this->_bConnected = true;
      this->_bWSConnectionStarted = false;
      this->_AnnounceSlaveIfApplicable();
      this->_AnnounceConsumerIfApplicable();
      Serial.printf("[WSc] Connected to url: %s\n", payload);
      _SlaveStatus.LastReply = millis();

      // send message to server when Connected
      //this->_wsClient.sendTXT("Connected");
      break;

   case WStype_TEXT:
   {
      Serial.printf("[WSc] got text: %s\n", payload);
      _SlaveStatus.LastReply = millis();
      DeserializationError error = deserializeJson(_jdocClientInput, (const char *)payload);
      if (error) {
         syslog.logf_P(LOG_ERR, "Error parsing JSON: %s!", error.c_str());
         return;
      }
      _jsonClientInput = _jdocClientInput.as<JsonObject>();

      if (_jsonClientInput.containsKey("RaceData")) {
         _jsonRaceData = _jdocRaceData.to<JsonObject>();
         _jsonRaceData = _jsonClientInput["RaceData"][0];
         
         _strJsonRaceData = "";
         serializeJson(_jsonRaceData, _strJsonRaceData);
         Serial.printf("got racedata from slave: %s\r\n", _strJsonRaceData.c_str());

      }
      else if (_jsonClientInput.containsKey("SystemData")) {
         Serial.printf("Got SystemData!\r\n");
         _ulLastSystemDataReceived = millis();
      }
      break;
   }

   case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);

      // send data to server
      // webSocket.sendBIN(payload, length);
      break;
   }
}

void SlaveHandlerClass::_SetDisconnected()
{
   this->_bConnected = false;
   Serial.printf("[WSc] Disconnected!\n");
}

void SlaveHandlerClass::_AnnounceSlaveIfApplicable()
{
   if (!_bIAmSlave || _bSlaveAnnounced) {
      return;
   }
   String strJson = String("{\"action\": {\"actionType\": \"AnnounceSlave\"}}");
   this->_wsClient.sendTXT(strJson);
   this->_bSlaveAnnounced = true;

   //If we are slave and we have announced ourselves as such, we can close the connection
   Serial.printf("[WSc] Announed slave, disconnecting...\r\n");
   this->_WsCloseConnection();
}

void SlaveHandlerClass::_AnnounceConsumerIfApplicable()
{
   if (_bIAmSlave || _bConsumerAnnounced) {
      return;
   }
   String strJson = String("{\"action\": {\"actionType\": \"AnnounceConsumer\"}}");
   this->_wsClient.sendTXT(strJson);
   this->_bConsumerAnnounced = true;

   //If we are slave and we have announced ourselves as such, we can close the connection
   Serial.printf("[WSc] Announed Consumer!\r\n");
}

void SlaveHandlerClass::_WsCloseConnection()
{
   this->_wsClient.disconnect();
   this->_bConnected = false;
}

bool SlaveHandlerClass::_ConnectionNeeded()
{
   //Serial.printf("Wifi Status: %u, Wifi.localIP: %s\r\n", WiFi.status(), WiFi.localIP().toString().c_str());
   if (WiFi.status() != WL_CONNECTED
       || (_bIAmSlave && WiFi.localIP().toString().equals("0.0.0.0"))) {
      return false;
   }
   if (_bIAmSlave) {
      return (!_bSlaveAnnounced);
   }
   else {
      return (_bSlaveConfigured);
   }
   return false;
}

void SlaveHandlerClass::resetConnection()
{
   this->_bSlaveAnnounced = false;
   this->_bConsumerAnnounced = false;
}

bool SlaveHandlerClass::sendToSlave(String strMessage)
{
   if (_bIAmSlave) {
      return false;
   }
   bool bResult = true;

   if (_bConnected) {
      _wsClient.sendTXT(strMessage);
   }
   else {
      bResult = false;
   }

   return bResult;
}

String& SlaveHandlerClass::getSlaveRaceData()
{
   Serial.printf("[SLAVEHANDLER] Slave is returning racedata: %s\r\n", _strJsonRaceData.c_str());
   return _strJsonRaceData;
}

JsonObject SlaveHandlerClass::getSlaveRaceData1()
{
   String strJsonRaceData;
   serializeJson(_jdocRaceData, strJsonRaceData);
   Serial.printf("[SLAVEHANDLER] Returning slave racedata: %s\r\n\r\n", strJsonRaceData.c_str());
   return _jsonRaceData;
}

char * SlaveHandlerClass::getSlaveRaceData2()
{
   char strJsonRaceData[2000];
   serializeJson(_jsonRaceData, strJsonRaceData);
   Serial.printf("[SLAVEHANDLER] Returning slave racedata: %s\r\n\r\n", strJsonRaceData);
   return strJsonRaceData;
}

void SlaveHandlerClass::_TestConnection()
{
   //FIXME: This connection test is not working...
   if (this->_ConnectionNeeded() && _bConnected) {
      if (millis() - _SlaveStatus.LastCheck > 1200) {
         _SlaveStatus.LastCheck = millis();
         Serial.printf("[WSc] Testing connection at %lu...\r\n", millis());
      }
      if (millis() - _SlaveStatus.LastReply > 6000) {
         Serial.printf("[WSc] Connection broken, reconnecting...\r\n");
         _wsClient.disconnect();
         this->_SetDisconnected();
      }
   }
}

bool SlaveHandlerClass::GetConnectionStatus()
{
   return _bConnected;
}

SlaveHandlerClass SlaveHandler;

