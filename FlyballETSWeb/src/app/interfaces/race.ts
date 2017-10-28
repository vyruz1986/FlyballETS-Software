import { RaceState } from '../enums/race-enums.enum';

export interface Race {
   id: Number,
   startTime: Number,
   endTime: Number,
   elapsedTime: Number,
   raceState: RaceState,
   raceStateFriendly: String,
   dogData: Array<{
      dogNumber: Number
      ,timing:Array<{
         time: Number, crossingTime: Number,
      }>
      , fault: boolean
      , running: boolean}>
}
