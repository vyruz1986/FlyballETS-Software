import { Pipe, PipeTransform } from '@angular/core';
import { DatePipe } from '@angular/common';

@Pipe({
  name: 'dogCrossingTime'
})
export class DogCrossingTimePipe implements PipeTransform {

  constructor(private datePipe:DatePipe){}

  transform(value: number): String {
    let returnString: String;
    if(value < 0){
      returnString = "- ";
    } else {
      returnString = "+ ";
    }

    let absValue = Math.abs(value);
    returnString += this.datePipe.transform(absValue, 's.SSS');
    return returnString;
  }

}
