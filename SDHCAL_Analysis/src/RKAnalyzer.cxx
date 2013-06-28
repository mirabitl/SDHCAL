#include "RKAnalyzer.h"
class LMFR
{
public:
  int time;
  std::map<int,IMPL::RawCalorimeterHitImpl*> mhit;
  int countHits(int level)
  {
    int nh=0;
    for (std::map<int,IMPL::RawCalorimeterHitImpl*>::iterator kt=mhit.begin(); kt!=mhit.end();kt++)
      {
       int ithr= (*kt).second->getAmplitude();
       if (ithr & (1<<level)) nh++;
      }
    return nh;
  }


};
class LMHR
{
public:
  int asicid;
  std::map<int,LMFR> mfr;
  int getNumberOfFrames(int lowtime=-1,int hightime=999999)
  {
    int nf=0;
    for (std::map<int,LMFR>::iterator kt=mfr.begin(); kt!=mfr.end();kt++)
      {
	  LMFR thefr =(*kt).second;
	  if (thefr.time<lowtime || thefr.time>hightime) continue;
	  nf++;
	}
    return nf;
  }
  int getNumberOfHits(int level,int lowtime=-1,int hightime=999999)
  {
    int nh=0;
    for (std::map<int,LMFR>::iterator kt=mfr.begin(); kt!=mfr.end();kt++)
      {
	  LMFR thefr =(*kt).second;
	  if (thefr.time<lowtime || thefr.time>hightime) continue;
	  
	  nh+= thefr.countHits(level);
	}
    return nh;
  }

};
class LMEDIF
{
public:
  int event_time;
  std::map<int,LMHR> mhr;
};
class LMDIF
{
public:
  int difid;
  std::map<int,LMHR> mhr;
  std::map<int,LMEDIF> mtim;
  int getNumberOfFrames(int lowtime=-1,int hightime=99999)
  {
    int nf=0;
    for (std::map<int,LMHR>::iterator kt=mhr.begin(); kt!=mhr.end();kt++)
      {
	  LMHR thehr =(*kt).second;

	  nf+=thehr.getNumberOfFrames(lowtime,hightime);
	}
    return nf;
  }
  int getNumberOfHits(int level,int lowtime=-1,int hightime=999999)
  {
    int nh=0;
     for (std::map<int,LMHR>::iterator kt=mhr.begin(); kt!=mhr.end();kt++)
      {
	  LMHR thehr =(*kt).second;
	  nh+=thehr.getNumberOfHits(level,lowtime,hightime);
	}
    return nh;
  }

};
class LMHCAL
{
public:
  std::map<int,LMDIF> mdif;
};

//====================================================================================================

RKAnalyzer::RKAnalyzer(DHCalOnlineReader* r) 
{reader_=r;}

void RKAnalyzer::initJob()
{
 MaxTriggerHandeledValue = 100000; //Take the total event number to put it in that in near future

 ClusterCut=3;

 //Init Var for online efficiency study
 EffSamplingWindows = 100 ;
 TimeCutMin_Eff = 0 ;
 TimeCutMax_Eff = 5 ;
 
 TriggerForEff = 0 ;
 GTrig0.clear();
 GTrig1.clear();
 
 MultSamplingWindows = 100;
 TriggerForMultInteg = 0 ;
 CntTrig0.clear();
 CntTrig1.clear();
	
 TriggerForMult = 0;	
 
 Asic_on_bord=48;
 
}

