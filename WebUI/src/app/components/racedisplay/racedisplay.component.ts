import { Component, OnInit, OnDestroy } from "@angular/core";

import { RaceData } from "../../interfaces/race";
import { WebsocketAction } from "../../interfaces/websocketaction";
import { EtsdataService } from "../../services/etsdata.service";
import { RaceControl } from "../../class/race-state";
import { cRaceData } from "../../class/race-data";
import { RaceCommandEnum } from "../../enums/race-state.enum";
import { LightStates } from "../../interfaces/light-states";

@Component({
   selector: "app-racedisplay",
   templateUrl: "./racedisplay.component.html",
   styleUrls: ["./racedisplay.component.scss"],
})
export class RacedisplayComponent implements OnInit {
   currentRaces: RaceData[] = [];

   raceControl: RaceControl = { RaceStates: undefined, RacingDogs: undefined, RerunsOff: undefined };

   lightStates: LightStates = { State: [0, 0, 0, 0, 0] };

   constructor(public etsDataService: EtsdataService) {
      //TODO why does making etsDataService private cause build to fail?
      this.etsDataService.dataStream.subscribe(
         (data) => {
            if (data.RaceData) {
               // Temporary fix to deal with response from ETS which does not send multiple race data
               this.HandleCurrentRaceData([data.RaceData]);
            } else if (data.LightsData) {
               this.lightStates.State = data.LightsData;
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
      this.onRaceCommand(3);
   }

   ngOnDestroy() {
      this.etsDataService.dataStream.unsubscribe();
   }

   HandleCurrentRaceData(raceData: RaceData[]) {
      if (this.currentRaces.length > 0){
         this.MergeRaceData(raceData);
      }
      else {
         this.currentRaces = [];
         raceData.forEach((element) => {
            if (typeof element == "object") {
               this.currentRaces.push(element);
            }
         });
      }
      this.UpdateRaceControl();
   }

   MergeRaceData(raceData: RaceData[]){
      let previousRaceData = this.currentRaces[0];
      let newRaceData = raceData[0];
      // ToDo read keys from interface
      Object.keys(previousRaceData).forEach((name) => {
         if (newRaceData[name] === undefined){
            newRaceData[name] = previousRaceData[name];
         }
      });
      this.currentRaces = [];
      this.currentRaces.push(newRaceData);
   }

   UpdateRaceControl(): void {
      this.raceControl = {
         RaceStates: this.currentRaces[0].raceState,
         RacingDogs: this.currentRaces[0].racingDogs,
         RerunsOff: this.currentRaces[0].rerunsOff
      };
      //console.log("RaceStates in racedisplay is %o", this.raceControl.RaceStates);
   }

   onRaceCommand(raceCommand: RaceCommandEnum) {
      let action: WebsocketAction = { actionType: "" };
      switch (raceCommand) {
         case RaceCommandEnum.CMD_START:
            action.actionType = "StartRace";
            break;
         case RaceCommandEnum.CMD_STOP:
            action.actionType = "StopRace";
            break;
         case RaceCommandEnum.CMD_RESET:
            action.actionType = "ResetRace";
            break;
         case RaceCommandEnum.CMD_UPDATE:
            action.actionType = "UpdateRace";
            break;
      }
      this.etsDataService.sendAction(action);
   }

   dogsChange(dogsCommand: number) {
      console.log(dogsCommand);
      if (dogsCommand == 3)
      {
         let action: WebsocketAction = { actionType: "SetDogs3" };
         this.etsDataService.sendAction(action);
      }
      else if (dogsCommand == 2)
      {
         let action: WebsocketAction = { actionType: "SetDogs2" };
         this.etsDataService.sendAction(action);
      }
      else if (dogsCommand == 1)
      {
         let action: WebsocketAction = { actionType: "SetDogs1" };
         this.etsDataService.sendAction(action);
      }
      else
      {
         let action: WebsocketAction = { actionType: "SetDogs4" };
         this.etsDataService.sendAction(action);
      }
   }

   onSetDogFault(dogFault: { raceNum: number; dogNum: number; fault: boolean }) {
      console.log("Setting fault for race %i, dog %i to value: %o", dogFault.raceNum, dogFault.dogNum, dogFault.fault);
      let DogAction: WebsocketAction = {
         actionType: "SetDogFault",
         actionData: {
            dogNr: dogFault.dogNum,
            faultState: dogFault.fault,
         },
      };
      this.etsDataService.sendAction(DogAction);
   }

   onSetRerunsOff(rerunsOff: boolean) {
      console.log("Setting reruns off to %o", rerunsOff);
      let Action: WebsocketAction = {
         actionType: "SetRerunsOff",
         actionData: { rerunsOff: rerunsOff, },
      };
      this.etsDataService.sendAction(Action);
   }

   reconnect() {
      this.onRaceCommand(3);
   }
}
