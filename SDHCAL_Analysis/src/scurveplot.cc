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
#include "GenericOnlineReader.h"
#include "BasicAnalyzer.h"
#include "DIFSlowControl.h"
#include "DCType.h"


using namespace std ;
using namespace lcio ;
/** dump the given event to screen
 */
#include "DCScurveMaker.h"
#include "BasicAnalyzer.h"


int main(int argc, char** argv ){


  // read file name from command line (only argument) 
  if( argc < 1 ) {
    cout << " usage: scurveplot filename " << endl ;
    exit(1) ;
  }
  
  DHCalEventReader dher;
  DCHistogramHandler rootHandler; 
  DCScurveMaker *a=new DCScurveMaker(&dher, &rootHandler);
  dher.registerAnalysis(a);
  for (int arg=1;arg<argc;++arg) {
    char* FILEN=argv[arg];
    std::string name(FILEN);
    cout <<"Opening "<<FILEN<<endl;
    dher.open(name);
    cout<<"Reading "<<FILEN<<endl;
    dher.readStream();
    cout<<"Closing "<<FILEN<<endl;
    dher.close();
  }
  cout<<"Calling endJob"<<endl;
  dher.endJob();
  cout<<"writing histos"<<endl;
  rootHandler.writeHistograms("scurvemaker.root");
  return 0 ;
}


