// Debug.h

#ifndef _DEBUG_h
#define _DEBUG_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <stdio.h>
#include <stdarg.h>
#include <WiFiUdp.h>
#include <Syslog.h>

#define LOG_EMERG 0 /* system is unusable */
#define LOG_ALERT 1 /* action must be taken immediately */
#define LOG_CRIT  2 /* critical conditions */
#define LOG_ERR   3 /* error conditions */
#define LOG_WARNING 4 /* warning conditions */
#define LOG_NOTICE  5 /* normal but significant condition */
#define LOG_INFO  6 /* informational */
#define LOG_DEBUG 7 /* debug-level messages */

#define DEBUG_MESSAGE_MAX_LENGTH    80

class DebugClass
{
protected:
   char * _TranslatePriority(uint8_t priority);
   WiFiUDP _DebugUDP;
   Syslog *_Syslog;
public:
   void init(char * SysLogDest, char * DeviceName, char * AppName);
   void DebugSend(uint8_t priority, const char * format, ...);
};

extern DebugClass Debug;

#endif

