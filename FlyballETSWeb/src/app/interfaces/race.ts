import { RaceState } from '../enums/race-enums.enum';
export interface DogTimingData{
   time: Number, crossingTime: Number
}
export interface DogData{
   dogNumber: Number
   , timing:Array<DogTimingData>
   , fault: boolean
   , running: boolean
}
export interface RaceData {
   id: Number,
   startTime: Number,
   endTime: Number,
   elapsedTime: Number,
   totalCrossingTime: Number,
   raceState: RaceState,
   raceStateFriendly: String,
   dogData: Array<DogData>
}

export interface RaceDataArray {
  Array<RaceData>
}