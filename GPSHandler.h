// GPSHandler.h

#ifndef _GPSHANDLER_h
#define _GPSHANDLER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

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
   unsigned long GetTimeStampAge();
   long GetMillisToEpochSecond(unsigned long lEpochSecond);
   unsigned long GetEpochTime();
};
extern GPSHandlerClass GPSHandler;
#endif

