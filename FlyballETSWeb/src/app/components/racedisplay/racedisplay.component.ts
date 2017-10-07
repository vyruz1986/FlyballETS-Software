import { Component, OnInit } from '@angular/core';

import { Race } from '../../interfaces/race';
import { RaceDataService } from '../../services/race-data.service'; 

@Component({
  selector: 'app-racedisplay',
  templateUrl: './racedisplay.component.html',
  styleUrls: ['./racedisplay.component.css'],
  providers: [RaceDataService]
})
export class RacedisplayComponent implements OnInit {

  currentRace:Race;

  constructor(private raceDataService:RaceDataService) { }

  ngOnInit() {
     //this.currentRace = this.raceDataService.getRaceData();
  }

  startRace() {
     console.log('starting race');
     this.raceDataService.sendAction({ action: "StartRace" });
  }

  stopRace() {
    console.log('stopping race');
  }
  resetRace() {
    console.log('resetting race');
  }
}
