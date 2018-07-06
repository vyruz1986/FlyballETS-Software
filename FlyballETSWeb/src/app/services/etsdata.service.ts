import { Injectable } from '@angular/core';
import { Observable, Observer, Subject, BehaviorSubject } from 'rxjs';
import { share, retry } from 'rxjs/operators';
import { WebsocketAction } from '../interfaces/websocketaction';
import { ConfigArray } from '../interfaces/config-array';
import { WebsocketDataRequest } from '../interfaces/websocket-data-request';

@Injectable()
export class EtsdataService {
   ETS_URL:string = "ws://"+ window.location.host +"/ws";
   wsObservable:Observable<any>;
   wsObserver:Observer<any>;
   private ws;
   public dataStream:BehaviorSubject<any>;
   isConnected:boolean;
   sessionEnded:boolean;
   constructor() {
      this.initializeWebSocket();
   }
   initializeWebSocket() {
      console.log("Reconnecting to ws");
      this.isConnected = false;
      this.wsObservable = Observable.create((observer) => {
         this.ws = new WebSocket(this.ETS_URL);
         this.ws.onopen = (e) => {
            this.isConnected = true;
            console.log("Connected!");
         };

         this.ws.onclose = (e) => {
            if (e.wasClean) {
              observer.complete();
            } else {
              observer.error(e);
            }
            this.isConnected = false;
         };
      
         this.ws.onerror = (e) => {
            observer.error(e);
            this.isConnected = false;
         }
      
         this.ws.onmessage = (e) => {
            observer.next(JSON.parse(e.data));
         }
      
         return () => {
            console.log("Connection closed gracefully");
            this.ws.close();
            this.isConnected = false;
         };
      }).pipe(share(), retry());

      this.wsObserver = {
         next: (data: Object) => {
            if (this.ws.readyState === WebSocket.OPEN) {
               console.log(data)
               this.ws.send(JSON.stringify(data));
            }
         },
         error: (err) => {
            console.log("Error sending data:");
            console.log(err);
         },
         complete: () => {

         }
      }

      this.dataStream = Subject.create(this.wsObserver, this.wsObservable);
   }

   sendAction(action:WebsocketAction) {
      this.dataStream.next({action: action});
   }

   sendConfig(config:ConfigArray) {
      this.dataStream.next(config);
   }

   getData(dataRequest:WebsocketDataRequest) {
      this.dataStream.next(dataRequest);
   }
}