void RKAnalyzer::initHistograms()
{
  std::string timeName[2]={"Synchronised","OffTime"};
  std::stringstream name("");
  std::stringstream difname("");
  std::stringstream timename("");
  std::stringstream levelname("");
  std::stringstream basename("");

  std::cout<<"Booking histogramms"<<endl;
  LCCollection* rhcol=0;	
  //Récup du pointeur sur la collection de hits
  try {
    rhcol = reader_->getEvent()->getCollection("DHCALRawHits");
  }
  catch (...)
    {
      throw " NO hits";
    }
  //Test du pointeur sur la collection
  if (rhcol <= 0) return; //rhcol pointeur sur la collection 
 
  

 
/////////////////////////////////
//Book histograms for geometry//
/////////////////////////////////

  //Boucle sur les elements de la collection pour voir quel dif il y a
	for (int i=0;i<rhcol->getNumberOfElements();i++)
	{
		IMPL::RawCalorimeterHitImpl* hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(i);
		if (hit==0) continue;	
		 int difid = hit->getCellID0()&0xFF;
	
		// Détecter le nombre difid si j'en ai 3 je fais le plot sinon non...

		for (int it=0;it<2;it++)//timename => "Synchronised","OffTime"
		{
			timename.str("");
			timename<<"/"<<timeName[it];
	  
			int nbinx = 96;
			int nbiny = 96;
			if (difid>1000) {nbinx=8; nbiny = 32;}

			for (int il=0;il<2;il++)
			{
				levelname.str("");
				levelname<<"/Level"<<il;
	
				basename.str("");
				basename<<timename.str()<<levelname.str();
			
				// Last Hit Map
				name.str("");
				name<<basename.str()<<"/Current_Hit_Map";
				reader_->BookTH2(name.str(),nbinx,0.1,nbinx+0.1,nbiny,0.1,nbiny+0.1);
			
				// Hit Map
				name.str("");
				name<<basename.str()<<"/Integrated_Hit_Map";
				reader_->BookTH2(name.str(),nbinx,0.1,nbinx+0.1,nbiny,0.1,nbiny+0.1);
			}
		}
    }
	
////////////////////////////////////	
//Book histograms for data quality//
////////////////////////////////////
	
	//GTC evolution
	name.str("");
	name<<"Quality/GTCTriggerEvolution";
	reader_->BookTH1(name.str(),MaxTriggerHandeledValue,0.1,MaxTriggerHandeledValue+0.1);	

	//DTC evolution
	name.str("");
	name<<"Quality/DTCTriggerEvolution";
	reader_->BookTH1(name.str(),MaxTriggerHandeledValue,0.1,MaxTriggerHandeledValue+0.1);
	

	//This histogram his made to find when whe have jumps in GTC
	name.str("");
	name<<"Quality/GTCTriggerJumps";
	reader_->BookTH1(name.str(),MaxTriggerHandeledValue,0.1,MaxTriggerHandeledValue+0.1);	
	
	name.str("");
	name<<"Quality/DTCTriggerJumps";
	reader_->BookTH1(name.str(),MaxTriggerHandeledValue,0.1,MaxTriggerHandeledValue+0.1);	
	
///////////////////////////////////
//Book histograms for efficiency //
///////////////////////////////////	
	name.str("");
	name<<"Quality/Level0/OnlineTimeEfficiency";
	reader_->BookTH1(name.str(),MaxTriggerHandeledValue,0.1,MaxTriggerHandeledValue+0.1);	
	
	name.str("");
	name<<"Quality/Level1/OnlineTimeEfficiency";
	reader_->BookTH1(name.str(),MaxTriggerHandeledValue,0.1,MaxTriggerHandeledValue+0.1);	

/////////////////////////////////////////////
//Book histograms for trigger multiplicity //
/////////////////////////////////////////////		
	name.str("");
	name<<"Quality/Level0/OnlineInTimeTriggerMult";
	reader_->BookTH1(name.str(),MaxTriggerHandeledValue,0.,MaxTriggerHandeledValue);	
	
	name.str("");
	name<<"Quality/Level1/OnlineInTimeTriggerMult";
	reader_->BookTH1(name.str(),MaxTriggerHandeledValue,0.,MaxTriggerHandeledValue);	
	
	name.str("");
	name<<"Quality/Level0/OnlineInTimeTriggerMultWinIntegrated";
	reader_->BookTH1(name.str(),MaxTriggerHandeledValue,0.,MaxTriggerHandeledValue);	
	
	name.str("");
	name<<"Quality/Level1/OnlineInTimeTriggerMultWinIntegrated";
	reader_->BookTH1(name.str(),MaxTriggerHandeledValue,0.,MaxTriggerHandeledValue);	
	
	name.str("");
	name<<"Quality/Level0/OnlineInTimeTriggerMultCluster";
	reader_->BookTH1(name.str(),MaxTriggerHandeledValue,0.,MaxTriggerHandeledValue);	
	
////////////////////////////////////////
//Book histograms for trigger quality //
////////////////////////////////////////	
 for (int i=0;i<rhcol->getNumberOfElements();i++){
	IMPL::RawCalorimeterHitImpl* hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(i);
	if (hit==0) continue;
	int difid = hit->getCellID0()&0xFF;
      difname.str("");
      difname<<"/DIF"<<difid;
      name.str("");
      name <<difname.str()<<"/GTC_TriggerJumps";
      if (reader_->GetTH1(name.str())!=NULL) continue;
		reader_->BookTH1(name.str(),MaxTriggerHandeledValue,0.,MaxTriggerHandeledValue);

      name.str("");
      name <<difname.str()<<"/DTC_TriggerJumps";
	  if (reader_->GetTH1(name.str())!=NULL) continue;
		reader_->BookTH1(name.str(),MaxTriggerHandeledValue,0.,MaxTriggerHandeledValue);	
  }
  
  /*
////////////////////////////////////
//Book histograms for memory depth//
////////////////////////////////////  
 for (int i=0;i<rhcol->getNumberOfElements();i++){
	IMPL::RawCalorimeterHitImpl* hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(i);
	if (hit==0) continue;
	int difid = hit->getCellID0()&0xFF;
      difname.str("");
      difname<<"/DIF"<<difid;
      name.str("");
      name <<difname.str()<<"/Memory_Depth";
      if (reader_->GetTH2(name.str())!=NULL) continue;
		reader_->BookTH2(name.str(),128.,0.,128.);
  }	
  
 */ 
}

