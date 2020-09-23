// GPSHandler.h

#ifndef _GPSHANDLER_h
#define _GPSHANDLER_h

#include "Arduino.h"
#include "TinyGPS++.h"

class GPSHandlerClass
{
protected:
   HardwareSerial* _SerialPort;
   void _HandleSerialPort();
   void _FormatUTCTime();
   TinyGPSPlus _Tgps;
   char _cUTCTime[25];
public:
   void init(HardwareSerial* SerialPort);
   void loop();
   char* GetUTCTimestamp();
   
};
extern GPSHandlerClass GPSHandler;
#endif

