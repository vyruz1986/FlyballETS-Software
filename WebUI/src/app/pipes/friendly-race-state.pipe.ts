import { Pipe, PipeTransform } from '@angular/core';

@Pipe({
  name: 'friendlyRaceState'
})
export class FriendlyRaceStatePipe implements PipeTransform {

  transform(raceState: number): String {
    let returnString: String;

    switch (raceState) {
      case 0:
        returnString = "Ready";
        break;
      case 1:
        returnString = "Starting";
        break;
      case 2:
        returnString = "Running";
        break;
      case 3:
        returnString = "Stopped";
        break;
      default:
        returnString = "Unknown";
        break;
    }

    return returnString;
  }

}
