#ifndef _RAWANALYZER_H

#define _RAWANALYZER_H
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

class RawAnalyzer : public DHCALAnalyzer
{
public:
	RawAnalyzer();

	virtual ~RawAnalyzer(){;}
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

  




private:


	int nAnalyzed_;

	DCHistogramHandler* rootHandler_;


	// Reader
	DHCalEventReader* reader_;

	bool writing_;
	bool draw_;

	IMPL::LCEventImpl* evt_;

	unsigned long long theBCID_;
	unsigned long long theBCIDSpill_;
	unsigned long long theAbsoluteTime_;


	uint32_t theMonitoringPeriod_;
	uint32_t theSkip_,theCount_[255][49],theTotalCount_[255][49];
	double theTotalTime_,theEventTotalTime_;
	std::string theMonitoringPath_;
	
	unsigned long long theStartBCID_;
	uint32_t _neff,_neff2,_neff3,_nall;

};
#endif
