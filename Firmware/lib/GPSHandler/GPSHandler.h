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

protected:
   HardwareSerial* _SerialPort;
   TinyGPSPlus _Tgps;
   void _HandleSerialPort();
   void _FormatTime(); 
   char _cUTCTime[25];
   char _cLocalTime[25];
   unsigned long long llLastGPSRead = 0;

public:
   void init(HardwareSerial* SerialPort);
   void loop();
   char* GetUTCTimestamp();
   char* GetLocalTimestamp();
   time_t timeLocal;
};
extern GPSHandlerClass GPSHandler;
#endif

