import { RaceStateEnum } from '../enums/race-state.enum';
export interface DogTimingData{
   time: String, crossingTime: String
}
export interface DogData{
   dogNumber: number,
   timing:Array<DogTimingData>,
   fault: boolean,
   running: boolean
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