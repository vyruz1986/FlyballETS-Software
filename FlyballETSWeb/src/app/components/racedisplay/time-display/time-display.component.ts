import { Component, OnInit, Input } from '@angular/core';

@Component({
   selector: 'app-time-display',
   templateUrl: './time-display.component.html',
   styleUrls: ['./time-display.component.css']
})
export class TimeDisplayComponent implements OnInit {
   @Input() elapsedTime:number;
   @Input() totalCrossingTime: number;
   constructor() { }

   ngOnInit() {
   }

}
