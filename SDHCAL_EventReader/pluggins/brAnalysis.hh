#ifndef _BRANALYSIS_HH

#define _BRANALYSIS_HH
#include <limits.h>
#include <vector>
#include "DHCALAnalyzer.h"
#include "DHCalEventReader.h"
#include "DCHistogramHandler.h"
#include "jsonGeo.hh"
#include <iostream>
#include <sys/timeb.h>



#include "IMPL/LCTOOLS.h"
#include "EVENT/RawCalorimeterHit.h" 
#include "IMPL/LCGenericObjectImpl.h"
#include "IMPL/RawCalorimeterHitImpl.h"
#include "UTIL/CellIDDecoder.h"

#include "UtilDefs.h"
#include "recoPoint.hh"
#include "recoTrack.hh"
class brAnalysis : public DHCALAnalyzer
{
public:
  brAnalysis();

  virtual ~brAnalysis(){;}
  void initialise();
  virtual void processEvent();
  virtual void initHistograms(){;}
  virtual void processRunHeader()
  {
    if (writing_)
      reader_->writeRunHeader();
  }
  void presetParameters();
  void setWriting(bool t){writing_=t;}
  virtual void setReader(DHCalEventReader* r){reader_=r;rootHandler_=DCHistogramHandler::instance();}

  virtual void initJob();
  virtual void endJob();
  virtual void initRun(){;}
  virtual void endRun(){;}
  void processPhysicEvent(uint32_t iseed);
  void draw(std::vector<recoPoint*> vp);




private:




  DCHistogramHandler* rootHandler_;


  // Reader
  DHCalEventReader* reader_;

  bool writing_;
  bool draw_;

  IMPL::LCEventImpl* evt_;


    uint64_t _bxId;
    uint32_t _gtc;
    double _t,_t0,_tspill;


    jsonGeo* _geo;
    std::map<uint32_t,std::bitset<64> > _tcount;
    std::map<uint32_t,std::vector<std::pair<DIFPtr*,uint32_t> > > _tframe;
    double _readoutTime,_readoutTotalTime;
    uint32_t _numberOfShower,_numberOfMuon;
    
    std::vector<recoTrack*> _vtk;

};
#endif
