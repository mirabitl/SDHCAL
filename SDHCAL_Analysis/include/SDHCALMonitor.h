#ifndef _SDHCALMONITOR_H

#define _SDHCALMONITOR_H
#include <limits.h>
#include <vector>
#include "DHCalEventReader.h"
#include "DCHistogramHandler.h"
#include <iostream>

#include "IMPL/LCTOOLS.h"
#include "EVENT/RawCalorimeterHit.h" 
#include "IMPL/LCGenericObjectImpl.h"
#include "IMPL/RawCalorimeterHitImpl.h"
#include "UTIL/CellIDDecoder.h"
#include "DifGeom.h"
#include "ChamberGeom.h"


#include <vector>
#include <map>
#include "UtilDefs.h"
#include "Array3D.h"
#include "RecoHit.h"
#include "RECOCluster.h"
#include "RecoPoint.h"
#include "RecoCandTk.h"
#include "HC.h"
#include "HTImage.h"
#include "Shower.h"
#include "Amas.h"


class SDHCALMonitor 
{
public:
	SDHCALMonitor(DHCalEventReader* r,DCHistogramHandler* h);
	void clear();
	void FillTimeAsic(IMPL::LCCollectionVec* rhcol);
	void DIFStudy( IMPL::LCCollectionVec* rhcol);	
	void trackHistos(std::vector<RecoCandTk> &tracks,std::vector<RecoPoint> &points,std::string tkdir="/OtherTracking");

	void setFirstChamber(uint32_t i);

	void setLastChamber(uint32_t i);
	void setExtrapolationMinimumPoint(uint32_t i);
	void setExtrapolationMinimumChi2(float i);
	void setChamberEdgeCut( float i);
	void setUseTk4(bool t);
	int getEventIntegratedTime(){return  theEventIntegratedTime_;}
private:

	uint32_t theTrackIndex_,theFirstChamber_,theLastChamber_,theExtrapolationMinimumPoint_;
	float theExtrapolationMinimumChi2_,theChamberEdgeCut_ ,theTrackAngularCut_,theExtrapolationDistanceCut_;
	bool useTk4_;
	DCHistogramHandler* rootHandler_;
	// Reader
	DHCalEventReader* reader_;

	std::map<uint32_t,uint32_t> theAsicCount_;
	int theIntegratedTime_;	
	int theEventIntegratedTime_;	
};
#endif
