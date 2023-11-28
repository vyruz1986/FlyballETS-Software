import { Pipe, PipeTransform } from '@angular/core';

@Pipe({
  name: 'batteryState'
})
export class BatteryStatePipe implements PipeTransform {

  transform(value: number): String {
    let returnString: String;
    if (value == 9911){
        returnString = "USB";
    } else if (value == 0) {
        returnString = "LOW";
    } else if (value == 9999) {
        returnString = "!!!";
    } else
        returnString = value.toString() + "%";

    return returnString;
  }

}