//
#include "GPSHandler.h"
#include "LCDController.h"

HardwareSerial GPSSerial(1);

TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120}; // UTC + 2 hours
TimeChangeRule CET = {"CET", Last, Sun, Oct, 3, 60};    // UTC + 1 hour
Timezone euCentral(CEST, CET);

void GPSHandlerClass::_HandleSerialPort()
{
   while (GPSSerial.available() > 0)
   {
      char cInChar = GPSSerial.read(); // Read a character
      _Tgps.encode(cInChar);
      // Serial.write(cInChar);
   }
}

void GPSHandlerClass::init(uint8_t _iGPSrxPin)
{
   GPSSerial.begin(9600, SERIAL_8N1, _iGPSrxPin, -1);
   // vTaskDelay(200);
   _HandleSerialPort();
   _FormatTime();
   log_i("Initial UTC time:  %s", _cUTCTime);
   log_i("Local system time: %s", _cLocalDateAndTime);
}

void GPSHandlerClass::loop()
{
   if ((millis() - llLastGPSRead) > 10000)
   {
      _HandleSerialPort();
      llLastGPSRead = millis();
      if (_Tgps.time.isUpdated() && (_Tgps.date.year() != 2000))
      {
         _FormatTime();
         LCDController.UpdateField(LCDController.GpsState, "G");
         LCDController.bExecuteLCDUpdate = true;
         if (!_bGSPconnected)
         {
            log_i("GPS connected. Updated UTC time: %s. Updated local time: %s", _cUTCTime, _cLocalDateAndTime);
            _bGSPconnected = true;
         }
      }
   }
}

char *GPSHandlerClass::GetUtcDateAndTime()
{
   if (!_bGSPconnected) // Use local time if GPS time is not available
   {
      tmElements_t tm;
      timeLocal = euCentral.toUTC(now());
      breakTime(timeLocal, tm);
      sprintf(_cUTCTime, "%i-%02i-%02iT%02i:%02i:%02iZ", tm.Year + 1970, tm.Month, tm.Day, tm.Hour, tm.Minute, tm.Second);
   }
   return _cUTCTime;
}

char *GPSHandlerClass::GetLocalTimestamp()
{
   sprintf(_cLocalTimestamp, "%02i:%02i:%02i", hour(), minute(), second());
   return _cLocalTimestamp;
}

char *GPSHandlerClass::GetDate()
{
   sprintf(_cDate, "%i-%02i-%02i", year(), month(), day());
   return _cDate;
}

void GPSHandlerClass::_FormatTime()
{
   tmElements_t tm;
   if (_Tgps.date.year() == 2000)
   {
      tm.Year = (2023 - 1970);
      tm.Month = 1;
      tm.Day = 1;
      tm.Hour = 12;
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
   sprintf(_cLocalDateAndTime, "%i-%02i-%02iT%02i:%02i:%02iZ", tm.Year + 1970, tm.Month, tm.Day, tm.Hour, tm.Minute, tm.Second);
}

GPSHandlerClass GPSHandler;