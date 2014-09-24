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


#include "RawAnalyzer.h"
#include "TApplication.h"
#include "TCanvas.h"
using namespace std ;
using namespace lcio ;

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

  bool rebuild=true;
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



  dher.ParseSteering("m3_sep14.xml");

  RawAnalyzer *ar= new RawAnalyzer();
  dher.registerAnalysis(ar);
  
  dher.initJob();
  try {
    dher.readStream(0);
    }
    catch(...)
      {
      printf(" Christophe y dit qu'il en a rien a peter \n");
      goto end;
      }
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
  //dher.endJob();




   return 0 ;
}