void RKAnalyzer::RKPadConvert(int asicid,int ipad, int &i, int &j, int asicType)
{
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Upper view (ASICs SIDE)
//Asics mapping (channel 0 to 63) on small chambers  with HARDROC I
//                            0		    5		  10		15	      20        25        30        35        40        45        50        55        60
const unsigned short MapJSmallHR1[64]={2,2,2,7,5,1,3,1,4,1,6,0,3,0,3,0,7,5,4,0,6,7,2,5,3,1,4,4,6,7,5,6,7,5,6,4,3,7,1,0,5,2,6,7,0,5,4,6,0,6,0,4,4,3,3,3,5,7,1,1,2,1,2,2};
const unsigned short MapISmallHR1[64]={7,6,5,7,7,7,7,5,7,6,7,7,5,5,6,6,6,6,6,4,6,5,4,5,4,4,5,4,5,4,4,4,3,3,3,3,3,2,3,3,2,3,2,1,0,1,2,1,2,0,1,1,0,1,2,0,0,0,2,1,2,0,1,0};
//(J Axis)
//
// 7	|11|05|00|06|08|04|10|03|  	
// 6	|15|09|01|14|18|17|20|16|  	     
// 5	|13|07|02|12|26|23|28|21|  	     
// 4	|19|25|22|24|27|30|31|29|  	     
// 3	|39|38|41|36|35|33|34|32|  	     TOP VIEW (ASICs SIDE)
// 2	|48|58|60|54|46|40|42|37|  	
// 1	|50|59|62|53|51|45|47|43| 
// 0	|44|61|63|55|52|56|49|57|   
//		  0  1  2  3  4  5  6  7    (I Axis)  ----->




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Upper view (ASICs SIDE)
//ASIC 90 deg rotation in the trigonometric way compare to the MapISmallHR1/MapJSmallHR1.
//Asics mapping on large chambers with HARDROC I
//                            0		    5		  10		15	      20        25        30        35        40        45        50        55        60
const unsigned short MapJLargeHR1[64]={0,1,2,0,0,0,0,2,0,1,0,0,2,2,1,1,1,1,1,3,1,2,3,2,3,3,2,3,2,3,3,3,4,4,4,4,4,5,4,4,5,4,5,6,7,6,5,6,5,7,6,6,7,6,5,7,7,7,5,6,5,7,6,7};
const unsigned short MapILargeHR1[64]={2,2,2,7,5,1,3,1,4,1,6,0,3,0,3,0,7,5,4,0,6,7,2,5,3,1,4,4,6,7,5,6,7,5,6,4,3,7,1,0,5,2,6,7,0,5,4,6,0,6,0,4,4,3,3,3,5,7,1,1,2,1,2,2};
//(J Axis)
//
// 7	|03|16|21|29|32|37|43|57|  	
// 6	|10|20|28|31|34|42|47|49|  	     
// 5	|04|17|23|30|33|40|45|56|  	     
// 4	|08|18|26|27|35|46|51|52|  	     
// 3	|06|14|12|24|36|54|53|55|  	     TOP VIEW (ASICs SIDE)
// 2	|00|01|02|22|41|60|62|63|  	
// 1	|05|09|07|25|38|58|59|61| 
// 0	|11|15|13|19|39|48|50|44|   
//		  0  1  2  3  4  5  6  7    (I Axis)  ----->
//				|	 |
//				|DIFF|
//				|____|	


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Upper view (ASICs SIDE)
//90 deg rotation in the trigonometric way compare to the MapILargeHR1/MapJLargeHR1.
//180 deg rotation in the trigonometric way compare to the MapISmallHR1/MapJSmallHR1.
//Asics mapping on large chambers with HARDROC II & IIB
//								 0		   5		 10		   15	     20        25        30        35        40        45        50        55        60
const unsigned short MapJLargeHR2[64]={1,1,2,2,3,3,4,4,5,5,6,6,7,7,4,3,2,0,0,1,0,5,6,7,7,6,5,4,3,2,1,0,0,1,2,3,4,5,6,7,7,6,0,0,1,1,0,2,3,4,5,7,7,6,6,5,5,4,4,3,3,2,2,1};
const unsigned short MapILargeHR2[64]={1,0,1,0,1,0,1,0,0,1,0,1,0,1,2,2,2,0,1,2,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,6,5,6,7,5,5,5,5,6,7,6,7,6,7,6,7,7,6,7,6,7};
//(J Axis)
//
// 7	|46|63|61|59|58|56|54|52|  	
// 6	|43|45|62|60|57|55|53|51|  	     
// 5	|42|44|47|48|49|50|41|40|  	     
// 4	|32|33|34|35|36|37|38|39|  	     
// 3	|31|30|29|28|27|26|25|24|  	     TOP VIEW (ASICs SIDE)
// 2	|20|19|16|15|14|21|22|23|  	
// 1	|18|00|02|04|06|09|11|13| 
// 0	|17|01|03|05|07|08|10|12|   
//		  0  1  2  3  4  5  6  7    (I Axis)  ----->
//				|	 |
//				|DIFF|
//				|____|	

unsigned short AsicShiftI[49]={	0,	0,	0,	0,	0,	8,	8,	8,	8,	16,	16,	16,	16,	24,	24,	24,	24,	32,	32,	32,	32,	40,	40,	40,	40,	48,	48,	48,	48,	56,	56,	56,	56,	64,	64,	64,	64,	72,	72,	72,	72,	80,	80,	80,	80,	88,	88,	88,	88};
unsigned short AsicShiftJ[49]={	0,	0,	8,	16,	24,	24,	16,	8,	0,	0,	8,	16,	24,	24,	16,	8,	0,	0,	8,	16,	24,	24,	16,	8,	0,	0,	8,	16,	24,	24,	16,	8,	0,	0,	8,	16,	24,	24,	16,	8,	0,	0,	8,	16,	24,	24,	16,	8,	0};

 
//Small chambers  HR1
if(asicType==0){
 i = MapISmallHR1[ipad]+AsicShiftI[asicid]+1;
 j = MapJSmallHR1[ipad]+AsicShiftJ[asicid]+1;
}
 
//First square meter HR1
if(asicType==1){
 i = MapILargeHR1[ipad]+AsicShiftI[asicid]+1;
 j = MapJLargeHR1[ipad]+AsicShiftJ[asicid]+1;
}


//Second Square meter HR2
if(asicType==2){
 i = MapILargeHR2[ipad]+AsicShiftI[asicid]+1;
 j = MapJLargeHR2[ipad]+AsicShiftJ[asicid]+1;
}
 
}

