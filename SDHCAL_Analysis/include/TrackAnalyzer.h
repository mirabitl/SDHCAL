#ifndef _TRACKANALYZER_H

#define _TRACKANALYZER_H
#include <limits.h>
#include <vector>
#include "DHCALAnalyzer.h"
#include "DHCalEventReader.h"
#include "DCHistogramHandler.h"
#include <iostream>
#include <sys/timeb.h>
#include "sqlite3.h"
#include <mysql/mysql.h>



#include "IMPL/LCTOOLS.h"
#include "EVENT/RawCalorimeterHit.h" 
#include "IMPL/LCGenericObjectImpl.h"
#include "IMPL/RawCalorimeterHitImpl.h"
#include "UTIL/CellIDDecoder.h"
#include "DifGeom.h"
#include "ChamberGeom.h"

#include "TNtuple.h"
#include "TTree.h"
#include "TFile.h"
#include "StructTree.h"
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
#include "SDHCALMonitor.h"
#include "libhoughStruct.h"
#include <ComputerTrack.h>

class TrackAnalyzer : public DHCALAnalyzer
{
public:
	TrackAnalyzer();
	TrackAnalyzer(DHCalEventReader* r,DCHistogramHandler* h);
	virtual ~TrackAnalyzer(){;}
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
	void drawHits(std::vector<RecoHit*> vrh);
	void draw(TrackInfo& t);
  

	void setCollectionName(std::string s){ collectionName_=s;}
	unsigned long long getExternalTriggerTime() { return (unsigned long long) long(externalTriggerTime_);}
	unsigned long long getAbsoluteFrameTime(int bc) { return (unsigned long long) long(externalTriggerTime_-bc);}


	void setDropFirstSpillEvent(bool t){dropFirstSpillEvent_=t;}

	void setClockSynchCut(unsigned int t){clockSynchCut_=t;}
	void setSpillSize(double t){spillSize_=t;}

	uint32_t NoiseStudy(std::map<uint32_t,std::bitset<255> > &timeDif,std::map<uint32_t,std::bitset<61> > &timeChamber);
	void FillTimeAsic(IMPL::LCCollectionVec* rhcol);
	void DIFStudy(IMPL::RawCalorimeterHitImpl* hit);
	uint32_t buildTracks(std::vector<RecoHit*> &vreco,std::string vdir="/Track");
	void drawDisplay();
	double checkTime();
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

	void PointsBuilder(std::vector<RecoHit*> &vrh);
	void createTrees(std::string s);
	void closeTrees();

	void findTimeSeeds( IMPL::LCCollectionVec* rhcol, int32_t nhit_min,std::vector<uint32_t>& candidate);
	void processSeed(IMPL::LCCollectionVec* rhcol,uint32_t seed);

	uint32_t CerenkovTagger(uint32_t difid,uint32_t seed);
	uint32_t PMAnalysis(uint32_t difid);


private:


	int nAnalyzed_;
	int nInSynch_,run_;
	double externalTriggerTime_,lastSpill_,lastPowerPulsedTime_;


	std::vector<RecoPoint> allpoints_;
	std::vector<RecoCandTk> tklist_;
	std::vector<RecoCandTk> tkgood_;
	std::map<uint32_t,std::vector<RecoPoint*> > chamberPoints_;
	std::map<uint32_t,uint32_t> asicCount_;
	std::map<uint32_t,double*> theCorreff_;
	double integratedTime_;
	RecoHit hitVolume_[60][96][96];

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

	HTImage* theHTx_;
	HTImage* theHTy_;

	struct timeb theTime_;
	struct timeb theCurrentTime_;

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
	TTree* tEvents_;
	TTree* tShowers_;
	TTree* tTracks_;
	TFile* treeFile_;
	uint32_t theNall_,theNedge_,theTag_,theEdge3_,theCore3_,theBigCore_,theCore1_,theCore2_,theNplans_;
	int32_t theWeights_;
	float theZMax_,theZFirst_,theZLast_,thePlanAfter_;
	TNtuple* theNtuple_;
	TFile* theNtupleFile_;
	bool useSqlite_,useMysql_;
	sqlite3* theDb_;
	uint64_t theEventRowId_;
	MYSQL theMysql_;
	std::string myName_,myPwd_,myHost_,myDatabase_;
	array3D<unsigned char> theImage_;
	array3D<unsigned char> theImageWeight_;
	array3D<float> image3_;
	unsigned char theImageBuffer_[60*96*96];
	unsigned char theImageWeightBuffer_[60*96*96];
	unsigned char theImageCoreBuffer_[60*96*96];
	unsigned char theImageEdgeBuffer_[60*96*96];
	float image3Buf_[60*96*96];
	std::vector<RecoHit*> theHitVector_;
	  

	std::vector<Amas> theAmas_;
	SDHCALMonitor* theMonitoring_;
	uint32_t theMonitoringPeriod_;
	std::string theMonitoringPath_;
	
	uint32_t ntkbetween;

	track_t theTrack_;
	point_t thePoint_;
	event_t theEvent_;
	cluster_t theCluster_;
	hit_t theHit_;
	shower_t theShower_;
	uint32_t theNbShowers_,theNbTracks_;
	ComputerTrack* theComputerTrack_;
	unsigned long long theLastBCID_,theIdxSpill_;
	float theTimeInSpill_[20],theCountSpill_[20],theLastRate_;
	float coreRatio_;
	bool isNewSpill_,isPion_,isElectron_,isMuon_,isShower_,isProton_;
	uint32_t theCerenkovTag_;
};
#endif
