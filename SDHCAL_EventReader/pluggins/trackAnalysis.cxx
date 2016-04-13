#define NPLANS_USED 48

#include "trackAnalysis.hh"
#include <TLine.h>
#include <TGraphErrors.h>
#include <TFitResult.h>
#include <TFitter.h>
#include <TF1.h>
#include <TPluginManager.h>
#include <stdint.h>
#include <math.h>
#include "TPolyLine3D.h"
#include "TVirtualPad.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include "recoTrack.hh"

#define STEP printf("%s %d\n",__FUNCTION__,__LINE__)



uint32_t trackAnalysis::PMAnalysis(uint32_t bifid)
{

 
  for (int itag=1;itag<=3;itag++)
    {
      std::stringstream ss;
      ss<<"/PMAnalysis/Tag"<<itag<<"/";

      TH1* hpattag= rootHandler_->GetTH1(ss.str()+"PatternTagNoSeed");

  if (hpattag==NULL)
    {
      hpattag =rootHandler_->BookTH1( ss.str()+"PatternTagNoSeed",100,0.,100.);


    }


  for (std::vector<DIFPtr*>::iterator itb = reader_->getDIFList().begin();itb!=reader_->getDIFList().end();itb++)
    {
      DIFPtr* d = (*itb);
      if(_geo->difGeo(d->getID())["type"].asString().compare("HR2BIF")!=0) continue;

      // Loop on frames
      
      for (uint32_t i=0;i<d->getNumberOfFrames();i++)
  	{
  	  //if (abs(d->getFrameTimeToTrigger(i)-seed)<200)
	  //printf(" Frame %d  Cerenkov time %d  \n",i,d->getFrameTimeToTrigger(i));
	  float ti=d->getFrameTimeToTrigger(i)*1.;
	  uint32_t tag=0;
	  for (uint32_t j=0;j<64;j++)
	    {
	      if (d->getFrameLevel(i,j,0)) tag +=1;
	      if (d->getFrameLevel(i,j,1)) tag +=2;
	    }
	  if (tag!=itag) continue;
	  float chb[100];
	  memset(chb,0,100*sizeof(float));
	  for (std::vector<DIFPtr*>::iterator it = reader_->getDIFList().begin();it!=reader_->getDIFList().end();it++)
	    {
	      DIFPtr* dc = (*it);

	      uint32_t chid = _geo->difGeo(dc->getID())["chamber"].asUInt();
	      std::stringstream s;
	      s<<ss.str()<<"BIFPOS"<<chid;
	      TH1* hpattag1= rootHandler_->GetTH1(s.str());
	      if (hpattag1==NULL)
		hpattag1 =rootHandler_->BookTH1(s.str(),801,-400.,400.);

	      s.str(std::string());
	      s<<ss.str()<<"HITPOS"<<chid;

	      TH2* hpatpos= rootHandler_->GetTH2(s.str());
	      if (hpatpos==NULL)
		hpatpos =rootHandler_->BookTH2(s.str(),32,0.1,32.1,48,0.1,48.1);
	      for (uint32_t j=0;j<dc->getNumberOfFrames();j++)
		{
		  
		  float tj=dc->getFrameTimeToTrigger(j)*1.;
		  if (chid==0)
		    {
		      uint32_t tag=0;
		      for (uint32_t k=0;k<64;k++)
			{
			  if (dc->getFrameLevel(j,k,0)) tag +=1;
			  if (dc->getFrameLevel(j,k,1)) tag +=2;
			}
		      if (tag!=itag) continue;
		    }

		    

		  hpattag1->Fill(ti-tj); 
		  //if (chid==10)
		  // printf(" Slot10 Frame %d time %d %f %f %f\n",j,dc->getFrameTimeToTrigger(j),ti,tj,ti-tj);
		  if ((ti-tj)>=-10. && (ti-tj)<0)
		    {
		      //printf("in %d-> %f %f \n",chid,ti,tj);
		      chb[chid]+=1.;
		      int iasic=dc->getFrameAsicHeader(j);
		      for (uint32_t k=0;k<64;k++)
			{
			  if (dc->getFrameLevel(j,k,0) || dc->getFrameLevel(j,k,1))
			    {
			      int ip,jp;
			      RecoHit h;
			      _geo->convert(dc->getID(),iasic,k,&h);
			      hpatpos->Fill(h.I()*1.,h.J()*1.);
			       
			    }
			}

		    }

		   if ((ti-tj)>=0 && (ti-tj)<=10.)
		    {
		      //printf("out %d-> %f %f \n",chid,ti,tj);
		      chb[50+chid]+=1.;}
		}

	    }
	  
	  for(int i=0;i<64;i++)
	    {
	      if (chb[i]) hpattag->Fill(i*1.+0.1,chb[i]);
	    }
	  hpattag->Fill(99.1);

	  //if (chb[60]==0) getchar();


	}
    
      


    }
    }
 
  //  getchar();
  return 0;
}


uint32_t trackAnalysis::CerenkovTagger(uint32_t difid,uint32_t seed)
{
  uint32_t tag=0;
  //printf(" The Seed %d \n",seed);
  float fs=seed*1.;
  for (std::vector<DIFPtr*>::iterator it = reader_->getDIFList().begin();it!=reader_->getDIFList().end();it++)
    {
      DIFPtr* d = (*it);
      if (d->getID()!=difid) continue;
      // Loop on frames
      
      for (uint32_t i=0;i<d->getNumberOfFrames();i++)
  	{
  	  //if (abs(d->getFrameTimeToTrigger(i)-seed)<200)
	  //printf("\t Cerenkov %d  %f\n",d->getFrameTimeToTrigger(i),fabs(seed-d->getFrameTimeToTrigger(i)));
	  float tf=d->getFrameTimeToTrigger(i);
  	  if (fabs(fs-tf)<10)
  	    {

	      
	      for (uint32_t j=0;j<64;j++)
		{
		  if (d->getFrameLevel(i,j,0)) tag +=1;
		  if (d->getFrameLevel(i,j,1)) tag +=2;
		}
	      return tag;

  	    }
  	}

	    
    }
  //  getchar();
  return 0;
}

void trackAnalysis::initHistograms()
{
  //  rootHandler_->BookTH1("/Clusters/EST1",100,0.,300.);
}

trackAnalysis::trackAnalysis() :trackIndex_(0),nAnalyzed_(0),clockSynchCut_(8), spillSize_(90000),maxHitCount_(500000),minHitCount_(20),
									     tkMinPoint_(3),tkExtMinPoint_(3),tkBigClusterSize_(32),tkChi2Cut_(0.01),tkDistCut_(5.),tkExtChi2Cut_(0.01),tkExtDistCut_(10.),tkAngularCut_(20.),zLastAmas_(134.),
									     findTracks_(true),dropFirstSpillEvent_(false),useSynchronised_(true),chamberEdge_(5.),rebuild_(false),oldAlgo_(true),collectionName_("DHCALRawHits"),
									     tkFirstChamber_(1),tkLastChamber_(61),useTk4_(false),offTimePrescale_(1),houghIndex_(0),theRhcolTime_(0.),theTimeSortTime_(0.),theTrackingTime_(0),
				theHistoTime_(0),theSeuil_(0),draw_(false),theSkip_(0),_monitor(NULL),theMonitoringPeriod_(0),theMonitoringPath_("/dev/shm/Monitoring"),ntkbetween(0),theBCIDSpill_(0),theLastBCID_(0),theSpillLength_(8.),_geo(NULL)
{
  
  reader_=DHCalEventReader::instance();
  
  rootHandler_ =DCHistogramHandler::instance();
  
  this->initialise();
  
}


