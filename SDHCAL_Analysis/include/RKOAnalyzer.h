#ifndef _RKANALYZER_H
#define _RKANALYZER_H
#include "DHCALAnalyzer.h"

#include "DHCalEventReader.h"
#include "DCHistogramHandler.h"

#include <iostream>
#include <math.h>
#include <assert.h>
#include <map>
class RKOAnalyzer : public DHCALAnalyzer
{
 public:
  RKOAnalyzer(DHCalEventReader* r,DCHistogramHandler* h);
  virtual void processEvent();
  virtual void initHistograms();
  virtual void processRunHeader(){;}
  virtual void initJob();
  virtual void endJob(){;}
  virtual void initRun(){;}
  virtual void endRun(){;}
  
  void RKPadConvert(int asicid,int ipad, int &i, int &j,int asicType);	
  void Clusteriser(float* HitX, float* HitY, int nHit, float* ClustX, float* ClustY, float* nHitInClust, int &nClust);	
	
  void Hit_Maps(LCCollection* rhcol);
  void ComputeOnlineHitMultiplicity(LCCollection* rhcol);
  void ComputeOnlineHitMultiplicityWindowsIntegrated(LCCollection* rhcol);
  
  void ComputeOnlineClusterMultiplicity(LCCollection* rhcol);
 
  void ComputeOnlineEfficiencyWindowsIntegrated(LCCollection* rhcol);	

  void TriggerQuality(LCCollection* rhcol);

 private:
 
  DHCalEventReader* reader_;
  DCHistogramHandler* handler_;
 private:
  int MaxTriggerHandeledValue;
  
  int ClusterCut; //Spatial cut on clustering function inclusive value (<=)
  
  //Var for efficiency/multiplicity study
 
  int TimeCutMin_Eff; //inclusive value
  int TimeCutMax_Eff; //inclusive value
  //Eff specific var
  int EffSamplingWindows;
  int TriggerForEff;
  vector<bool> GTrig0;
  vector<bool> GTrig1;
  
  //Mult specific var
  int TriggerForMult;
  int MultSamplingWindows;
  int TriggerForMultInteg;
  vector<int> CntTrig0;
  vector<int> CntTrig1;
  
  //Trigger Quality
  std::map<std::string,IntVec> lastParams;  
  std::map<std::string,int> GTC_Offset;  
  std::map<std::string,int> DTC_Offset;     

  //Memory Depth
  int Asic_on_bord;
};
#endif
