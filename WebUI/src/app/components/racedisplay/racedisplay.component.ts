import { Component, OnInit, OnDestroy } from "@angular/core";

import { RaceData } from "../../interfaces/race";
import { WebsocketAction } from "../../interfaces/websocketaction";
import { EtsdataService } from "../../services/etsdata.service";
import { RaceState } from "../../class/race-state";
import { RaceCommandEnum } from "../../enums/race-state.enum";
import { LightStates } from "../../interfaces/light-states";

@Component({
   selector: "app-racedisplay",
   templateUrl: "./racedisplay.component.html",
   styleUrls: ["./racedisplay.component.scss"],
})
export class RacedisplayComponent implements OnInit {
   currentRaces: RaceData[] = [];

   raceStates: RaceState = { RaceStates: [], StartTimes: [] };

   lightStates: LightStates[] = [{ State: [0, 0, 0, 0, 0] }];

   constructor(public etsDataService: EtsdataService) {
      //TODO why does making etsDataService private cause build to fail?
      this.etsDataService.dataStream.subscribe(
         (data) => {
            if (data.RaceData) {
               // Temporary fix to deal with response from ETS which does not send multiple race data
               this.HandleCurrentRaceData([data.RaceData]);
            } else if (data.LightsData) {
               this.lightStates[0].State = data.LightsData;
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

   ngOnInit() {}

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
      }
      this.etsDataService.sendAction(action);
   }

   onSetDogFault(dogFault: { raceNum: number; dogNum: number; fault: boolean }) {
      console.log("Setting fault for race %i, dog %i to value: %o", dogFault.raceNum, dogFault.dogNum, dogFault.fault);
      let StopAction: WebsocketAction = {
         actionType: "SetDogFault",
         actionData: {
            dogNumber: dogFault.dogNum,
            faultState: dogFault.fault,
         },
      };
      this.etsDataService.sendAction(StopAction);
   }

   HandleCurrentRaceData(raceData: RaceData[]) {
      this.currentRaces = [];
      raceData.forEach((element) => {
         if (typeof element == "object") {
            this.currentRaces.push(element);
         }
      });
      this.raceStates = { RaceStates: [], StartTimes: [] };
      this.currentRaces.forEach((element) => {
         this.raceStates.RaceStates.push(element.raceState);
         this.raceStates.StartTimes.push(element.startTime);
      });
   }

   reconnect() {}
}
