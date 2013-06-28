#include "RKAnalyzerTB_Oct.h"
#include <UTIL/LCTOOLS.h>
#include <iomanip>

#define DEBUG
#define DEBUG_ALL

//Made for october 2010 testbeam

RKAnalyzerTB_Oct::RKAnalyzerTB_Oct(DHCalOnlineReader* r) 
{reader_=r;}

void RKAnalyzerTB_Oct::initJob()
{

	status=1;
	ClusterCut=3; //KEEP
	
	Chamber1_Z=0;
	Chamber2_Z=6.5;
	Chamber3_Z=48.5;
	Chamber4_Z=117;

	Ch2_X_Offset=0;
	Ch2_Y_Offset=0;
	
	Ch3_X_Offset=0;
	Ch3_Y_Offset=0;
	
	
	Ch4_X_Offset=0;
	Ch4_Y_Offset=0;
}

void RKAnalyzerTB_Oct::initHistograms()
{
  std::cout<<"Booking histogramms"<<endl;
  LCCollection* rhcol=0;	
 
//Recup du pointeur sur la collection de hits
  try {
    rhcol = reader_->getEvent()->getCollection("DHCALRawHits");
  }
  catch (...)
    {
      throw " NO hits";
    }


  //Test du pointeur sur la collection
 
/////////////////////////////////
//Book histograms for geometry//
/////////////////////////////////
 
if (reader_->GetTH2("Chamber1")==NULL)
		reader_->BookTH2("Chamber1",96., 0., 96.,96., 0., 96.);

 
if (reader_->GetTH1("Chamber1_X")==NULL)
		reader_->BookTH1("Chamber1_X",96, 0., 96.);

 if (reader_->GetTH1("Chamber1_Y")==NULL)
		reader_->BookTH1("Chamber1_Y",96, 0., 96.);
		 
 
 if (reader_->GetTH2("Chamber2")==NULL)
		reader_->BookTH2("Chamber2",96, 0., 96.,96., 0., 96.);
		
if (reader_->GetTH1("Chamber2_X")==NULL)
		reader_->BookTH1("Chamber2_X",96, 0., 96.);

 if (reader_->GetTH1("Chamber2_Y")==NULL)
		reader_->BookTH1("Chamber2_Y",96., 0., 96.);

		
 if (reader_->GetTH2("Chamber3")==NULL)
		reader_->BookTH2("Chamber3",96., 0., 96.,96., 0., 96.);
		
 if (reader_->GetTH1("Chamber3_X")==NULL)
		reader_->BookTH1("Chamber3_X",96., 0., 96.);

 if (reader_->GetTH1("Chamber3_Y")==NULL)
		reader_->BookTH1("Chamber3_Y",96., 0., 96.);	
			
 
 if (reader_->GetTH2("Chamber4")==NULL)
		reader_->BookTH2("Chamber4",96., 0., 96.,96., 0., 96.);

 if (reader_->GetTH1("Chamber4_X")==NULL)
		reader_->BookTH1("Chamber4_X",96., 0., 96.);

if (reader_->GetTH1("Chamber4_Y")==NULL) 
		reader_->BookTH1("Chamber4_Y",96., 0., 96.);


//Precise alignment by fitting	
if (reader_->GetTH1("Chamber1_X_Offset")==NULL)
		reader_->BookTH1("Chamber1_X_Offset",100., 0., 10.);
	
if (reader_->GetTH1("Chamber1_Y_Offset")==NULL) 
		reader_->BookTH1("Chamber1_Y_Offset",100., 0., 10.);
	
if (reader_->GetTH1("Chamber2_X_Offset")==NULL)
		reader_->BookTH1("Chamber2_X_Offset",100., 0., 10.);
	
if (reader_->GetTH1("Chamber2_Y_Offset")==NULL) 
		reader_->BookTH1("Chamber2_Y_Offset",100., 0., 10.);
	
if (reader_->GetTH1("Chamber3_X_Offset")==NULL)
		reader_->BookTH1("Chamber3_X_Offset",100., 0., 10.);
	
if (reader_->GetTH1("Chamber3_Y_Offset")==NULL) 
		reader_->BookTH1("Chamber3_Y_Offset",100., 0., 10.);
	
if (reader_->GetTH1("Chamber4_X_Offset")==NULL)
		reader_->BookTH1("Chamber4_X_Offset",100., 0., 10.);
	
if (reader_->GetTH1("Chamber4_Y_Offset")==NULL) 
		reader_->BookTH1("Chamber4_Y_Offset",100., 0., 10.);
	 
}

