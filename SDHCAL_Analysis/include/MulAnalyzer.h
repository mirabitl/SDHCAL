#ifndef _MULANALYZER_H

#define _MULANALYZER_H
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

#include "UtilDefs.h"
#include "Array3D.h"
#include "RecoHit.h"
#include "RECOCluster.h"
#include "DifGeom.h"
#include "ChamberGeom.h"
class MulAnalyzer : public DHCALAnalyzer
{
public:
	MulAnalyzer();

	virtual ~MulAnalyzer(){;}
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
	uint32_t buildVolume(IMPL::LCCollectionVec* rhcol,uint32_t seed);
  




private:

	std::string collectionName_;
	int nAnalyzed_;

	DCHistogramHandler* rootHandler_;


	// Reader
	DHCalEventReader* reader_;

	bool writing_;
	bool draw_;

	IMPL::LCEventImpl* evt_;


	uint32_t theSkip_;

	array3D<unsigned char> theImage_;
	unsigned char theImageBuffer_[60*96*96];
	RecoHit hitVolume_[60][96][96];
};
#endif