void trackAnalysis::initialise()
{
  headerWritten_=false;
  //  TVirtualFitter::SetDefaultFitter("Minuit"); 
  //  gPluginMgr->AddHandler("ROOT::Math::Minimizer", "Minuit", "TMinuitMinimizer", "Minuit", "TMinuitMinimizer(const char *)"); 
  integratedTime_=0;
 
 
  




}
void trackAnalysis::initJob()
{
  presetParameters();
  if (_geo!=NULL)
    _monitor=new hitMonitor(_geo);
}
void trackAnalysis::endJob(){
  if (theMonitoringPeriod_!=0)
    {
      rootHandler_->writeXML(theMonitoringPath_);
      //char c;c=getchar();putchar(c); if (c=='.') exit(0);;
		
    }


	
	
}
void trackAnalysis::presetParameters()
{
  std::map<std::string,MarlinParameter> m=reader_->getMarlinParameterMap();
  std::map<std::string,MarlinParameter>::iterator it;
  try
    {
        if ((it=m.find("geometry"))!=m.end())
	 _geo=new jsonGeo(it->second.getStringValue());

  
      if ((it=m.find("ClockSynchCut"))!=m.end()) clockSynchCut_=it->second.getIntValue();
      if ((it=m.find("SpillSize"))!=m.end()) spillSize_=it->second.getDoubleValue();
      if ((it=m.find("MaxHitCount"))!=m.end()) maxHitCount_=it->second.getIntValue();
      if ((it=m.find("MinHitCount"))!=m.end()) minHitCount_=it->second.getIntValue();
      if ((it=m.find("MinChambersInTime"))!=m.end()) minChambersInTime_=it->second.getIntValue();
      if ((it=m.find("TkMinPoint"))!=m.end()) tkMinPoint_=it->second.getIntValue();
      if ((it=m.find("TkExtMinPoint"))!=m.end()) tkExtMinPoint_=it->second.getIntValue();
      if ((it=m.find("TkBigClusterSize"))!=m.end()) tkBigClusterSize_=it->second.getIntValue();
      if ((it=m.find("TkChi2Cut"))!=m.end()) tkChi2Cut_=it->second.getDoubleValue();
      if ((it=m.find("TkDistCut"))!=m.end()) tkDistCut_=it->second.getDoubleValue();
      if ((it=m.find("TkExtChi2Cut"))!=m.end()) tkExtChi2Cut_=it->second.getDoubleValue();
      if ((it=m.find("TkExtDistCut"))!=m.end()) tkExtDistCut_=it->second.getDoubleValue();
      if ((it=m.find("TkAngularCut"))!=m.end()) tkAngularCut_=it->second.getDoubleValue();
      if ((it=m.find("ChamberEdge"))!=m.end()) chamberEdge_=it->second.getDoubleValue();
      if ((it=m.find("FindTracks"))!=m.end()) findTracks_=it->second.getBoolValue();
      if ((it=m.find("DropFirstSpillEvent"))!=m.end()) dropFirstSpillEvent_=it->second.getBoolValue();
      if ((it=m.find("UseSynchronised"))!=m.end()) useSynchronised_=it->second.getBoolValue();
      if ((it=m.find("UseTk4"))!=m.end()) useTk4_=it->second.getBoolValue();
      if ((it=m.find("Rebuild"))!=m.end()) rebuild_=it->second.getBoolValue();
      if ((it=m.find("OldAlgo"))!=m.end()) oldAlgo_=it->second.getBoolValue();
      if ((it=m.find("CollectionName"))!=m.end()) collectionName_=it->second.getStringValue();
      if ((it=m.find("TkFirstChamber"))!=m.end()) tkFirstChamber_=it->second.getIntValue();
      if ((it=m.find("TkLastChamber"))!=m.end()) tkLastChamber_=it->second.getIntValue();
      if ((it=m.find("OffTimePrescale"))!=m.end()) offTimePrescale_=it->second.getIntValue();
      if ((it=m.find("Seuil"))!=m.end()) theSeuil_=it->second.getIntValue();
      if ((it=m.find("Interactif"))!=m.end()) draw_=it->second.getBoolValue();
      if ((it=m.find("SkipEvents"))!=m.end()) theSkip_=it->second.getIntValue();
      if ((it=m.find("zLastAmas"))!=m.end()) zLastAmas_=it->second.getDoubleValue();
      if ((it=m.find("MonitoringPath"))!=m.end()) theMonitoringPath_=it->second.getStringValue();
      if ((it=m.find("MonitoringPeriod"))!=m.end()) theMonitoringPeriod_=it->second.getIntValue();
      if ((it=m.find("SpillLength"))!=m.end())
	{
	  theSpillLength_=it->second.getDoubleValue();
	  //printf("I found %f spill length \n",theSpillLength_);
	  // getchar();
	}

      DEBUG_PRINT("Interactif %d \n",draw_);

      //getchar();

    }
  catch (std::string s)
    {
      std::cout<<__PRETTY_FUNCTION__<<" error "<<s<<std::endl;
      //exit(-1);
    }
  //getchar();	
}
bool trackAnalysis::decodeTrigger(LCCollection* rhcol, double tcut)
{
  // if (rhcol->getNumberOfElements()==0) return true;

  // Find Trigger information
  IntVec vTrigger;IMPL::RawCalorimeterHitImpl* hit;
  unsigned int difid=0;
  // Find the first read DIF id for this trigger

  if (rhcol->getNumberOfElements()!=0)
    {
		
      try {
	hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(0);
      }
      catch (std::exception e)
	{
	  std::cout<<"No hits "<<std::endl;
	  return false;
	}
      if (hit!=0) 
	difid = hit->getCellID0()&0xFF;
    }

  if (difid==0) return false;

  //Find the parameters
  std::stringstream pname("");
  pname <<"DIF"<<difid<<"_Triggers";

  rhcol->getParameters().getIntVals(pname.str(),vTrigger);

  if (vTrigger.size()==0) return false; 
  //for (int i=0;i<vTrigger.size();i++)
  ///  std::cout<<vTrigger[i]<<std::endl;

  // Decode Large Bunch Crossing
  unsigned long long Shift=16777216ULL;//to shift the value from the 24 first bits

  unsigned long long  lbc=0;
  unsigned long long  lbci=0;
  uint32_t  lb5=vTrigger[4] ;
  uint32_t  lb4=vTrigger[3] ;

  lbc = lb5*Shift+ lb4;

  theDTC_=vTrigger[0];
  theGTC_=vTrigger[1];
  theBCID_=lbc;

  //lbc =lb4*Shift+lb5;
  double tTrigger_= lbc*(2E-7);
  DEBUG_PRINT("Time stamp ==========================>: %d %d %llu %f\n",lb4,lb5,lbc,tTrigger_);
  // Fill Trigger info
  // DEBUG_PRINT("creqtion de htspill \n");
  TH1* htspill= rootHandler_->GetTH1("SpillDif");
  if (htspill==NULL)
    {
      htspill =rootHandler_->BookTH1( "SpillDif",500,0.,100.);
    }
  //DEBUG_PRINT("apres creqtion de htspill \n");

  // Calculate tiem differences since the last trigger
  double tdif = tTrigger_-externalTriggerTime_;
  //std::cout<<lbc<<" "<<externalTriggerTime_<<" "<<tdif<<std::endl;
#ifdef DEBUG
  if (tdif>50 || tdif <-1E-3)
    {
      cout<<tdif << " strange time  "<<externalTriggerTime_<<endl;
      //streamlog_out(DEBUG)<<lb4<<endl;
      //streamlog_out(DEBUG)<<lb5<<endl;
      //streamlog_out(DEBUG)<<lbc<<endl;
		
    }
  //streamlog_out(DEBUG)<<lbc <<" "<<tdif<<" # hits "<<rhcol->getNumberOfElements()<<std::endl;
#endif
  isNewSpill_=(tdif>tcut);
  if (tdif>tcut) 
    {
      lastSpill_=tTrigger_;
      std::cout<<"New Spill "<<tdif<<"===========================================================>"<<npi_<<std::endl; 
      npi_=0;
      htspill->Fill(tdif);
    }
  externalTriggerTime_=tTrigger_;
  //  for (unsigned int i=0;i<vTrigger.size();i++) streamlog_out(MESSAGE)<<i<<" "<<vTrigger[i]<<std::endl;




  // Drop the first event of the Spill
  //  streamlog_out(MESSAGE)<<dropFirstSpillEvent_<<std::endl;
  if (tdif>tcut && dropFirstSpillEvent_) return false;
  if ((tTrigger_-lastSpill_)<1. && dropFirstSpillEvent_) 
    {
      DEBUG_PRINT("Event dropped %f %f \n",tTrigger_,lastSpill_);
      return false;
    }
  // TH1* htdiff= rootHandler_->GetTH1("TimeDif");
  // if (htdiff==NULL)
  //     {
  //         htdiff =rootHandler_->BookTH1( "TimeDif",20000,0.,20000.);
  //     }
  //    htdiff->Fill(tdif*1000.);




  return true;
}

