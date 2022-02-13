import { Component, OnInit, OnDestroy } from '@angular/core';
import { SystemData } from '../../interfaces/system-data';
import { EtsdataService } from '../../services/etsdata.service';

@Component({
  selector: 'app-system-data',
  templateUrl: './system-data.component.html',
  styleUrls: ['./system-data.component.scss']
})
export class SystemDataComponent implements OnInit {

  hmsTime:string;

  systemData:SystemData = {batteryPercentage: 0, PwrOnTag: 0, RaceID: 0, uptime: 0, systemTimestamp: null, numClients: 0, freeHeap: 0};
  constructor(private etsDataService:EtsdataService) {
    this.etsDataService.dataStream.subscribe((data) => {
      if(data.SystemData) {
        this.systemData = <SystemData>data.SystemData;
        this.formatHMSTime(data.SystemData.uptime / 1000);
      }
    });
  }  

  formatHMSTime(d:number){
    
    var h = Math.floor(d / 3600);
    var m = Math.floor(d % 3600 / 60);
    var s = Math.floor(d % 3600 % 60 / 10);
    
    this.hmsTime = ('0' + h).slice(-1) + ":" + ('0' + m).slice(-2) + ":" + ('0' + s).slice(-1);
  }

  ngOnInit() {}

  ngOnDestroy() {
    this.etsDataService.dataStream.unsubscribe();
  }
}
