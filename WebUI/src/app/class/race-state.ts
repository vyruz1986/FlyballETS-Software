import { RaceStateEnum } from '../enums/race-state.enum';

export class RaceControl {
   
   constructor(
      public RaceStates: RaceStateEnum,
      public RacingDogs: number,
      public RerunsOff: boolean
   ) { }
}
