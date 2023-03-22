import { Component, OnInit, Input, Output, EventEmitter } from '@angular/core';
import { DogData } from '../../../interfaces/race';
import { WebsocketAction } from '../../../interfaces/websocketaction';

@Component({
   selector: 'app-dog-details',
   templateUrl: './dog-details.component.html',
   styleUrls: ['./dog-details.component.scss']
})
export class DogDetailsComponent implements OnInit {
   @Input() dogData:Array<DogData> = [];
   @Input() raceIndex:number;
   @Output() setDogFault = new EventEmitter<{dogNum: number, fault: boolean}>();
   constructor() { }

   ngOnInit() {
   }

   EmitDogFault(dogNr: number, faultState: boolean) {
      let dogFault = {raceIndex: this.raceIndex, dogNum: dogNr, fault: faultState};
      this.setDogFault.emit(dogFault);
   }
}
