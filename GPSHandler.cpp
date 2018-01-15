// 
// 
// 

#include "GPSHandler.h"
#include <TinyGPS++.h>
#include "syslog.h"

void GPSHandlerClass::_HandleSerialPort()
{
   while (_SerialPort->available() > 0)
   {
      char cInChar = _SerialPort->read(); // Read a character
      _Tgps.encode(cInChar);
   }
}

void GPSHandlerClass::init(HardwareSerial * SerialPort)
{
   _SerialPort = SerialPort;
   _FormatUTCTime();
   syslog.logf_P("GPS Class initialized!");
}

void GPSHandlerClass::loop()
{
   _HandleSerialPort();
   
   if (_Tgps.time.isUpdated())
   {
      _FormatUTCTime();
      syslog.logf_P(LOG_DEBUG, "UTC time: %s", _cUTCTime);
   }
}

char* GPSHandlerClass::GetUTCTimestamp()
{
   return _cUTCTime;
}

void GPSHandlerClass::_FormatUTCTime()
{
   sprintf(_cUTCTime, "%i-%02i-%02iT%02i:%02i:%02i.%02iZ",
      _Tgps.date.year()
      , _Tgps.date.month()
      , _Tgps.date.day()
      , _Tgps.time.hour()
      , _Tgps.time.minute()
      , _Tgps.time.second()
      , _Tgps.time.centisecond());
}

GPSHandlerClass GPSHandler;