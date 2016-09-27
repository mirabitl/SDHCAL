#ifndef _HITMONITOR_HH

#define _HITMONITOR_HH
#include <limits.h>
#include <vector>
#include "jsonGeo.hh"
#include "DCHistogramHandler.h"
#include <iostream>
#include "DHCalEventReader.h"

#include "IMPL/LCTOOLS.h"
#include "EVENT/RawCalorimeterHit.h" 
#include "IMPL/LCGenericObjectImpl.h"
#include "IMPL/RawCalorimeterHitImpl.h"
#include "UTIL/CellIDDecoder.h"



#include <vector>
#include <map>
#include "RecoHit.hh"
#include "planeCluster.hh"
#include "recoTrack.hh"
#include "TricotCluster.hh"


class hitMonitor 
{
public:
  hitMonitor(jsonGeo* g);
  void clear();
  void FillTimeAsic(IMPL::LCCollectionVec* rhcol,bool difanalysis=false);
  void DIFStudy( IMPL::LCCollectionVec* rhcol,bool external=false);	


  void setFirstChamber(uint32_t i);

  void setLastChamber(uint32_t i);
  void setExtrapolationMinimumPoint(uint32_t i);
  void setExtrapolationMinimumChi2(float i);
  void setChamberEdgeCut( float i);
  void setUseTk4(bool t);
  int getEventIntegratedTime(){return  theEventIntegratedTime_;}
  void trackHistos(std::vector<recoTrack*> &tracks,std::vector<recoPoint*> &clusters,std::string tkdir);
  void clusterHistos(std::vector<TricotCluster> &tcl,std::vector<planeCluster*> &clusters,std::string tkdir);
private:

  uint32_t theTrackIndex_,theFirstChamber_,theLastChamber_,theExtrapolationMinimumPoint_;
  float theExtrapolationMinimumChi2_,theChamberEdgeCut_ ,theTrackAngularCut_,theExtrapolationDistanceCut_;
  bool useTk4_;
  DCHistogramHandler* rootHandler_;
	

  std::map<uint32_t,uint64_t> theAsicCount_;
  int theIntegratedTime_;	
  int theEventIntegratedTime_;
  jsonGeo* _geo;
};
#endif
