import { Component, OnInit, Input } from '@angular/core';

@Component({
   selector: 'app-time-display',
   templateUrl: './time-display.component.html',
   styleUrls: ['./time-display.component.scss']
})

export class TimeDisplayComponent implements OnInit {

   /*msElapsedTime: string;
   msCleanTime: string;*/

   @Input() elapsedTime: String;
   @Input() cleanTime: String;
   
   constructor() { }

   ngOnInit() { }
   /*ngOnInit() {
      this.formatElapsedTime(this.elapsedTime);
      this.formatCleanTime(this.cleanTime);

   }

   formatElapsedTime(d:number){
      var s = Math.floor(d % 100);
      var ms = Math.floor(d * 100);      
      this.msElapsedTime = ('  ' + s).slice(-2) + "." + ('0' + ms).slice(-2);
   }

   formatCleanTime(d:number){
      var s = Math.floor(d % 100);
      var ms = Math.floor(d * 100);      
      this.msCleanTime = ('     ' + s).slice(-5) + "." + ('0' + ms).slice(-2);
   }*/
}