void RKAnalyzer::Clusteriser(float* HitX, float* HitY, int nHit, float* ClustX, float* ClustY, float* nHitInClust, int &nClust)
{
	int CntClust=0;
	bool Use[nHit];
	for(int iInit=0; iInit<nHit; iInit++) {Use[iInit]=true;}
			
	for (int iCa=0; iCa<nHit; iCa++){
		if(Use[iCa]){
			float CntHit=1;
			float TotX=HitX[iCa];
			float TotY=HitY[iCa];
					
			for (int iCb=(iCa+1); iCb<nHit; iCb++){ 
				if((sqrt((HitX[iCa]-HitX[iCb])*(HitX[iCa]-HitX[iCb]))<=ClusterCut)&&(sqrt((HitY[iCa]-HitY[iCb])*(HitY[iCa]-HitY[iCb]))<=ClusterCut)&&Use[iCb]){
					TotX+= HitX[iCb];
					TotY+= HitY[iCb];
					CntHit++;
					Use[iCb]=false;
				}
			}
			ClustX[CntClust]=TotX/CntHit;
			ClustY[CntClust]=TotY/CntHit;
			nHitInClust[CntClust]=CntHit;					
			Use[iCa]=false;	
			CntClust++;
		}
	}
	nClust=CntClust;		
}

void RKAnalyzer::ComputeOnlineEfficiencyWindowsIntegrated(LCCollection* rhcol)
{
	//Evaluate event in/off Time Windows
	bool PresenceHit = false;
	bool GoodHit0 =false;
	bool GoodHit1 =false;
	for (int i=0;i<rhcol->getNumberOfElements();i++){
		IMPL::RawCalorimeterHitImpl* hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(i);
		int bc = hit->getTimeStamp();
		bool thr[2];
		int ithr= hit->getAmplitude();
		thr[0] = ithr &1;
		thr[1] = ithr &2;
	  
		if(thr[0]){PresenceHit=true;}
		else {continue;}
	  
		if((bc>=TimeCutMin_Eff)&&(bc<=TimeCutMax_Eff))
		{
			if(thr[0])	GoodHit0=true;
			if(thr[1])	GoodHit1=true;
		}
	 } 
	 
	//Manage buffer and compute efficiency
	if((PresenceHit)&&(TriggerForEff<MaxTriggerHandeledValue)){ //security if to avoid problems
		TriggerForEff++;
		if(GoodHit0){GTrig0.push_back(true);}//Upload windows sampling buffer and increase counter
		else{GTrig0.push_back(false);}//Upload windows sampling buffer
		if(GoodHit1){GTrig1.push_back(true);}//Upload windows sampling buffer and increase counter
		else{GTrig1.push_back(false);}//Upload windows sampling buffer
		
		//Remove firsts event of buffers
		if(GTrig0.size()>=EffSamplingWindows){																																																																											
			GTrig0.erase(GTrig0.begin());
			GTrig1.erase(GTrig1.begin());	
		}
		
		
		TH1* T_Eff_Th0 = reader_->GetTH1("Quality/Level0/OnlineTimeEfficiency");
		if (T_Eff_Th0==NULL)
			T_Eff_Th0 = reader_->BookTH1("Quality/Level0/OnlineTimeEfficiency",MaxTriggerHandeledValue,0.1,MaxTriggerHandeledValue+0.1);
	  
		TH1* T_Eff_Th1= reader_->GetTH1("Quality/Level1/OnlineTimeEfficiency");
		if (T_Eff_Th1==NULL)
			T_Eff_Th1 = reader_->BookTH1("Quality/Level1/OnlineTimeEfficiency",MaxTriggerHandeledValue,0.1,MaxTriggerHandeledValue+0.1);
		
		//Compute efficiency for Threshold 0 case:
		int GoodEvForLocalEff=0;
		for(int tpo=0; tpo<GTrig0.size();tpo++){if(GTrig0[tpo]){GoodEvForLocalEff++;}}
		double ComputedEff = (GoodEvForLocalEff/(double)GTrig0.size());
		double ComputedError = sqrt((ComputedEff*(1-ComputedEff))/(double)GTrig0.size());
		//cout<<"LocalEff: "<<ComputedEff*100<<"% Error: "<<ComputedError*100<<"%"<<endl<<endl;
		T_Eff_Th0->SetBinContent(TriggerForEff, (ComputedEff*100));
		T_Eff_Th0->SetBinError(TriggerForEff, (ComputedError*100));
			
		//Compute efficiency for Tthreshold 1 case:
		GoodEvForLocalEff=0;
		for(int tpo=0; tpo<GTrig1.size();tpo++){if(GTrig1[tpo]){GoodEvForLocalEff++;}}
		ComputedEff = (GoodEvForLocalEff/(double)GTrig1.size());
		ComputedError = sqrt((ComputedEff*(1-ComputedEff))/(double)GTrig1.size());
		//cout<<"LocalEff: "<<ComputedEff*100<<"% Error: "<<ComputedError*100<<"%"<<endl<<endl;
		T_Eff_Th1->SetBinContent(TriggerForEff, (ComputedEff*100));
		T_Eff_Th1->SetBinError(TriggerForEff, (ComputedError*100));
	}  
}

void RKAnalyzer::ComputeOnlineHitMultiplicity(LCCollection* rhcol)
{
	//Evaluate event in/off Time Windows
	bool PresenceHit = false;
	int CntHit0 = 0;
	int CntHit1 = 0;
	for (int i=0;i<rhcol->getNumberOfElements();i++){
		IMPL::RawCalorimeterHitImpl* hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(i);
		int bc = hit->getTimeStamp();
		bool thr[2];
		int ithr= hit->getAmplitude();
		thr[0] = ithr &1;
		thr[1] = ithr &2;
	  
		if(thr[0]){PresenceHit=true;}
		else {continue;}
	  
		if((bc>=TimeCutMin_Eff)&&(bc<=TimeCutMax_Eff))
		{
			if(thr[0])	CntHit0++;
			if(thr[1])	CntHit1++;
		}
	 } 
	
	if(CntHit0){
		TriggerForMult++;
		
		TH1* T_TiggerMult_Th0 = reader_->GetTH1("Quality/Level0/OnlineInTimeTriggerMult");
		if (T_TiggerMult_Th0==NULL)
			T_TiggerMult_Th0 = reader_->BookTH1("Quality/Level0/OnlineInTimeTriggerMult",MaxTriggerHandeledValue,0.1,MaxTriggerHandeledValue+0.1);

		T_TiggerMult_Th0->SetBinContent(TriggerForMult, CntHit0);
	
	}
	
	if(CntHit1){
		
		TH1* T_TiggerMult_Th1= reader_->GetTH1("Quality/Level1/OnlineInTimeTriggerMult");
		if (T_TiggerMult_Th1==NULL)
			T_TiggerMult_Th1 = reader_->BookTH1("Quality/Level1/OnlineInTimeTriggerMult",MaxTriggerHandeledValue,0.1,MaxTriggerHandeledValue+0.1);
		T_TiggerMult_Th1->SetBinContent(TriggerForMult, CntHit1);
		
	}	 
}

