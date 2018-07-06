import { Component, OnInit, OnDestroy } from '@angular/core';
import { WebsocketDataRequest } from '../../interfaces/websocket-data-request';
import { ConfigArray } from '../../interfaces/config-array';
import { ConfigData } from '../../class/config-data';
import { SecureEtsDataService } from '../../services/secure-ets-data.service';

@Component({
  selector: 'app-config',
  templateUrl: './config.component.html',
  styleUrls: ['./config.component.css']
})
export class ConfigComponent implements OnInit {

   sessionEnded: boolean;
   submitted: boolean = false;
   isAuthenticated: boolean = false;

   configData = new ConfigData("","","");

  constructor(public secEtsDataService:SecureEtsDataService) {  //TODO why does making etsDataService private cause build to fail?
    this.subscribeToAuth();
    //this.setupDataService();
  }

  subscribeToAuth(){
   this.secEtsDataService.isAuthenticated.subscribe(
      (authenticated:boolean) => {
        this.isAuthenticated = authenticated;
        if(authenticated){
           this.secEtsDataService.initializeWebSocket();
           this.setupDataService()

           //TODO: This is not working for some reason. The config data is requested too soon, before the ws connection is established
           //setTimeout(this.requestConfigData(), 3000);
           //this.requestConfigData();
        } else {
          this.secEtsDataService.dataStream.unsubscribe();
        }
      }
    )
  }

  setupDataService(){
    this.secEtsDataService.dataStream.subscribe(
      (data) => {
        console.log('SecDataSub:');
        console.log(data);
        if(data.dataResult && data.dataResult.success) {
          this.handleConfigData(data.dataResult.configData);
        }
        if(data.authenticated === false){
          this.secEtsDataService.setAuthenticated(false);
        }
      },
      (err) => {
         console.log(err);
      },
      () => {
         console.log("disconnected");
      }
    );
  }
   
   ngOnInit() {
   }

   handleConfigData(newConfigData) {
      this.configData = newConfigData;
      console.log(this.configData);
   }

   requestConfigData() {
      let request:WebsocketDataRequest = {
         getData: "config"
      };
      console.log("Getting data");
      this.secEtsDataService.getData(request);
   }

   handleConfigResult(configResult) {
      this.submitted = false;
      if(configResult.success) {
         console.log("Config saved successfully!");
      }
   }

   onSubmitConfig() {
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
      //this.secEtsDataService.sendConfig(newConfigArray);
      console.log("new config sent!");
    }
}
