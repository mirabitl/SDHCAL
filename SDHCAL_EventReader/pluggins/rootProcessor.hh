#ifndef _ROOTPROCESSOR_HH

#define _ROOTPROCESSOR_HH
#include <limits.h>
#include <vector>
#include "DHCALAnalyzer.h"
#include "DHCalEventReader.h"
#include "DCHistogramHandler.h"
#include <iostream>
#include <sys/timeb.h>



#include "IMPL/LCTOOLS.h"
#include "EVENT/RawCalorimeterHit.h" 
#include "IMPL/LCGenericObjectImpl.h"
#include "IMPL/RawCalorimeterHitImpl.h"
#include "UTIL/CellIDDecoder.h"
#include "jsonGeo.hh"

#include "TFile.h"

#include <vector>
#include <map>
#include "RecoHit.hh"
#include "planeCluster.hh"
#include "hitMonitor.hh"
#include "recoTrack.hh"
#include "UtilDefs.h"
#include "datasource.hh"

class rootProcessor : public DHCALAnalyzer
{
public:
  rootProcessor();
  virtual ~rootProcessor(){;}
  void initialise();
  virtual void processEvent();
  virtual void initHistograms();
  virtual void processRunHeader()
  {
    //if (writing_)
    //  reader_->writeRunHeader();
  }
  void presetParameters();
  void setWriting(bool t){writing_=t;}
  virtual void setReader(DHCalEventReader* r){reader_=r;rootHandler_=DCHistogramHandler::instance();}

  virtual void initJob();
  virtual void endJob();
  virtual void initRun(){;}
  virtual void endRun(){;}

  bool decodeTrigger(LCCollection* rhcol);
  void prepareDataSources();

  

  void setCollectionName(std::string s){ collectionName_=s;}
 

  inline void setcollectionName(std::string t){collectionName_=t;}



  void processSeed(IMPL::LCCollectionVec* rhcol,uint32_t seed);
  uint32_t fillVolume(uint32_t seed);
  
  std::vector<uint32_t> cleanMap(uint32_t nchmin);

  std::string ptime(std::string s);
    
private:


  int nAnalyzed_;
  

  struct timeval diffT, startT, endT;
  double integratedTime_;


  DCHistogramHandler* rootHandler_;
  int hrtype_;
  int lastrunnb_;
  unsigned int currentTime_,currentEvent_,lastSpyEvent_;
  // Control
  bool dropFirstSpillEvent_;
  std::string collectionName_;



  // Reader
  DHCalEventReader* reader_;

  bool writing_;
  IMPL::LCEventImpl* evt_;

  double theSpillLength_;
  unsigned long long theBCID_;
  unsigned long long theBCIDSpill_;
  unsigned long long theAbsoluteTime_;
  unsigned long long theTimeZero_;

  uint32_t theDTC_,theGTC_;
  uint32_t theNall_,theNedge_,theTag_,theEdge3_,theCore3_,theBigCore_,theCore1_,theCore2_,theNplans_;
  int32_t theWeights_;


  std::vector<RecoHit> _hits;

  RecoHit _vhits[40000];
  uint32_t _nvhits;
	  


  unsigned long long theLastBCID_,theIdxSpill_;
  
  std::bitset<64> nPlansReal_,nPlansInteraction_,nPlansAll_;
  std::map<uint32_t,levbdim::datasource*> _sources;
  uint32_t _dsidx[255];
  uint32_t _runNumber;
  jsonGeo* _geo;
 
};
#endif