void RKAnalyzer::ComputeOnlineHitMultiplicityWindowsIntegrated(LCCollection* rhcol)
{
	//Evaluate event in/off Time Windows
	bool PresenceHit = false;
	int CntHit0 = 0;
	int CntHit1 = 0;
	for (int i=0;i<rhcol->getNumberOfElements();i++){
		IMPL::RawCalorimeterHitImpl* hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(i);
		int bc = hit->getTimeStamp();
		bool thr[2];
		int ithr= hit->getAmplitude();
		thr[0] = ithr &1;
		thr[1] = ithr &2;
	  
		if(thr[0]){PresenceHit=true;}
		else {continue;}
	  
		if((bc>=TimeCutMin_Eff)&&(bc<=TimeCutMax_Eff))
		{
			if(thr[0])	CntHit0++;
			if(thr[1])	CntHit1++;
		}
	 } 
	 
	//Manage buffer and compute efficiency
	if((PresenceHit)&&(TriggerForMultInteg<MaxTriggerHandeledValue)){ //security if to avoid problems
		TriggerForMultInteg++;
		if(CntHit0){CntTrig0.push_back(CntHit0);}//Upload windows sampling buffer and increase counter
		else{CntTrig0.push_back(0);}//Upload windows sampling buffer
		if(CntHit1){CntTrig1.push_back(CntHit1);}//Upload windows sampling buffer and increase counter
		else{CntTrig1.push_back(0);}//Upload windows sampling buffer
		
		//Remove firsts event of buffers
		if(CntTrig0.size()>=MultSamplingWindows){																																																																											
			CntTrig0.erase(CntTrig0.begin());
			CntTrig1.erase(CntTrig1.begin());	
		}
		
		
		TH1* T_TiggerMult_Th0 = reader_->GetTH1("Quality/Level0/OnlineInTimeTriggerMultWinIntegrated");
		if (T_TiggerMult_Th0==NULL)
			T_TiggerMult_Th0 = reader_->BookTH1("Quality/Level0/OnlineInTimeTriggerMultWinIntegrated",MaxTriggerHandeledValue,0.1,MaxTriggerHandeledValue+0.1);
	  
		TH1* T_TiggerMult_Th1= reader_->GetTH1("Quality/Level1/OnlineInTimeTriggerMultWinIntegrated");
		if (T_TiggerMult_Th1==NULL)
			T_TiggerMult_Th1 = reader_->BookTH1("Quality/Level1/OnlineInTimeTriggerMultWinIntegrated",MaxTriggerHandeledValue,0.1,MaxTriggerHandeledValue+0.1);
		
		//Compute Hit Multiplicity for Threshold 0 case:
		double CntEvForTriggMult=0;
		double TriggMultSumm=0;
		for(int tpo=0; tpo<CntTrig0.size();tpo++){if(CntTrig0[tpo]>0){CntEvForTriggMult++; TriggMultSumm+=CntTrig0[tpo];}}
		double ComputedMult = (TriggMultSumm/CntEvForTriggMult);
		T_TiggerMult_Th0->SetBinContent(TriggerForMultInteg, ComputedMult);

			
		//Compute Hit Multiplicity for Tthreshold 1 case:
		CntEvForTriggMult=0;
		TriggMultSumm=0;
		for(int tpo=0; tpo<CntTrig1.size();tpo++){if(CntTrig1[tpo]>0){CntEvForTriggMult++; TriggMultSumm+=CntTrig1[tpo];}}
		ComputedMult = (TriggMultSumm/CntEvForTriggMult);
		T_TiggerMult_Th1->SetBinContent(TriggerForMultInteg, ComputedMult);
	}  
}

