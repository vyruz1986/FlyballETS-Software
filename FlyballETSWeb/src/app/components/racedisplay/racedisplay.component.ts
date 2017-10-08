import { Component, OnInit } from '@angular/core';

import { Race } from '../../interfaces/race';
import { RaceDataService } from '../../services/race-data.service';
import { WebsocketService } from '../../services/websocket.service';
import { WebsocketAction } from '../../interfaces/websocketaction';

@Component({
  selector: 'app-racedisplay',
  templateUrl: './racedisplay.component.html',
  styleUrls: ['./racedisplay.component.css'],
  providers: [WebsocketService, RaceDataService]
})
export class RacedisplayComponent implements OnInit {

  currentRace:Race = {
    id: 0,
    startTime: 0,
    endTime: 0,
    elapsedTime: 0,
    raceState: 0,
    raceStateFriendly: "Stopped"
  };

  constructor(private raceDataService:RaceDataService) {
    raceDataService.dataStream.subscribe(
      msg => {
        console.log("Response from websocket: ");
        console.log(msg);
        if(msg.RaceData) {
          this.HandleCurrentRaceData(msg.RaceData);
        }
      },
      err => {console.log(err)},
      () => {console.log("complete")}
    )
  }

  ngOnInit() {
     //this.currentRace = this.raceDataService.getRaceData();
  }

  startRace() {
     console.log('starting race');
     let StartAction:WebsocketAction = {
       action: "StartRace"
      };
     this.raceDataService.sendAction(StartAction);
  }

  stopRace() {
    console.log('stopping race');
    let StopAction:WebsocketAction = {
      action: "StopRace"
     };
    this.raceDataService.sendAction(StopAction);
  }
  resetRace() {
    console.log('resetting race');
  }

  HandleCurrentRaceData(raceData) {
    console.log("Received current race data!");
    this.currentRace.elapsedTime = raceData.ElapsedTime;
    this.currentRace.raceState = raceData.RaceState;
    switch(raceData.RaceState) {
      case 0:
        this.currentRace.raceStateFriendly = "Stopped";
        break;
      case 1:
        this.currentRace.raceStateFriendly = "Starting";
        break;
      case 2:
        this.currentRace.raceStateFriendly = "Running";
        break;
    }
  }
}
