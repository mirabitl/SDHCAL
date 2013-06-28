#ifndef _RKANALYZER_TB_Oct_H
#define _RKANALYZER_TB_Oct_H
#include "DHCalEventReader.h"
#include "DCHistogramHandler.h"
#include "DHCALAnalyzer.h"
#include "DHCalOnlineReader.h"
#include <iostream>
#include <math.h>
#include <assert.h>
#include <map>
#include "TGraph.h"
#include "TF1.h"

class RKAnalyzerTB_Oct : public DHCALAnalyzer
{
 public:
  RKAnalyzerTB_Oct(DHCalOnlineReader* r);
  virtual void processEvent();
  virtual void initHistograms();
 
  virtual void initJob();
  virtual void endJob(){;}
  virtual void initRun(){;}
  virtual void endRun(){;}
  
  void processRunHeader();	
	
  void RKPadConvert(int asicid,int ipad, int &i, int &j,int asicType);	
  float ZLayer(int DIFid);
  float X_M2_Shift(int x, int DIFid);

  void Clusteriser(Float_t* HitX, Float_t* HitY, Float_t* HitZ, Int_t nHit, Float_t* ClustX, Float_t* ClustY, Float_t* ClustZ, Float_t* nHitInClust, Int_t &nClust);	
  void BeamProfiles(LCCollection* rhcol);
  void SumBeamProfiles(LCCollection* rhcol);
	

  void SetStatus(int st){status=st;}	
  void FirstAlign();	
  void SecondAlign(LCCollection* rhcol);	
 	
	
 private:
 
 
  DHCalOnlineReader* reader_;
  int status;
  
  int ClusterCut; //Spatial cut on clustering function inclusive value (<=)
  
  float Chamber1_Z;
  float Chamber2_Z;
  float Chamber3_Z;
  float Chamber4_Z;
	
  float Ch1_X_Offset_fine;	
  float Ch1_Y_Offset_fine;		
	
  float Ch2_X_Offset;
  float Ch2_Y_Offset;
  float Ch2_X_Offset_fine;	
  float Ch2_Y_Offset_fine;		
	
  float Ch3_X_Offset;
  float Ch3_Y_Offset;
  float Ch3_X_Offset_fine;	
  float Ch3_Y_Offset_fine;	
	
  float Ch4_X_Offset;
  float Ch4_Y_Offset;
  float Ch4_X_Offset_fine;	
  float Ch4_Y_Offset_fine;	

	
};
#endif
