#pragma once
#include <rom/rtc.h>

struct stDogTimeData
{
   unsigned long Time;
   long CrossingTime;
};

struct stDogData
{
   uint8_t DogNumber;
   //String DogName;
   stDogTimeData Timing[4];
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

struct stLightsState
{
   uint8_t State[5];
};