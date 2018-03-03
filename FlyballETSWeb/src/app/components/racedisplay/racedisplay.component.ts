import { Component, OnInit, OnDestroy } from '@angular/core';

import { Race } from '../../interfaces/race';
import { WebsocketAction } from '../../interfaces/websocketaction';
import { EtsdataService } from '../../services/etsdata.service';

@Component({
  selector: 'app-racedisplay',
  templateUrl: './racedisplay.component.html',
  styleUrls: ['./racedisplay.component.css']
})
export class RacedisplayComponent implements OnInit {

  currentRace:Race = {
    id: 0,
    startTime: 0,
    endTime: 0,
    elapsedTime: 0,
    totalCrossingTime: 0,
    raceState: 0,
    raceStateFriendly: "Stopped",
    dogData: []
  };

  startDisabled: boolean;
  stopDisabled: boolean;
  resetDisabled: boolean;
  constructor(public etsDataService:EtsdataService) {  //TODO why does making etsDataService private cause build to fail?
      this.etsDataService.dataStream.subscribe(
         (data) => {
            if(data.RaceData) {
               this.HandleCurrentRaceData(data.RaceData);
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

  startRace() {
     console.log('starting race');
     let StartAction:WebsocketAction = {
       action: "StartRace"
      };
     this.etsDataService.sendAction(StartAction);
  }

  stopRace() {
    console.log('stopping race');
    let StopAction:WebsocketAction = {
      action: "StopRace"
     };
    this.etsDataService.sendAction(StopAction);
  }
  resetRace() {
    console.log('resetting race');
    let StopAction:WebsocketAction = {
      action: "ResetRace"
     };
    this.etsDataService.sendAction(StopAction);
  }

  HandleCurrentRaceData(raceData:Race) {
    this.currentRace =  raceData;
    
    switch(this.currentRace.raceState) {
      case 0:
        this.currentRace.raceStateFriendly = "Stopped";
        break;
      case 1:
        this.currentRace.raceStateFriendly = "Starting";
        break;
      case 2:
        this.currentRace.raceStateFriendly = "Running";
        break;
      case 3:
        this.currentRace.raceStateFriendly = "Scheduled";
        break;
      default:
        this.currentRace.raceStateFriendly = "Unknown";
        break;
    }
    this.startDisabled = !(this.currentRace.raceState == 0 && this.currentRace.startTime == 0);
    this.stopDisabled = (this.currentRace.raceState == 0);
    this.resetDisabled = !(this.currentRace.raceState == 0 && this.currentRace.startTime != 0);
  }

  reconnect() {
  }

  
}