void trackAnalysis::processSeed(IMPL::LCCollectionVec* rhcol,uint32_t seed)
{

 
  

  currentTime_=seed;
  
  theAbsoluteTime_=theBCID_-currentTime_;
  if (theBCIDSpill_==0) theBCIDSpill_=theAbsoluteTime_;
  if (theAbsoluteTime_-theBCIDSpill_>theSpillLength_/2E-7) theBCIDSpill_=theAbsoluteTime_;

  int nhits=0;
  theNplans_=0;
  
  std::bitset<60> chhit(0);
  std::map<uint32_t,std::vector<IMPL::RawCalorimeterHitImpl*> >::iterator iseed=reader_->getPhysicsEventMap().find(seed);
   if (iseed==reader_->getPhysicsEventMap().end()) 
   {
      INFO_PRINT("Impossible \n");
      return ;
   }
   
   theCerenkovTag_=this->CerenkovTagger(3,seed);
   uint32_t tag=theCerenkovTag_;
   //printf("%d %d \n",seed,theCerenkovTag_);
   
   theNplans_=this->fillVector(seed);
   if (_hits.size()<80) return;
   this->drawHits();
   char c;c=getchar();putchar(c); if (c=='.') exit(0);
   if (theNplans_<minChambersInTime_) return;
   
  return;

}

void trackAnalysis::processEvent()
{

  

  if (reader_->getEvent()==0) return;
    
  evt_=reader_->getEvent();
  //theSkip_=380;
  
  if (evt_->getEventNumber()<=theSkip_) return;
  
  printf("Processing %d - %d \n",evt_->getRunNumber(),evt_->getEventNumber());
  if (evt_->getRunNumber()!=_runNumber)
    {
      _runNumber=evt_->getRunNumber();
      //reader_->logbookBeamEnergy(_runNumber);

    }

    
  nAnalyzed_++;
  
  IMPL::LCCollectionVec* rhcol=NULL;
  bool rhcoltransient=false;
  try {
    rhcol=(IMPL::LCCollectionVec*) evt_->getCollection(collectionName_);
    rebuild_=false;
  }
  catch (...)
    {
      try 
	{
	  evt_->getCollection("RU_XDAQ");
	  rebuild_=true;
	}
      catch (...)
	{
	   DEBUG_PRINT("No raw data or calo hits \n");
	  exit(0);
	}
    }
  if (rebuild_)
    {
      
      reader_->parseRawEvent();
      DEBUG_PRINT("End of parseraw \n");
      //reader_->flagSynchronizedFrame();
      std::vector<uint32_t> seed;
      if (useSynchronised_ )
	{
	  //DEBUG_PRINT("Calling FastFlag2\n");
			
	  reader_->findTimeSeeds(minChambersInTime_,seed);
	  // DEBUG_PRINT("End of FastFlag2 \n");
			
	}
      else
	{
			
	  seed.clear();
	}
      //
      // getchar();
      seed.clear();
      //INFO_PRINT("Calling CreaetRaw %d\n",minChambersInTime_);
      //reader_->findDIFSeeds(minChambersInTime_);
      //rhcol=reader_->createRawCalorimeterHits(reader_->getDIFSeeds());
      
      rhcol=reader_->createRawCalorimeterHits(seed);
      evt_->addCollection(rhcol,collectionName_);
      rhcoltransient=false; 

    }
  else
    rhcol=(IMPL::LCCollectionVec*) evt_->getCollection(collectionName_);

  
  //INFO_PRINT("End of CreaetRaw %d \n",rhcol->getNumberOfElements());  
  if (rhcol->getNumberOfElements()>4E6) return;
  
  //_monitor->FillTimeAsic(rhcol);

  //  LCTOOLS::printParameters(rhcol->parameters());
  //DEBUG_PRINT("Time Stamp %d \n",evt_->getTimeStamp());
  if (rhcol==NULL) return;
  if (rhcol->getNumberOfElements()==0) return;
  //DEBUG_PRINT("Calling decodeTrigger\n");
  // TESTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
  
  if (!decodeTrigger(rhcol,spillSize_) ) { if (rhcoltransient) delete rhcol;return;}
  
  //if (isNewSpill_) return;
  
  if (evt_->getEventNumber()%100 ==0)
    rootHandler_->writeSQL();
  //    rootHandler_->writeXML(theMonitoringPath_);
  
  PMAnalysis(3);
  
  reader_->findTimeSeeds(7);
  
  std::vector<uint32_t> vseeds=reader_->getTimeSeeds();

  
   INFO_PRINT("================>  %d  Number of seeds %d \n",evt_->getEventNumber(),(int) vseeds.size());

  if (vseeds.size()==0)  { if (rhcoltransient) delete rhcol;return;}
   
  bool hasPion=false;
  for (uint32_t is=0;is<vseeds.size();is++)
    {
      //printf("%d %d %x \n",is,vseeds[is],rhcol);
      this->processSeed(rhcol,vseeds[is]);


      
    }
  
  if ((theBCID_-theLastBCID_)*2E-7>5)
    {
      theBCIDSpill_=theBCID_;
      theIdxSpill_=0;
      memset(theCountSpill_,0,20*sizeof(float));
      memset(theTimeInSpill_,0,20*sizeof(float));
      printf("===============================================================================================> NEW SPILL : %f\n",theBCIDSpill_*2E-7);
    }
  else
    theIdxSpill_+=1;

  //  theCountSpill_[theIdxSpill_%20] =  theNbShowers_+theNbTracks_;
  theCountSpill_[theIdxSpill_%10] =  0;
  theTimeInSpill_[theIdxSpill_%10] = _monitor->getEventIntegratedTime()*2E-7;

  // Integrated 10 last
  float nc=0;
  float tc=0;

  for (int i=0;i<10;i++)
    {
      nc+=theCountSpill_[i];
      tc+=theTimeInSpill_[i];
      //printf("%f ",theCountSpill_[i]);
    }

  //INFO_PRINT("\n %d Number of showers/tracks %d,%d Event time %f -> Absolute bcid  %f-> Rate %f %f %f\n",evt_->getEventNumber(),theNbShowers_,theNbTracks_,theMonitoring_->getEventIntegratedTime()*2E-7,(theBCID_-theBCIDSpill_)*2E-7,nc,tc,nc/tc);
  theLastRate_=nc/tc;
  theLastBCID_=theBCID_;
  //etchar();

  if (rhcoltransient) delete rhcol;

  return;
}  

