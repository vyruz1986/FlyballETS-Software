// 
// 
// 

#include "SlaveHandler.h"

void SlaveHandlerClass::init()
{
   _bSlaveConnected = false;
}
void SlaveHandlerClass::loop()
{

}

void SlaveHandlerClass::setSlaveIp(IPAddress ipSlaveIP)
{
   _bSlaveConnected = true;
   this->_SlaveIP = ipSlaveIP;
}

void SlaveHandlerClass::removeSlave()
{
   _bSlaveConnected = false;
}

bool SlaveHandlerClass::slavePresent()
{
   return _bSlaveConnected;
}

SlaveHandlerClass SlaveHandler;

