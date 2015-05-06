#ifndef _SHOWERANALYZER_H

#define _SHOWERANALYZER_H
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

class ShowerAnalyzer : public DHCALAnalyzer
{
public:
	ShowerAnalyzer();
	ShowerAnalyzer(DHCalEventReader* r,DCHistogramHandler* h);
	virtual ~ShowerAnalyzer(){;}
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
  
	void trackHistos();
	void findTracks();
	void findTracks1();
	void CosmicFinder();
	void appendHits(RecoCandTk& t);
	void setCollectionName(std::string s){ collectionName_=s;}
	unsigned long long getExternalTriggerTime() { return (unsigned long long) long(externalTriggerTime_);}
	unsigned long long getAbsoluteFrameTime(int bc) { return (unsigned long long) long(externalTriggerTime_-bc);}


	void setDropFirstSpillEvent(bool t){dropFirstSpillEvent_=t;}

	void setClockSynchCut(unsigned int t){clockSynchCut_=t;}
	void setSpillSize(double t){spillSize_=t;}

	uint32_t NoiseStudy(std::map<uint32_t,std::bitset<255> > &timeDif,std::map<uint32_t,std::bitset<61> > &timeChamber);
	void FillTimeAsic(IMPL::LCCollectionVec* rhcol);
	void DIFStudy(IMPL::RawCalorimeterHitImpl* hit);
	uint32_t buildTracks(std::vector<RecoHit*> &vreco);
	uint32_t buildClusters(std::vector<RecoHit*> &vreco);
	void buildEdges();

	void ShowerBuilder(std::vector<RecoHit*> &vreco);
	void ImageBuilder(std::vector<RecoHit*> &vreco);
	void sobel_filtering(unsigned char i[60][96], float j[60][96]);

	static void sobel_volume(unsigned char image1[60][96][96],float image2[60][96][96] );

	void drawDisplay();
	void HT();
	void HT2D();
	void HT3D();
	void HTOld();
	double checkTime();
	void findHoughCandidates3D(uint32_t stop,std::vector<RecoCandTk> &tkSeed);

	void findHoughCandidates(uint32_t type,uint32_t stop,std::vector<HC> &vX);
	void mergeHoughCandidate(std::vector<HC> &vX,std::vector<HC> &vY,std::vector<RecoCandTk> &tkSeed);
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


	void createTrees(std::string s);
	void closeTrees();

	void PointsBuilder(std::vector<RecoHit*> &vrh);

	void TracksBuilder(Shower &ish,std::vector<RecoHit*> &vrh);
	void TracksBuilder();
	
	void FillTrackTree();
	void readCalibration(uint32_t run) throw (std::string); 

	void openSqlite(std::string fileName);
	int32_t fillEventTable(uint32_t run, uint32_t event,uint64_t bcid,uint32_t time,uint32_t e3,uint32_t c3,int32_t w,uint32_t np);
	int32_t fillComponentsTable(uint32_t *nh,double *components);
	int32_t fillAmasTable(Amas* a);
	int32_t fillAmasSummaryTable(uint32_t core[3],uint32_t edge[3],uint32_t namas,uint32_t nafter,uint32_t tag,double zlast);
	int32_t fillShowerTable(shower_t &s);
	int32_t fillTrackTable(uint32_t npoint,double ax,double bx,double ay,double by,double chi2);
	int32_t fillPointTable(uint32_t trackid,uint32_t plan,double z,double xext,double yext,double xclus=999.,double yclus=999.);
	int32_t executeQuery(std::string stmt);
	uint32_t getLastInsertId();
	void decodeAccount(std::string account);
	void connect(std::string account);

	void findTimeSeeds( IMPL::LCCollectionVec* rhcol, int32_t nhit_min,std::vector<uint32_t>& candidate);
	uint32_t buildVolume(IMPL::LCCollectionVec* rhcol,uint32_t seed);
	uint32_t buildVolume(uint32_t seed);
	void processSeed(IMPL::LCCollectionVec* rhcol,uint32_t seed);
	//void EdgeDetection(unsigned char imagev[60][96][96],unsigned char core[60][96][96],unsigned char edge[60][96][96]);
	void EdgeDetection(array3D<unsigned char> &i,array3D<unsigned char> &c,array3D<unsigned char> &e);
	//void sobel_volume(unsigned char *image1,float *image2,uint32_t x_size1,uint32_t y_size1,uint32_t z_size1 );
	static void sobel_volume(array3D<unsigned char> &im1,array3D<float> &im2);
	void draw(array3D<unsigned char> &all,array3D<unsigned char> &core,array3D<unsigned char> &edge);
	void draw(std::vector<RecoCandTk> &tracks,std::vector<RecoPoint> &points);
	uint32_t mergeAmas(array3D<unsigned char> &core,array3D<unsigned char> &edge);

	void newHT(array3D<unsigned char> &core);
	void newHT2(array3D<unsigned char> &core);
	void newHT3(array3D<unsigned char> &core);

	void track2Db(std::vector<RecoCandTk> &tracks,std::vector<RecoPoint> &points);
	void drawph(houghParam* p);

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
	bool isNewSpill_;
};
#endif
