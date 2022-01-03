//
#include "GPSHandler.h"

TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120}; //UTC + 2 hours
TimeChangeRule CET = {"CET", Last, Sun, Oct, 3, 60};    //UTC + 1 hour
Timezone euCentral (CEST, CET);

void GPSHandlerClass::_HandleSerialPort()
{
   while (_SerialPort->available() > 0)
   {
      char cInChar = _SerialPort->read(); // Read a character
      _Tgps.encode(cInChar);
   }
}

void GPSHandlerClass::init(HardwareSerial *SerialPort)
{
   _SerialPort = SerialPort;
   delay(200);
   _HandleSerialPort();
   _FormatTime();
   ESP_LOGI(__FILE__, "Initial UTC time:  %s", _cUTCTime);
   ESP_LOGI(__FILE__, "Local system time: %s", _cLocalTime);
}

void GPSHandlerClass::loop()
{
   if ((GET_MICROS / 1000 - llLastGPSRead) > 1000)
   {
      _HandleSerialPort();
      if (_Tgps.time.isUpdated() && (_Tgps.date.year() != 2000))
      {
         _FormatTime();
         //ESP_LOGD(__FILE__, "GPS updated UTC time: %s. Updated local time: %s", _cUTCTime, _cLocalTime);
      }
      else
      {
         tmElements_t tm;
         breakTime(now(), tm);
         sprintf(_cLocalTime, "%i-%02i-%02iT%02i:%02i:%02iZ", tm.Year + 1970, tm.Month, tm.Day, tm.Hour, tm.Minute, tm.Second);
         //ESP_LOGD(__FILE__, "System local time: %s", _cLocalTime);
      }
      llLastGPSRead = GET_MICROS / 1000;
   }
}

char *GPSHandlerClass::GetUTCTimestamp()
{
   return _cUTCTime;
}

char *GPSHandlerClass::GetLocalTimestamp()
{
   return _cLocalTime;
}

void GPSHandlerClass::_FormatTime()
{
   tmElements_t tm;
   if(_Tgps.date.year() == 2000)
   {
      tm.Year = (2022 - 1970);
      tm.Month = 1;
      tm.Day = 1;
      tm.Hour = 15;
      tm.Minute = 00;
      tm.Second = 00;
   }
   else
   {
      tm.Year = _Tgps.date.year() - 1970;
      tm.Month = _Tgps.date.month();
      tm.Day = _Tgps.date.day();
      tm.Hour = _Tgps.time.hour();
      tm.Minute = _Tgps.time.minute();
      tm.Second = _Tgps.time.second();
   }
   time_t utc = makeTime(tm);
   sprintf(_cUTCTime, "%i-%02i-%02iT%02i:%02i:%02iZ", tm.Year + 1970, tm.Month, tm.Day, tm.Hour, tm.Minute, tm.Second);
   timeLocal = euCentral.toLocal(utc);
   setTime(timeLocal);
   breakTime(timeLocal, tm);
   sprintf(_cLocalTime, "%i-%02i-%02iT%02i:%02i:%02iZ", tm.Year + 1970, tm.Month, tm.Day, tm.Hour, tm.Minute, tm.Second);
}

GPSHandlerClass GPSHandler;