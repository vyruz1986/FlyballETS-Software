import { Component, OnInit } from '@angular/core';

import { Race } from '../../interfaces/race';
import { WebsocketService } from '../../services/websocket.service';
import { WebsocketAction } from '../../interfaces/websocketaction';

@Component({
  selector: 'app-racedisplay',
  templateUrl: './racedisplay.component.html',
  styleUrls: ['./racedisplay.component.css'],
  providers: [WebsocketService, WebsocketService]
})
export class RacedisplayComponent implements OnInit {

  currentRace:Race = {
    id: 0,
    startTime: 0,
    endTime: 0,
    elapsedTime: 0,
    raceState: 0,
    raceStateFriendly: "Stopped",
    dogTimes: [
      {dogNumber: 0,  time: 0, crossingTime: 0, fault: false, running: false},
      {dogNumber: 0,  time: 0, crossingTime: 0, fault: false, running: false},
      {dogNumber: 0,  time: 0, crossingTime: 0, fault: false, running: false},
      {dogNumber: 0,  time: 0, crossingTime: 0, fault: false, running: false}
    ]
  };

  startDisabled: boolean;
  stopDisabled: boolean;
  resetDisabled: boolean;

  isConnected: boolean;
  sessionEnded: boolean;

  constructor(private raceDataService:WebsocketService) {
    this.raceDataService = raceDataService;
    this.initiateConnection();
  }

  ngOnInit() {
     this.isConnected = false;
     this.sessionEnded = false;
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
    let StopAction:WebsocketAction = {
      action: "ResetRace"
     };
    this.raceDataService.sendAction(StopAction);
  }

  HandleCurrentRaceData(raceData) {
    //console.log("Received current race data!");
    this.currentRace.elapsedTime = raceData.ElapsedTime;
    this.currentRace.raceState = raceData.RaceState;
    this.currentRace.startTime = raceData.StartTime;
    this.currentRace.endTime = raceData.EndTime;
    this.currentRace.dogTimes = raceData.DogData;
    for (let i in raceData.DogData) {
      this.currentRace.dogTimes[i].dogNumber = raceData.DogData[i].DogNumber;
      this.currentRace.dogTimes[i].time = raceData.DogData[i].Time;
      this.currentRace.dogTimes[i].crossingTime = raceData.DogData[i].CrossingTime;
      this.currentRace.dogTimes[i].fault = raceData.DogData[i].Fault;
      this.currentRace.dogTimes[i].running = raceData.DogData[i].Running;
    }

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
    this.startDisabled = !(this.currentRace.raceState == 0 && this.currentRace.startTime == 0);
    this.stopDisabled = (this.currentRace.raceState == 0);
    this.resetDisabled = !(this.currentRace.raceState == 0 && this.currentRace.startTime != 0);
  }

  reconnect() {
    this.initiateConnection();
  }

  initiateConnection() {
    console.log("Attempting WS connection!");
    this.isConnected = false;
    this.sessionEnded = false;
    this.raceDataService.dataStream.subscribe(
      msg => {
        this.isConnected = true;
        //console.log("Response from websocket: ");
        //console.log(msg);
        if(msg.RaceData) {
          this.HandleCurrentRaceData(msg.RaceData);
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
    )
  }
}
