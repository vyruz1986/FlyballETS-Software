import { RaceStateEnum } from '../enums/race-state.enum';

export class RaceState {
   
   constructor(
      public RaceStates: Array<RaceStateEnum>,
      public StartTimes: Array<number>,
      public RacingDogs: Array<number>
   ) { }
}
