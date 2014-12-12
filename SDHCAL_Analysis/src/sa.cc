#include "lcio.h"
#include <stdio.h>
#include <bitset>
#include <vector>
#include "IO/LCReader.h"
#include "IMPL/LCTOOLS.h"
#include "IMPL/LCRunHeaderImpl.h" 
#include "IMPL/LCEventImpl.h" 

#include "EVENT/SimCalorimeterHit.h" 
#include "EVENT/CalorimeterHit.h" 
#include "EVENT/RawCalorimeterHit.h" 
#include "IMPL/LCGenericObjectImpl.h"
#include <IMPL/LCCollectionVec.h>
#include "IMPL/RawCalorimeterHitImpl.h"
#include "IMPL/LCEventImpl.h"
#include "IMPL/LCCollectionVec.h"
#include "IMPL/LCFlagImpl.h"
// #include "EVENT/SimTrackerHit.h" 

#include "UTIL/CellIDDecoder.h"

#include <cstdlib>
#include "DCBufferReader.h"
#include "DIFSlowControl.h"
#include "DCType.h"
#include "ShowerAnalyzer.h"
#include "StripAnalyzer.h"
#include "RawAnalyzer.h"
#include "MulAnalyzer.h"
#include "TApplication.h"
#include "TCanvas.h"
using namespace std ;
using namespace lcio ;
/** dump the given event to screen
 */
#include "GenericOnlineReader.h"