float RKAnalyzerTB_Oct::ZLayer(int DIFid){
	//Chamber1
	if(DIFid==	22)	return Chamber1_Z;
	
	//Chamber2
	if(DIFid==	26)	return Chamber2_Z;
	
	//Chamber3
	if(DIFid==	2)	return Chamber3_Z;
	if(DIFid==	13)	return Chamber3_Z;
	if(DIFid==	24)	return Chamber3_Z;
	
	//Chamber4
	if(DIFid==	5)	return Chamber4_Z;
	if(DIFid==	21)	return Chamber4_Z;
	if(DIFid==	23)	return Chamber4_Z;
	
	//Unknown DIF
	return -1;
}

float RKAnalyzerTB_Oct::X_M2_Shift(int x, int DIFid){
	//Dependant of DIF id on M2 ==Settings
	//Chamber 3
	if(DIFid==2)	return x;
	if(DIFid==13)	return x+=32;		
	if(DIFid==24)	return x+=64;
	
	//Chamber 3
	if(DIFid==23)	return x;
	if(DIFid==21)	return x+=32;		
	if(DIFid==5)	return x+=64;		
	return x;
}			

void RKAnalyzerTB_Oct::RKPadConvert(int asicid,int ipad, int &i, int &j, int asicType)
{
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Upper view (ASICs SIDE)
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
//       0  1  2  3  4  5  6  7    (I Axis)  ----->
//				|	 |
//				|DIFF|
//				|____|	

unsigned short AsicShiftI[49]={	0,	0,	0,	0,	0,	8,	8,	8,	8,	16,	16,	16,	16,	24,	24,	24,	24,	32,	32,	32,	32,	40,	40,	40,	40,	48,	48,	48,	48,	56,	56,	56,	56,	64,	64,	64,	64,	72,	72,	72,	72,	80,	80,	80,	80,	88,	88,	88,	88};
unsigned short AsicShiftJ[49]={	0,	0,	8,	16,	24,	24,	16,	8,	0,	0,	8,	16,	24,	24,	16,	8,	0,	0,	8,	16,	24,	24,	16,	8,	0,	0,	8,	16,	24,	24,	16,	8,	0,	0,	8,	16,	24,	24,	16,	8,	0,	0,	8,	16,	24,	24,	16,	8,	0};

i = MapILargeHR2[ipad]+AsicShiftI[asicid]+1;
j = MapJLargeHR2[ipad]+AsicShiftJ[asicid]+1;

 
}

void RKAnalyzerTB_Oct::Clusteriser(Float_t* HitX, Float_t* HitY, Float_t* HitZ, Int_t nHit, Float_t* ClustX, Float_t* ClustY, Float_t* ClustZ, Float_t* nHitInClust, Int_t &nClust)
{
	int CntClust=0;
	bool Use[nHit];
	for(int iInit=0; iInit<nHit; iInit++) {Use[iInit]=true;}
			
	for (int iCa=0; iCa<nHit; iCa++){
		if(Use[iCa]){
			Float_t CntHit=1;
			Float_t TotX=HitX[iCa];
			Float_t TotY=HitY[iCa];
		
					
			for (int iCb=(iCa+1); iCb<nHit; iCb++){ 
				if((sqrt((HitX[iCa]-HitX[iCb])*(HitX[iCa]-HitX[iCb]))<=ClusterCut)&&(sqrt((HitY[iCa]-HitY[iCb])*(HitY[iCa]-HitY[iCb]))<=ClusterCut)&&(HitZ[iCa]==HitZ[iCb])&&Use[iCb]){
					TotX+= HitX[iCb];
					TotY+= HitY[iCb];
					CntHit++;
					Use[iCb]=false;
				}
			}
			ClustX[CntClust]=TotX/CntHit;
			ClustY[CntClust]=TotY/CntHit;
			ClustZ[CntClust]=HitZ[iCa];
			nHitInClust[CntClust]=CntHit;
			Use[iCa]=false;	
			CntClust++;
		}
	}
	nClust=CntClust;		
}
	