static TCanvas* TCPlot=NULL;
static TCanvas* TCHits=NULL;
static TCanvas* TCShower=NULL;
static TCanvas* TCEdge=NULL;
static TCanvas* TCHT=NULL;
static TCanvas* TCCluster=NULL;
void trackAnalysis::drawHits()
{

  TH3* hcgposi = rootHandler_->GetTH3("InstantHitMap");
  if (hcgposi==NULL)
    {
      hcgposi =rootHandler_->BookTH3("InstantHitMap",200,-10.,150.,120,-10.,110.,120,-10.,110.);
    }
  else
    {
      hcgposi->Reset();
    }
  TH2* hpx = rootHandler_->GetTH2("realx");
  TH2* hpy = rootHandler_->GetTH2("realy");

  if (hpx==NULL)
    {
      hpx =rootHandler_->BookTH2("realx",200,-10.,150.,120,-10.,120.);
      hpy =rootHandler_->BookTH2("realy",200,-10.,150.,120,-10.,120.);
    }
  else
    {
      hpx->Reset();
      hpy->Reset();
    }

  if (hcgposi!=0 )
    {
      hcgposi->Reset();
      
      for (std::vector<RecoHit>::iterator ih=_hits.begin();ih!=_hits.end();ih++)
	hcgposi->Fill(ih->Z(),ih->X(),ih->Y());//(*ih)->Z(),(*ih)->X(),(*ih)->Y());
      



      if (TCHits==NULL)
	{
	  TCHits=new TCanvas("TCHits","tChits1",1300,600);
	  TCHits->Modified();
	  TCHits->Draw();
	  TCHits->Divide(2,2);
	}
      TCHits->cd(1);
      hcgposi->SetMarkerStyle(25);
      hcgposi->SetMarkerColor(kRed);
      hcgposi->Draw("P");
      /*
      if (ish!=0)
	{
	  TPolyLine3D *pl3d1 = new TPolyLine3D(2);
	  double* v=ish->l0;    
	  double* x=ish->xm;
	  double r=ish->lambda[0]*100;
	  pl3d1->SetPoint(0,x[2],x[0],x[1]);
	  pl3d1->SetPoint(1,x[2]+v[2]*r,x[0]+v[0]*r,x[1]+v[1]*r);
			
	  pl3d1->SetLineWidth(3);
	  pl3d1->SetLineColor(1);
	  pl3d1->Draw("SAME");
	  TPolyLine3D *pl3d2 = new TPolyLine3D(2);
	  v=ish->l1;    
	  r=ish->lambda[1]*100;
	  pl3d2->SetPoint(0,x[2],x[0],x[1]);
	  pl3d2->SetPoint(1,x[2]+v[2]*r,x[0]+v[0]*r,x[1]+v[1]*r);
			
	  pl3d2->SetLineWidth(3);
	  pl3d2->SetLineColor(2);
	  pl3d2->Draw("SAME");
	  TPolyLine3D *pl3d3 = new TPolyLine3D(2);
	  v=ish->l2;    
	  r=ish->lambda[2]*100;
	  pl3d3->SetPoint(0,x[2],x[0],x[1]);
	  pl3d3->SetPoint(1,x[2]+v[2]*r,x[0]+v[0]*r,x[1]+v[1]*r);



			
	  pl3d3->SetLineWidth(3);
	  pl3d3->SetLineColor(3);
	  pl3d3->Draw("SAME");
	}
      */
      TCHits->cd(2);
      // Now loop on clusters
      TH2* hdisp = rootHandler_->GetTH2("displacement");

      if (hdisp==NULL)
	{
	  hdisp =rootHandler_->BookTH2("displacement",30,0,M_PI/2.,30,-M_PI,M_PI);
	}
      else
	hdisp->Reset();
      std::vector<recoTrack> vtk;
      for (std::vector<planeCluster*>::iterator ic=realClusters_.begin();ic!=realClusters_.end();ic++)
	{
	  planeCluster* c0=(*ic);
	  hpx->Fill(c0->Z(),c0->X());
	  hpy->Fill(c0->Z(),c0->Y());
	  if ((*ic)->isUsed()) continue;
	  for (std::vector<planeCluster*>::iterator jc=realClusters_.begin();jc!=realClusters_.end();jc++)
	    {
	      if ((*jc)->isUsed()) continue;
	      if ((*jc)->Z()<=(*ic)->Z()) continue;
	      planeCluster* c1=(*jc);
	      ROOT::Math::XYZVector d=(*c1)-(*c0);
	      if (d.Mag2()>100.) continue;


	      bool good=false;
	      recoTrack tk;
	      for (std::vector<planeCluster*>::iterator kc=realClusters_.begin();kc!=realClusters_.end();kc++)
		{
		  if ((*kc)->isUsed()) continue;
		  if ((*kc)->Z()<=(*jc)->Z()) continue;
		  planeCluster* c2=(*kc);
		  ROOT::Math::XYZVector d1=(*c2)-(*c1);
		  if (d1.Mag2()>100.) continue;
		  double s=d.Dot(d1)/sqrt(d.Mag2()*d1.Mag2());
		  //std::cout<<s<<std::endl;
		  if (abs(s-1.)<1E-2)
		    {
		      (*ic)->setUse(true);
		      (*jc)->setUse(true);
		      (*kc)->setUse(true);

		      tk.addPoint((*ic));
		      tk.addPoint((*jc));
		      tk.addPoint((*kc));
		      good=true;
		      break;
		    }
		}
	      if (good)
		{
		  for (std::vector<planeCluster*>::iterator kc=realClusters_.begin();kc!=realClusters_.end();kc++)
		    {
		      if ((*kc)->isUsed()) continue;
		      if (tk.distance((*kc))<1.5)
			{
			  tk.addPoint((*kc));
			  (*kc)->setUse(true);
			}
		      //std::cout<<tk.distance((*kc))<<std::endl;
		    }
		  if (tk.size()>3)
		    {
		      std::cout<<tk;
		      tk.Dump();

		      vtk.push_back(tk);
		    }

		  hdisp->Fill(d.Theta(),d.Phi(),1.);
		}
	    }
	}
      /*
      for (std::vector<recoTrack>::iterator it=vtk.begin();it!=vtk.end();it++)
	{
	  std::cout<<(*it);
	  // for (std::vector<planeCluster*>::iterator kc=realClusters_.begin();kc!=realClusters_.end();kc++)
	  //   {
	  //     std::cout<<it->distance((*kc))<<std::endl;
	      
	  //   }
	  for (std::vector<ROOT::Math::XYZPoint*>::iterator kc=it->points().begin();kc!=it->points().end();kc++)
	    {
	      std::cout<<"\t Z:"<<(*kc)->Z()<<std::endl;
	    }


	}
      */ 
      
      //for (int i=1;i<=15;i++)
      //	for (int j=1;j<15;j++)
      //  if (hdisp->GetBinContent(i,j)<12) hdisp->SetBinContent(i,j,0);
      
      hcgposi->SetMarkerStyle(25);
      hcgposi->SetMarkerColor(kRed);
      hdisp->Draw("COLZ");
      TCHits->Modified();
      TCHits->Draw();

     
      TCHits->cd(3);
      
      hpx->SetLineColor(kRed);
      hpx->Draw("BOX");
      for (std::vector<recoTrack>::iterator it=vtk.begin();it!=vtk.end();it++)
	{
	  it->linex()->Draw("SAME");
	}
      TCHits->cd(4);
      


      hpy->SetLineColor(kRed);
      hpy->Draw("BOX");

      for (std::vector<recoTrack>::iterator it=vtk.begin();it!=vtk.end();it++)
	{
	  it->liney()->Draw("SAME");
	}
      
      TCHits->Modified();
      TCHits->Draw();
      TCHits->Update();
    }
  
}



