export class ConfigData {
   constructor(
      public APName: string,
      public APPass: string,
      public AdminPass: string,
      public RunDirectionInverted: string,
      public StartingSequenceNAFA: string,
      public LaserOnTimer: string,
      public Accuracy3digits: string,
      public CommaInCsv: string
   ) { }
}