int main(int argc, char** argv ){

  char* FILEN ;
  char* FILEO;
  int runNumber=0 ;
  int evtNumber=0 ;
  int nthEvent=1 ;

  // read file name from command line (only argument) 
  if( argc < 2 ) {

    cout << " usage: dumpevent filename runNum evtNum " << endl ;
    cout << "    or: dumpevent filename n      " << endl ;
    cout << "  where the first dumps the event with the specified run and event number" << endl ;
    cout << "  and the second simply dumps the n-th event in the file" << endl ;

    exit(1) ;
  }
  
  FILEN = argv[1] ;



  bool writing=(argc==3);
  if (writing)
    {
      FILEO= argv[2];
    }
  TApplication theApp("tapp", &argc, argv);
 
  

  DHCalEventReader  dher;
  dher.setXdaqShift(24); //DimDaq etal 
  //dher.setXdaqShift(94); //XDaq
  dher.setDropFirstRU(false);
  DCHistogramHandler rootHandler;
  //  LMBasicAnalyzer *a= new LMBasicAnalyzer( &dher,&rootHandler);

  bool rebuild=false;
  // if (rebuild)
  //   a->setCollectionName("DHCALRawHitsNew");
  // a->registerDIF(23);
  // a->mask(23,1,23);
  // a->mask(23,1,24);
  // a->mask(23,1,25);
  // a->mask(23,1,27);
  // a->mask(23,1,28);
  // a->mask(23,1,29);
  // a->mask(23,1,30);


  std::string name(FILEN);

  dher.open(name);



  dher.ParseSteering("m3_bonneteau.xml");
  //dher.ParseSteering("tomuvol_agm14.xml");
 
#define USESTREAM
#ifdef USESTREAM
  ShowerAnalyzer *a= new ShowerAnalyzer( &dher,&rootHandler);
  //a->setminChambersInTime(3);
  //RawAnalyzer *ar= new RawAnalyzer();
  //MulAnalyzer *am= new MulAnalyzer();
  //StripAnalyzer *as= new StripAnalyzer();
  a->setrebuild(rebuild);
  //dher.registerAnalysis(ar);
  dher.registerAnalysis(a);
  
  dher.initJob();
  //if (writing) 
   // {std::string nameo(FILEO);

    //  dher.openOutput(FILEO);
   //   a->setWriting(true);
   // }
  //else
  //a->setWriting(false);
  try {
    dher.readStream(5000);
    //dher.startReadMemory("/dev/shm/monitor",155,10000);
    }
    catch(...)
      {
      printf(" Christophe y dit qu'il en a rien a peter \n");
      goto end;
      }
#else


  if (writing) 
    {std::string nameo(FILEO);

      dher.openOutput(FILEO);
    }
  int nevt=0;
  
  //  dher.readStream();

  try {
    dher.readRun();
    if (writing)
      {
  // 	  //	  dher.buildEvent();
   	  if (dher.getRunHeader()!=0) dher.writeRunHeader();
      	}

  }
  catch (...)
    {
      std::cout<<"No run Header"<<std::endl;
    }
  // int ier = dher.readEvent();
  // nevt++;

  // //LCTOOLS::dumpRunHeader( dher.getRunHeader() ) ;

  
 
  // //getchar();
  // int runnb=0;
  // if (ier>0)
  //   {
  //     runnb=dher.getEvent()->getRunNumber();
  //     //     dher.buildEvent(rebuild);
  //     dher.parseRawEvent();
  //     dher.flagSynchronizedFrame();
  //     if (writing)
  // 	{
  // 	  //	  dher.buildEvent();
  // 	  if (dher.getRunHeader()!=0) dher.writeRunHeader();
  // 	  dher.writeEvent(true);
  // 	}
  //   }
  int ier=1;
  while (ier>0)
    {

      ier = dher.readEvent();
      std::cout<<"event "<<ier<<std::endl;
      if (ier<=0) break;
      //
  //LCTOOLS::printLCGenericObjects(dher.getEvent()->getCollection("RU_XDAQ"));
  //LCTOOLS::dumpEventDetailed( dher.getEvent() ) ;
  //getchar();
  try {

    //    dher.buildEvent(rebuild);
    //LCTOOLS::dumpEvent( dher.getEvent() ) ;
    //    getchar();
    // dher.analyzeEvent();
      // getchar();
      dher.parseRawEvent();
      dher.flagSynchronizedFrame();
      IMPL::LCEventImpl* evt_=dher.getEvent();
      IMPL::LCCollectionVec* HitVec=dher.createRawCalorimeterHits(true);
      std::cout<<evt_->getEventNumber()<<" Number of Hit "<<HitVec->getNumberOfElements()<<std::endl;


      IMPL::LCEventImpl* evtOutput_ =new IMPL::LCEventImpl();
      evtOutput_->setRunNumber(evt_->getRunNumber());
      evtOutput_->setEventNumber(evt_->getEventNumber());
      evtOutput_->setTimeStamp(evt_->getTimeStamp());
      evtOutput_->setDetectorName(evt_->getDetectorName());
      evtOutput_->setWeight(evt_->getWeight());
      evtOutput_->addCollection(HitVec,"DHCALRawHits");
      LCTOOLS::printRawCalorimeterHits(HitVec);
      //getchar();
      if (writing)
	dher.write(evtOutput_);
      else
	delete evtOutput_;
  }
  catch (std::string e )
    {
      std::cout<<e<<std::endl;
      break;
    }
      //      if (1>0) continue;
      nevt++;
      if (ier<=0) break;
      if (writing)
	{
	  // dher.buildEvent();
	  //	  dher.writeEvent(true);

	}
      else
	{
	
	}
      if (nevt==100) break;
    }
#endif
 end:
  if (writing) 
    dher.closeOutput();
  else
    {
      int runnb=dher.getEvent()->getRunNumber();
      std::stringstream name("");
      name<<"/tmp/LMNewAnalyzer_"<<runnb<<".root";
      rootHandler.writeHistograms(name.str());
    }
  std::cout<<"FINI ICI"<<std::endl;
  dher.endJob();




   return 0 ;
}

