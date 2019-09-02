#include <SystemManager.h>

void SystemManagerClass::init()
{
   _ulScheduledRebootTS = 0;
   _iOpMode = SettingsManager.getSetting("OperationMode").toInt();
}

void SystemManagerClass::loop()
{
   if (_ulScheduledRebootTS > 0 && millis() > _ulScheduledRebootTS)
   {
      ESP.restart();
   }
}

void SystemManagerClass::scheduleReboot(unsigned long ulRebootTs)
{
   if (ulRebootTs > millis())
   {
      _ulScheduledRebootTS = ulRebootTs;
   }
}

bool SystemManagerClass::CheckMasterSlaveConnection()
{
   if (_iOpMode == SystemModes::MASTER)
   {
      return SlaveHandler.GetConnectionStatus();
   }
   else
   {
      return WebHandler.MasterConnected();
   }
}

SystemManagerClass SystemManager;