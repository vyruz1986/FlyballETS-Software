// 
// 
// 

#include "GPSHandler.h"
#include <TinyGPS++.h>
#include "syslog.h"
#include <TimeLib.h>

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
      setTime(
         _Tgps.time.hour()
         , _Tgps.time.minute()
         , _Tgps.time.second()
         , _Tgps.date.day()
         , _Tgps.date.month()
         , _Tgps.date.year());
      _FormatUTCTime();
      long lAge = (_Tgps.time.centisecond() * 10) - (_Tgps.time.age());
      //syslog.logf_P(LOG_DEBUG, "UTC time: %s + %ld ms", _cUTCTime, lAge);
   }
}

char* GPSHandlerClass::GetUTCTimestamp()
{
   return _cUTCTime;
}

unsigned long GPSHandlerClass::GetTimeStampAge()
{
   return _Tgps.time.age();
}

long GPSHandlerClass::GetMillisToEpochSecond(unsigned long lEpochSecond)
{
   long lDiff = (lEpochSecond - now()) * 1000;
   lDiff -= (_Tgps.time.centisecond() * 10);
   lDiff -= _Tgps.time.age();

   return lDiff;
}

unsigned long GPSHandlerClass::GetEpochTime()
{
   return now();
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