#define DBG printf("%d %s\n",__LINE__,__PRETTY_FUNCTION__);


void trackAnalysis::clearClusters()
{
  realClusters_.clear();
  interactionClusters_.clear();
  
  for (std::vector<planeCluster*>::iterator ic=allClusters_.begin();ic!=allClusters_.end();ic++)
    delete (*ic);
  
  allClusters_.clear();
  nPlansReal_.reset();
  nPlansInteraction_.reset();
  nPlansAll_.reset();
    
}
void trackAnalysis::fillPlaneClusters()
{
  //printf("DEBUT\n");
  
  clearClusters();
  
  //return;
  for (std::vector<RecoHit>::iterator ih=_hits.begin();ih<_hits.end();ih++)
    {
      
      if (ih->isTagged(RecoHit::CORE)==1) continue;
      bool merged=false;
      for (std::vector<planeCluster*>::iterator ic=realClusters_.begin();ic!=realClusters_.end();ic++)
	{
	  if (ih->chamber()!=(*ic)->chamber()) continue;
	  merged=(*ic)->Append(&(*ih),2.); // avant 4 et normalement 2
	  if (merged) break;
	}
      if (merged) continue;
      planeCluster* c= new planeCluster(&(*ih));
      realClusters_.push_back(c);
      allClusters_.push_back(c);
    }
  
  //printf("OLA %d %d \n",allClusters_.size(),realClusters_.size());
  for (std::vector<RecoHit>::iterator ih=_hits.begin();ih<_hits.end();ih++)
    {
      if (ih->isTagged(RecoHit::CORE)!=1) continue;
      bool merged=false;
      for (std::vector<planeCluster*>::iterator ic=interactionClusters_.begin();ic!=interactionClusters_.end();ic++)
	{
	  if (ih->chamber()!=(*ic)->chamber()) continue;
	  merged=(*ic)->Append(&(*ih),4.); // avant 4 et normalement 2
	  if (merged) break;
	}
      if (merged) continue;
      planeCluster* c= new planeCluster(&(*ih));
      interactionClusters_.push_back(c);
      allClusters_.push_back(c);
    }
  
  // Move small cluster from interactionClusters_ to realClusters_
  for (std::vector<planeCluster*>::iterator ic=interactionClusters_.begin();ic!=interactionClusters_.end();)
    {
      if ((*ic)->getHits()->size()>=5) 
	++ic;
      else
	{
	  realClusters_.push_back((*ic));
	  interactionClusters_.erase(ic);
	}
    }
  
  for (std::vector<planeCluster*>::iterator ic=realClusters_.begin();ic!=realClusters_.end();)
    {
      if ((*ic)->getHits()->size()<5) 
	++ic;
      else
	{
	  interactionClusters_.push_back((*ic));
	  realClusters_.erase(ic);
	}
    }
  // Now find first and last interaction planes
  
  lastInteractionPlane_=0;
  firstInteractionPlane_=100;
  for (std::vector<planeCluster*>::iterator ic=interactionClusters_.begin();ic!=interactionClusters_.end();ic++)
    {
      uint32_t ch=(*ic)->plan();
      nPlansInteraction_.set(ch,true);
      nPlansAll_.set(ch,true);
      if (lastInteractionPlane_<ch) lastInteractionPlane_=ch;
      if (firstInteractionPlane_>ch) firstInteractionPlane_=ch;
    }

  // Fill Position/layer buffer
  

}



#ifdef BUILDPRINCIPAL
double xmin[100];
double xmax[100];
double ymin[100];
double ymax[100];

