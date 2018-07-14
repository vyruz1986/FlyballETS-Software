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
   @Output() setDogFault = new EventEmitter<{dogNum, fault}>();
   constructor() { }

   ngOnInit() {
   }

   EmitDogFault(dogNumber: number, faultState: boolean) {
      let dogFault = {raceIndex: this.raceIndex, dogNum: dogNumber, fault: faultState};
      this.setDogFault.emit(dogFault);
   }

}
