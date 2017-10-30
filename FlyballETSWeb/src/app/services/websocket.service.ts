import { Injectable } from '@angular/core';
import { Observable, Subject } from 'rxjs/Rx';
import { WebsocketAction } from "../interfaces/websocketaction";
import 'rxjs/add/operator/retryWhen';
import { WebsocketDataRequest } from '../interfaces/websocket-data-request';
import { ConfigArray } from '../interfaces/config-array';

@Injectable()
export class WebsocketService {
   public dataStream;
   //constructor(wsUrl:string) {
   constructor() {
      //this.dataStream = Observable.webSocket(wsUrl);
      this.dataStream = Observable.webSocket('ws://' + window.location.host + '/ws');
   }
   sendAction(action:WebsocketAction) {
      this.dataStream.next(JSON.stringify(action));
   }

   sendConfig(config:ConfigArray) {
      this.dataStream.next(JSON.stringify(config));
   }

   getData(dataRequest:WebsocketDataRequest) {
      this.dataStream.next(JSON.stringify(dataRequest));
   }
}