uint32_t trackAnalysis::buildPrincipal(std::vector<planeCluster*> &vrh,std::string vdir)
{


  //INFO_PRINT("Avant Principal %d\n",npBuf_);
  pcaComponents c=RecoHit::calculateComponents<planeCluster>(vrh);




  //  if (theComputerTrack_->getTracks().size()>0) theNbTracks_++;
  uint32_t nmip=0;
  
  TrackInfo tk0;
  tk0.clear();
  double* x=isha.xm;
  double* v=isha.l2;
  // Z X
  double z0=x[2];
  double x0=x[0];
  double y0=x[1];

  double x1=x[0]+v[0];
  double y1=x[1]+v[1];
  double z1=x[2]+v[2];
  double ax,ay,bx,by;
  ax=(x1-x0)/(z1-z0);
  bx=x1-ax*z1;
  ay=(y1-y0)/(z1-z0);
  by=y1-ay*z1;
  tk0.set_ax(ax);
  tk0.set_bx(bx);
  tk0.set_ay(ay);
  tk0.set_by(by);
  //INFO_PRINT("Avant regression ");
  double cut=8.;
  for (uint32_t i=0;i<npBuf_;i++)
    {
      if (abs(tk0.closestApproach(_x[i],_y[i],_z[i]))<cut)
	{
	  // 3 hits on tag
	  if (_layer[i]>60)
	    printf(" bad stub %f %f %f %d \n",_x[i],_y[i],_z[i],_layer[i]);
	  tk0.add_point(_x[i],_y[i],_z[i],_layer[i]);

	}
      
    }
  
  if (tk0.size()<3)
    {
   return 0;
    }
  tk0.regression();
  //INFO_PRINT("Avant Second fit ");
  TrackInfo tk;tk.clear();
  tk.set_ax(tk0.ax());
  tk.set_bx(tk0.bx());
  tk.set_ay(tk0.ay());
  tk.set_by(tk0.by());
  cut=8.;
  for (uint32_t i=0;i<npBuf_;i++)
    {
      if (abs(tk.closestApproach(_x[i],_y[i],_z[i]))<cut)
	{
	  // 3 hits on tag
	  if (_layer[i]>60)
	    printf(" bad stub %f %f %f %d \n",_x[i],_y[i],_z[i],_layer[i]);
	  tk.add_point(_x[i],_y[i],_z[i],_layer[i]);
	  if (tk.size()>=3)
	    {
	      tk.regression();
	      cut=2.;
	    }
	}
      
    }
  
  //  INFO_PRINT("Apres second fit \n");
  if (tk.size()<tkMinPoint_)
    {
   return 0;
    }

  //if (tk.size()<minChambersInTime_) continue;
  //if (fabs(tk.ax())<1.E-2) continue;
  //if (fabs(tk.ax())<0.5 && fabs(tk.ay())<0.5) theNbTracks_++;
  //this->draw(tk);
  //char c;c=getchar();putchar(c); if (c=='.') exit(0);
  uint32_t fch=int(ceil(tk.zmin()*10))/28+1;
  uint32_t lch=int(ceil(tk.zmax()*10))/28+1;
  
  std::stringstream st;
  st<<vdir<<"/";
  TH1* hnp= rootHandler_->GetTH1(st.str()+"Npoints");
  TH1* hnpl= rootHandler_->GetTH1(st.str()+"Nplanes");
  TH1* hax= rootHandler_->GetTH1(st.str()+"ax");
  TH1* hay= rootHandler_->GetTH1(st.str()+"ay");
  TH1* hwt= rootHandler_->GetTH1(st.str()+"hitweight");

  if (hnp==NULL)
    {
      hnp=  rootHandler_->BookTH1(st.str()+"Npoints",51,-0.1,50.9);
      hnpl=  rootHandler_->BookTH1(st.str()+"Nplanes",51,-0.1,50.9);
      hax=  rootHandler_->BookTH1(st.str()+"ax",200,-5.,5.);
      hay=  rootHandler_->BookTH1(st.str()+"ay",200,-5.,5.);


    }

  // Calcul de l'efficacite

  // Track info
  
  hnp->Fill(tk.size()*1.);
  hax->Fill(tk.ax());
  hay->Fill(tk.ay());
  fch=tkFirstChamber_;lch=tkLastChamber_;
  for (int ip=fch;ip<=lch;ip++)
    if (tk.plane(ip)) hnpl->Fill(ip*1.);
  //  std::cout<<tk.planes()<<std::endl;
  //getchar();
    
  for (uint32_t ip=fch;ip<=lch;ip++)
    {
      //INFO_PRINT("Plan %d studied \n",ip);
      TrackInfo tex;
	      
      tk.exclude_layer(ip,tex);
      
      uint32_t npext=tex.size();
      
      if (npext<tkExtMinPoint_) continue; // Au moins 4 plans dans l'estrapolation touches 

      //if (ip>1 && !tex.plane(ip-1)) continue;
      //if (ip>2 && !tex.plane(ip-2)) continue;
      
      //if (ip<lch && !tex.plane(ip+1)) continue;
      //if (ip<(lch-1) && !tex.plane(ip+2)) continue;
      
	//if (npext<minChambersInTime_) continue;
      
      std::stringstream s;
      s<<st.str()<<"Plan"<<ip<<"/";
	      
      TH2* hext= rootHandler_->GetTH2(s.str()+"ext");
      TH2* hfound= rootHandler_->GetTH2(s.str()+"found");
      TH2* hnear= rootHandler_->GetTH2(s.str()+"near");
      TH2* hfound1= rootHandler_->GetTH2(s.str()+"found1");
      TH2* hfound2= rootHandler_->GetTH2(s.str()+"found2");
      TH2* hmul= rootHandler_->GetTH2(s.str()+"mul");
      TH1* hdx= rootHandler_->GetTH1(s.str()+"dx");
      TH1* hdy= rootHandler_->GetTH1(s.str()+"dy");
      TH2* hmiss= rootHandler_->GetTH2(s.str()+"missing");
      float dz0=0.,distz=60.; // 2.8
      float xext=tex.xext(dz0+(ip-1)*distz);
      float yext =tex.xext(dz0+(ip-1)*distz);
      
      std::map<uint32_t,ChamberPos>& pos= reader_->getPositionMap();
      

      for (std::map<uint32_t,ChamberPos>::iterator ich=pos.begin();ich!=pos.end();ich++)
	{

	  
	  if ((*ich).second.getPlan()!=ip) continue;
	  xext=tex.xext((*ich).second.getZ0());
	  yext =tex.yext((*ich).second.getZ0());
	  break;
	}
      //INFO_PRINT("%f %f \n",xext,yext);
      //if (yext< 5 || yext>30 || xext<2 || xext>29) continue;
      if (hext==NULL)
	{
		  
	  double xi=1000,xa=-1000,yi=1000,ya=-1000;
	  for (std::map<uint32_t,ChamberPos>::iterator ich=pos.begin();ich!=pos.end();ich++)
	    {
	      if ((*ich).second.getPlan()!=ip) continue;
	      if ((*ich).second.getX0()<xi) xi= (*ich).second.getX0();
	      if ((*ich).second.getY0()<yi) yi= (*ich).second.getY0();
	      if ((*ich).second.getX0()>xa) xa= (*ich).second.getX0();
	      if ((*ich).second.getY0()>ya) ya= (*ich).second.getY0();
	      if ((*ich).second.getX1()<xi) xi= (*ich).second.getX1();
	      if ((*ich).second.getY1()<yi) yi= (*ich).second.getY1();
	      if ((*ich).second.getX1()>xa) xa= (*ich).second.getX1();
	      if ((*ich).second.getY1()>ya) ya= (*ich).second.getY1();
		      
	    }
	  xmin[ip]=xi;
	  xmax[ip]=xa;
	  ymin[ip]=yi;
	  ymax[ip]=ya;
	  int nx=int(xa-xi)+1;
	  int ny=int(ya-yi)+1;

	  hext= rootHandler_->BookTH2(s.str()+"ext",nx,xi,xa,ny,yi,ya);
	  hfound= rootHandler_->BookTH2(s.str()+"found",nx,xi,xa,ny,yi,ya);
	  hnear= rootHandler_->BookTH2(s.str()+"near",nx,xi,xa,ny,yi,ya);
	  hfound1= rootHandler_->BookTH2(s.str()+"found1",nx,xi,xa,ny,yi,ya);
	  hfound2= rootHandler_->BookTH2(s.str()+"found2",nx,xi,xa,ny,yi,ya);
	  hmiss= rootHandler_->BookTH2(s.str()+"missing",nx,xi,xa,ny,yi,ya);
	  hmul= rootHandler_->BookTH2(s.str()+"mul",nx,xi,xa,ny,yi,ya);
	  hdx=  rootHandler_->BookTH1(s.str()+"dx",400,-4.,4.);
	  hdy=  rootHandler_->BookTH1(s.str()+"dy",400,-4.,4.);
	}
      if (xext<xmin[ip]+chamberEdge_ || xext>xmax[ip]-chamberEdge_) continue;
      if (yext<ymin[ip]+chamberEdge_ || yext>ymax[ip]-chamberEdge_) continue;
      
      hext->Fill(xext,yext);
      //bool 
      float dist=1E9;
      bool th1=false,th2=false;
      float dxi,dyi,xn,yn,nhi=0;
      for (std::vector<planeCluster*>::iterator ic=realClusters_.begin();ic!=realClusters_.end();ic++)
	{
	  if ((*ic)->plan()!=ip) continue;
	  ChamberPos& cp=reader_->getPosition((*ic)->chamber());
	  // DEBUG_PRINT(" %d (%f,%f,%f) (%f,%f,%f) (%d,%d) \n",
	  //	 cp.getId(),cp.getX0(),cp.getY0(),cp.getZ0(),cp.getX1(),cp.getY1(),cp.getZ1(),cp.getXsize(),cp.getYsize());
	  double x,y,z;
	  cp.calculateGlobal((*ic)->X(),(*ic)->Y(),x,y,z);
	  xext=tex.xext(z);
	  yext=tex.yext(z);
	  float dx=xext-x;
	  float dy=yext-y;

	  double dap=tex.closestApproach(x,y,z);
	  //  DEBUG_PRINT(" (%f,%f,%f) %f %f \n",x,y,z,dap,sqrt(dx*dx+dy*dy));
	  //getchar();
	  if (dap<dist)
	    {
		      
	      dist=dap;
	      dxi=dx;
	      dyi=dy;
	      nhi=(*ic)->size();
	      xn=x;
	      yn=y;
	      th1=false,th2=false;
	      for (std::vector<RecoHit*>::iterator ih=(*ic)->getHits()->begin();ih!=(*ic)->getHits()->end();ih++)
		{
		  if ((*ih)->isTagged(RecoHit::THR1)!=0) th1=true;
		  if ((*ih)->isTagged(RecoHit::THR2)!=0) th2=true;
		}

	    }

	}
      // Cut a 1.5 au lieu de 6
      if (dist<tkExtDistCut_)
	{
	  hdx->Fill(dxi);
	  hdy->Fill(dyi);
	  hmul->Fill(xext,yext,nhi*1.);
	  hfound->Fill(xext,yext);
	  hnear->Fill(xn,yn);
	  if (th1||th2)  hfound1->Fill(xext,yext);
	  if (th2)  hfound2->Fill(xext,yext);
	}
      else
	 hmiss->Fill(xext,yext);
    }



  //this->draw(tk);

	
   DEBUG_PRINT("==> MIPS hit %d -> %.2f\n",nmip,nmip*100./vrh.size()); 
 
 
 
   return 1;
}





