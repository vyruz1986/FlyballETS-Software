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

   isConnected: boolean;
   sessionEnded: boolean;
   submitted: boolean;

   configData = new ConfigData("","","");

   constructor(public secEtsDataService:SecureEtsDataService) {//TODO why does making etsDataService private cause build to fail?
      this.subscribeToDataService();
   }

   subscribeToDataService() {
      this.secEtsDataService.dataStream.subscribe((data) => {
         if(data.dataResult && data.dataResult.success && data.dataResult.configData) {
            this.handleConfigData(data.dataResult.configData);
         } else if (data.configResult) {
            this.handleConfigResult(data.configResult);
         }
      });
   }
   
   ngOnInit() {
      this.submitted = false;
      console.log("OnInit");
      this.secEtsDataService.connectionStatus.subscribe((connected) => {
         if(connected){
            this.requestConfigData();
         }
      });
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
      this.secEtsDataService.sendConfig(newConfigArray);
      console.log("new config sent!");
    }
}
