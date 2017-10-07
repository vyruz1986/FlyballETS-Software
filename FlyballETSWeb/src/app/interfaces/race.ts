import { RaceState } from '../enums/race-enums.enum';

export interface Race {
   id: number,
   startTime: Date,
   endTime: Date,
   elapsedTime: Number,
   raceState: RaceState
}
