// 
// 
// 

#include "SlaveHandler.h"
#include "SettingsManager.h"
#include "enums.h"

void SlaveHandlerClass::init()
{
   this->_bConnected = false;
   this->_bIAmSlave = false;

   if (SettingsManager.getSetting("OperationMode").toInt() == SystemModes::SLAVE) {
      this->_bIAmSlave = true;
      this->_RemoteIP = IPAddress(192, 168, 20, 1);
   }
}
void SlaveHandlerClass::loop()
{
   if (!_bConnected && _ulLastConnectAttempt - millis() > CONNECT_RETRY) {
      this->_ulLastConnectAttempt = millis();
      this->_ConnectRemote();
   }
}

void SlaveHandlerClass::setRemoteIp(IPAddress ipSlaveIP)
{
   this->_RemoteIP = ipSlaveIP;
}

void SlaveHandlerClass::removeSlave()
{
   _bConnected = false;
}

bool SlaveHandlerClass::slavePresent()
{
   return (_bConnected && !_bIAmSlave);
}

void SlaveHandlerClass::_ConnectRemote()
{

}

SlaveHandlerClass SlaveHandler;