void trackAnalysis::draw(TrackInfo& t)
{


  TH3* hcgposi = NULL;

  if (hcgposi==NULL)
    {
      hcgposi =rootHandler_->BookTH3("InstantTkMap",66,t.zmin()-10.,t.zmax()+10,150,-50.,150.,200,-50.,150.);
       DEBUG_PRINT("Booking %f %f \n",t.zmin()-10.,t.zmax()+10);
    }
  else
    {
      hcgposi->Reset();
    }

  if (hcgposi!=0 )
    {
      hcgposi->Reset();
      for (int ip=0;ip<t.size();ip++)
	{
	  //if (allpoints_[i].Charge()<7) continue;
	  hcgposi->Fill(t.z(ip),t.x(ip),t.y(ip));//(*ih)->Z(),(*ih)->X(),(*ih)->Y());
	   DEBUG_PRINT("%d %f %f %f \n",ip,t.z(ip),t.x(ip),t.y(ip));
	}


      if (TCHT==NULL)
	{
	  TCHT=new TCanvas("TCHT","tcht1",1300,600);
	  TCHT->Modified();
	  TCHT->Draw();
	  TCHT->Divide(2,2);
	}
      TCHT->cd(1);
      hcgposi->SetMarkerStyle(25);
      hcgposi->SetMarkerColor(kRed);
      hcgposi->Draw("P");

      TCHT->cd(2);
      TProfile2D* hpy1=hcgposi->Project3DProfile("zx");
      hpy1->SetLineColor(kGreen);
		

      hpy1->Draw("BOX");

      TLine* l = new TLine(t.zmin(),t.yext(t.zmin()),t.zmax(),t.yext(t.zmax()));
      l->SetLineColor(2);
      l->Draw("SAME");



      TCHT->cd(3);
      TProfile2D* hpy2=hcgposi->Project3DProfile("yx");
      hpy2->SetLineColor(kBlue);
		

      hpy2->Draw("BOX");


      TLine* l1 = new TLine(t.zmin(),t.xext(t.zmin()),t.zmax(),t.xext(t.zmax()));
      l1->SetLineColor(2);
      l1->Draw("SAME");


      TCHT->Modified();
      TCHT->Draw();
      TCHT->Update();
    }



}
#endif
#define makekey(i,j,k) ((i<<16)|(j<<8)|k)
#define K2I(kk) ((kk>>16)&0XFF)
#define K2J(kk) ((kk>>8)&0XFF)
#define K2K(kk) (kk&0XFF)
uint32_t trackAnalysis::fillVolume(uint32_t seed)
{
  std::map<uint32_t,std::vector<IMPL::RawCalorimeterHitImpl*> >::iterator iseed=reader_->getPhysicsEventMap().find(seed);
  if (iseed==reader_->getPhysicsEventMap().end()) 
    {
      DEBUG_PRINT("Impossible \n");
      return 0;
    }

  // clean key
  //_keys.clear();
  _hits.clear();

  std::bitset<61> planes(0);
  

  uint32_t ncount=0;
  
  if (iseed->second.size()>4096) return 0;
  for (std::vector<IMPL::RawCalorimeterHitImpl*>::iterator ihit=iseed->second.begin();ihit!=iseed->second.end();ihit++)
    {
      IMPL::RawCalorimeterHitImpl* hit =(*ihit);
      RecoHit h(_geo,hit);
      planes.set(h.plan());
      _hits.push_back(h);
      //_keys.push_back(makekey(I,J,chid));

      ncount++;
    }
  DEBUG_PRINT("Total number of Hit in buildVolume %d  %d => planes %d \n",ncount,seed,planes.count());
  return planes.count();
}