/*
void RKAnalyzerTB_Oct::ComputeOnlineHitMultiplicity(LCCollection* rhcol)
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
	  
		if(thr[1]){PresenceHit=true;}
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
}*/

void RKAnalyzerTB_Oct::BeamProfiles(LCCollection* rhcol)
{
	cout<<"Event pass"<<endl; 
	
for (int i=0;i<rhcol->getNumberOfElements();i++){
      
    
	TH2* Ch1= reader_->GetTH2("Chamber1");		
	if (Ch1==NULL)
		Ch1 = reader_->BookTH2("Chamber1",96., 0., 96.,96., 0., 96.);
      
        TH1* Ch1_X = reader_->GetTH1("Chamber1_X");		
	if (Ch1_X==NULL)
		Ch1_X = reader_->BookTH1("Chamber1_X",96., 0., 96.);
      
	TH1* Ch1_Y = reader_->GetTH1("Chamber1_Y");		
	if (Ch1_Y==NULL)
		Ch1_Y = reader_->BookTH1("Chamber1_Y",96., 0., 96.);     
      
      	TH2* Ch2= reader_->GetTH2("Chamber2");		
	if (Ch2==NULL)
		Ch2 = reader_->BookTH2("Chamber2",96., 0., 96.,96., 0., 96.);
      
        TH1* Ch2_X = reader_->GetTH1("Chamber2_X");		
	if (Ch2_X==NULL)
		Ch2_X = reader_->BookTH1("Chamber2_X",96., 0., 96.);
      
	TH1* Ch2_Y = reader_->GetTH1("Chamber2_Y");		
	if (Ch2_Y==NULL)
		Ch2_Y = reader_->BookTH1("Chamber2_Y",96., 0., 96.);    
		
	TH2* Ch3= reader_->GetTH2("Chamber3");		
	if (Ch3==NULL)
		Ch3 = reader_->BookTH2("Chamber3",96., 0., 96.,96., 0., 96.);
      
        TH1* Ch3_X = reader_->GetTH1("Chamber3_X");		
	if (Ch3_X==NULL)
		Ch3_X = reader_->BookTH1("Chamber3_X",96., 0., 96.);
      
	TH1* Ch3_Y = reader_->GetTH1("Chamber3_Y");		
	if (Ch3_Y==NULL)
		Ch3_Y = reader_->BookTH1("Chamber3_Y",96., 0., 96.);    
		
	TH2* Ch4= reader_->GetTH2("Chamber4");		
	if (Ch4==NULL)
		Ch4 = reader_->BookTH2("Chamber4",96., 0., 96.,96., 0., 96.);
      
        TH1* Ch4_X = reader_->GetTH1("Chamber4_X");		
	if (Ch4_X==NULL)
		Ch4_X = reader_->BookTH1("Chamber4_X",96., 0., 96.);
      
	TH1* Ch4_Y = reader_->GetTH1("Chamber4_Y");		
	if (Ch4_Y==NULL)
		Ch4_Y = reader_->BookTH1("Chamber4_Y",96., 0., 96.);    		
            
      
      
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

      if (bc<10){	
	RKPadConvert(asicid,channel,x,y,2);
		  y=X_M2_Shift(y,difid);

	if(difid==22){
		Ch1->Fill(x,y);
		Ch1_X->Fill(x);
		Ch1_Y->Fill(y);	
	}	
	if(difid==26){
		Ch2->Fill(x,(32-y));
		Ch2_X->Fill(x);
		Ch2_Y->Fill(32-y);	
	}
	if((difid==23)|| (difid==21)||(difid==5)){
		Ch3->Fill(x,y);
		Ch3_X->Fill(x);
		Ch3_Y->Fill(y);	
	}
	if((difid==2)|| (difid==13)||(difid==24)){
		Ch4->Fill(x,y);
		Ch4_X->Fill(x);
		Ch4_Y->Fill(y);	
	}

      } 
	 
  }//eof for i
}


