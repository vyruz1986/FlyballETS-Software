// 
// 
// 

#include "Debug.h"

void DebugClass::init(char * SysLogDest, char * DeviceName, char * AppName)
{
   Syslog _Syslog(_DebugUDP, SysLogDest, 514, DeviceName, AppName, LOG_KERN);
   _Syslog.setSerialPrint(true);
}

void DebugClass::DebugSend(uint8_t priority, const char * format, ...) {

   char buffer[DEBUG_MESSAGE_MAX_LENGTH + 1];

   va_list args;
   va_start(args, format);
   int len = vsnprintf(buffer, DEBUG_MESSAGE_MAX_LENGTH, format, args);
   va_end(args);

   char * cPrio = _TranslatePriority(priority);

   Serial.printf("%s: %s\r\n", cPrio, buffer);
   if (len > DEBUG_MESSAGE_MAX_LENGTH) {
      Serial.printf(" (...)\n");
   }

   //_Syslog->log(LOG_ALERT, "testing!");

}

char * DebugClass::_TranslatePriority(uint8_t priority)
{
   switch (priority)
   {
   case 0:
      return "EMERG";
      break;
   case 1:
      return "ALERT";
      break;
   case 2:
      return "CRIT";
      break;
   case 3:
      return "ERROR";
      break;
   case 4:
      return "WARN";
      break;
   case 5:
      return "NOTICE";
      break;
   case 6:
      return "INFO";
      break;
   case 7:
      return "DEBUG";
      break;
   }
}

DebugClass Debug;