void RKAnalyzer::Hit_Maps(LCCollection* rhcol)
{
	// Loop on all frames
	std::stringstream name("");
  
	std::vector<std::string> curMaps;

	 //Reset de la curent hit map
    reader_->ListHisto("Current_Hit_Map",2,curMaps);
    for (unsigned int i=0;i<curMaps.size();i++) 
		reader_->GetTH2(curMaps[i])->Reset();

	for (int i=0;i<rhcol->getNumberOfElements();i++)
    {
	  int x,y;
      IMPL::RawCalorimeterHitImpl* hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(i);
      if (hit==0) continue;
      
	  int difid = hit->getCellID0()&0xFF;
      int asicid = (hit->getCellID0()&0xFF00)>>8;
      int channel= (hit->getCellID0()&0x3F0000)>>16;
      int bc = hit->getTimeStamp();
      bool thr[2];
      int ithr= hit->getAmplitude();
      thr[0] = ithr &1;
      thr[1] = ithr &2;
	
	  //Fabrication nom pour histo 
	  name.str("");
      if (bc<5)
		name<<"/Synchronised";
      else
		name<<"/OffTime";

	  int asictype =1;	
	  int nbinx = 96;
      int nbiny= 32;
      if (difid>1000) {nbinx=8; nbiny=32;}

	  //Recupération des histo 	 
      TH2* hl0int = reader_->GetTH2(name.str()+"/Level0/Integrated_Hit_Map");
      if (hl0int==NULL)
		hl0int =reader_->BookTH2( name.str()+"/Level0/Integrated_Hit_Map",nbinx,0.1,nbinx+0.1,nbiny,0.1,nbiny+0.1);
      TH2* hl1int = reader_->GetTH2(name.str()+"/Level1/Integrated_Hit_Map");
      if (hl1int==NULL)
		hl1int =reader_->BookTH2( name.str()+"/Level1/Integrated_Hit_Map",nbinx,0.1,nbinx+0.1,nbiny,0.1,nbiny+0.1);
      TH2* hl0cur= reader_->GetTH2(name.str()+"/Level0/Current_Hit_Map");
      if (hl0cur==NULL)
		hl0cur = reader_->BookTH2(name.str()+"/Level0/Current_Hit_Map",nbinx,0.1,nbinx+0.1,nbiny,0.1,nbiny+0.1);
      TH2* hl1cur= reader_->GetTH2(name.str()+"/Level1/Current_Hit_Map");
      if (hl1cur==NULL)
		hl1cur = reader_->BookTH2(name.str()+"/Level1/Current_Hit_Map",nbinx,0.1,nbinx+0.1,nbiny,0.1,nbiny+0.1);
     
      if (thr[0]) {
		int asic=asicid;
		if (difid>1000) asic=(asic-1)%4+1;
		
		//DCBufferReader::PadConvert(asic,channel,x,y);
		RKPadConvert(asic,channel,x,y, asictype); 

		int offset=0;
		if(difid==6) offset=0;
		if(difid==9) offset=32;		
		if(difid==10) offset=64;
		
		if (hl0int) hl0int->Fill(x*1.,(y*1.+ offset));
		if (hl0cur) hl0cur->Fill(x*1.,(y*1.+ offset));
      }
	  
      if (thr[1]) {
		int asic=asicid;
		if (difid>1000) asic=(asic-1)%4+1;
		//DCBufferReader::PadConvert(asic,channel,x,y);
		RKPadConvert(asic,channel,x,y, asictype);


		//Faire un system ici qui utilise le xml pour placer l'offset en fonction de la position de chaque dif...
		//Faire un plot graphique par M2 ou board... on selectionne les m2 en fonction du numerot de dif...
		//Il faut donc declarer tout ca en initialisant les histogrames, et envoyer les infos au processeur pour qu'il les gères
		int offset;
		if(difid==6) offset=0;
		if(difid==9) offset=32;		
		if(difid==10) offset=64;

		if (hl1int) hl1int->Fill(x*1.,(y*1.+ offset));
		if (hl1cur) hl1cur->Fill(x*1.,(y*1.+ offset));
      }
  
    }

}

void RKAnalyzer::ComputeOnlineClusterMultiplicity(LCCollection* rhcol)
{
	int nHit=0;
	// Loop on all frames
	for (int i=0;i<rhcol->getNumberOfElements();i++)
    {
      IMPL::RawCalorimeterHitImpl* hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(i);
      if (hit==0) continue;
	  else nHit++;
	}

    float tHitX[nHit];
	float tHitY[nHit];

	int nextHit=0;
	for (int i=0;i<rhcol->getNumberOfElements();i++)
    {
	  int x,y;
      IMPL::RawCalorimeterHitImpl* hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(i);
      if (hit==0) continue;
	  int difid = hit->getCellID0()&0xFF;
      int asicid = (hit->getCellID0()&0xFF00)>>8;
      int channel= (hit->getCellID0()&0x3F0000)>>16;
      int bc = hit->getTimeStamp();
      bool thr[2];
      int ithr= hit->getAmplitude();
      thr[0] = ithr &1;
      thr[1] = ithr &2;
	
	  //Fabrication nom pour histo 
	 
     
	  if((bc>=TimeCutMin_Eff)&&(bc<=TimeCutMax_Eff)){

	  int asictype =1;	
	  int nbinx = 96;
      int nbiny= 32;
      if (difid>1000) {nbinx=8; nbiny=32;}
     
      if (thr[0]) {
		int asic=asicid;
		if (difid>1000) asic=(asic-1)%4+1;
		RKPadConvert(asic,channel,x,y, asictype); 
	
	    //Dependant of asic id
		if(difid==6){;}
		else{
		 if(difid==9) x+=32;		
		 else{ 
			if(difid==10) x+=64;
			else assert(0);//This processor has to be modified to handle other diffs
		 }
		}
		tHitX[nextHit]=x;
		tHitY[nextHit]=y;
		nextHit++;
      }
	  	
    }
    }
	
	
	float tClustX[nHit];
	float tClustY[nHit];
	float tnHitInClust[nHit];
	int nClusters=0;
	
	Clusteriser(tHitX, tHitY, nextHit, tClustX, tClustY, tnHitInClust, nClusters);
	
	
	TH1* T_MultClust_Th0 = reader_->GetTH1("Quality/Level0/OnlineInTimeTriggerMultCluster");
	if (T_MultClust_Th0==NULL)
			T_MultClust_Th0 = reader_->BookTH1("Quality/Level0/OnlineInTimeTriggerMultCluster",MaxTriggerHandeledValue,0.,MaxTriggerHandeledValue);
	
//	std::cout<<"Clust numb: "<<nClusters<<endl;
	for(int iTp=0; iTp<nClusters; iTp++){
		T_MultClust_Th0->Fill(tnHitInClust[iTp]);
//		cout<<"Num hit in cluster: "<<tnHitInClust[iTp]<<endl;
		//if(tnHitInClust[iTp]==0)getchar();
	}
	

}

