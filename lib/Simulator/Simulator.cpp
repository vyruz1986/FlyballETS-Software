// file:	Simulator.cpp
//
// summary:	Implements the simulator class. Since this class is memory intensive, it should only be
// included if actual simulation is wished.
#include "Simulator.h"

/// <summary>
///   Program the interrupt triggers which should be simulated here. See end of file for a collection of records from actual races.
/// </summary>
const SimulatorClass::SimulatorRecord SimulatorClass::SimulatorQueue[60] PROGMEM = {
    //See the end of this file for a collection of races
    {1, -102316, 1},
    {2, -87824, 1},
    {2, -80456, 0},
    {2, -78800, 1},
    {1, 28952, 0},
    {2, 44596, 0},
    {2, 4364748, 1},
    {2, 4366600, 0},
    {2, 4368436, 1},
    {1, 4380404, 1},
    {1, 4382296, 0},
    {1, 4384164, 1},
    {2, 4489344, 0},
    {1, 4505604, 0},
    {1, 4730036, 1},
    {2, 4744732, 1},
    {1, 4857296, 0},
    {2, 4872020, 0},
    {2, 9344132, 1},
    {1, 9361276, 1},
    {1, 9552148, 0},
    {2, 9575988, 0},
    {2, 15477068, 1},
    {1, 15494804, 1},
    {1, 15549608, 0},
    {1, 15563036, 1},
    {2, 15580812, 0},
    {1, 15594584, 0},
    {1, 15610220, 1},
    {1, 15613904, 0},
    {1, 15909920, 1},
    {2, 15925776, 1},
    {1, 16042356, 0},
    {2, 16060036, 0},
    {2, 20674000, 1},
    {1, 20690804, 1},
    {2, 20795000, 0},
    {1, 20803392, 0},
    {1, 21335620, 1},
    {2, 21355152, 1},
    {2, 21469528, 0},
    {2, 21473956, 1},
    {1, 21497288, 0},
    {2, 21521176, 0},
    {2, 26444356, 1},
    {1, 26461276, 1},
    {2, 26582668, 0},
    {1, 26600304, 0},
    {1, 34022032, 1},
    {2, 34130784, 1},
    {2, 35350988, 0},
    {2, 35720860, 1},
    {2, 35731936, 0},
    {2, 35768820, 1},
    {2, 47982712, 0},
    {1, 48095780, 0}

};

/// <summary>
///   Initialises this object.
/// </summary>
///
/// <param name="iS1Pin">  Zero-based index of the S1 pin. </param>
/// <param name="iS2Pin">  Zero-based index of the S2 pin. </param>
void SimulatorClass::init(uint8_t iS1Pin, uint8_t iS2Pin)
{
   _iS1Pin = iS1Pin;
   //pinMode(_iS1Pin, OUTPUT);
   //digitalWrite(_iS1Pin, LOW);
   _iS2Pin = iS2Pin;
   //pinMode(_iS2Pin, OUTPUT);
   //digitalWrite(_iS2Pin, LOW);
   _iDataPos = 0;
   PROGMEM_readAnything(&SimulatorQueue[_iDataPos], PendingRecord);
   ESP_LOGI(__FILE__, "Simulator started!\r\n");
}

/// <summary>
///   Main entry-point for this application. Should be called every main loop cycle if simulation is wished.
/// </summary>
void SimulatorClass::Main()
{
   if (RaceHandler.RaceState == RaceHandler.STOPPED)
   {
      if (_iDataPos != 0)
      {
         //We've stopped a race, reset data position to 0
         _iDataPos = 0;
         PROGMEM_readAnything(&SimulatorQueue[_iDataPos], PendingRecord);
      }
      return;
   }
   if (PendingRecord.lTriggerTime == 0)
   {
      //Pending record doesn't contain valid data, this means we've reched the end of our queue
      return;
   }
   //Simulate sensors
   if (RaceHandler.RaceState != RaceHandler.STOPPED && PendingRecord.lTriggerTime <= (long)RaceHandler._lRaceTime)
   {
      if (RaceHandler._QueueEmpty())
      {
         if ((PendingRecord.lTriggerTime < 0 && RaceHandler.RaceState == RaceHandler.STARTING) || (PendingRecord.lTriggerTime > 0 && RaceHandler.RaceState == RaceHandler.RUNNING))
         {
            RaceHandler._QueuePush({PendingRecord.iSensorNumber, (RaceHandler._lRaceStartTime + PendingRecord.lTriggerTime), PendingRecord.iState});
            //And increase pending record
            _iDataPos++;
            PROGMEM_readAnything(&SimulatorQueue[_iDataPos], PendingRecord);
         }
      }
   }
}