void trackAnalysis::TagIsolated(uint32_t fpl,uint32_t lpl)
{
  TH1* hweight= rootHandler_->GetTH1("/HitStudy/showerweight");
  TH1* hnv= rootHandler_->GetTH1("/HitStudy/nv");
  TH2* hweight2= rootHandler_->GetTH2("/HitStudy/showerweight2");
  if (hweight==NULL)
    {
      //hweight=(TH1F*) rootHandler_->BookTH1("showerweight",100,0.,2.);
      hweight= rootHandler_->BookTH1("/HitStudy/showerweight",110,-0.1,0.99);
      hnv= rootHandler_->BookTH1("/HitStudy/nv",150,0.,150.);
      hweight2= rootHandler_->BookTH2("/HitStudy/showerweight2",150,0.,150.,110,-0.1,0.99);
    }
  //hweight->Reset(); // commented by LM21_01_2015

  uint32_t nmax=0;
  uint32_t nedge=0,ncore=0,niso=0;
  int32_t ixmin=-6,ixmax=6; // 6 avant
  std::vector<RecoHit*> vnear_;
  for (std::vector<RecoHit>::iterator it=_hits.begin();it!=_hits.end();it++)
    {
      vnear_.clear();
      for (std::vector<RecoHit>::iterator jt=_hits.begin();jt!=_hits.end();jt++)
	 {
	   if (jt==it) continue;
	   ROOT::Math::XYZVector d=(*it)-(*jt);
	   if (d.Mag2()<36.)
	     vnear_.push_back(&(*jt));
	 }
      /*
       int izmin=-2;
       int izmax=+2;
       if (it->chamber()<=fpl+2) {izmin=0;izmax=4;}
       if (it->chamber()>=lpl-2) {izmin=-4;izmax=0;}
       std::vector<RecoHit*> vnear_;vnear_.clear();
       RecoHit* h0=&(*it);
       for (std::vector<RecoHit>::iterator jt=_hits.begin();jt!=_hits.end();jt++)
	 {
	   if ((*it)==(*jt)) continue;
	   if (jt->chamber()<it->chamber()+izmin) continue;
	   if (jt->chamber()>it->chamber()+izmax) continue;
	   if (jt->I()<it->I()+ixmin) continue;
	   if (jt->I()>it->I()+ixmax) continue;
	   if (jt->J()<it->J()+ixmin) continue;
	   if (jt->J()>it->J()+ixmax) continue;

	   RecoHit* h1=&(*jt);
	   
	   float x0=h0->X(),y0=h0->Y(),z0=h0->Z(),x1=h1->X(),y1=h1->Y(),z1=h1->Z();
	   float dist=sqrt((x1-x0)*(x1-x0)+(y1-y0)*(y1-y0)+(z1-z0)*(z1-z0));
	   dist=abs(x1-x0)+abs(y1-y0)+abs(z1-z0)/2.;
	   if (dist<4.) vnear_.push_back(h1); // was 4
	 }
      */
      RecoHit* h0=&(*it);
       pcaComponents c=RecoHit::calculateComponents<RecoHit>(vnear_);
       double w=0;
       if (c[5]!=0) w=sqrt((c[4]+c[3])/c[5]);
       if (c[5]==0 || vnear_.size()<=3)
	 {
	   h0->setTag(RecoHit::ISOLATED,true);niso++;
	   hnv->Fill(vnear_.size()*1.);
	 }
       else
	 {
	   hweight->Fill(w);
	   hweight2->Fill(vnear_.size()*1.,w);
	   if (w<0.25 && vnear_.size()<20) // 0.3 before  
	     {h0->setTag(RecoHit::EDGE,true);nedge++;}
	   else
	     {h0->setTag(RecoHit::CORE,true);ncore++;}
	 }
       
    }
  coreRatio_=ncore*1./(nedge+niso);
  return;
}


uint32_t trackAnalysis::fillVector(uint32_t seed)
{
  

 
  //INFO_PRINT("%s-%d %d %f  \n",__PRETTY_FUNCTION__,__LINE__,reader_->getPositionMap().begin()->second.getPlan(),reader_->getPositionMap().begin()->second.getZ0());    
  uint32_t nplans=this->fillVolume(seed);
  uint32_t nhit=_hits.size();
  
  //INFO_PRINT("%s-%d %d %f  \n",__PRETTY_FUNCTION__,__LINE__,reader_->getPositionMap().begin()->second.getPlan(),reader_->getPositionMap().begin()->second.getZ0());    
  if (nhit<minHitCount_) return 0;
  if (nhit>maxHitCount_) return 0;
  this->TagIsolated(1,48);
//  STEP;
 
  theTkHitVector_.clear();
  for (std::vector<RecoHit>::iterator it=_hits.begin();it!=_hits.end();it++)
    {
      RecoHit h=(*it);
      if (h.isTagged(RecoHit::ISOLATED)!=1) 
	theTkHitVector_.push_back(&h);
    }
  
  this->fillPlaneClusters();
  INFO_PRINT("Hits %d tk %d ===> %d clusters %d Real %d Interaction \n",_hits.size(),theTkHitVector_.size(),allClusters_.size(),realClusters_.size(),interactionClusters_.size());
  
  //this->tagMips();

  return nplans;
}
#ifdef TAGMIPS
void trackAnalysis::tagMips()
{
  
  DEBUG_PRINT("Clusters=> %d ",npBuf_);
  theComputerTrack_->associate(npBuf_,_x,_y,_z,_layer);
  
  uint32_t nmip=0;
  for (unsigned int i=0;i<theComputerTrack_->getCandidates().size();i++)
    {
      RecoCandTk& tk = theComputerTrack_->getCandidates()[i];
#ifdef CORRECT_GEOM
      tk.ax_/=1.04125;
      tk.bx_/=1.04125;
      tk.ay_/=1.04125;
      tk.by_/=1.04125;
#endif
      for (std::vector<planeCluster*>::iterator ic=realClusters_.begin();ic!=realClusters_.end();ic++)
	    {
	      float dx=tk.getXext((*ic)->Z())-(*ic)->X();
	      float dy=tk.getYext((*ic)->Z())-(*ic)->Y();
	      if (sqrt(dx*dx+dy*dy)<2.)
		{
		  // DEBUG_PRINT("Point (%f,%f,%f) dist %f %f -> %f  \n",(*ic)->X(),(*ic)->Y(),(*ic)->Z(),dx,dy,sqrt(dx*dx+dy*dy));
		for (std::vector<RecoHit*>::iterator ih=(*ic)->getHits()->begin();ih!=(*ic)->getHits()->end();ih++)
		  {
		    if ((*ih)->isTagged(RecoHit::MIP)==0)
		      {
			// DEBUG_PRINT("Point (%f,%f,%f) dist %f %f -> %f  \n",(*ic)->X(),(*ic)->Y(),(*ic)->Z(),dx,dy,sqrt(dx*dx+dy*dy));
			(*ih)->setTag(RecoHit::MIP,true);
			hitVolume_[(*ih)->chamber()-1][(*ih)->I()-1][(*ih)->J()-1].setTag(RecoHit::MIP,true);
			nmip++;
		      }
		  }
		}
	    }
	}
  
   DEBUG_PRINT("==> MIPS hit %d -> %.2f Length %f \n",nmip,nmip*100./theHitVector_.size(),theComputerTrack_->Length()); 

}
#endif


extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
    DHCALAnalyzer* loadAnalyzer(void)
    {
      DHCALAnalyzer* d= new trackAnalysis();
      printf(" Ona cree %x \n",d);
  
      return d;
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
    void deleteAnalyzer(DHCALAnalyzer* obj)
    {
      delete obj;
    }
}
