import { Injectable } from '@angular/core';
import { Observable, Subject } from 'rxjs/Rx';
import { WebsocketService } from './websocket.service';
import { WebsocketAction } from "../interfaces/websocketaction";
import 'rxjs/add/operator/retryWhen';

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
   /*
   public dataStream: Subject<any>;

   constructor(wsService: WebsocketService) {
      this.dataStream = <Subject<any>>wsService
         .connect(ETS_URL)
         .map((response:any): any => {
            let data = JSON.parse(response.data);
            return data
         })
         .retry(3)
   }
*/
   public dataStream;
   constructor() {
      this.dataStream = Observable.webSocket(ETS_URL);
   }
   sendAction(action:WebsocketAction) {
      this.dataStream.next(JSON.stringify(action));
   }
}
