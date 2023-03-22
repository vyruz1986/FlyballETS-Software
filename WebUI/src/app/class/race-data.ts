import { RaceStateEnum } from '../enums/race-state.enum';

export class cDogTimingData {
    constructor(
        public time: String,
        public crossing: String
    ) { }
}

export class cDogData {
    constructor(
        public dogNr: number,
        public fault: boolean,
        public running: boolean,
        public timing: [cDogTimingData, cDogTimingData, cDogTimingData, cDogTimingData]
    ) { }
}

export class cRaceData {
    constructor(
        public id: number,
        public elapsedTime: String,
        public cleanTime: String,
        public raceState: RaceStateEnum,
        public racingDogs: number,
        public rerunsOff: boolean,
        public dogData: [cDogData, cDogData, cDogData, cDogData]
    ) { }
}