void RKAnalyzer::TriggerQuality(LCCollection* rhcol)
{
	//LCTOOLS::printParameters(rhcol->getParameters());
	StringVec intKeys;
	IntVec intVec ;	
    int nIntParameters = rhcol->getParameters().getIntKeys( intKeys ).size() ;
	for(int i=0; i< nIntParameters ; i++ ){
		intVec.clear();
		rhcol->getParameters().getIntVals( intKeys[i], intVec ) ;
		
	
		if(lastParams.find(intKeys[i])!=lastParams.end()){ //Test d'exitence
			IntVec previntVec = (IntVec)lastParams[(std::string)intKeys[i]];

			if(GTC_Offset.find(intKeys[i])!=GTC_Offset.end()){
				if((intVec[0]-previntVec[0]-(int)GTC_Offset[(std::string)intKeys[i]])>1){
					int theJump = intVec[0]-previntVec[0]-(int)GTC_Offset[(std::string)intKeys[i]]-1;
					std::pair<std::string,int> pr(intKeys[i], theJump);
					GTC_Offset.erase(intKeys[i]);
					GTC_Offset.insert(pr);					
				}
			}else{
				std::pair<std::string,int> pr(intKeys[i], 0);
				GTC_Offset.insert(pr);
			}
			if(DTC_Offset.find(intKeys[i])!=DTC_Offset.end()){
				if((intVec[1]-previntVec[1]-(int)DTC_Offset[(std::string)intKeys[i]])>1){
					int theJump = intVec[1]-previntVec[1]-(int)DTC_Offset[(std::string)intKeys[i]]-1;
					std::pair<std::string,int> pr(intKeys[i], theJump);
					DTC_Offset.erase(intKeys[i]);
					DTC_Offset.insert(pr);					
				}
			}else{
				std::pair<std::string,int> pr(intKeys[i], 0);
				DTC_Offset.insert(pr);
			}
		}
		
		if((GTC_Offset.find(intKeys[i])!=GTC_Offset.end())&&(DTC_Offset.find(intKeys[i])!=DTC_Offset.end())){
			//Save in histogramm
			std::stringstream name("");
			if (strstr(intKeys[i].c_str(),"DIF") != NULL){
				int len=intKeys[i].length();
				string temp=intKeys[i].substr(3,len-8); // "_Triggers"=9char
				int pos=temp.find("_");
				int dif_ = atoi(temp.substr(0,pos).c_str());
			
				name<<"/DIF"<<dif_;
			
				TH1* hGTC = reader_->GetTH1(name.str()+"/GTC_TriggerJumps");
				if (hGTC==NULL)
					hGTC =reader_->BookTH1(name.str()+"/GTC_TriggerJumps",MaxTriggerHandeledValue,0.,MaxTriggerHandeledValue);
			
				TH1* hDTC = reader_->GetTH1(name.str()+"/DTC_TriggerJumps");
				if (hDTC==NULL)
					hDTC =reader_->BookTH1(name.str()+"/DTC_TriggerJumps",MaxTriggerHandeledValue,0.,MaxTriggerHandeledValue);
				
				hGTC->SetBinContent(reader_->getEvent()->getEventNumber(),(int)GTC_Offset[(std::string)intKeys[i]]);
				hDTC->SetBinContent(reader_->getEvent()->getEventNumber(),(int)DTC_Offset[(std::string)intKeys[i]]);	
			}
		}
		
	 }
	 
	 //Copy current parameters in object for next
	 for(int i=0; i< nIntParameters ; i++ ){
		intVec.clear();
		rhcol->getParameters().getIntVals( intKeys[i], intVec ) ;
		std::pair<std::string,IntVec> pr(intKeys[i],intVec);
		lastParams.erase(intKeys[i]);
		lastParams.insert(pr);		
	 }
}

