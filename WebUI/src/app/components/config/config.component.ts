import { Component, OnInit, OnDestroy } from "@angular/core";
import { WebsocketDataRequest } from "../../interfaces/websocket-data-request";
import { ConfigArray } from "../../interfaces/config-array";
import { ConfigData } from "../../class/config-data";
import { SecureEtsDataService } from "../../services/secure-ets-data.service";

@Component({
   selector: "app-config",
   templateUrl: "./config.component.html",
   styleUrls: ["./config.component.scss"],
})
export class ConfigComponent implements OnInit {
   sessionEnded: boolean;
   submitted: boolean = false;
   isAuthenticated: boolean = false;
   configData = new ConfigData("", "", "", "", "", "");

   constructor(public secEtsDataService: SecureEtsDataService) {}

   ngOnInit() {
      this.subscribeToAuth();
      this.subscribeToConnected();
   }

   subscribeToAuth() {
      this.secEtsDataService.isAuthenticated.subscribe(
         (authenticated: boolean) => {
            this.isAuthenticated = authenticated;
            if (authenticated) {
               this.secEtsDataService.initializeWebSocket();
               this.setupDataService();
            } else {
               console.log("Not authenticated. /wsa monitoring inactive.");
               this.secEtsDataService.dataStream.unsubscribe();
            }
         }
      );
   }

   subscribeToConnected() {
      this.secEtsDataService.isWsConnetced.subscribe(
         (connected: boolean) => {
            console.log(['Is wsa connected? = ', connected])
            if (connected) {
               this.requestConfigData();
            }
         }
      );
   }

   setupDataService() {
      this.secEtsDataService.dataStream.subscribe(
         (data) => {
            console.log("SecDataSub:");
            console.log(data);
            if (data.dataResult && data.dataResult.success) {
                  this.handleConfigData(data.dataResult.configData);
            }
            if (data.configResult) {
               this.submitted = false;
               if (data.configResult.success) {
                  console.log("Config saved successfully!");
                  this.sessionEnded = true;
               }
               else {
                  console.log("Config not saved!");
               }
            }
            if (data.authenticated === false) {
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

   handleConfigData(newConfigData) {
      this.configData = newConfigData;
      // console.log(this.configData);
   }

   requestConfigData() {
      let request: WebsocketDataRequest = {
         getData: "config",
      };
      console.log("Getting data");
      this.secEtsDataService.getData(request);
   }

   onSubmitConfig() {
      let newConfigArray: ConfigArray = {
         config: [
            { name: "APName", value: this.configData.APName },
            { name: "APPass", value: this.configData.APPass },
            { name: "AdminPass", value: this.configData.AdminPass },
            { name: "RunDirectionInverted", value: this.configData.RunDirectionInverted },
            { name: "StartingSequenceNAFA", value: this.configData.StartingSequenceNAFA },
            { name: "LaserOnTimer", value: this.configData.LaserOnTimer },
         ],
      };
      this.submitted = true;
      this.secEtsDataService.sendConfig(newConfigArray);
      console.log("new config sent!");
   }

   ngOnDestroy() {
      this.secEtsDataService.dataStream.unsubscribe();
   }

   reconnect() { }
}
