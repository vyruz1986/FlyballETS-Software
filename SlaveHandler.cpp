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

   this->_wsClient.setReconnectInterval(CONNECT_RETRY);
}
void SlaveHandlerClass::loop()
{
   if (!_bConnected
      && WiFi.status() == WL_CONNECTED
      && !this->_RemoteIP.toString().equals("0.0.0.0")
      && _ulLastConnectAttempt - millis() > CONNECT_RETRY
      && !_bWSConnectionStarted)
   {
      this->_ulLastConnectAttempt = millis();
      this->_ConnectRemote();
   }

   this->_wsClient.loop();
}

void SlaveHandlerClass::setRemoteIp(IPAddress ipSlaveIP)
{
   this->_RemoteIP = ipSlaveIP;
   this->_bWSConnectionStarted = false;
   this->_ConnectRemote();
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
}

void SlaveHandlerClass::_WsEvent(WStype_t type, uint8_t * payload, size_t length)
{
   switch (type) {
   case WStype_DISCONNECTED:
      this->_bConnected = false;
      Serial.printf("[WSc] Disconnected!\n");
      break;
   case WStype_CONNECTED:
      this->_bConnected = true;
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

SlaveHandlerClass SlaveHandler;

