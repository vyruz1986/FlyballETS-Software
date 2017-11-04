import { Component, OnInit, OnDestroy } from '@angular/core';
import { EtsdataService } from '../../services/etsdata.service';
import { LightStates } from '../../interfaces/light-states';

@Component({
   selector: 'app-lights',
   templateUrl: './lights.component.html',
   styleUrls: ['./lights.component.css']
})
export class LightsComponent implements OnInit {

   lightStates:LightStates = {State:[]};
   
   constructor(private etsDataService:EtsdataService) {
      this.etsDataService.dataStream.subscribe((data) => {
         if(data.LightsData) {
            console.log(data.LightsData);
            this.lightStates.State = data.LightsData;
         }
      });
   }

   ngOnInit() {
      this.lightStates.State = []
   }

}
