#include "basicreader.hh"
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <map>
#include <bitset>
#include "DHCalEventReader.h"
#include "TApplication.h"
#include "TCanvas.h"

int main(int argc, char** argv )
{
  basicreader bs("/tmp");
  TApplication theApp("tapp", &argc, argv);
  bs.geometry("/home/acqilc/SDHCAL/SDHCAL_EventReader/pluggins/m3_avril2015.json");
  //bs.open("/data/NAS/June2016/SMM_160616_163121_732786.dat");
  // bs.open("/data/NAS/June2016/SMM_160616_110612_732783.dat");
  //bs.open("/data/NAS/June2016/SMM_170616_052256_732795.dat");
  //bs.open("/data/NAS/June2016/SMM_170616_092331_732799.dat");
  // bs.open("/data/NAS/June2016/SMM_160616_110612_732783.dat");
  /*
bs.addRun(733633,"/data/srv02/RAID6/Oct2016/SMM_071016_123856_733633.dat");
bs.addRun(733636,"/data/srv02/RAID6/Oct2016/SMM_071016_124907_733636.dat");
bs.addRun(733637,"/data/srv02/RAID6/Oct2016/SMM_071016_125306_733637.dat");
bs.addRun(733641,"/data/srv02/RAID6/Oct2016/SMM_071016_153619_733641.dat");
bs.addRun(733642,"/data/srv02/RAID6/Oct2016/SMM_071016_154539_733642.dat");
bs.addRun(733643,"/data/srv02/RAID6/Oct2016/SMM_071016_155358_733643.dat");
bs.addRun(733644,"/data/srv02/RAID6/Oct2016/SMM_071016_155937_733644.dat");
bs.addRun(733645,"/data/srv02/RAID6/Oct2016/SMM_071016_165755_733645.dat");
bs.addRun(733646,"/data/srv02/RAID6/Oct2016/SMM_071016_170520_733646.dat");
bs.addRun(733647,"/data/srv02/RAID6/Oct2016/SMM_071016_173728_733647.dat");
bs.addRun(733650,"/data/srv02/RAID6/Oct2016/SMM_071016_193047_733650.dat");
bs.addRun(733653,"/data/srv02/RAID6/Oct2016/SMM_071016_205435_733653.dat");
bs.addRun(733654,"/data/srv02/RAID6/Oct2016/SMM_071016_210657_733654.dat");
bs.addRun(733655,"/data/srv02/RAID6/Oct2016/SMM_071016_232430_733655.dat");
bs.addRun(733655,"/data/srv02/RAID6/Oct2016/SMM_071016_233612_733655.dat");
bs.addRun(733656,"/data/srv02/RAID6/Oct2016/SMM_081016_012606_733656.dat");
bs.addRun(733656,"/data/srv02/RAID6/Oct2016/SMM_081016_015222_733656.dat");
bs.addRun(733658,"/data/srv02/RAID6/Oct2016/SMM_081016_033908_733658.dat");
bs.addRun(733659,"/data/srv02/RAID6/Oct2016/SMM_081016_035422_733659.dat");
bs.addRun(733660,"/data/srv02/RAID6/Oct2016/SMM_081016_035811_733660.dat");
bs.addRun(733660,"/data/srv02/RAID6/Oct2016/SMM_081016_054542_733660.dat");
bs.addRun(733665,"/data/srv02/RAID6/Oct2016/SMM_081016_082718_733665.dat");
bs.addRun(733665,"/data/srv02/RAID6/Oct2016/SMM_081016_092637_733665.dat");
bs.addRun(733665,"/data/srv02/RAID6/Oct2016/SMM_081016_110948_733665.dat");
bs.addRun(733675,"/data/srv02/RAID6/Oct2016/SMM_081016_160612_733675.dat");
bs.addRun(733678,"/data/srv02/RAID6/Oct2016/SMM_081016_171614_733678.dat");
bs.addRun(733679,"/data/srv02/RAID6/Oct2016/SMM_081016_173543_733679.dat");
bs.addRun(733680,"/data/srv02/RAID6/Oct2016/SMM_091016_010348_733680.dat");
bs.addRun(733680,"/data/srv02/RAID6/Oct2016/SMM_091016_041603_733680.dat");
bs.addRun(733680,"/data/srv02/RAID6/Oct2016/SMM_091016_065059_733680.dat");
bs.addRun(733683,"/data/srv02/RAID6/Oct2016/SMM_091016_072800_733683.dat");
bs.addRun(733686,"/data/srv02/RAID6/Oct2016/SMM_091016_104731_733686.dat");
bs.addRun(733688,"/data/srv02/RAID6/Oct2016/SMM_091016_122843_733688.dat");
bs.addRun(733688,"/data/srv02/RAID6/Oct2016/SMM_091016_124430_733688.dat");
bs.addRun(733689,"/data/srv02/RAID6/Oct2016/SMM_091016_140032_733689.dat");
bs.addRun(733689,"/data/srv02/RAID6/Oct2016/SMM_091016_154423_733689.dat");
bs.addRun(733692,"/data/srv02/RAID6/Oct2016/SMM_091016_163928_733692.dat");
bs.addRun(733693,"/data/srv02/RAID6/Oct2016/SMM_091016_164335_733693.dat");
bs.addRun(733693,"/data/srv02/RAID6/Oct2016/SMM_091016_182544_733693.dat");
bs.addRun(733696,"/data/srv02/RAID6/Oct2016/SMM_091016_184359_733696.dat");
bs.addRun(733696,"/data/srv02/RAID6/Oct2016/SMM_091016_202828_733696.dat");
bs.addRun(733698,"/data/srv02/RAID6/Oct2016/SMM_091016_211028_733698.dat");
bs.addRun(733698,"/data/srv02/RAID6/Oct2016/SMM_091016_223807_733698.dat");
bs.addRun(733699,"/data/srv02/RAID6/Oct2016/SMM_091016_232004_733699.dat");
bs.addRun(733700,"/data/srv02/RAID6/Oct2016/SMM_101016_001144_733700.dat");
bs.addRun(733701,"/data/srv02/RAID6/Oct2016/SMM_101016_001759_733701.dat");
bs.addRun(733702,"/data/srv02/RAID6/Oct2016/SMM_101016_004241_733702.dat");
bs.addRun(733705,"/data/srv02/RAID6/Oct2016/SMM_101016_012948_733705.dat");
bs.addRun(733707,"/data/srv02/RAID6/Oct2016/SMM_101016_013700_733707.dat");
bs.addRun(733708,"/data/srv02/RAID6/Oct2016/SMM_101016_014138_733708.dat");
bs.addRun(733710,"/data/srv02/RAID6/Oct2016/SMM_101016_015433_733710.dat");
bs.addRun(733711,"/data/srv02/RAID6/Oct2016/SMM_101016_020437_733711.dat");
bs.addRun(733711,"/data/srv02/RAID6/Oct2016/SMM_101016_033112_733711.dat");
bs.addRun(733711,"/data/srv02/RAID6/Oct2016/SMM_101016_045829_733711.dat");
bs.addRun(733718,"/data/srv02/RAID6/Oct2016/SMM_101016_054606_733718.dat");
bs.addRun(733718,"/data/srv02/RAID6/Oct2016/SMM_101016_063034_733718.dat");
bs.addRun(733718,"/data/srv02/RAID6/Oct2016/SMM_101016_075846_733718.dat");
bs.addRun(733719,"/data/srv02/RAID6/Oct2016/SMM_101016_092544_733719.dat");
bs.addRun(733720,"/data/srv02/RAID6/Oct2016/SMM_101016_103528_733720.dat");
bs.addRun(733722,"/data/srv02/RAID6/Oct2016/SMM_101016_111905_733722.dat");
bs.addRun(733723,"/data/srv02/RAID6/Oct2016/SMM_101016_113444_733723.dat");
bs.addRun(733724,"/data/srv02/RAID6/Oct2016/SMM_101016_120745_733724.dat");
bs.addRun(733724,"/data/srv02/RAID6/Oct2016/SMM_101016_132143_733724.dat");
bs.addRun(733725,"/data/srv02/RAID6/Oct2016/SMM_101016_144929_733725.dat");
bs.addRun(733728,"/data/srv02/RAID6/Oct2016/SMM_101016_151024_733728.dat");
bs.addRun(733738,"/data/srv02/RAID6/Oct2016/SMM_101016_175002_733738.dat");
bs.addRun(733740,"/data/srv02/RAID6/Oct2016/SMM_101016_175744_733740.dat");
bs.addRun(733741,"/data/srv02/RAID6/Oct2016/SMM_101016_181441_733741.dat");
bs.addRun(733742,"/data/srv02/RAID6/Oct2016/SMM_101016_181943_733742.dat");
bs.addRun(733743,"/data/srv02/RAID6/Oct2016/SMM_101016_224057_733743.dat");
bs.addRun(733743,"/data/srv02/RAID6/Oct2016/SMM_111016_004226_733743.dat");
bs.addRun(733748,"/data/srv02/RAID6/Oct2016/SMM_111016_005214_733748.dat");
bs.addRun(733750,"/data/srv02/RAID6/Oct2016/SMM_111016_022420_733750.dat");
bs.addRun(733750,"/data/srv02/RAID6/Oct2016/SMM_111016_024904_733750.dat");
bs.addRun(733750,"/data/srv02/RAID6/Oct2016/SMM_111016_044205_733750.dat");
bs.addRun(733750,"/data/srv02/RAID6/Oct2016/SMM_111016_063534_733750.dat");
bs.addRun(733754,"/data/srv02/RAID6/Oct2016/SMM_111016_072721_733754.dat");
bs.addRun(733754,"/data/srv02/RAID6/Oct2016/SMM_111016_084258_733754.dat");
bs.addRun(733756,"/data/srv02/RAID6/Oct2016/SMM_111016_105343_733756.dat");
bs.addRun(733756,"/data/srv02/RAID6/Oct2016/SMM_111016_113540_733756.dat");
bs.addRun(733757,"/data/srv02/RAID6/Oct2016/SMM_111016_135741_733757.dat");
bs.addRun(733758,"/data/srv02/RAID6/Oct2016/SMM_111016_140953_733758.dat");
bs.addRun(733759,"/data/srv02/RAID6/Oct2016/SMM_111016_151355_733759.dat");
acqilc@lyosdhcal9:~$ 

   */
  /* June 2016 5 pC  Inutilisables .... too high rate*/
  bs.addRun(732792,"/data/srv02/RAID6/June2016/SMM_170616_010640_732792.dat");
  /* State 1 pC seuil 1
  bs.addRun(733696,"/data/srv02/RAID6/Oct2016/SMM_091016_184359_733696.dat");

 bs.addRun(733660,"/data/srv02/RAID6/Oct2016/SMM_081016_035811_733660.dat");
bs.addRun(733660,"/data/srv02/RAID6/Oct2016/SMM_081016_054542_733660.dat");
bs.addRun(733665,"/data/srv02/RAID6/Oct2016/SMM_081016_082718_733665.dat");
bs.addRun(733665,"/data/srv02/RAID6/Oct2016/SMM_081016_092637_733665.dat");
bs.addRun(733665,"/data/srv02/RAID6/Oct2016/SMM_081016_110948_733665.dat");
bs.addRun(733675,"/data/srv02/RAID6/Oct2016/SMM_081016_160612_733675.dat");
bs.addRun(733683,"/data/srv02/RAID6/Oct2016/SMM_091016_072800_733683.dat");
bs.addRun(733686,"/data/srv02/RAID6/Oct2016/SMM_091016_104731_733686.dat");
bs.addRun(733689,"/data/srv02/RAID6/Oct2016/SMM_091016_140032_733689.dat");


bs.addRun(733693,"/data/srv02/RAID6/Oct2016/SMM_091016_164335_733693.dat");
bs.addRun(733693,"/data/srv02/RAID6/Oct2016/SMM_091016_182544_733693.dat");
bs.addRun(733696,"/data/srv02/RAID6/Oct2016/SMM_091016_202828_733696.dat");
  */
  /* State 2012 224 et 6.9
  bs.addRun(733754,"/data/srv02/RAID6/Oct2016/SMM_111016_084258_733754.dat");

  
  bs.addRun(733750,"/data/srv02/RAID6/Oct2016/SMM_111016_063534_733750.dat");
  
bs.addRun(733754,"/data/srv02/RAID6/Oct2016/SMM_111016_072721_733754.dat");
bs.addRun(733756,"/data/srv02/RAID6/Oct2016/SMM_111016_105343_733756.dat");
bs.addRun(733756,"/data/srv02/RAID6/Oct2016/SMM_111016_113540_733756.dat");

bs.addRun(733724,"/data/srv02/RAID6/Oct2016/SMM_101016_120745_733724.dat");
bs.addRun(733724,"/data/srv02/RAID6/Oct2016/SMM_101016_132143_733724.dat");
bs.addRun(733728,"/data/srv02/RAID6/Oct2016/SMM_101016_151024_733728.dat");
bs.addRun(733742,"/data/srv02/RAID6/Oct2016/SMM_101016_181943_733742.dat");
bs.addRun(733743,"/data/srv02/RAID6/Oct2016/SMM_101016_224057_733743.dat");
bs.addRun(733743,"/data/srv02/RAID6/Oct2016/SMM_111016_004226_733743.dat");
bs.addRun(733750,"/data/srv02/RAID6/Oct2016/SMM_111016_022420_733750.dat");
bs.addRun(733750,"/data/srv02/RAID6/Oct2016/SMM_111016_024904_733750.dat");
bs.addRun(733750,"/data/srv02/RAID6/Oct2016/SMM_111016_044205_733750.dat");

bs.addRun(733655,"/data/srv02/RAID6/Oct2016/SMM_071016_232430_733655.dat");
bs.addRun(733655,"/data/srv02/RAID6/Oct2016/SMM_071016_233612_733655.dat");
*/
  
  
  
  DHCalEventReader  dher;
  DCHistogramHandler rootHandler;
  bs.Read();
}