void RKAnalyzerTB_Oct::SumBeamProfiles(LCCollection* rhcol)
{	
	for (int i=0;i<rhcol->getNumberOfElements();i++){
		
		
		TH2* Ch= reader_->GetTH2("ChamberSum");		
		if (Ch==NULL)
			Ch = reader_->BookTH2("ChamberSum",96., 0., 96.,96., 0., 96.);
		
        TH1* Ch_X = reader_->GetTH1("ChamberSum_X");		
		if (Ch_X==NULL)
			Ch_X = reader_->BookTH1("ChamberSum_X",96., 0., 96.);
		
		TH1* Ch_Y = reader_->GetTH1("ChamberSum_Y");		
		if (Ch_Y==NULL)
			Ch_Y = reader_->BookTH1("ChamberSum_Y",96., 0., 96.);     
	
		
		
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
		
		if (bc<10){	
			
			RKPadConvert(asicid,channel,x,y,2);
			y=X_M2_Shift(y,difid);
		
			
			if(difid==22){
				Ch->Fill(x,y);
				Ch_X->Fill(x);
				Ch_Y->Fill(y);	
			}	
			if(difid==26){
				Ch->Fill((x - Ch2_X_Offset) , (32 - y - Ch2_Y_Offset));
				Ch_X->Fill(x - Ch2_X_Offset);
				Ch_Y->Fill(32 - y - Ch2_Y_Offset);	
			}
			if((difid==23)|| (difid==21)||(difid==5)){
				Ch->Fill((x - Ch3_X_Offset) , (y - Ch3_Y_Offset));
				Ch_X->Fill(x - Ch3_X_Offset);
				Ch_Y->Fill(y - Ch3_Y_Offset);	
			}
			if((difid==2)|| (difid==13)||(difid==24)){
				Ch->Fill((x - Ch4_X_Offset) , (y - Ch4_Y_Offset));
				Ch_X->Fill(x - Ch4_X_Offset);
				Ch_Y->Fill(y - Ch4_Y_Offset);	
			}
			
		} 
		
	}//eof for i
}


void RKAnalyzerTB_Oct::FirstAlign(){

	TH1* Ch1_X = reader_->GetTH1("Chamber1_X");		
	TH1* Ch1_Y = reader_->GetTH1("Chamber1_Y");		
	
	TH1* Ch2_X = reader_->GetTH1("Chamber2_X");		
	TH1* Ch2_Y = reader_->GetTH1("Chamber2_Y");		  
	
	TH1* Ch3_X = reader_->GetTH1("Chamber3_X");		
	TH1* Ch3_Y = reader_->GetTH1("Chamber3_Y");		

	TH1* Ch4_X = reader_->GetTH1("Chamber4_X");	
	TH1* Ch4_Y = reader_->GetTH1("Chamber4_Y");		

	
  //Reference is chamber1
	Ch2_X_Offset = Ch2_X->GetMean() - Ch1_X->GetMean();
	Ch2_Y_Offset = Ch2_Y->GetMean() - Ch1_Y->GetMean();
	cout<<"Chamber 2 offset : ("<<Ch2_X_Offset<<" , "<<Ch2_Y_Offset<<")"<<endl;
	
	Ch3_X_Offset = Ch3_X->GetMean() - Ch1_X->GetMean();
	Ch3_Y_Offset = Ch3_Y->GetMean() - Ch1_Y->GetMean();
	cout<<"Chamber 3 offset : ("<<Ch3_X_Offset<<" , "<<Ch3_Y_Offset<<")"<<endl;
	
	
	Ch4_X_Offset = Ch4_X->GetMean() - Ch1_X->GetMean();
	Ch4_Y_Offset = Ch4_Y->GetMean() - Ch1_Y->GetMean();
	cout<<"Chamber 4 offset : ("<<Ch4_X_Offset<<" , "<<Ch4_Y_Offset<<")"<<endl;


	
}

