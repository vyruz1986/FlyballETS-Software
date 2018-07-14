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
}
void SlaveHandlerClass::loop()
{
   if (millis() - this->_ulLastConnectCheck > CONNECT_CHECK) {
      this->_ulLastConnectCheck = millis();
      Serial.printf("C: %i, CN: %i, CS: %i, S: %i, SC: %i\r\n", _bConnected, _ConnectionNeeded(), _bWSConnectionStarted, _bIAmSlave, _bSlaveConfigured);
      if (!_bConnected
         && this->_ConnectionNeeded()
         && !_bWSConnectionStarted)
      {
         this->_ConnectRemote();
      }

      if (this->_bConnected && !this->_ConnectionNeeded()) {
         this->_WsCloseConnection();
      }

      //FIXME: This connection test is not working...
      /*
      if (this->_ConnectionNeeded() && _bConnected) {
         //Serial.printf("[WSc] Testing connection...\r\n");
         String ping = "ping";
         bool bPingResult = _wsClient.sendPing(ping);
         if (!bPingResult) {
            //Connection broken
            this->_SetDisconnected();
         }
         //Serial.printf("[WSc] Result %i\r\n", bPingResult);
      }
      */
   }
   if (this->_bConnected || this->_ConnectionNeeded()) {
      this->_wsClient.loop();
   }
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
      Serial.printf("[WSc] Connected to url: %s\n", payload);

      // send message to server when Connected
      //this->_wsClient.sendTXT("Connected");
      break;

   case WStype_TEXT:
   {
      Serial.printf("[WSc] got text: %s\n", payload);
      //StaticJsonDocument<bsRaceDataArray> jsonRequestDoc;
      DynamicJsonDocument jsonRequestDoc;
      JsonObject request = jsonRequestDoc.to<JsonObject>();
      DeserializationError error = deserializeJson(jsonRequestDoc, payload);
      if (error) {
         syslog.logf_P(LOG_ERR, "Error parsing JSON: %s!", error.c_str());
      } else if (request.containsKey("RaceData")) {
         JsonObject jsonSlaveRaceData = request["RaceData"][0].as<JsonObject>();
         serializeJson(jsonSlaveRaceData, _strJsonRaceData);
         _strJsonRaceData = "";
         Serial.printf("got racedata from slave: %s\r\n", _strJsonRaceData.c_str());
      }
      else if (request.containsKey("SystemData")) {
         Serial.printf("Got SystemData!\r\n");
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

void SlaveHandlerClass::_WsCloseConnection()
{
   this->_wsClient.disconnect();
   this->_bConnected = false;
}

bool SlaveHandlerClass::_ConnectionNeeded()
{
   if (WiFi.status() != WL_CONNECTED) {
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
   Serial.printf("Slave is returning racedata: %s\r\n", _strJsonRaceData.c_str());
   return _strJsonRaceData;
}

SlaveHandlerClass SlaveHandler;

