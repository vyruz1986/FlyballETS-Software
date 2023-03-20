import { RaceStateEnum } from '../enums/race-state.enum';
export interface DogTimingData {
   time: String,
   crossing: String
}
export interface DogData {
   dogNr: number,
   fault: boolean,
   running: boolean,
   timing:Array<DogTimingData>
}
export interface RaceData {
   id: number,
   elapsedTime: String,
   cleanTime: String,
   raceState: RaceStateEnum,
   racingDogs: number,
   rerunsOff: boolean,
   dogData: Array<DogData>
}