#pragma once
#include <rom/rtc.h>

struct stDogData
{
   uint8_t DogNumber;
   //String DogName;
   unsigned long Time;
   unsigned long CrossingTime;
   boolean Running;
   boolean Fault;
};

struct stRaceData
{
   uint Id;
   unsigned long StartTime;
   unsigned long EndTime;
   unsigned long ElapsedTime;
   uint8_t RaceState;
   stDogData DogData[4];
};

struct stSystemData
{
   unsigned long Uptime;
   uint32_t FreeHeap;
   RESET_REASON CPU0ResetReason;
   RESET_REASON CPU1ResetReason;
   uint NumClients;
};