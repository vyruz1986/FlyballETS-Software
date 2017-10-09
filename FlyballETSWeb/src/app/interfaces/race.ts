import { RaceState } from '../enums/race-enums.enum';

export interface Race {
   id: Number,
   startTime: Number,
   endTime: Number,
   elapsedTime: Number,
   raceState: RaceState,
   raceStateFriendly: String,
   dogTimes: Array<{dogNumber: Number, time: Number, crossingTime: Number, fault: boolean, running: boolean}>
}