// void FillSlowControlV1(bitset<72*8> &bs, IMPL::LCRunHeaderImpl* Rh)
// {	 
//   // Asic Id
//   int asicid=0;
//   for (int j=0;j<8;j++) if (bs[j+9]!=0) asicid += (1<<(7-j));
//   char Name[10];
//   sprintf(Name,"Asic_%i_",asicid);
//   string name(Name);
//   // Slow Control
//   Rh->parameters().setValue((name+"SSC0"),(int) bs[575]);
//   Rh->parameters().setValue((name+"SSC1"),(int)bs[574]);
//   Rh->parameters().setValue((name+"SSC2"),(int)bs[573]);
//   Rh->parameters().setValue((name+"Choix_caisson"),(int)bs[572]);
//   Rh->parameters().setValue((name+"SW_50k"),(int)	bs[571]);
//   Rh->parameters().setValue((name+"SW_100k"),(int)bs[570]);
//   Rh->parameters().setValue((name+"SW_100f"),(int)bs[569]);
//   Rh->parameters().setValue((name+"SW_50f"),(int)	bs[568]);
	 
//   Rh->parameters().setValue((name+"Valid_DC"),(int) bs[567]);
//   Rh->parameters().setValue((name+"ON_Discri"),(int)bs[566]);
//   Rh->parameters().setValue((name+"ON_Fsb"),(int)	bs[565]);
//   Rh->parameters().setValue((name+"ON_Otaq"),(int)bs[564]);
//   Rh->parameters().setValue((name+"ON_W"),(int)bs[563]);
//   Rh->parameters().setValue((name+"ON_Ss"),(int)bs[562]);
//   Rh->parameters().setValue((name+"ON_Buf"),(int)bs[561]);
//   Rh->parameters().setValue((name+"ON_Paf"),(int)bs[560]);
//   // Gain
//   lcio::IntVec Gain(64);
//   for(int i=0;i<64;i++)
//     {
//       Gain[i]=0;
//       for (int j=0;j<6;j++) if (bs[176+i*6+j]!=0) Gain[i]+= (1<<j);
//     }
//   Rh->parameters().setValues((name+"Gain"),Gain)	;
//   // cTest
//   lcio::IntVec cTest(64);
//   for(int i=0;i<64;i++) cTest[i]=bs[112+i];
//   Rh->parameters().setValues((name+"cTest"),cTest);
//   Rh->parameters().setValue((name+"ON_Otabg"),(int) bs[111]);
//   Rh->parameters().setValue((name+"ON_Dac"),(int)	bs[110]);
//   Rh->parameters().setValue((name+"ON_Otadac"),(int) bs[109]);
//   // DAC
//   int dac1=0;
//   for (int j=0;j<10;j++) if (bs[j+99]!=0) dac1+= (1<<j);
//   Rh->parameters().setValue((name+"DAC1"),dac1);	
//   int dac0=0;
//   for (int j=0;j<10;j++) if (bs[j+89]!=0) dac0+= (1<<j);
//   Rh->parameters().setValue((name+"DAC0"),dac0);	
//   // Valid
//   lcio::IntVec Valid_trig(64);
//   for (int j=0;j<64;j++) Valid_trig[j]=(int) bs[25+j];
//   Rh->parameters().setValues((name+"Valid_trig"),Valid_trig);
  

//   Rh->parameters().setValue((name+"EN_Raz_Ext"),(int) bs[23]);
//   Rh->parameters().setValue((name+"EN_Raz_Int"),(int)bs[22]);
//   Rh->parameters().setValue((name+"EN_Out_Raz_Int"),(int)bs[21]);
//   Rh->parameters().setValue((name+"EN_Trig_Ext"),(int)bs[20]);
//   Rh->parameters().setValue((name+"EN_Trig_Int"),(int)bs[19]);		
//   Rh->parameters().setValue((name+"EN_Out_Trig_Int"),(int) bs[18]);
//   Rh->parameters().setValue((name+"Bypass_Chip"),(int)bs[17]);

//   Rh->parameters().setValue((name+"HardrocHeader"),(int)asicid);
	
//   Rh->parameters().setValue((name+"EN_Out_Discri"),(int) bs[8]);

//   Rh->parameters().setValue((name+"EN_Transmit_On"),(int) bs[7]);
//   Rh->parameters().setValue((name+"EN_Dout"),(int) bs[6]);
//   Rh->parameters().setValue((name+"EN_RamFull"),(int) bs[5]);
	
//   return;		
																																																																				
// }
