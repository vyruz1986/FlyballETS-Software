import { Component, OnInit, Input, OnChanges, SimpleChanges, SimpleChange, Output, EventEmitter } from '@angular/core';
import { RaceControl } from '../../../class/race-state';
import { RaceCommandEnum } from '../../../enums/race-state.enum';

@Component({
   selector: 'app-race-control',
   templateUrl: './race-control.component.html',
   styleUrls: ['./race-control.component.scss']
})
export class RaceControlComponent implements OnChanges, OnInit {
   @Input() raceControl: RaceControl;
   @Output() raceCommand = new EventEmitter<RaceCommandEnum>();
   @Output() dogsCommand = new EventEmitter<number>();
   @Output() setRerunsOff = new EventEmitter<boolean>();
   startDisabled: boolean = true;
   stopDisabled: boolean = true;
   resetDisabled: boolean = true;
   dogsChangeDisabled: boolean = true;
   rerunsOffDisabled: boolean = true;

   overallMaxState: number;

   constructor() { }

   numberOfDogs = [
      { id: 4, label: "4" },
      { id: 3, label: "3" },       
      { id: 2, label: "2" },
      { id: 1, label: "1" }
   ]

   setTagClass(overallMaxState) {
      switch (overallMaxState) {
      case 0: 
         return 'badge badge-pill badge-success';
      case 1: 
         return 'badge badge-pill badge-warning';
      case 2: 
         return 'badge badge-pill badge-danger';
      case 3: 
         return 'badge badge-pill badge-secondary';
      }
   }

   ngOnInit() {
      if (this.raceControl.RaceStates === undefined) {
         return;
      }
      this.UpdateDisabled();
   }

   ngOnChanges(changes: SimpleChanges) {
      const changedRaceControl: SimpleChange = changes.raceControl;
      this.raceControl = changedRaceControl.currentValue;
      //console.log("RaceStates is %o", this.raceControl.RaceStates);
      this.UpdateDisabled();
   }

   dogsChange(dogsValue: number) {
      this.dogsCommand.emit(dogsValue);
   }

   EmitRerunsOff(rerunsOff: boolean) {
      this.setRerunsOff.emit(rerunsOff);
   }

   UpdateDisabled() {
      this.overallMaxState = this.raceControl.RaceStates;
      this.startDisabled = !(this.overallMaxState == 0);
      this.stopDisabled = !(this.overallMaxState == 1 || this.overallMaxState == 2);
      this.resetDisabled = !(this.overallMaxState == 3);
      this.dogsChangeDisabled = !(this.overallMaxState == 0);
      this.rerunsOffDisabled = !(this.overallMaxState == 0);
   }

   EmitCommand(command: RaceCommandEnum) {
      this.raceCommand.emit(command);
   }
}