/// <summary>
///   The simulator class object.
/// </summary>
SimulatorClass Simulator;

/*
Race1:
   //Dog1
   { 1, 41708, 1 }
   , { 2, 60692, 1 }
   , { 1, 176848, 0 }
   , { 2, 197732, 0 }
   //Dog1<->Dog2
   , { 1, 4675580, 1 }  //This is the 2nd dog coming in too erly!
   , { 2, 4692252, 1 }
   , { 2, 4830180, 0 }
   , { 1, 4849032, 0 }  //Sequence was ABba: This proves 2 dogs passed --> Perfect crossing
   //Dog2<->Dog3
   , { 2, 9058416, 1 }
   , { 1, 9074780, 1 }
   , { 1, 9215868, 0 }
   , { 2, 9234968, 0 }  //Sequence was BAab: This proves 2 dogs passed --> Perfect crossing
   //Dog3<->Dog4
   , { 2, 13497276, 1 }
   , { 2, 13500924, 0 }
   , { 2, 13502728, 1 }
   , { 1, 13514992, 1 }
   , { 1, 13668000, 0 }
   , { 1, 13669344, 1 }
   , { 1, 13681828, 0 }
   , { 2, 13701464, 0 } //BbBAaAaB --> Filtered to BbAa: This proves 2 dogs passed --> Perfect crossing
   //Waiting for dog4 to come back
   , { 2, 18028788, 1 }//Dog 4 coming in, race finished

Race2:
   { 1, -31596, 1 }
   , { 1, -29720, 0 }
   , { 1, -26480, 1 }
   , { 2, -13288, 1 }
   , { 1, 79012, 0 }
   , { 2, 95292, 0 }    //AaABab --> ABab: Dog0 going in

   , { 2, 3907168, 1 }
   , { 2, 3910840, 0 }
   , { 2, 3914488, 1 }
   , { 1, 3920676, 1 }
   , { 1, 3924420, 0 }
   , { 1, 3926228, 1 }
   , { 2, 4007708, 0 } 
   , { 2, 4010724, 1 }
   , { 2, 4014444, 0 }
   , { 1, 4018772, 0 }
   , { 1, 4022160, 1 }
   , { 1, 4023996, 0 } ////BbBAaAbBbaAa --> BAba: Dog 0 coming back

   , { 1, 4099916, 1 }
   , { 2, 4117636, 1 }
   , { 1, 4229376, 0 }
   , { 2, 4245756, 0 }  //ABab: Dog 1 going in

   , { 1, 8396072, 1 }
   , { 2, 8410032, 1 }
   , { 2, 8411868, 0 }
   , { 2, 8413120, 1 }
   , { 2, 8529568, 0 }
   , { 1, 8547784, 0 }  //ABbBba --> ABba : Perfect crossing: Dog 1 coming back, dog2 going in

   , { 2, 12516324, 1 }
   , { 2, 12521824, 0 }
   , { 2, 12523644, 1 }
   , { 1, 12533588, 1 }
   , { 1, 12537336, 0 }
   , { 1, 12537964, 1 }
   , { 2, 12629576, 0 }
   , { 1, 12634732, 0 } //BbBAaAbaAa --> BAba: Dog2 coming back

   //, { 1, 12641944, 1 }
   //, { 1, 12643772, 0 } //Aa: ???

Race3 (GT Team 4):
   { 1, -102316, 1 }
   , { 2, -87824, 1 }
   , { 2, -80456, 0 }
   , { 2, -78800, 1 }
   , { 1, 28952, 0 }
   , { 2, 44596, 0 }
   , { 2, 4364748, 1 }
   , { 2, 4366600, 0 }
   , { 2, 4368436, 1 }
   , { 1, 4380404, 1 }
   , { 1, 4382296, 0 }
   , { 1, 4384164, 1 }
   , { 2, 4489344, 0 }
   , { 1, 4505604, 0 }
   , { 1, 4730036, 1 }
   , { 2, 4744732, 1 }
   , { 1, 4857296, 0 }
   , { 2, 4872020, 0 }
   , { 2, 9344132, 1 }
   , { 1, 9361276, 1 }
   , { 1, 9552148, 0 }
   , { 2, 9575988, 0 }
   , { 2, 15477068, 1 }
   , { 1, 15494804, 1 }
   , { 1, 15549608, 0 }
   , { 1, 15563036, 1 }
   , { 2, 15580812, 0 }
   , { 1, 15594584, 0 }
   , { 1, 15610220, 1 }
   , { 1, 15613904, 0 }
   , { 1, 15909920, 1 }
   , { 2, 15925776, 1 }
   , { 1, 16042356, 0 }
   , { 2, 16060036, 0 }
   , { 2, 20674000, 1 }
   , { 1, 20690804, 1 }
   , { 2, 20795000, 0 }
   , { 1, 20803392, 0 }
   , { 1, 21335620, 1 }
   , { 2, 21355152, 1 }
   , { 2, 21469528, 0 }
   , { 2, 21473956, 1 }
   , { 1, 21497288, 0 }
   , { 2, 21521176, 0 }
   , { 2, 26444356, 1 }
   , { 1, 26461276, 1 }
   , { 2, 26582668, 0 }
   , { 1, 26600304, 0 }
   , { 1, 34022032, 1 }
   , { 2, 34130784, 1 }
   , { 2, 35350988, 0 }
   , { 2, 35720860, 1 }
   , { 2, 35731936, 0 }
   , { 2, 35768820, 1 }
   , { 2, 47982712, 0 }
   , { 1, 48095780, 0 }

   //Race GR T4 - 25/07/2015 (3th run in movie MAH00065.MP4)
   { 1, -29236, 1 }
   , { 2, -13724, 1 }
   , { 1, 101080, 0 }
   , { 2, 118704, 0 }
   , { 2, 4431492, 1 }
   , { 2, 4433344, 0 }
   , { 2, 4437024, 1 }
   , { 1, 4444444, 1 }
   , { 1, 4448220, 0 }
   , { 1, 4451816, 1 }
   , { 2, 4555588, 0 }
   , { 1, 4571884, 0 }
   , { 1, 4764900, 1 }
   , { 2, 4783296, 1 }
   , { 1, 4891888, 0 }
   , { 2, 4907432, 0 }
   , { 2, 9345748, 1 }
   , { 1, 9366044, 1 }
   , { 2, 9487728, 0 }
   , { 1, 9505208, 0 }
   , { 1, 9611220, 1 }
   , { 2, 9634504, 1 }
   , { 1, 9750116, 0 }
   , { 2, 9766092, 0 }
   , { 2, 14489980, 1 }
   , { 1, 14506052, 1 }
   , { 2, 14614192, 0 }
   , { 1, 14635252, 0 }
   , { 1, 15158440, 1 }
   , { 2, 15186624, 1 }
   , { 1, 15329176, 0 }
   , { 2, 15348496, 0 }
   , { 2, 20337320, 1 }
   , { 1, 20354000, 1 }
   , { 2, 20479384, 0 }
   , { 1, 20496216, 0 } //This was the end of the run, dog 0 was early but did not come back


   //Race GR T4 - 25/07/2015 (4th run in movie MAH00065.MP4)
   { 1, 796, 1 }
   , { 2, 17748, 1 }
   , { 2, 23276, 0 }
   , { 2, 24332, 1 }
   , { 1, 134032, 0 }
   , { 2, 150208, 0 }
   , { 1, 4383940, 1 }
   , { 2, 4389808, 1 }
   , { 2, 4525776, 0 }
   , { 1, 4528556, 0 }
   , { 2, 9092276, 1 }
   , { 1, 9108100, 1 }
   , { 2, 9228060, 0 }
   , { 2, 9237604, 1 }
   , { 1, 9354632, 0 }
   , { 2, 9370068, 0 }  //Gates clear!

   //The next 2 records are 'bogus' signals that were picked up, shortly after the gate was cleared, not visible on 120fps video
   //Code has been implemented to filter these out.
   , { 2, 9370240, 1 }
   , { 2, 9373992, 0 }

   , { 2, 14112436, 1 }
   , { 1, 14127916, 1 }
   , { 2, 14233032, 0 }
   , { 1, 14250128, 0 }
   , { 1, 14656284, 1 }
   , { 2, 14670520, 1 }
   , { 2, 14799688, 0 }
   , { 2, 14800436, 1 }
   , { 1, 14817364, 0 }
   , { 2, 14840280, 0 }
   , { 2, 20071076, 1 }
   , { 1, 20091288, 1 }
   , { 2, 20221648, 0 }
   , { 1, 20249052, 0 }
   , { 1, 25835668, 1 }
   , { 2, 25849524, 1 }
   , { 1, 25956512, 0 }
   , { 2, 25975860, 0 }
   , { 2, 30514448, 1 }
   , { 1, 30531412, 1 }
   , { 2, 30663400, 0 }
   , { 1, 30682888, 0 }

   //Race GR T4 - 25/07/2015 (5th run in movie MAH00065.MP4)
   { 1, -28800, 1 }
   , { 2, -10224, 1 }
   , { 1, 104840, 0 }
   , { 2, 123264, 0 }
   , { 2, 4466408, 1 }
   , { 2, 4468260, 0 }
   , { 2, 4470096, 1 }
   , { 1, 4480300, 1 }
   , { 2, 4589168, 0 }
   , { 1, 4603780, 0 }
   , { 1, 4631772, 1 }
   , { 2, 4644748, 1 }
   , { 1, 4756680, 0 }
   , { 2, 4770752, 0 }
   , { 1, 9202316, 1 }
   , { 2, 9205116, 1 }
   , { 2, 9356192, 0 }
   , { 1, 9359928, 0 }
   , { 2, 13956748, 1 }
   , { 1, 13973624, 1 }
   , { 2, 14075716, 0 }
   , { 1, 14094872, 0 }
   , { 1, 14901832, 1 }
   , { 2, 14921844, 1 }
   , { 2, 14927388, 0 }
   , { 2, 14928988, 1 }
   , { 1, 15067104, 0 }
   , { 2, 15090280, 0 }
   , { 2, 20220144, 1 }
   , { 1, 20243444, 1 }
   , { 2, 20374536, 0 }
   , { 1, 20389816, 0 } //Run ends here, Dog0 had a fault but was not send back

   //Race GR T4 - 25/07/2015 (7th run in movie MAH00065.MP4)
   { 1, 20372, 1 }
   , { 2, 43596, 1 }
   , { 1, 150264, 0 }
   , { 2, 166708, 0 }
   , { 2, 4497580, 1 }
   , { 1, 4518960, 1 }
   , { 2, 4630704, 0 }
   , { 1, 4643116, 0 }
   , { 1, 4930508, 1 }
   , { 2, 4943400, 1 }
   , { 1, 5053528, 0 }
   , { 2, 5072772, 0 }
   , { 2, 9643712, 1 }
   , { 1, 9660716, 1 }
   , { 1, 9797480, 0 }
   , { 2, 9825772, 0 }
   , { 2, 15996888, 1 }
   , { 1, 16015540, 1 }
   , { 2, 16104428, 0 }
   , { 2, 16109804, 1 }
   , { 1, 16113960, 0 }
   , { 2, 16115204, 0 }
   , { 1, 16115688, 1 }
   , { 2, 16127408, 1 }
   , { 1, 16128576, 0 }
   , { 2, 16134772, 0 }
   , { 1, 16803380, 1 }
   , { 2, 16825368, 1 }
   , { 1, 16936636, 0 }
   , { 2, 16953500, 0 }
   , { 2, 19096528, 1 }
   , { 2, 19098380, 0 } //Race ends here but dog wasn't really back yet. Probably sensor triggered by wind.

   //Race GR T2 - 25/07/2015 (1st run in movie MAH00066.MP4 starting at 04:05m)
   //This is a good test sine there is an 'ivisible' crossing of 2 dogs between dog 2 and 3
   { 1, 275140, 1 }
   , { 2, 293484, 1 }
   , { 1, 400120, 0 }
   , { 2, 417324, 0 }
   , { 2, 4674524, 1 }
   , { 1, 4689784, 1 }
   , { 2, 4796104, 0 }
   , { 1, 4809000, 0 }
   , { 1, 4848856, 1 }
   , { 2, 4867352, 1 }
   , { 1, 4981496, 0 }
   , { 2, 5005080, 0 }
   , { 2, 9308268, 1 }
   , { 1, 9315684, 1 }
   , { 1, 9317616, 0 }
   , { 1, 9318304, 1 }
   , { 2, 9448888, 0 }
   , { 1, 9452400, 0 }
   , { 2, 13532488, 1 }
   , { 1, 13543572, 1 }
   , { 1, 13728660, 0 }
   , { 2, 13745884, 0 }
   , { 2, 18070120, 1 }

   //Faulty numbers on WebUI
   {1,162901,1},
   {2,190287,1},
   {1,292299,0},
   {1,308171,1},
   {2,311576,0},
   {1,315275,0},
   {1,327828,1},
   {1,331783,0},
   {2,5216625,1},
   {1,5233268,1},
   {1,5240338,0},
   {1,5242136,1},
   {2,5362002,0},
   {2,5367346,1},
   {1,5468382,0},
   {2,5488550,0},
   {2,9910598,1},
   {2,9915971,0},
   {2,9919530,1},
   {1,9923699,1},
   {1,10092756,0},
   {2,10112577,0},
   {2,14495214,1},
   {1,14509473,1},
   {1,14511453,0},
   {1,14515396,1},
   {2,14627607,0},
   {1,14641081,0}

*/
