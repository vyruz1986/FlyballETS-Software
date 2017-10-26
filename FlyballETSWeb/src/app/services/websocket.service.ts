import { Injectable } from '@angular/core';
import { Observable, Subject } from 'rxjs/Rx';
import { WebsocketAction } from "../interfaces/websocketaction";
import 'rxjs/add/operator/retryWhen';
import { WebsocketDataRequest } from '../interfaces/websocket-data-request';
import { ConfigArray } from '../interfaces/config-array';

const ETS_URL = 'ws://' + window.location.host + '/ws';


@Injectable()
export class WebsocketService {
   public dataStream;
   constructor() {
      this.dataStream = Observable.webSocket(ETS_URL);
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
