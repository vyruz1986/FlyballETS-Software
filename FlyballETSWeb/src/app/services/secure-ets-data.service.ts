import { Injectable } from '@angular/core';
import { ConfigArray } from '../interfaces/config-array';
import { Subject, BehaviorSubject, Observer } from 'rxjs';
import { Observable } from 'rxjs/Observable';
import { WebsocketDataRequest } from '../interfaces/websocket-data-request';

@Injectable()
export class SecureEtsDataService {
   ETS_URL:string = "ws://"+ window.location.host +"/wsa";
   //ETS_URL:string = "ws://192.168.20.1/wsa";
   //ETS_URL:string = "ws://blablabla/ws";
   wsObservable:Observable<any>;
   wsObserver:Observer<any>;
   private ws;
   public dataStream:Subject<any>;
   isAuthenticated:boolean;
   connectionStatus:BehaviorSubject<boolean>;

   constructor() {
      this.isAuthenticated = true;
      this.connectionStatus = new BehaviorSubject(false);
      this.initializeWebSocket();
      console.log("constructor");
      
   }
   initializeWebSocket() {
      console.log("Reconnecting to ws");
      this.isConnected = false;
      this.wsObservable = Observable.create((observer) => {
         if(!this.isAuthenticated){
            console.log("Aborting");
            observer.error();
         }
         this.ws = new WebSocket(this.ETS_URL);
         this.ws.onopen = (e) => {
            this.isConnected = true;
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
            console.log(e);
            this.isConnected = false;
         }
      
         this.ws.onmessage = (e) => {
            let data = JSON.parse(e.data);
            console.log(data);
            if(data.success === false && data.authenticated === false)
            {
               console.log("received not authenticated error");
               this.isAuthenticated = false;
               observer.error(e);
            }
            observer.next(JSON.parse(e.data));
         }
      
         return () => {
            console.log("Connection closed gracefully");
            this.ws.close();
            this.isConnected = false;
         };
      }).share().retryWhen((errors) => {errors.delay(2000)});

      this.wsObserver = {
         next: (data: Object) => {
            if (this.ws.readyState === WebSocket.OPEN) {
               console.log(data)
               this.ws.send(JSON.stringify(data));
            } else {
               console.log("Socket was not ready");
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

   public set isConnected(value:boolean){
      this.connectionStatus.next(value);
   }
   public get isConnected(){
      return this.connectionStatus.value;
   }

   enableRetry(){
      this.wsObservable.retry();
      this.dataStream.retry();
   }

   disableRetry(){
      console.log("disabling retries...");
      this.wsObservable.retry(0);
      this.dataStream.retry(0);

   }

   sendConfig(config:ConfigArray) {
      this.dataStream.next(config);
   }

   getData(dataRequest:WebsocketDataRequest) {
      return new Promise((resolve, reject) => {
         if(this.isConnected && this.isAuthenticated){
            this.dataStream.next(dataRequest);
            resolve(true);
         }
      });
   }
}
