#pragma once
#include <rom/rtc.h>

struct stDogTimeData
{
   long long Time;
   long long CrossingTime;
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
   long long StartTime;
   long long EndTime;
   long long ElapsedTime;
   uint8_t RaceState;
   stDogData DogData[4];
   long long TotalCrossingTime;
};

struct stSystemData
{
   long long Uptime;
   uint32_t FreeHeap;
   RESET_REASON CPU0ResetReason;
   RESET_REASON CPU1ResetReason;
   uint NumClients;
   char* UTCSystemTime;
   uint BatteryPercentage;
};

struct stLightsState
{
   uint8_t State[5];
};

struct stInputSignal
{
   uint8_t Pin;
   long long LastTriggerTime;
   word CoolDownTime;
};