void RKAnalyzerTB_Oct::SecondAlign(LCCollection* rhcol){
	
	TH1* Ch1_X_Offset_h = reader_->GetTH1("Chamber1_X_Offset");		
	TH1* Ch1_Y_Offset_h  = reader_->GetTH1("Chamber1_Y_Offset");		
	
	TH1* Ch2_X_Offset_h  = reader_->GetTH1("Chamber2_X_Offset");		
	TH1* Ch2_Y_Offset_h  = reader_->GetTH1("Chamber2_Y_Offset");		  
	
	TH1* Ch3_X_Offset_h  = reader_->GetTH1("Chamber3_X_Offset");		
	TH1* Ch3_Y_Offset_h  = reader_->GetTH1("Chamber3_Y_Offset");		
	
	TH1* Ch4_X_Offset_h  = reader_->GetTH1("Chamber4_X_Offset");	
	TH1* Ch4_Y_Offset_h  = reader_->GetTH1("Chamber4_Y_Offset");		
	
	Float_t HitX[1000];
	Float_t HitY[1000];
	Float_t HitZ[1000];
	Float_t ClustX[1000];
	Float_t ClustY[1000];
	Float_t ClustZ[1000];
	Float_t nHitInClust[1000];
	Int_t nClust=0;
	
	for (int i=0;i<rhcol->getNumberOfElements();i++){
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
		
		//Fill the HitXYZ
		if (bc<10){	
			RKPadConvert(asicid,channel,x,y,2);
			y=X_M2_Shift(y,difid);
			
			if(difid==22){
				HitX[i]=x;
				HitY[i]=y;
				HitZ[i]=ZLayer(difid);
			}	
			if(difid==26){
				HitX[i]=(x - Ch2_X_Offset);
				HitY[i]=(32 - y - Ch2_Y_Offset);
				HitZ[i]=ZLayer(difid);
			}
			if((difid==23)|| (difid==21)||(difid==5)){
				HitX[i]=(x - Ch3_X_Offset);
				HitY[i]=(y - Ch3_Y_Offset);
				HitZ[i]=ZLayer(difid);
							}
			if((difid==2)|| (difid==13)||(difid==24)){
				HitX[i]=(x - Ch4_X_Offset);
				HitY[i]=(y - Ch4_Y_Offset);
				HitZ[i]=ZLayer(difid);
			}
		}//endof if bc<10	
	}	
	
	//Clustering Chamber by chamber
	Clusteriser(HitX, HitY, HitZ,(Int_t) rhcol->getNumberOfElements(), ClustX, ClustY, ClustZ, nHitInClust, nClust);
	
	//Fit tracks
	TGraph* Xclust_Proj = new TGraph(nClust, (Float_t*)ClustZ, (Float_t*)ClustX); 
	TGraph* Yclust_Proj = new TGraph(nClust, (Float_t*)ClustZ, (Float_t*)ClustY);
	TF1 *FonctionAxclust = new TF1 ("FonctionAxclust","([0]*x+[1])",0.,118.);//Range 0->118 cm
	TF1 *FonctionAyclust = new TF1 ("FonctionAyclust","([0]*x+[1])",0.,118.);//Range 0->118 cm
	FonctionAxclust->SetParameter(0,0);//Idéal horizontal track
	FonctionAyclust->SetParameter(0,0);
	Xclust_Proj->Fit("FonctionAxclust");
	Yclust_Proj->Fit("FonctionAyclust");
	
	//Use fit results to align precisely
	if((FonctionAxclust->GetProb()>=0.1)&&(FonctionAyclust->GetProb()>=0.1)) //To adjust?
	{
		for(int i=0; i<nClust; i++){
			//eq type: x=az+b
			float Hit_theo_X = (FonctionAxclust->GetParameter(0))*ClustZ[i] + FonctionAxclust->GetParameter(1);	
			float Hit_theo_Y = (FonctionAyclust->GetParameter(0))*ClustZ[i] + FonctionAyclust->GetParameter(1);
		    if(Chamber1_Z == ClustZ[i]){
				Ch1_X_Offset_h ->Fill(ClustX[i]-Hit_theo_X);
				Ch1_Y_Offset_h ->Fill(ClustY[i]-Hit_theo_Y);
			}
			if(Chamber2_Z == ClustZ[i]){
				Ch2_X_Offset_h ->Fill(ClustX[i]-Hit_theo_X);
				Ch2_Y_Offset_h ->Fill(ClustY[i]-Hit_theo_Y);
			}
			if(Chamber3_Z == ClustZ[i]){
				Ch3_X_Offset_h ->Fill(ClustX[i]-Hit_theo_X);
				Ch3_Y_Offset_h ->Fill(ClustY[i]-Hit_theo_Y);
			}
			if(Chamber4_Z == ClustZ[i]){
				Ch4_X_Offset_h ->Fill(ClustX[i]-Hit_theo_X);
				Ch4_Y_Offset_h ->Fill(ClustY[i]-Hit_theo_Y);
			}
		}	
		
	}
	
	Ch1_X_Offset_fine = Ch1_X_Offset_h->GetMean();
	Ch1_Y_Offset_fine = Ch1_Y_Offset_h->GetMean();
	
	Ch2_X_Offset_fine = Ch2_X_Offset_h->GetMean();
	Ch2_Y_Offset_fine = Ch2_Y_Offset_h->GetMean();
	
	Ch3_X_Offset_fine = Ch3_X_Offset_h->GetMean();
	Ch3_Y_Offset_fine = Ch3_Y_Offset_h->GetMean();
	
	Ch4_X_Offset_fine = Ch4_X_Offset_h->GetMean();
	Ch4_Y_Offset_fine = Ch4_Y_Offset_h->GetMean();
}


