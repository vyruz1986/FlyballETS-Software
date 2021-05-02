import { RaceStateEnum } from '../enums/race-state.enum';
export interface DogTimingData{
   time: String, crossingTime: String
}
export interface DogData{
   dogNumber: number
   , timing:Array<DogTimingData>
   , fault: boolean
   , running: boolean
}
export interface RaceData {
   id: number,
   startTime: number,
   endTime: number,
   elapsedTime: number,
   totalCrossingTime: number,
   raceState: RaceStateEnum,
   raceStateFriendly: String,
   dogData: Array<DogData>
}