/*
void RKAnalyzer::Memory_Depth(LCCollection* rhcol)
{
	std::map<int,std::map<int,int>> Counters; 
	
	//J'écris dans un map de map la valeur des compteurs des asicsx 
	
	(int)Counters[(std::string)intKeys[i]]-1;
					std::pair<std::string,int> pr(intKeys[i], theJump);
					GTC_Offset.erase(intKeys[i]);
					GTC_Offset.insert(pr);					
	
	for (int i=0;i<rhcol->getNumberOfElements();i++)
    {
      IMPL::RawCalorimeterHitImpl* hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(i);
      if (hit==0) continue;
      
	  int difid = hit->getCellID0()&0xFF;
      int asicid = (hit->getCellID0()&0xFF00)>>8;
 	
	  
	  
	   
		
		 
	  //Fabrication nom pour histo 
	  name.str("");
      if (bc<5)
		name<<"/Synchronised";
      else
		name<<"/OffTime";

	  int asictype =1;	
	  int nbinx = 96;
      int nbiny= 32;
      if (difid>1000) {nbinx=8; nbiny=32;}

     
      if (thr[0]) {
		int asic=asicid;
		if (difid>1000) asic=(asic-1)%4+1;
		
		//DCBufferReader::PadConvert(asic,channel,x,y);
		RKPadConvert(asic,channel,x,y, asictype); 

		int offset=0;
		if(difid==6) offset=0;
		if(difid==9) offset=32;		
		if(difid==10) offset=64;
		
		if (hl0int) hl0int->Fill(x*1.,(y*1.+ offset));
		if (hl0cur) hl0cur->Fill(x*1.,(y*1.+ offset));
      }

	
	
	//LCTOOLS::printParameters(rhcol->getParameters());
	StringVec intKeys;
	IntVec intVec ;	
    int nIntParameters = rhcol->getParameters().getIntKeys( intKeys ).size() ;
	for(int i=0; i< nIntParameters ; i++ ){
		intVec.clear();
		rhcol->getParameters().getIntVals( intKeys[i], intVec ) ;
		
	
		if(lastParams.find(intKeys[i])!=lastParams.end()){ //Test d'exitence
			IntVec previntVec = (IntVec)lastParams[(std::string)intKeys[i]];

			if(GTC_Offset.find(intKeys[i])!=GTC_Offset.end()){
				if((intVec[0]-previntVec[0]-(int)GTC_Offset[(std::string)intKeys[i]])>1){
					int theJump = intVec[0]-previntVec[0]-(int)GTC_Offset[(std::string)intKeys[i]]-1;
					std::pair<std::string,int> pr(intKeys[i], theJump);
					GTC_Offset.erase(intKeys[i]);
					GTC_Offset.insert(pr);					
				}
			}else{
				std::pair<std::string,int> pr(intKeys[i], 0);
				GTC_Offset.insert(pr);
			}
			if(DTC_Offset.find(intKeys[i])!=DTC_Offset.end()){
				if((intVec[1]-previntVec[1]-(int)DTC_Offset[(std::string)intKeys[i]])>1){
					int theJump = intVec[1]-previntVec[1]-(int)DTC_Offset[(std::string)intKeys[i]]-1;
					std::pair<std::string,int> pr(intKeys[i], theJump);
					DTC_Offset.erase(intKeys[i]);
					DTC_Offset.insert(pr);					
				}
			}else{
				std::pair<std::string,int> pr(intKeys[i], 0);
				DTC_Offset.insert(pr);
			}
		}
		
		if((GTC_Offset.find(intKeys[i])!=GTC_Offset.end())&&(DTC_Offset.find(intKeys[i])!=DTC_Offset.end())){
			//Save in histogramm
			std::stringstream name("");
			if (strstr(intKeys[i].c_str(),"DIF") != NULL){
				int len=intKeys[i].length();
				string temp=intKeys[i].substr(3,len-8); // "_Triggers"=9char
				int pos=temp.find("_");
				int dif_ = atoi(temp.substr(0,pos).c_str());
			
				name<<"/DIF"<<dif_;
			
				TH2* hMD = reader_->GetTH2(name.str()+"/Memory_Depth");
				if (hMD==NULL)
					hMD =reader_->BookTH2(name.str()+"/Memory_Depth",128.,0.,128.);
			
				
				hMD->Fill(reader_->getEvent()->getEventNumber(),(int)DTC_Offset[(std::string)intKeys[i]]);	
			}
		}
		
	 }
	 
	 //Copy current parameters in object for next
	 for(int i=0; i< nIntParameters ; i++ ){
		intVec.clear();
		rhcol->getParameters().getIntVals( intKeys[i], intVec ) ;
		std::pair<std::string,IntVec> pr(intKeys[i],intVec);
		lastParams.erase(intKeys[i]);
		lastParams.insert(pr);		
	 }
	 
	 for(int i=0; i<
	 name<<"/DIF"<<dif_;
			
				TH2* hMD = reader_->GetTH2(name.str()+"/Memory_Depth");
				if (hMD==NULL)
					hMD =reader_->BookTH2(name.str()+"/Memory_Depth",128.,0.,128.);
			
				
				hMD->Fill(reader_->getEvent()->getEventNumber(),(int)DTC_Offset[(std::string)intKeys[i]]);
}*/

//void MemoryDepthByAsic(un par board)
/*
Histo 2D (Profondeur_memoire(0-128), Numérot_asic(0-48))
Histo BC simplement
Histo 2D des (last frame time, Numérot_asic)

*/
void RKAnalyzer::processEvent()
{
   //Check that there is events	
  if (reader_->getEvent()==0) return;
  
  //Book histograms on first event
  try {
	if (reader_->getEvent()->getEventNumber()==1) this->initHistograms();
  } 
  catch (std::string s){
	std::cout<<" error on histogram booking"<<s<<std::endl;
  }
	
  LCCollection* rhcol=0;
  try 
    {
      rhcol = reader_->getEvent()->getCollection("DHCALRawHits");
    }
  catch (...)
    {
      std::cout<<" No collection try to build it" <<std::endl;
      reader_->buildEvent();
      rhcol = reader_->getEvent()->getCollection("DHCALRawHits");
    }
   
  if (rhcol == 0) return;

  Hit_Maps(rhcol);
  ComputeOnlineEfficiencyWindowsIntegrated(rhcol);
  ComputeOnlineHitMultiplicity(rhcol);
  ComputeOnlineHitMultiplicityWindowsIntegrated(rhcol);	
  ComputeOnlineClusterMultiplicity(rhcol);	
  TriggerQuality(rhcol);
  //std::cout<<rhcol->getNumberOfElements() <<" frames found "<<std::endl;
  //Data Quality Histograms
	  
	//std::cout<<"try to print parameters: "<<std::endl;
	//LCTOOLS::printParameters(rhcol->getParameters());
      
	//std::vector< std::string > IntColNames;
	//const EVENT::LCParameters params = rhcol->getParameters();
	//params->getIntKeys(IntColNames); 
	
	//for (std::vector<std::string >::iterator itt = IntColNames.begin();itt!=IntColNames.end();itt++)  
	//	std::cout<<"Key is: "<<itt<<endl; 

		//GTC Jumps detection
	  //Quality/GTCTriggerJumps

	  
	 // GTC_->Fill();
  
//#endif
}
