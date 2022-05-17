// GPSHandler.h

#ifndef _GPSHANDLER_h
#define _GPSHANDLER_h

#include "Arduino.h"
#include "config.h"
#include <TinyGPS++.h>
#include <Timezone.h>

class GPSHandlerClass
{
private:
   bool _bGSPconnected = false;

protected:
   HardwareSerial* _SerialPort;
   TinyGPSPlus _Tgps;
   void _HandleSerialPort();
   void _FormatTime(); 
   char _cUTCTime[25];
   char _cLocalDateAndTime[25];
   char _cLocalTimestamp[9];
   char _cDate[11];
   unsigned long long llLastGPSRead = 0;

public:
   void init(HardwareSerial* SerialPort);
   void loop();
   char* GetLocalDateAndTime();
   char* GetUtcDateAndTime();
   char* GetLocalTimestamp();
   char* GetDate();
   time_t timeLocal;
};
extern GPSHandlerClass GPSHandler;
#endif

