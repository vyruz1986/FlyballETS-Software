import { Injectable } from '@angular/core';
import { Observable, Subject } from 'rxjs/Rx';
import { WebsocketService } from './websocket.service';
import { WebsocketAction, WebsocketActionResult } from "../interfaces/websocketaction";

const CHAT_URL = 'ws://echo.websocket.org/';


@Injectable()
export class RaceDataService {

   public action: Subject<WebsocketAction>;

   constructor(wsService: WebsocketService) {
      this.action = <Subject<WebsocketAction>>wsService
         .connect(CHAT_URL)
         .map((result: MessageEvent): WebsocketAction => {
            let data = JSON.parse(result.data);
            return {
               response: {
                  success: data.success,
                  error: data.error
               }
            }
         });
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

      let data = JSON.stringify({ type: "action", actionType: action });
      console.log(data);
   }
}
