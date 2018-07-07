// 
// 
// 

#include <WebSocketsClient.h>
#include "SlaveHandler.h"
#include "SettingsManager.h"
#include "enums.h"

void SlaveHandlerClass::init()
{
   this->_bConnected = false;
   this->_bIAmSlave = false;
   this->_bWSConnectionStarted = false;
   if (SettingsManager.getSetting("OperationMode").toInt() == SystemModes::SLAVE) {
      this->_bIAmSlave = true;
      this->_RemoteIP = IPAddress(192, 168, 20, 1);
   }

   this->_wsClient.onEvent(std::bind(&SlaveHandlerClass::_WsEvent, this
      , std::placeholders::_1
      , std::placeholders::_2
      , std::placeholders::_3));

   this->_wsClient.setReconnectInterval(CONNECT_CHECK);
}
void SlaveHandlerClass::loop()
{
   if (millis() - this->_ulLastConnectCheck > CONNECT_CHECK) {
      this->_ulLastConnectCheck = millis();
      if (!_bConnected
         && WiFi.status() == WL_CONNECTED
         && !this->_RemoteIP.toString().equals("0.0.0.0")
         && !_bWSConnectionStarted)
      {
         this->_ulLastConnectCheck = millis();
         this->_ConnectRemote();
      }

      //FIXME: This connection test is not working...
      if (_bConnected) {
         Serial.printf("[WSc] Testing connection...\r\n");
         String ping = "ping";
         bool bPingResult = _wsClient.sendPing(ping);
         if (!bPingResult) {
            //Connection broken
            this->_SetDisconnected();
         }
         Serial.printf("[WSc] Result %i\r\n", bPingResult);
      }
   }

   this->_wsClient.loop();
}

void SlaveHandlerClass::setRemoteIp(IPAddress ipSlaveIP)
{
   this->_RemoteIP = ipSlaveIP;
   this->_bWSConnectionStarted = false;
}

void SlaveHandlerClass::removeSlave()
{
   this->_RemoteIP = IPAddress(0, 0, 0, 0);
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
      Serial.printf("[WSc] get text: %s\n", payload);

      // send message to server
      // webSocket.sendTXT("message here");
      break;
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
   if (!_bIAmSlave) {
      return;
   }
   String strJson = String("{\"actionType\": \"AnnounceSlave\"}");
   this->_wsClient.sendTXT(strJson);
}

SlaveHandlerClass SlaveHandler;

