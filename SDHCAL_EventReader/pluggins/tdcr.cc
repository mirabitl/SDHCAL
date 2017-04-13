#include "tdcreadbinary.hh"
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
#include <dirent.h>
#include <fnmatch.h>

int main(int argc, char** argv )
{
  tdcreadbinary bs("/tmp");
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
  //bs.addRun(732792,"/data/srv02/RAID6/June2016/SMM_170616_010640_732792.dat");
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
  
  
  //  bs.addRun(734689,"/data/srv02/RAID6/Dome0916/SMM_050417_170113_734689.dat");
  //bs.addRun(734690,"/data/srv02/RAID6/Dome0916/SMM_060417_104234_734690.dat");
  //bs.addRun(734691,"/data/srv02/RAID6/Dome0916/SMM_060417_111405_734691.dat");
  //bs.addRun(734692,"/data/srv02/RAID6/Dome0916/SMM_060417_112531_734692.dat");
  //bs.addRun(734693,"/data/srv02/RAID6/Dome0916/SMM_060417_113630_734693.dat");
  //bs.addRun(734695,"/data/srv02/RAID6/Dome0916/SMM_060417_133638_734695.dat");
  //bs.addRun(734696,"/data/srv02/RAID6/Dome0916/SMM_060417_141925_734696.dat");
  //bs.addRun(734697,"/data/srv02/RAID6/Dome0916/SMM_060417_144308_734697.dat");
  //bs.addRun(734698,"/data/srv02/RAID6/Dome0916/SMM_060417_145503_734698.dat");
  //bs.addRun(734707,"/data/srv02/RAID6/Dome0916/SMM_070417_063254_734707.dat");
  //bs.addRun(734708,"/data/srv02/RAID6/Dome0916/SMM_070417_065109_734708.dat");
  //bs.addRun(734716,"/data/srv02/RAID6/Dome0916/SMM_070417_103018_734716.dat");
  // bs.addRun(734720,"/data/srv02/RAID6/Dome0916/SMM_070417_113308_734720.dat");

  //bs.addRun(734721,"/data/srv02/RAID6/Dome0916/SMM_070417_140432_734721.dat");
  //bs.addRun(734722,"/data/srv02/RAID6/Dome0916/SMM_070417_141907_734722.dat");
  // PR 0
  /// bs.addRun(734732,"/data/srv02/RAID6/Dome0916/SMM_070417_151758_734732.dat");
  //PR 1
  ///bs.addRun(734733,"/data/srv02/RAID6/Dome0916/SMM_070417_153153_734733.dat");
  //PR 3
  //bs.addRun(734734,"/data/srv02/RAID6/Dome0916/SMM_070417_153851_734734.dat");
  // PR 4
  //bs.addRun(734735,"/data/srv02/RAID6/Dome0916/SMM_070417_155603_734735.dat");

  // MASK 32
  //bs.addRun(734740,"/data/srv02/RAID6/Dome0916/SMM_080417_162526_734740.dat");
  //bs.addRun(734741,"/data/srv02/RAID6/Dome0916/SMM_080417_165040_734741.dat");
  // MASK 64
  //bs.addRun(734742,"/data/srv02/RAID6/Dome0916/SMM_080417_170012_734742.dat");
  //bs.addRun(734743,"/data/srv02/RAID6/Dome0916/SMM_080417_170817_734743.dat");
  // MASK 128
  //bs.addRun(734744,"/data/srv02/RAID6/Dome0916/SMM_080417_171702_734744.dat");
  //bs.addRun(734746,"/data/srv02/RAID6/Dome0916/SMM_080417_173206_734746.dat");
  // MASK 256
  //bs.addRun(734748,"/data/srv02/RAID6/Dome0916/SMM_090417_085513_734748.dat");
  // Mask 512
  //bs.addRun(734749,"/data/srv02/RAID6/Dome0916/SMM_090417_090515_734749.dat");
  /*
  bs.addRun(734750,"/data/srv02/RAID6/Dome0916/SMM_090417_091706_734750.dat");
  // Mask 1024
  bs.addRun(734751,"/data/srv02/RAID6/Dome0916/SMM_090417_092418_734751.dat");
  // Mask 2048
  bs.addRun(734752,"/data/srv02/RAID6/Dome0916/SMM_090417_092704_734752.dat");
  // Maks 4096
  bs.addRun(734753,"/data/srv02/RAID6/Dome0916/SMM_090417_093030_734753.dat");
  // 8192
  bs.addRun(734754,"/data/srv02/RAID6/Dome0916/SMM_090417_093337_734754.dat");
  // 16384
  // &value=2147483648 PR 31
  bs.addRun(0,"/data/srv02/RAID6/Dome0916/SMM_090417_093942_734756.dat");
  // PR 30
  bs.addRun(0,"/data/srv02/RAID6/Dome0916/SMM_090417_094409_734757.dat");
  // PR 29
  bs.addRun(0,"/data/srv02/RAID6/Dome0916/SMM_090417_094810_734758.dat");
  */
  // ALL
  //bad bs.addRun(0,"/data/srv02/RAID6/Dome0916/SMM_090417_113300_734759.dat");
  //bs.addRun(0,"/data/srv02/RAID6/Dome0916/SMM_090417_125608_734760.dat");
  // bad bs.addRun(0,"/data/srv02/RAID6/Dome0916/SMM_090417_192114_734761.dat");

  // refere bs.addRun(0,"/data/srv02/RAID6/Dome0916/SMM_090417_221033_734762.dat");

  // Apres les capa
  //bs.addRun(0,"/data/srv02/RAID6/Dome0916/SMM_100417_111538_734763.dat");

  // idem avec spill court
  //bs.addRun(0,"/data/srv02/RAID6/Dome0916/SMM_100417_133123_734764.dat");

  // idem 1 clock
  //bs.addRun(0,"/data/srv02/RAID6/Dome0916/SMM_100417_140251_734765.dat");

  // 10 clock 200 fF On
  //bs.addRun(0,"/data/srv02/RAID6/Dome0916/SMM_100417_150608_734766.dat");

  // Derniere capa ajoutees
  //bs.addRun(0,"/data/srv02/RAID6/Dome0916/SMM_100417_162143_734768.dat");

   // Apres modifDHCAL_734769_I0_0.slcio
  //   bs.addRun(0,"/data/srv02/RAID6/Dome0916/SMM_110417_183924_734769.dat");
  // bs.addRun(0,"/data/srv02/RAID6/Dome0916/SMM_110417_200402_734770.dat");
  //bs.addRun(0,"/data/srv02/RAID6/Dome0916/SMM_120417_054140_734771.dat");

  // SC changed
  //bs.addRun(0,"/data/srv02/RAID6/Dome0916/SMM_120417_151905_734772.dat");

   // Verison 0
  //bs.addRun(0,"/data/srv02/RAID6/Dome0916/SMM_130417_063346_734773.dat");

  // Version 0 oped corr
  //  bs.addRun(0,"/data/srv02/RAID6/Dome0916/SMM_130417_081038_734774.dat");

  // Coreection de Nahalie
  //bs.addRun(0,"/data/srv02/RAID6/Dome0916/SMM_130417_113403_734775.dat");

  std::stringstream spat;
  spat<<"*"<<atol(argv[1])<<"*.dat";
  struct dirent **namelist;
  int n;
  std::cout<<"Pattern "<<spat.str()<<std::endl;
  std::string dirp="/data/srv02/RAID6/Dome0916/";
  n = scandir(dirp.c_str(), &namelist, NULL, alphasort);
  if (n < 0)
    perror("scandir");
  else {
    while (n--) {

      if (fnmatch(spat.str().c_str(), namelist[n]->d_name, 0)==0)
	{
	  printf("%s %d \n", namelist[n]->d_name,fnmatch(spat.str().c_str(), namelist[n]->d_name, 0));
	  printf("found\n");
	  std::stringstream sf;
	  sf<<dirp<<"/"<< namelist[n]->d_name;
	  bs.addRun(0,sf.str());
	}
      free(namelist[n]);
    }
    free(namelist);
  }

  
  DCHistogramHandler rootHandler;
  DHCalEventReader  dher;
  
  bs.Read();
  bs.end();
}
