import { Injectable } from '@angular/core';
import { Observable, Subject } from 'rxjs/Rx';
import { WebsocketService } from './websocket.service';
import { WebsocketAction } from "../interfaces/websocketaction";

const ETS_URL = 'ws://192.168.20.1/ws';


@Injectable()
export class RaceDataService {

   /*
   public actions: Subject<WebsocketAction>;

   constructor(wsService: WebsocketService) {
      this.actions = <Subject<WebsocketAction>>wsService
         .connect(ETS_URL)
         .map((response:MessageEvent): WebsocketAction => {
            let data = JSON.parse(response.data);
            return {
               action: data.action,
               response: {
                  success: data.success,
                  error: data.error
               }
            }
         })
   }
   */

   public dataStream: Subject<any>;

   constructor(wsService: WebsocketService) {
      this.dataStream = <Subject<any>>wsService
         .connect(ETS_URL)
         .map((response:any): any => {
            let data = JSON.parse(response.data);
            return data
         })
   }

   raceData = {
      "id": 1,
      "startTime": "Mon Sep 25 2017 19:27:01 GMT+0200 (Romance Daylight Time)",
      "endTime": "Mon Sep 25 2017 19:27:28 GMT+0200 (Romance Daylight Time)",
      "elapsedTime": 27845,
      "raceState": 1
   };

   getRaceData() {
      return this.raceData;
   }

   sendAction(action:WebsocketAction) {
      this.dataStream.next(action);
   }
}
