import { Time } from "@angular/common/src/i18n/locale_data_api";

export interface SystemData {
   uptime:number,
   freeHeap:number,
   CPU0ResetReason:number,
   CPU1ResetReason:number,
   numClients:number,
   systemTimestamp:Date
}