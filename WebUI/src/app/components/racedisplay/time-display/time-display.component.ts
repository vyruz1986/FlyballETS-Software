import { Component, OnInit, Input } from '@angular/core';

@Component({
   selector: 'app-time-display',
   templateUrl: './time-display.component.html',
   styleUrls: ['./time-display.component.scss']
})

export class TimeDisplayComponent implements OnInit {

   msElapsedTime: string;
   msNetTime: string;

   @Input() elapsedTime: number;
   @Input() NetTime: number;
   
   constructor() { }

   ngOnInit() {
      this.formatElapsedTime(this.elapsedTime);
      this.formatNetTime(this.NetTime);

   }

   formatElapsedTime(d:number){
      var s = Math.floor(d % 100);
      var ms = Math.floor(d * 100);      
      this.msElapsedTime = ('  ' + s).slice(-2) + "." + ('0' + ms).slice(-2);
   }

   formatNetTime(d:number){
      var s = Math.floor(d % 100);
      var ms = Math.floor(d * 100);      
      this.msNetTime = ('     ' + s).slice(-5) + "." + ('0' + ms).slice(-2);
   }
}
