import { Injectable } from '@angular/core';
import { ConfigArray } from '../interfaces/config-array';
import { Observable, Observer, Subject, BehaviorSubject } from 'rxjs';
import { share } from 'rxjs/operators';
import { WebsocketDataRequest } from '../interfaces/websocket-data-request';

@Injectable()
export class SecureEtsDataService {
   ETS_URL: string = "ws://" + window.location.host + "/wsa";
   //ETS_URL:string = "ws://192.168.20.1/wsa";
   wsObservable: Observable<any>;
   wsObserver: Observer<any>;
   private ws;
   public dataStream: BehaviorSubject<any>;
   isConnected: boolean;
   sessionEnded: boolean;

   private _isAuthenticated = new BehaviorSubject<boolean>(false);
   //private _isAuthenticated = new BehaviorSubject<boolean>(true);
   public isAuthenticated: Observable<boolean> = this._isAuthenticated.asObservable();

   private _isConnected = new BehaviorSubject<boolean>(false);
   public isWsConnetced: Observable<boolean> = this._isConnected.asObservable();

   public setAuthenticated(authenticated: boolean): void {
      console.log('authenticated: ' + authenticated);
      this._isAuthenticated.next(authenticated);
   }

   constructor() {
      this.initializeWebSocket();
   }

   getWsObservable(): Observable<any> {
      return this.wsObservable;
   }

   initializeWebSocket() {
      console.log("Reconnecting to wsa");
      this.isConnected = false;
      this.wsObservable = Observable.create((observer) => {
         this.ws = new WebSocket(this.ETS_URL);
         console.log('created wsa');
         this.ws.onopen = (e) => {
            this.isConnected = true;
            this._isConnected.next(this.isConnected);
            console.log("Connected wsa!");
         };

         this.ws.onclose = (e) => {
            if (e.wasClean) {
               observer.complete();
            } else {
               observer.error(e);
            }
            this.isConnected = false;
            this._isConnected.next(this.isConnected);
            console.log('onClose');
         };

         this.ws.onerror = (e) => {
            observer.error(e);
            this.isConnected = false;
            this._isConnected.next(this.isConnected);
            console.log('onError');
         }

         this.ws.onmessage = (e) => {
            observer.next(JSON.parse(e.data));
         }

         return () => {
            console.log("Connection closed gracefully");
            this.ws.close();
            this.sessionEnded = true;
            this.isConnected = false;
            this._isConnected.next(this.isConnected);
         };
      }).pipe(share());
      console.log('observer created');

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

   sendConfig(config: ConfigArray) {
      this.dataStream.next(config);
   }

   getData(dataRequest: WebsocketDataRequest) {
      this.dataStream.next(dataRequest);
   }
}