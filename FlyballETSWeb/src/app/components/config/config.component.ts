import { Component, OnInit, OnDestroy } from '@angular/core';
import { WebsocketService } from '../../services/websocket.service';
import { WebsocketDataRequest } from '../../interfaces/websocket-data-request';
import { ConfigArray } from '../../interfaces/config-array';
import { ConfigData } from '../../class/config-data';

@Component({
  selector: 'app-config',
  templateUrl: './config.component.html',
  styleUrls: ['./config.component.css'],
  providers: []
})
export class ConfigComponent implements OnInit {

  isConnected: boolean;
  sessionEnded: boolean;
  submitted: boolean;
  disposeMe;

  configData = new ConfigData("","","");

  //configDataService = new WebsocketService('ws://' + window.location.host + '/wsa');
  configDataService = new WebsocketService();

  constructor() {
    this.initiateConnection();
  }

  ngOnInit() {
     this.isConnected = false;
     this.sessionEnded = false;
     this.submitted = false;
     this.requestConfigData();
     this.configDataService
  }

  ngOnDestroy() {
    this.disposeMe.unsubscribe();
  }

  handleConfigData(newConfigData) {
    this.configData = newConfigData;
    console.log(this.configData);
  }

  requestConfigData() {
    let request:WebsocketDataRequest = {
      getData: "config"
    };
    this.configDataService.getData(request);
  }

  handleConfigResult(configResult) {
    this.submitted = false;
    if(configResult.success) {
      console.log("Config saved successfully!");
    }
  }

  onSubmit() {
    let newConfigArray:ConfigArray = {
      config: [
        {name: "APName", value: this.configData.APName},
        {name: "APPass", value: this.configData.APPass},
        {name: "AdminPass", value: this.configData.AdminPass},
        {name: "UserPass", value: this.configData.UserPass}
      ]
    };
    console.log(newConfigArray);
    this.submitted = true;
    this.configDataService.sendConfig(newConfigArray);
    console.log("new config sent!");
  }
  
  initiateConnection() {
    console.log("Attempting WS connection!");
    this.isConnected = false;
    this.sessionEnded = false;
    this.disposeMe = this.configDataService.dataStream.subscribe(
      msg => {
        this.isConnected = true;
        console.log("received data: ");
        console.log(msg);
        if(msg.dataResult && msg.dataResult.success && msg.dataResult.configData) {
          this.handleConfigData(msg.dataResult.configData);
        } else if (msg.configResult) {
          this.handleConfigResult(msg.configResult);
        }
      },
      err => {
        this.isConnected = false;
        this.initiateConnection();  //Retry connection since this was unexpected
      },
      () => {
        this.sessionEnded = true; 
      }
    )
  }
}
