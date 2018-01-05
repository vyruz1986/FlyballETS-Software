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
   String _strSerialData;
   boolean _bSerialMessageComplete;
   void _HandleSerialPort();
   void _HandleSerialMessage();
   TinyGPSPlus _Tgps;
public:
   void init(HardwareSerial* SerialPort);
   void loop();
   
};
extern GPSHandlerClass GPSHandler;
#endif