/*
void RKAnalyzerTB_Oct::ComputeOnlineClusterMultiplicity(LCCollection* rhcol) //Valid in M2 only
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
     
      if ((thr[0])&&(difid<1000)) {
		int asic=asicid;
		RKPadConvert(asic,channel,x,y, asictype); 
	
	    //Dependant of asic id
		if(difid==6){;}
		else{
		 if(difid==9) x+=32;		
		 else{ 
			if(difid==10) x+=64;
			//else assert(0);//This processor has to be modified to handle other diffs
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
*/


/*
void RKAnalyzerTB_Oct::TrackingTool(LCCollection* rhcol)
{
	int nHit=0;
	// Loop on all frames
	for (int i=0;i<rhcol->getNumberOfElements();i++)
    {
      IMPL::RawCalorimeterHitImpl* hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(i);
      if (hit==0) continue;
	  else nHit++;
	}

	std::map<int,FloatVec> tHit_X;		//key: DIFid	val: float vect of X hits
	std::map<int,FloatVec> tHit_Y;		//key: DIFid	val: float vect of Y hits
	std::map<int,FloatVec> tCluster_X;  //key: DIFid	val: float vect of X clusters
	std::map<int,FloatVec> tCluster_Y;  //key: DIFid	val: float vect of Y clusters


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
		//Geom stuf
		int asictype =1;	  //Attention ici cela peut poser probleme ------------------------------------
		int nbinx = 96;
		int nbiny= 32;
		if (difid>1000) {nbinx=8; nbiny=32;}
     
		if (thr[0]) {
			int asic=asicid;
			if (difid>1000) asic=(asic-1)%4+1;
			RKPadConvert(asic,channel,x,y, asictype); 
	
			//Dependant of DIF id on M2 do nothing on other HITs
			x = X_M2_Shift(x, difid);
		
			FloatVec tmp_fVec;
			
			tmp_fVec.clear();
			//Recup float vector précedent si il existe, sinon on en créé un et on ajoute le hit dedans et on le renvoie sur la map du detecteur
			if(tHit_X.find(difid)!=lastParams.end()){ //Test d'exitence
				tmp_fVec = (FloatVec)tHit_X[(int)difid];
			}
			tmp_fVec.pushback(x);
			std::pair<int,FloatVec> pr1(diffid, tmp_fVec);
			tHit_X.erase(diffid); //Not necessary if insert erase before inserting (to check)
			tHit_X.insert(pr1);		
					
			tmp_fVec.clear();
			//Recup float vector précedent si il existe, sinon on en créé un et on ajoute le hit dedans et on le renvoie sur la map du detecteur
			if(tHit_Y.find(difid)!=lastParams.end()){ //Test d'exitence
				tmp_fVec = (FloatVec)tHit_Y[(int)difid];
			}
			tmp_fVec.pushback(x);
			std::pair<int,FloatVec> pr2(diffid, tmp_fVec);
			tHit_Y.erase(diffid); //Not necessary if insert erase before inserting (to check)
			tHit_Y.insert(pr2);			
      }
      }//Time cut selection
	}
	
	
	float tClustX[nHit];
	float tClustY[nHit];
	float tnHitInClust[nHit];
	int nClusters=0;
	
	//Make detectors sub-lists to send it to clusteriser

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

*/

void RKAnalyzerTB_Oct::processRunHeader()
{;}

void RKAnalyzerTB_Oct::processEvent()
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
  
  //Dump event	
  //UTIL::LCTOOLS::dumpEventDetailed(reader_->getEvent());	
  
  if(status==1)
 	 BeamProfiles(rhcol);	

  if(status==2)
     SumBeamProfiles(rhcol);
	 SecondAlign(rhcol);
	 

}



