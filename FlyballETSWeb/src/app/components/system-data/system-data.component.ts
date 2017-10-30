import { Component, OnInit, OnDestroy } from '@angular/core';
import { SystemData } from '../../interfaces/system-data';
import { WebsocketService } from '../../services/websocket.service';

@Component({
  selector: 'app-system-data',
  templateUrl: './system-data.component.html',
  styleUrls: ['./system-data.component.css']
})
export class SystemDataComponent implements OnInit {

  systemData:SystemData = {uptime: 0, freeHeap: 0, CPU0ResetReason: 0, CPU1ResetReason: 0, numClients: 0};
  disposeMe;
  isConnected:boolean;
  sessionEnded:boolean;
  dataService:WebsocketService = new WebsocketService();
  constructor() {
    this.initiateConnection();
  }

  ngOnInit() {}

  ngOnDestroy(){
    this.disposeMe.unsubscribe();
  }
    
  initiateConnection() {
    console.log("Attempting WS connection!");
    this.isConnected = false;
    this.sessionEnded = false;
    this.disposeMe = this.dataService.dataStream.subscribe(
      msg => {
        this.isConnected = true;
        //console.log("Response from websocket: ");
        //console.log(msg);
        if(msg.SystemData) {
          this.systemData = <SystemData>msg.SystemData;
        }
      },
      err => {
        //console.log("ws error: ");
        //console.log(err);
        this.isConnected = false;
        this.initiateConnection();  //Retry connection since this was unexpected
      },
      () => {
        console.log("ws closed");
        this.sessionEnded = true; 
      }
    );
  }

}
