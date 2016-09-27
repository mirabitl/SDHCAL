#ifndef _TRACKANALYSIS_HH

#define _TRACKANALYSIS_HH
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

#include "TricotCluster.hh"

class trackAnalysis : public DHCALAnalyzer
{
public:
  trackAnalysis();
  virtual ~trackAnalysis(){;}
  void initialise();
  virtual void processEvent();
  virtual void initHistograms();
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

  bool decodeTrigger(LCCollection* rhcol, double tcut);
  void drawHits();
  void drawCluster();
  void align();

  void setCollectionName(std::string s){ collectionName_=s;}
  unsigned long long getExternalTriggerTime() { return (unsigned long long) long(externalTriggerTime_);}
  unsigned long long getAbsoluteFrameTime(int bc) { return (unsigned long long) long(externalTriggerTime_-bc);}


  void setDropFirstSpillEvent(bool t){dropFirstSpillEvent_=t;}

  void setClockSynchCut(unsigned int t){clockSynchCut_=t;}
  void setSpillSize(double t){spillSize_=t;}
  inline void setuseSynchronised(bool t){useSynchronised_=t; }
  inline void setoldAlgo(bool t){oldAlgo_=t;}
  inline bool getoldAlgo(){return oldAlgo_;}
  inline void settkMinPoint(int t){tkMinPoint_=t;}
  inline void settkExtMinPoint(int t){tkExtMinPoint_=t;}
  inline void settkBigClusterSize(int t){tkBigClusterSize_=t;}
  inline void setspillSize(float t){spillSize_=t;}
  inline void settkChi2Cut(float t){tkChi2Cut_=t;}
  inline void settkDistCut(float t){tkDistCut_=t;}
  inline void settkExtChi2Cut(float t){tkExtChi2Cut_=t;}
  inline void settkExtDistCut(float t){tkExtDistCut_=t;}
  inline void settkAngularCut(float t){tkAngularCut_=t;}
  inline void setclockSynchCut(int t){clockSynchCut_=t;}
  inline void setminChambersInTime(int t){minChambersInTime_=t;}
  inline void setmaxHitCount(int t){maxHitCount_=t;}
  inline void setchamberEdge(float t){chamberEdge_=t;}
  inline void setrebuild(bool t) {rebuild_=t;}
  inline void setcollectionName(std::string t){collectionName_=t;}



  void processSeed(IMPL::LCCollectionVec* rhcol,uint32_t seed);
  uint32_t fillVolume(uint32_t seed);
  void TagIsolated(uint32_t fp,uint32_t lp);
  uint32_t fillVector(uint32_t seed);
  uint32_t CerenkovTagger(uint32_t difid,uint32_t seed);
  uint32_t PMAnalysis(uint32_t difid);
  void clearClusters();
  void fillPlaneClusters();
  void tagMips();
  std::vector<uint32_t> cleanMap(uint32_t nchmin);
  struct PlaneCompare {
    bool operator()(const planeCluster* l, const planeCluster* r) {
      return l->Z() < r->Z();
    }
  };
  void ptime(std::string s);
    
private:


  int nAnalyzed_;
  int nInSynch_,run_;
  double externalTriggerTime_,lastSpill_,lastPowerPulsedTime_;

  struct timeval diffT, startT, endT;


  std::map<uint32_t,uint32_t> asicCount_;
  std::map<uint32_t,double*> theCorreff_;
  double integratedTime_;


  DCHistogramHandler* rootHandler_;
  int hrtype_;
  int lastrunnb_;
  unsigned int currentTime_,currentEvent_,lastSpyEvent_;
  // Control
  bool rebuild_;
  bool dropFirstSpillEvent_;
  bool findTracks_;
  bool useSynchronised_;
  bool oldAlgo_;
  int tkMinPoint_;
  int tkExtMinPoint_;
  int tkBigClusterSize_;
  float spillSize_;
  float tkChi2Cut_;
  float tkDistCut_;
  float tkExtChi2Cut_;
  float tkExtDistCut_;
  float tkAngularCut_;
  int clockSynchCut_;
  int minChambersInTime_;
  int maxHitCount_;
  int minHitCount_;
  int tkFirstChamber_;
  int tkLastChamber_;
  float chamberEdge_;
  uint32_t trackIndex_;
  uint32_t houghIndex_;
  bool useTk4_;
  std::string collectionName_;
  uint32_t offTimePrescale_;


  // Reader
  DHCalEventReader* reader_;

  bool writing_;
  bool headerWritten_;
  bool draw_;

  IMPL::LCEventImpl* evt_;


  double theRhcolTime_;
  double theTimeSortTime_;
  double theTrackingTime_;
  double theHistoTime_;
  double zLastAmas_;
  int theSeuil_;
  int32_t theSkip_,npi_;
  unsigned long long theBCID_;
  unsigned long long theBCIDSpill_;
  unsigned long long theAbsoluteTime_;

  uint32_t theDTC_,theGTC_;
  uint32_t theNall_,theNedge_,theTag_,theEdge3_,theCore3_,theBigCore_,theCore1_,theCore2_,theNplans_;
  int32_t theWeights_;
  float theZMax_,theZFirst_,theZLast_,thePlanAfter_;

  std::vector<RecoHit> _hits;
  std::vector<RecoHit*> theTkHitVector_;
	  


  hitMonitor* _monitor;
  uint32_t theMonitoringPeriod_;
  std::string theMonitoringPath_;
	
  uint32_t ntkbetween;

  unsigned long long theLastBCID_,theIdxSpill_;
  float theTimeInSpill_[20],theCountSpill_[20],theLastRate_,theSpillLength_;
  float coreRatio_;
  bool isNewSpill_,isPion_,isElectron_,isMuon_,isShower_,isProton_;
  uint32_t theCerenkovTag_;


  // Clusters
  std::vector<planeCluster*> allClusters_,realClusters_,interactionClusters_;

  std::bitset<64> nPlansReal_,nPlansInteraction_,nPlansAll_;
  uint32_t firstInteractionPlane_,lastInteractionPlane_;
  uint32_t _runNumber;
  jsonGeo* _geo;
  std::vector<TStripCluster*> _vts;
  
  std::vector<TricotCluster> _tcl;
  std::vector<recoTrack*> _vtk;
  double _pMip,_pMipCand;
  uint32_t _nStripPlanes,_nPadPlanes;
};
#endif
