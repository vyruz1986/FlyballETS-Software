#pragma once
#include <rom/rtc.h>

struct stSystemData
{
   unsigned long Uptime;
   char *FwVer;
   uint16_t PwrOnTag;
   int RaceID;
   uint NumClients;
   char *LocalSystemTime;
   uint BatteryPercentage = 0;
   char *RunDirection;
};

struct stLightsState
{
   uint8_t State[5];
};