import { Component, OnInit, Input, OnChanges, SimpleChanges, SimpleChange, Output, EventEmitter } from '@angular/core';
import { RaceState } from '../../../class/race-state';
import { RaceStateEnum, RaceCommandEnum } from '../../../enums/race-state.enum';

@Component({
   selector: 'app-race-control',
   templateUrl: './race-control.component.html',
   styleUrls: ['./race-control.component.scss']
})
export class RaceControlComponent implements OnChanges, OnInit {
   @Input() raceStates: RaceState;
   @Output() raceCommand = new EventEmitter<RaceCommandEnum>();
   startDisabled: boolean = true;
   stopDisabled: boolean = true;
   resetDisabled: boolean = true;

   overallMaxState: number;

   constructor() { }

   ngOnInit() {
      if (this.raceStates === undefined) {
         return;
      }
      this.UpdateDisabled();
   }

   ngOnChanges(changes: SimpleChanges) {
      const changedRaceStates: SimpleChange = changes.raceStates;
      this.raceStates = changedRaceStates.currentValue;
      this.UpdateDisabled();
   }

   UpdateDisabled() {
      this.overallMaxState = Math.max(...this.raceStates.RaceStates);

      this.startDisabled = !(this.overallMaxState == 0);
      this.stopDisabled = !(this.overallMaxState == 1 || this.overallMaxState == 2);
      this.resetDisabled = !(this.overallMaxState == 3);
   }

   EmitCommand(command: RaceCommandEnum) {
      this.raceCommand.emit(command);
   }
}
