// 
// 
// 

#include "GPSHandler.h"
#include "Debug.h"
#include <TinyGPS++.h>

void GPSHandlerClass::_HandleSerialPort()
{
   while (_SerialPort->available() > 0)
   {
      char cInChar = _SerialPort->read(); // Read a character
      _Tgps.encode(cInChar);
      if (cInChar == '\n') //Check if buffer contains complete serial message, terminated by newline (\n)
      {
         //Serial message in buffer is complete, null terminate it and store it for further handling
         _bSerialMessageComplete = true;
         _strSerialData += '\0'; // Null terminate the string
         break;
      }
      _strSerialData += cInChar; // Store it
   }
}

void GPSHandlerClass::_HandleSerialMessage()
{
   Debug.DebugSend(LOG_INFO, "GPS: Serial message received: %s", _strSerialData.c_str());

   Debug.DebugSend(LOG_INFO, "GPS: Time from GPS: %i", _Tgps.time.value());
   


   //Reset serial input string and flag
   _strSerialData = "";
   _bSerialMessageComplete = false;
}

void GPSHandlerClass::init(HardwareSerial * SerialPort)
{
   _SerialPort = SerialPort;
   _bSerialMessageComplete = false;
   _strSerialData = "";
   Debug.DebugSend(LOG_INFO, "GPS Class initialized!");
}

void GPSHandlerClass::loop()
{
   _HandleSerialPort();
   if (_bSerialMessageComplete)
   {
      _HandleSerialMessage();
   }

}

GPSHandlerClass GPSHandler;