#define NPLANS_USED 48

#include "TrackAnalyzer.h"
#include "DIFUnpacker.h"
#include <TLine.h>
#include <TGraphErrors.h>
#include <TFitResult.h>
#include <TFitter.h>
#include <TF1.h>
#include <TPluginManager.h>
#include <stdint.h>
#include <math.h>
#include <ext/hash_map>
using namespace __gnu_cxx;
#include <boost/pool/poolfwd.hpp>
#include <boost/pool/singleton_pool.hpp>
#include "TPolyLine3D.h"
#include "TVirtualPad.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
//#include <lapacke.h>
#include "DHShower.h"
using namespace boost; 
typedef boost::singleton_pool<RecoHit, sizeof(RecoHit)> RecoHitPool;
typedef std::vector<RecoHit*>::iterator recit;

#include <HoughLocal.h>

#include "ChamberAnalyzer.h"

#define posError 0.5
#define STEP printf("%s %d\n",__FUNCTION__,__LINE__)
uint32_t TrackAnalyzer::PMAnalysis(uint32_t bifid)
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
      if (d->getID()!=bifid) continue;
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


	      std::map<unsigned int,DifGeom>::iterator idg = reader_->getDifMap().find(dc->getID());
	      DifGeom& difgeom = idg->second;
	      uint32_t chid = idg->second.getChamberId();
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
			      DifGeom::PadConvert(iasic,k,ip,jp,2);
			      hpatpos->Fill(ip*1.,jp*1.);
			       
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


uint32_t TrackAnalyzer::CerenkovTagger(uint32_t difid,uint32_t seed)
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

void TrackAnalyzer::initHistograms()
{
  //  rootHandler_->BookTH1("/Clusters/EST1",100,0.,300.);
}
TrackAnalyzer::TrackAnalyzer() :trackIndex_(0),nAnalyzed_(0),clockSynchCut_(8), spillSize_(90000),maxHitCount_(500000),minHitCount_(10),
									     tkMinPoint_(3),tkExtMinPoint_(3),tkBigClusterSize_(32),tkChi2Cut_(0.01),tkDistCut_(5.),tkExtChi2Cut_(0.01),tkExtDistCut_(10.),tkAngularCut_(20.),zLastAmas_(134.),
									     findTracks_(true),dropFirstSpillEvent_(false),useSynchronised_(true),chamberEdge_(5.),rebuild_(false),oldAlgo_(true),collectionName_("DHCALRawHits"),
									     tkFirstChamber_(1),tkLastChamber_(61),useTk4_(false),offTimePrescale_(1),houghIndex_(0),theRhcolTime_(0.),theTimeSortTime_(0.),theTrackingTime_(0),
									   theHistoTime_(0),theSeuil_(0),draw_(false),theSkip_(0),theMonitoring_(NULL),theMonitoringPeriod_(0),theMonitoringPath_("/dev/shm/Monitoring"),ntkbetween(0),theBCIDSpill_(0),theLastBCID_(0),theSpillLength_(8.)
{
  reader_=DHCalEventReader::instance();
  rootHandler_ =DCHistogramHandler::instance();
  this->initialise();
  HoughCut cuts;

  theComputerTrack_=new ComputerTrack(&cuts);
  theComputerTrack_->DefaultCuts();
  theMonitoring_=new SDHCALMonitor(reader_,rootHandler_);
}

TrackAnalyzer::TrackAnalyzer(DHCalEventReader* r,DCHistogramHandler* h) :trackIndex_(0),nAnalyzed_(0),clockSynchCut_(8), spillSize_(90000),maxHitCount_(500000),
									     tkMinPoint_(3),tkExtMinPoint_(3),tkBigClusterSize_(32),tkChi2Cut_(0.01),tkDistCut_(5.),tkExtChi2Cut_(0.01),tkExtDistCut_(10.),tkAngularCut_(20.),zLastAmas_(134.),
									     findTracks_(true),dropFirstSpillEvent_(false),useSynchronised_(true),chamberEdge_(5.),rebuild_(false),oldAlgo_(true),collectionName_("DHCALRawHits"),
									     tkFirstChamber_(1),tkLastChamber_(61),useTk4_(false),offTimePrescale_(1),houghIndex_(0),theRhcolTime_(0.),theTimeSortTime_(0.),theTrackingTime_(0),
									   theHistoTime_(0),theSeuil_(0),draw_(false),theSkip_(0),theMonitoring_(NULL),theMonitoringPeriod_(0),theMonitoringPath_("/dev/shm/Monitoring"),ntkbetween(0),theBCIDSpill_(0),theLastBCID_(0)
{
  reader_=r;
  rootHandler_ =h;
  this->initialise();

  HoughCut cuts;
  theComputerTrack_=new ComputerTrack(&cuts);

  theComputerTrack_->DefaultCuts();
  theMonitoring_=new SDHCALMonitor(r,h);
}

void TrackAnalyzer::initialise()
{
  headerWritten_=false;
  //  TVirtualFitter::SetDefaultFitter("Minuit"); 
  //  gPluginMgr->AddHandler("ROOT::Math::Minimizer", "Minuit", "TMinuitMinimizer", "Minuit", "TMinuitMinimizer(const char *)"); 
  integratedTime_=0;
  asicCount_.clear();
  DCBufferReader::setDAQ_BC_Period(0.2);
  
  theTime_.time=0;
  theTime_.millitm=0;
  theDb_=NULL;
  useSqlite_=false;
  useMysql_=false;
  theImage_.initialise(theImageBuffer_,60,96,96);
  theImageWeight_.initialise(theImageWeightBuffer_,60,96,96);

}
void TrackAnalyzer::initJob(){presetParameters();}
void TrackAnalyzer::endJob(){
  if (theMonitoringPeriod_!=0)
    {
      rootHandler_->writeXML(theMonitoringPath_);
      //char c;c=getchar();putchar(c); if (c=='.') exit(0);;
		
    }

  closeTrees();
	
	
}
void TrackAnalyzer::presetParameters()
{
  std::map<std::string,MarlinParameter> m=reader_->getMarlinParameterMap();
  std::map<std::string,MarlinParameter>::iterator it;
  try
    {
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
bool TrackAnalyzer::decodeTrigger(LCCollection* rhcol, double tcut)
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
  double tTrigger_= lbc*(DCBufferReader::getDAQ_BC_Period()*1E-6);
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

uint32_t TrackAnalyzer::NoiseStudy(std::map<uint32_t,std::bitset<255> > &timeDif,std::map<uint32_t,std::bitset<61> > &timeChamber)
{
  float n_dif[200],n_chamber[60];
  memset(n_dif,0,200*sizeof(float));
  memset(n_chamber,0,60*sizeof(float));
  double tmin=9999999;
  double tmax=-1;
  // Loop on DIFs
  for (std::map<uint32_t,std::bitset<255> >::iterator it=timeDif.begin();it!=timeDif.end();it++)
    {
      if (it->first<tmin) tmin=it->first;
      if (it->first>tmax) tmax=it->first;
      for (unsigned int ib=0;ib<200;ib++)
	if (it->second[ib]!=0) n_dif[ib]=n_dif[ib]+1;
    }
  if (tmax<=tmin) return 0;
  TH1* httmin= rootHandler_->GetTH1("TimeMin");
  TH1* httmax= rootHandler_->GetTH1("TimeMax");

  if (httmin==NULL)
    {
      httmin =rootHandler_->BookTH1( "TimeMin",2000,0.,4000000.);
      httmax =rootHandler_->BookTH1( "TimeMax",2000,0.,4000000.);

    }
  httmin->Fill(tmin*1.);
  httmax->Fill(tmax*1.);



  for (std::map<uint32_t,std::bitset<61> >::iterator it=timeChamber.begin();it!=timeChamber.end();it++)
    {
      for (unsigned int ib=0;ib<60;ib++)
	if (it->second[ib]!=0) n_chamber[ib]=n_chamber[ib]+1;

    }


  for (std::map<unsigned int,DifGeom>::iterator idg=reader_->getDifMap().begin();idg!=reader_->getDifMap().end();idg++)
    {
      n_dif[idg->first]=n_dif[idg->first]/((tmax-tmin)*DCBufferReader::getDAQ_BC_Period()*1.E-6*48*64);
      std::stringstream namec("");
      uint32_t chid = idg->second.getChamberId();
      namec<<"/Chamber"<<chid<<"/DIF"<<idg->first;

      TH1* hnoise = rootHandler_->GetTH1(namec.str()+"/NoiseFrequency");
      if (hnoise==NULL)
	{
	  hnoise =rootHandler_->BookTH1( namec.str()+"/NoiseFrequency",2000,0.,100.);
	}
      hnoise->Fill(n_dif[idg->first]);
    }


  for (std::map<unsigned int,ChamberGeom>::iterator idg=reader_->getChamberMap().begin();idg!=reader_->getChamberMap().end();idg++)
    {

      n_chamber[idg->first]=n_chamber[idg->first]/((tmax-tmin)*DCBufferReader::getDAQ_BC_Period()*1.E-6*144*64);
      std::stringstream namec("");
      namec<<"/Chamber"<<idg->first;

      TH1* hnoise = rootHandler_->GetTH1(namec.str()+"/NoiseFrequency");
      if (hnoise==NULL)
	{
	  hnoise =rootHandler_->BookTH1( namec.str()+"/NoiseFrequency",2000,0.,100.);
	}
      hnoise->Fill(n_chamber[idg->first]);

    }
  return int(tmax-tmin);
}


void TrackAnalyzer::DIFStudy(IMPL::RawCalorimeterHitImpl* hit)
{
  unsigned int difid = hit->getCellID0()&0xFF;
  std::map<unsigned int,DifGeom>::iterator idg = reader_->getDifMap().find(difid);
  DifGeom& difgeom = idg->second;
  uint32_t chid = idg->second.getChamberId();
  int asicid = (hit->getCellID0()&0xFF00)>>8;
  int channel= (hit->getCellID0()&0x3F0000)>>16;
  //streamlog_out(MESSAGE)<<"ch-"<<channel<<std::endl;
  unsigned int bc = hit->getTimeStamp();

  bool thr[3];
  //      DEBUG_PRINT("%x \n",hit->getCellID0());
  int ithr= hit->getAmplitude()&0x3;
  if (ithr<=0 || ithr>3)
    {
      std::cout<<difid<<" had:"<<asicid<<":"<<channel<<":"<<bc<<":"<<ithr<<std::endl;
      return;
    }
  thr[0] = (ithr == 1);
  thr[1] = (ithr == 2);
  thr[2] = (ithr == 3);
  int asic=asicid;int x,y;
  if (difid>1000) asic=(asic-1)%4+1; // Small chamber
  if (chid<49)
    DifGeom::PadConvert(asic,channel,x,y,2);
  else
    DifGeom::PadConvert(asic,channel,x,y,11);
  int difLocalI=int(x);
  int difLocalJ=int(y);
  int chamberLocalI=difgeom.toGlobalX(difLocalI);
  int chamberLocalJ=difgeom.toGlobalY(difLocalJ);

  //DEBUG_PRINT("%d %d %d %d %d %d \n",x,y,difLocalI,difLocalJ,chamberLocalI,chamberLocalJ);
  std::stringstream namec("");
  namec<<"/Chamber"<<chid<<"/DIF"<<difid;

  TH1* hhits0 = rootHandler_->GetTH1(namec.str()+"/Hits0");	   
  TH1* hhits1 = rootHandler_->GetTH1(namec.str()+"/Hits1");	   
  TH1* hhits2 = rootHandler_->GetTH1(namec.str()+"/Hits2");
  TH1* hetd = rootHandler_->GetTH1(namec.str()+"/EventTime");
  TH1* hetdz = rootHandler_->GetTH1(namec.str()+"/EventTimeZoom");
	
  if (hhits0==0)
    {
		
      hhits0 =rootHandler_->BookTH1( namec.str()+"/Hits0",48*64,0.1,48*64+0.1);
      hhits1 =rootHandler_->BookTH1( namec.str()+"/Hits1",48*64,0.1,48*64+0.1);
      hhits2 =rootHandler_->BookTH1( namec.str()+"/Hits2",48*64,0.1,48*64+0.1);
      hetd =rootHandler_->BookTH1(namec.str()+"/EventTime",10000,0.,15E6);
      hetdz =rootHandler_->BookTH1(namec.str()+"/EventTimeZoom",10000,0.,10000);
    }
  if (thr[0]||thr[2]) hhits0->SetBinContent((asic-1)*64+channel+1,hhits0->GetBinContent((asic-1)*64+channel+1)+1);
  if (thr[1]||thr[0]||thr[2]) hhits1->SetBinContent((asic-1)*64+channel+1,hhits1->GetBinContent((asic-1)*64+channel+1)+1);
  if (thr[2]) hhits2->SetBinContent((asic-1)*64+channel+1,hhits2->GetBinContent((asic-1)*64+channel+1)+1);
  hetd->Fill(bc*1.);

  hetdz->Fill(bc*1.);

  std::stringstream namech("");
  namech<<"/Chamber"<<chid;

  TH2* hthr0 = rootHandler_->GetTH2(namech.str()+"/Seuil0");
  TH2* hthr1 = rootHandler_->GetTH2(namech.str()+"/Seuil1");
  TH2* hthr2 = rootHandler_->GetTH2(namech.str()+"/Seuil2");
  if (hthr0==NULL)
    {
      hthr0 =rootHandler_->BookTH2( namech.str()+"/Seuil0",96,0.,96.,96,0.,96.);
      hthr1 =rootHandler_->BookTH2( namech.str()+"/Seuil1",96,0.,96.,96,0.,96.);
      hthr2 =rootHandler_->BookTH2( namech.str()+"/Seuil2",96,0.,96.,96,0.,96.);
    }
  if (thr[0]||thr[2]) hthr0->Fill(chamberLocalI*1.,chamberLocalJ*1.);
  if (thr[1]||thr[2]||thr[0]) hthr1->Fill(chamberLocalI*1.,chamberLocalJ*1.);
  if (thr[2]) hthr2->Fill(chamberLocalI*1.,chamberLocalJ*1.);


}

void TrackAnalyzer::FillTimeAsic(IMPL::LCCollectionVec* rhcol)
{
  DEBUG_PRINT("On rentre \n");
  //  std::map<uint32_t,uint32_t> count;
  //count.clear();
  TH1* hoccall= (TH1F*) rootHandler_->GetTH1("AsicOccupancy");
  TH1* hoccalldif= (TH1F*) rootHandler_->GetTH1("AsicOccupancyDIF");
  TH1* hoccallchamber= (TH1F*) rootHandler_->GetTH1("AsicOccupancyChamber");
	
  if (hoccall==0)
    {
      hoccall=rootHandler_->BookTH1("AsicOccupancy",255*48,0.,255*48.);
      hoccalldif=rootHandler_->BookTH1("AsicOccupancyDIF",255,0.,255.);
      hoccallchamber=rootHandler_->BookTH1("AsicOccupancyChamber",61,0.,61.);
    }
  hoccalldif->Reset();
  hoccallchamber->Reset();

  double tmin=99999999.;
  double tmax=0.;
  //IMPL::LCCollectionVec* rhcol=(IMPL::LCCollectionVec*) evt_->getCollection(collectionName_);
  for (int i=0;i<rhcol->getNumberOfElements();i++)
    {
      IMPL::RawCalorimeterHitImpl* hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(i);
      if (hit==0) continue;
      DIFStudy(hit);
      // Decode
      unsigned int difid = hit->getCellID0()&0xFF;
      int asicid = (hit->getCellID0()&0xFF00)>>8;
      int channel= (hit->getCellID0()&0x3F0000)>>16;
      unsigned int bc = hit->getTimeStamp();
      if (bc>5E6) continue;
      if (bc<tmin) tmin=bc;
      if (bc>tmax) tmax=bc;

	
      std::map<unsigned int,DifGeom>::iterator idg = reader_->getDifMap().find(difid);
      DifGeom& difgeom = idg->second;
      uint32_t chid = idg->second.getChamberId();
      uint32_t key=(chid<<16)|(difid<<8)|asicid;
     
      std::map<uint32_t,uint32_t>::iterator it=asicCount_.find(key);
      if (asicCount_.find(key)!=asicCount_.end()) 
	it->second=it->second+1;
      else
	{
	  uint32_t n=1;
	  std::pair<uint32_t,uint32_t> p(key,n);
	  asicCount_.insert(p);
	}
    }
  integratedTime_+=(tmax-tmin);
  std::cout<<tmin<<" "<<tmax<<" => Integrated time " <<integratedTime_<<std::endl;

  for (std::map<uint32_t,uint32_t>::iterator it=asicCount_.begin();it!=asicCount_.end();it++)
    {
      uint32_t chid =(it->first>>16)&0xFF;
      uint32_t difid =(it->first>>8)&0xFF;
      uint32_t asicid =(it->first)&0xFF;
		
      std::stringstream namec("");
      namec<<"/Chamber"<<chid<<"/DIF"<<difid;
		

      TH1* hocc= (TProfile*) rootHandler_->GetTH1(namec.str()+"/AsicOccupancy");	   
      TH1* hoccn= (TH1*) rootHandler_->GetTH1(namec.str()+"/AsicOccupancyNumber");	   
      if (hocc==0)
	{
			
	  hocc =rootHandler_->BookTH1( namec.str()+"/AsicOccupancy",48,0.,48.);
	  hoccn =rootHandler_->BookTH1( namec.str()+"/AsicOccupancyNumber",48,0.,48.);
	}

		
      hoccn->SetBinContent(asicid,it->second);
      hoccall->SetBinContent(difid*48+asicid,it->second/(integratedTime_*DCBufferReader::getDAQ_BC_Period()*1.E-6));
		
      hocc->SetBinContent(asicid,it->second/(integratedTime_*DCBufferReader::getDAQ_BC_Period()*1.E-6));
      float focc=it->second/(integratedTime_*DCBufferReader::getDAQ_BC_Period()*1.E-6);
      if (focc>hoccallchamber->GetBinContent(chid)) hoccallchamber->SetBinContent(chid,focc);
      if (focc>hoccalldif->GetBinContent(difid)) hoccalldif->SetBinContent(difid,focc);


    }
  TH1* htdiff= rootHandler_->GetTH1("TimeDif");

  if (htdiff==NULL)
    {
      htdiff =rootHandler_->BookTH1( "TimeDif",2000,0.,4000000.);

    }
  htdiff->Fill((tmax-tmin)*1.);
}
double  TrackAnalyzer::checkTime()
{

  ftime(&theCurrentTime_);  
  double dt=theCurrentTime_.time-theTime_.time+(theCurrentTime_.millitm-theTime_.millitm)*1E-3;
  theTime_.time= theCurrentTime_.time;  theTime_.millitm= theCurrentTime_.millitm;
  return dt;
}
void TrackAnalyzer::findTimeSeeds( IMPL::LCCollectionVec* rhcol, int32_t nhit_min,std::vector<uint32_t>& candidate)
{
  map<uint32_t,uint32_t> tcount;
  map<uint32_t,int32_t> tedge;

  // Tcount is the time histo
  for (uint32_t i=0;i<rhcol->getNumberOfElements();i++)
    {
      IMPL::RawCalorimeterHitImpl* hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(i);
      if (hit==0) continue;
      uint32_t bc = hit->getTimeStamp();
      map<uint32_t,uint32_t>::iterator it=tcount.find(bc);
      if (it!=tcount.end()) 
	it->second=it->second+1;
      else
	{
	  std::pair<uint32_t,uint32_t> p(bc,1);
	  tcount.insert(p);
	}
    }
	
  std::vector<uint32_t> seed;
  seed.clear();
	
  //d::cout<<"Size =>"<<tcount.size()<<std::endl;
  // Tedge is convolute with +1 -1 +1 apply to tcount[i-1],tcount[i],tcount[i+1]
  for (map<uint32_t,uint32_t>::iterator it=tcount.begin();it!=tcount.end();it++)
    {
      //std::cout<<it->first<<" "<<it->second<<std::endl;
		
      map<uint32_t,uint32_t>::iterator ita=tcount.find(it->first+1);
      map<uint32_t,uint32_t>::iterator itb=tcount.find(it->first-1);
      int32_t c=-1*it->second;
      if (ita!=tcount.end()) c+=ita->second;
      if (itb!=tcount.end()) c+=itb->second;
      std::pair<uint32_t,int32_t> p(it->first,c);
      tedge.insert(p);
		
    }
  //d::cout<<"Size Edge =>"<<tedge.size()<<std::endl;
  // Now ask for a minimal number of hits
  uint32_t nshti=0;
  for (map<uint32_t,int32_t>::iterator it=tedge.begin();it!=tedge.end();)
    {
      //std::cout<<it->first<<"====>"<<it->second<<" count="<<tcount[it->first]<<std::endl;
      if (it->second<-1*(nhit_min-2))
	{
			
	  //std::cout<<it->first<<"====>"<<it->second<<" count="<<tcount[it->first]<<std::endl;

	  seed.push_back(it->first);
	  it++;
	}
      else
	tedge.erase(it++);
    }
	
  // for (std::vector<uint32_t>::iterator is=seed.begin();is!=seed.end();is++)
  //   std::cout<<" seed " <<(*is)<<" count "<<tcount[(*is)]<<std::endl      ;
  // Merge adjacent seeds
  candidate.clear();
  for (uint32_t i=0;i<seed.size();)
    {
      if ((i+1)<=(seed.size()-1))
	{
	  if (seed[i+1]-seed[i]<=5)
	    {
	      //candidate.push_back(int((seed[i+1]+seed[i])/2));
	      uint32_t max_c=0;
	      uint32_t max_it=0;
	      uint32_t imin=seed[i];
	      uint32_t imax=seed[i+1];
	      if (seed[i+1]>seed[i])
		{
		}
	      for (uint32_t it=imin;it<=imax;it++)
		{
		  if (tcount.find(it)==tcount.end()) continue;
		  if (tcount[it]>max_c) {max_c=tcount[it];max_it=it;}
		}
	      if (max_it!=0)
		candidate.push_back(max_it);
	      else
		candidate.push_back(seed[i]);
	      i+=2;
	    }
	  else
	    {
	      candidate.push_back(seed[i]);
	      i++;
	    }
	}
      else
	{
	  candidate.push_back(seed[i]);
	  i++;
	}

		
    }
  //td::cout<<candidate.size()<<" good showers "<< tedge.size()<<std::endl;
  std::sort(candidate.begin(),candidate.end(),std::greater<uint32_t>());

  /*  
      for (std::vector<uint32_t>::iterator is=candidate.begin();is!=candidate.end();is++)
    std::cout<<(*is)<<" ---> "<<tcount[(*is)]<<std::endl;
  */
  return ;
}

void TrackAnalyzer::processSeed(IMPL::LCCollectionVec* rhcol,uint32_t seed)
{

 
  
  ShowerParams ish;
  ShowerParams isha;

  //std::vector<RecoHit*> vrh;

  currentTime_=seed;
  
  theAbsoluteTime_=theBCID_-currentTime_;
  if (theBCIDSpill_==0) theBCIDSpill_=theAbsoluteTime_;
  if (theAbsoluteTime_-theBCIDSpill_>theSpillLength_/2E-7) theBCIDSpill_=theAbsoluteTime_;

  //INFO_PRINT("GTC %d DTC %d BCID %llu Current Time %llu Time SPill %f Distance %f SL %f \n",theGTC_,theDTC_,theBCID_,currentTime_,theBCIDSpill_*2E-7,(theAbsoluteTime_-theBCIDSpill_)*2E-7,theSpillLength_/2E-7);
    theEvent_.idx++;
    theEvent_.bcid=theBCID_;
    theEvent_.dtc=theDTC_;
    theEvent_.gtc=theGTC_;
    theEvent_.run=evt_->getRunNumber();
    theEvent_.event=evt_->getEventNumber();
    theEvent_.time=currentTime_;
    theEvent_.energy=reader_->getBeamEnergy();
    theEvent_.tospill=(theAbsoluteTime_-theBCIDSpill_)*2E-7;
  // DEBUG_PRINT("Building voulume for %d \n",seed);
  //uint32_t nhits=buildVolume(rhcol,seed);
  //  DEBUG_ DEBUG_PRINT("1");
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

   if (theNplans_<minChambersInTime_) return;
   
  //printf("TAG=======================> %d \n",tag);
  TH1* hnoctag= rootHandler_->GetTH1("NoCTag");
  TH1* hctag1= rootHandler_->GetTH1("CTag1");
  TH1* hctag2= rootHandler_->GetTH1("CTag2");
  TH1* hctag3= rootHandler_->GetTH1("CTag3");
  TH1* hpatnotag= rootHandler_->GetTH1("PatternNoTag");
  TH1* hpattag= rootHandler_->GetTH1("PatternTag");
  TH1* hr012= rootHandler_->GetTH1("r012");
  if (hnoctag==NULL)
    {
      hnoctag =rootHandler_->BookTH1( "NoCTag",1000,0.,3000.);
      hctag1 =rootHandler_->BookTH1( "CTag1",100,0.,300.);
      hctag2 =rootHandler_->BookTH1( "CTag2",100,0.,300.);
      hctag3 =rootHandler_->BookTH1( "CTag3",100,0.,300.);
      hpatnotag =rootHandler_->BookTH1( "PatternNoTag",100,0.1,100.1);
      hpattag =rootHandler_->BookTH1( "PatternTag",100,0.1,100.1);
      hr012 =rootHandler_->BookTH1( "r012",500,0.,0.5);

    }
  //this->drawHits(theHitVector_);getchar();  
  
  if (theCerenkovTag_==0)
    {
      //std::cout<<chhit<<"->no tag"<<std::endl;
      
      for(int i=0;i<64;i++)
	{
	  if (chhit[i]!=0) hpatnotag->Fill(i*1.);
	}
      hpatnotag->Fill(100.);
    }
  else
    {
      for(int i=0;i<64;i++)
	{
	  if (chhit[i]!=0) hpattag->Fill(i*1.);
	}
      hpattag->Fill(100.);
    }
  if (tag==1)
    hctag1->Fill(theHitVector_.size());
  if (tag==2)
    hctag2->Fill(theHitVector_.size());
  if (tag==3)
    hctag3->Fill(theHitVector_.size());

  //this->drawHits(theHitVector_);
  
  if (theTkHitVector_.size()>8000) return;
  if (theNplans_<minChambersInTime_) return;
  if (theTkHitVector_.size()<theNplans_) return;
  
  //INFO_PRINT(" %d Hit vector %d  tk %d pl %d \n",seed,theHitVector_.size(),theTkHitVector_.size(),theNplans_);
  //Shower::computePrincipalComponents(theTkHitVector_,(double*) &isha);
  
  
  //Shower::PrintComponents(isha);
  Shower::TPrincipalComponents(theTkHitVector_,(double*) &isha);
  
  // Shower::PrintComponents(isha);
  hr012->Fill(sqrt((isha.lambda[0]+isha.lambda[1])/isha.lambda[2]));
  //for (std::vector<RecoHit*>::iterator it=theHitVector_.begin();it!=theHitVector_.end();it++) delete((*it));

  if (sqrt((isha.lambda[0]+isha.lambda[1])/isha.lambda[2])>0.1)
    {
      
      if (interactionClusters_.size()>=1)
	{

	TH1* hnrc= rootHandler_->GetTH1("/Topology/NumberOfRealCluster");
	TH1* hnic= rootHandler_->GetTH1("/Topology/NumberOfInteractionCluster");
	TH1* hnrp= rootHandler_->GetTH1("/Topology/NumberOfRealPlane");
	TH1* hnip= rootHandler_->GetTH1("/Topology/NumberOfInteractionPlane");
	TH1* hnicratio= rootHandler_->GetTH1("/Topology/ProportionOfInteractionCluster");
	TH2* hnhitvsnic=rootHandler_->GetTH2("/Topology/HitVsInteractionCluster");
	TH2* hnhitvsnip=rootHandler_->GetTH2("/Topology/HitVsInteractionPlan");
	TH2* hnhitvsnrp=rootHandler_->GetTH2("/Topology/HitVsRealPlan");
	TH2* hnipvsnrp=rootHandler_->GetTH2("/Topology/InteractionPlanVsRealPlan");
	TH1* hmtl= rootHandler_->GetTH1("/Topology/MipsTrackLength");
	if (hnrc==NULL)
	  {
	    hnrc =rootHandler_->BookTH1( "/Topology/NumberOfRealCluster",400,0.,400.);
	    hnic =rootHandler_->BookTH1( "/Topology/NumberOfInteractionCluster",100,0,100.);
	    hnicratio =rootHandler_->BookTH1("/Topology/ProportionOfInteractionCluster",100,0.,1.);
	    hnhitvsnic =rootHandler_->BookTH2( "/Topology/HitVsInteractionCluster",100,0.,100.,1000,0.,2000.);
	    hnhitvsnip =rootHandler_->BookTH2( "/Topology/HitVsInteractionPlan",64,0.,64.,1000,0.,2000.);
	    hnhitvsnrp =rootHandler_->BookTH2( "/Topology/HitVsRealPlan",64,0.,64.,1000,0.,2000.);
	    hnipvsnrp =rootHandler_->BookTH2( "/Topology/InteractionPlanVsRealPlan",64,0.,64.,64,0.,64);
	    hnrp =rootHandler_->BookTH1( "/Topology/NumberOfRealPlane",64,0.,64.);
	    hnip =rootHandler_->BookTH1( "/Topology/NumberOfInteractionPlane",64,0.,64.);
	    hmtl =rootHandler_->BookTH1( "/Topology/MipsTrackLength",200,0.,200.);
	    

	    
	  }
	hmtl->Fill(theComputerTrack_->Length());
	hnrc->Fill(realClusters_.size()*1.);
	hnic->Fill(interactionClusters_.size()*1.);
	hnrp->Fill(nPlansReal_.count()*1.);
	hnip->Fill(nPlansInteraction_.count()*1.);
	if (nPlansInteraction_.count()>4 && (firstInteractionPlane_>4 || nPlansAll_.count()>30)
	    && theHitVector_.size()*1./nPlansAll_.count()>2.2 )
	  hnicratio->Fill(nPlansInteraction_.count()*1./nPlansAll_.count());
	hnhitvsnic->Fill(interactionClusters_.size()*1.,theHitVector_.size()*1.);
	hnhitvsnip->Fill(nPlansInteraction_.count()*1.,theHitVector_.size()*1.);
	hnhitvsnrp->Fill(nPlansReal_.count()*1.,theHitVector_.size()*1.);
	hnipvsnrp->Fill(nPlansReal_.count()*1.,nPlansInteraction_.count()*1.);

	theEvent_.tracklength=theComputerTrack_->Length();
	theEvent_.rclu=realClusters_.size();
	theEvent_.iclu=interactionClusters_.size();
	theEvent_.rplan=nPlansReal_.count();
	theEvent_.iplan=nPlansInteraction_.count();
	theEvent_.aplan=nPlansAll_.count();
	theEvent_.bsplan=nPlansAll_.to_ulong();
	theEvent_.bsrplan=nPlansReal_.to_ulong();
	theEvent_.bsiplan=nPlansInteraction_.to_ulong();
	theEvent_.fpi= firstInteractionPlane_;
	theEvent_.lpi=lastInteractionPlane_;


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
	theEvent_.x0=bx;
	theEvent_.y0=by;
	


	uint32_t counts[3][5];
	uint32_t scounts[3][5];
	uint32_t ir;
	memset(counts,0,15*sizeof(uint32_t));
	memset(scounts,0,15*sizeof(uint32_t));
	for (std::vector<RecoHit*>::iterator ih=theHitVector_.begin();ih!=theHitVector_.end();ih++)
	  {
	    if ((*ih)->getFlag(RecoHit::THR0)) ir=0;
	    if ((*ih)->getFlag(RecoHit::THR1)) ir=1;
	    if ((*ih)->getFlag(RecoHit::THR2)) ir=2;
	    counts[ir][0]++;
	    if ((*ih)->plan()>=firstInteractionPlane_) 	    scounts[ir][0]++;
	    if ((*ih)->getFlag(RecoHit::MIP)!=0) 
	      {
	      counts[ir][1]++;
	      if ((*ih)->plan()>=firstInteractionPlane_) scounts[ir][1]++;
	      }
	    else 
	      {
		if ((*ih)->getFlag(RecoHit::CORE)!=0) counts[ir][2]++;
		if ((*ih)->getFlag(RecoHit::EDGE)!=0) counts[ir][3]++;
		if ((*ih)->getFlag(RecoHit::ISOLATED)!=0) counts[ir][4]++;
		if ((*ih)->plan()>=firstInteractionPlane_)
		  {
		    if ((*ih)->getFlag(RecoHit::CORE)!=0) scounts[ir][2]++;
		    if ((*ih)->getFlag(RecoHit::EDGE)!=0) scounts[ir][3]++;
		    if ((*ih)->getFlag(RecoHit::ISOLATED)!=0) scounts[ir][4]++;

		  }
	      }
	  }
	theEvent_.m0=counts[0][1];
	theEvent_.c0=counts[0][2];
	theEvent_.e0=counts[0][3];
	theEvent_.i0=counts[0][4];
	theEvent_.m1=counts[1][1];
	theEvent_.c1=counts[1][2];
	theEvent_.e1=counts[1][3];
	theEvent_.i1=counts[1][4];
	theEvent_.m2=counts[2][1];
	theEvent_.c2=counts[2][2];
	theEvent_.e2=counts[2][3];
	theEvent_.i2=counts[2][4];
	theEvent_.sm0=scounts[0][1];
	theEvent_.sc0=scounts[0][2];
	theEvent_.se0=scounts[0][3];
	theEvent_.si0=scounts[0][4];
	theEvent_.sm1=scounts[1][1];
	theEvent_.sc1=scounts[1][2];
	theEvent_.se1=scounts[1][3];
	theEvent_.si1=scounts[1][4];
	theEvent_.sm2=scounts[2][1];
	theEvent_.sc2=scounts[2][2];
	theEvent_.se2=scounts[2][3];
	theEvent_.si2=scounts[2][4];
	theEvent_.npoint=counts[0][0]+counts[1][0]+counts[2][0];

	tEvents_->Fill();
	
	if (nPlansInteraction_.count()>4 && (firstInteractionPlane_>4 || nPlansAll_.count()>30)
	    && theHitVector_.size()*1./nPlansAll_.count()>2.2 && theComputerTrack_->Length()>5 &&
	    (theAbsoluteTime_-theBCIDSpill_<3E7))
	  hnoctag->Fill(theHitVector_.size());	  
	}
      //this->drawHits(theTkHitVector_,&isha);
      //char c;c=getchar();putchar(c); if (c=='.') exit(0);

      return;
    }
  //INFO_PRINT(" Mean event parameter %f %f %f => %f \n",isha.lambda[0],isha.lambda[1],isha.lambda[2],sqrt((isha.lambda[0]+isha.lambda[1])/isha.lambda[2]));
  //this->drawHits(theHitVector_,&isha);
  //char c;c=getchar();putchar(c); if (c=='.') exit(0);
  
  this->buildPrincipal(theTkHitVector_,"/TrackPrincipal");
  
  //this->buildTracks(theTkHitVector_,"/TrackNoCut");
  return;
  if (theComputerTrack_->getTracks().size()>0) {
  INFO_PRINT("  Track Found Mean event parameter %f %f %f => %f \n",isha.lambda[0],isha.lambda[1],isha.lambda[2],sqrt((isha.lambda[0]+isha.lambda[1])/isha.lambda[2]));
  //char c;c=getchar();putchar(c); if (c=='.') exit(0);

  }
  return;
  if (tag!=0)
    {
      
      if (tag==1 || tag==3)
	this->buildTracks(theHitVector_,"/TrackPM1");
      if (tag==2|| tag==3)
	this->buildTracks(theHitVector_,"/TrackPM2");
      if (tag==3)
	this->buildTracks(theHitVector_,"/TrackPM_BOTH");

      //INFO_PRINT(" Mean event parameter  %d %f %f %f => %f %d TKS \n",theHitVector_.size(),isha.lambda[0],isha.lambda[1],isha.lambda[2],sqrt((isha.lambda[0]+isha.lambda[1])/isha.lambda[2]),theComputerTrack_->getTracks().size()); 
   
      //  DEBUG_PRINT("6\n");
      if (theComputerTrack_->getTracks().size()>0) tag+=4;

      if (theComputerTrack_->getTracks().size()==0 && false)
	{
	}
      return;

    }
  else
    if (tag==0)
      {
	this->buildTracks(theHitVector_,"/TrackNOPM");
	if (theHitVector_.size()>500 && false)
	  {this->drawHits(theHitVector_);getchar();  }
      return;
    }
 
  return;

}

void TrackAnalyzer::processEvent()
{

  
  checkTime();
  if (reader_->getEvent()==0) return;
    
  evt_=reader_->getEvent();
  //theSkip_=380;
  
  if (evt_->getEventNumber()<=theSkip_) return;
  
  printf("Processing %d - %d \n",evt_->getRunNumber(),evt_->getEventNumber());
  if (evt_->getRunNumber()!=_runNumber)
    {
      _runNumber=evt_->getRunNumber();
      reader_->logbookBeamEnergy(_runNumber);

    }

  if (nAnalyzed_==0)
    {
      std::stringstream s;
      s<<"./Shower"<<evt_->getRunNumber()<<"_"<<reader_->getBeamEnergy()<<".root";
      std::cout<<" Tree file "<<s.str()<<std::endl;
      this->createTrees(s.str());
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
      evt_->addCollection(rhcol,"DHCALRawHits");
      rhcoltransient=false; 

    }
  else
    rhcol=(IMPL::LCCollectionVec*) evt_->getCollection(collectionName_);

  
  //INFO_PRINT("End of CreaetRaw %d \n",rhcol->getNumberOfElements());  
  if (rhcol->getNumberOfElements()>4E6) return;
  
  theMonitoring_->FillTimeAsic(rhcol);

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
  
  reader_->findTimeSeeds(minChambersInTime_);
  
  std::vector<uint32_t> vseeds=reader_->getTimeSeeds();

  
   INFO_PRINT("================>  %d  Number of seeds %d \n",evt_->getEventNumber(),(int) vseeds.size());

  if (vseeds.size()==0)  { if (rhcoltransient) delete rhcol;return;}
   
  theNbShowers_=0;
  theNbTracks_=0;
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
  theCountSpill_[theIdxSpill_%10] =  theNbShowers_;
  theTimeInSpill_[theIdxSpill_%10] = theMonitoring_->getEventIntegratedTime()*2E-7;

  // Integrated 10 last
  float nc=0;
  float tc=0;
  DEBUG_PRINT("showers %uud %d %d ",theIdxSpill_,theNbShowers_,theNbTracks_);
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
void TrackAnalyzer::drawDisplay()
{

  TH3* hcgposi = rootHandler_->GetTH3("InstantClusterMap");
  TH3* hcgposi1 = rootHandler_->GetTH3("InstantClusterMap1");
  TH3* hcgposi2 = rootHandler_->GetTH3("InstantClusterMap2");
  TH3* hcgposi3 = rootHandler_->GetTH3("InstantClusterMap3");

  if (hcgposi==NULL)
    {
      hcgposi =rootHandler_->BookTH3("InstantClusterMap",52,-2.8,145.6,100,0.,100.,100,0.,100.);
      hcgposi1 =rootHandler_->BookTH3("InstantClusterMap1",52,-2.8,145.6,100,0.,100.,100,0.,100.);
      hcgposi2 =rootHandler_->BookTH3("InstantClusterMap2",52,-2.8,145.6,100,0.,100.,100,0.,100.);
      hcgposi3 =rootHandler_->BookTH3("InstantClusterMap3",52,-2.8,145.6,100,0.,100.,100,0.,100.);
    }
  else
    {
      hcgposi->Reset();
      hcgposi1->Reset();
      hcgposi2->Reset();
      hcgposi3->Reset();
    }

  if (hcgposi!=0 )
    {
      hcgposi->Reset();
      for (unsigned int i =0;i<allpoints_.size();i++)
	{
	  //if (allpoints_[i].Charge()<7) continue;
	  hcgposi->Fill(allpoints_[i].Z(),allpoints_[i].X(),allpoints_[i].Y());
	  RECOCluster& c=allpoints_[i].getCluster();
	  if (c.getHits()->size()>4) continue;
	  for (std::vector<RecoHit>::iterator iht=c.getHits()->begin();iht!=c.getHits()->end();iht++)
	    {
	      int ithr= (*iht).getAmplitude()&0x3;
	      if (ithr==3) 
		hcgposi3->Fill((*iht).Z(),(*iht).X(),(*iht).Y());
	      else 
		if (ithr==1) 
		  hcgposi2->Fill((*iht).Z(),(*iht).X(),(*iht).Y());
		else
		  hcgposi1->Fill((*iht).Z(),(*iht).X(),(*iht).Y());
	    }
	}


      if (TCPlot==NULL)
	{
	  TCPlot=new TCanvas("TCPlot","tcplot1",1300,600);
	  TCPlot->Modified();
	  TCPlot->Draw();
	  TCPlot->Divide(2,2);
	}
      TCPlot->cd(1);
      hcgposi1->SetMarkerStyle(25);
      hcgposi2->SetMarkerStyle(25);
      hcgposi3->SetMarkerStyle(25);
      hcgposi1->SetMarkerSize(.2);
      hcgposi2->SetMarkerSize(.2);
      hcgposi3->SetMarkerSize(.2);
      hcgposi1->SetMarkerColor(kGreen);

      hcgposi2->SetMarkerColor(kBlue);
      hcgposi3->SetMarkerColor(kRed);
      hcgposi1->Draw("p");
      hcgposi2->Draw("pSAME");
      hcgposi3->Draw("pSAME");

      TCPlot->cd(2);
      TProfile2D* hpx1=hcgposi1->Project3DProfile("yx");
      hpx1->SetLineColor(kGreen);
		
      hpx1->Draw("BOX");
      TProfile2D* hpx2=hcgposi2->Project3DProfile("yx");
      hpx2->SetLineColor(kBlue);
		

      hpx2->Draw("BOXSAME");
      TProfile2D* hpx3=hcgposi3->Project3DProfile("yx");
      hpx3->SetLineColor(kRed);
		

      hpx3->Draw("BOXSAME");
      for (unsigned int i=0;i<tkgood_.size();i++)
	{
	  RecoCandTk& tk = tkgood_[i];
	  DEBUG_PRINT("%f %d \n",tk.chi2_,tk.getList().size());
	  TLine* l = new TLine(-1.,tk.getXext(-1.),132.,tk.getXext(132));
	  l->SetLineColor(2);
	  l->Draw("SAME");
	}
      //   std::vector<uint32_t> vmax;
      // 	  std::vector<float> vtheta;
      // 	  std::vector<float> vr;
      // 	  theHTx_->findMaxima(vmax,vtheta,vr);
      // 	  for (uint32_t iht=0;iht<vmax.size();iht++)
      // 	    {
      // 	      float ax_= -1./tan(vtheta[iht]);
      // 	      float bx_= vr[iht]/sin(vtheta[iht])-50.*ax_;
      // 	      DEBUG_PRINT("%d %d %f %f\n",iht,vmax[iht],ax_,bx_);
      // 	      TLine* l = new TLine(-1.,ax_*-1.+bx_,132.,ax_*132+bx_);
      // 	      l->SetLineColor(3);
      // 	      l->Draw("SAME");
      // 	    }
      TCPlot->cd(3);
      TProfile2D* hpy1=hcgposi1->Project3DProfile("zx");
      hpy1->SetLineColor(kGreen);
		

      hpy1->Draw("BOX");
      TProfile2D* hpy2=hcgposi2->Project3DProfile("zx");
      hpy1->SetLineColor(kBlue);
		

      hpy2->Draw("BOXSAME");
      TProfile2D* hpy3=hcgposi3->Project3DProfile("zx");
      hpy3->SetLineColor(kRed);
		

      hpy3->Draw("BOXSAME");

      //hcgposi->Project3DProfile("zx")->Draw("P");
      for (unsigned int i=0;i<tkgood_.size();i++)
	{
	  RecoCandTk& tk = tkgood_[i];
	  TLine* l=new TLine(-1.,tk.getYext(-1.),132.,tk.getYext(132));
	  l->SetLineColor(2);
	  l->Draw("SAME");
	}
      TCPlot->cd(4);
      hcgposi->Project3DProfile("yz")->Draw("Box");

      // double zplane[4]={0.,6.,83.1,131.7};
      // for (int ip=0;ip<4;ip++)
      // 	{
      // 	  TCPlot->cd(4+ip);
      // 	  std::stringstream name("");
      // 	  name<<"chamber"<<ip;
      // 	  TH2F* hch = new TH2F(name.str().c_str(),name.str().c_str(),96,0.,96.,96,0.,96.);
      // 	  for (unsigned int i=0;i<tkgood_.size();i++)
      // 	    {
      // 	      RecoCandTk& tk = tkgood_[i];
      // 	      for (unsigned int irp=0;irp<tk.getList().size();irp++)
      // 		{
      // 		  RecoPoint& r=tk.getList()[irp];
      // 		  if (fabs(r.Z()-zplane[ip])>0.1) continue;
      // 		  RECOCluster& cl=r.getCluster();
      // 		  for (unsigned int ih=0;ih<cl.getHits()->size();ih++)
      // 		    hch->Fill((*cl.getHits())[ih].first*1.,(*cl.getHits())[ih].second*1.);
      // 		}
      // 	    }
      // 	  hch->Draw("box");
      // 	}
      TCPlot->Modified();
      TCPlot->Draw();
      TCPlot->Update();
      //::usleep(2);
      //std::stringstream ss("");
      //ss<<"/tmp/Display_"<<evt_->getRunNumber()<<"_"<<evt_->getEventNumber()<<"_"<<currentTime_<<".png";
      //TCPlot->SaveAs(ss.str().c_str());
      //char cmd[256];
      //sprintf(cmd,"display %s",ss.str().c_str());
      // system(cmd)
      //delete c;
    }

}
void TrackAnalyzer::drawHits(std::vector<RecoHit*> vrh,ShowerParams* ish)
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

  if (hcgposi!=0 )
    {
      hcgposi->Reset();
      for (std::vector<RecoHit*>::iterator ih=vrh.begin();ih!=vrh.end();ih++)
	{
	  //if (allpoints_[i].Charge()<7) continue;
	  ChamberPos& cp=reader_->getPosition((*ih)->chamber());
	  //	  printf(" Chamber %d (%f,%f,%f) (%f,%f,%f) (%d,%d) \n",
	  //	 cp.getId(),cp.getX0(),cp.getY0(),cp.getZ0(),cp.getX1(),cp.getY1(),cp.getZ1(),cp.getXsize(),cp.getYsize());
	  double x,y,z;
	  cp.calculateGlobal((*ih)->X(),(*ih)->Y(),x,y,z);
	  hcgposi->Fill(z,x,y);//(*ih)->Z(),(*ih)->X(),(*ih)->Y());

	  //printf("HIT %d %d  Global (%f,%f,%f) from Local (%f,%f) \n",(*ih)->dif(),(*ih)->getAsic(),z,x,y,(*ih)->X(),(*ih)->Y());
	}


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

      TCHits->cd(2);
      TProfile2D* hpy1=hcgposi->Project3DProfile("zx");
      hpy1->SetLineColor(kGreen);
      hpy1->Draw("BOX");
      if (ish!=0)
	{
	  double* x=ish->xm;
	  double* v=ish->l2;
	  // Z X
	  double x0=x[2];
	  double y0=x[1];
	  double x1=x[2]+v[2];
	  double y1=x[1]+v[1];
	  double a=(y1-y0)/(x1-x0);
	  double b=y1-a*x1;
	  TLine* l = new TLine(0.,b,100.,a*100+b);
	  l->SetLineColor(4);
	  l->Draw("SAME");
	}


      TCHits->cd(3);
      TProfile2D* hpy2=hcgposi->Project3DProfile("yx");
      hpy2->SetLineColor(kBlue);
		

      hpy2->Draw("BOX");
      if (ish!=0)
	{
	  double* x=ish->xm;
	  double* v=ish->l2;
	  // Z X
	  double x0=x[2];
	  double y0=x[0];
	  double x1=x[2]+v[2];
	  double y1=x[0]+v[0];
	  double a=(y1-y0)/(x1-x0);
	  double b=y1-a*x1;
	  TLine* l = new TLine(0.,b,100.,a*100+b);
	  l->SetLineColor(4);
	  l->Draw("SAME");
	}



      TCHits->cd(4);
      TCHits->Modified();
      TCHits->Draw();
      TCHits->Update();
    }

}



#define DBG printf("%d %s\n",__LINE__,__PRETTY_FUNCTION__);

void TrackAnalyzer::createTrees(std::string s)
{

  treeFile_ = new TFile(s.c_str(),"recreate");
  treeFile_->cd();

  tEvents_ = new TTree("events","Events");

  theEvent_.idx=0;

  tEvents_->Branch("bcid",&theEvent_.bcid,"bcid/l");
  tEvents_->Branch("idx",&theEvent_.idx,"idx/I");

  tEvents_->Branch("energy",&theEvent_.energy,"energy/D");
  tEvents_->Branch("x0",&theEvent_.x0,"x0/D");
  tEvents_->Branch("y0",&theEvent_.y0,"y0/D");
  tEvents_->Branch("run",&theEvent_.run,"run/i");
  tEvents_->Branch("event",&theEvent_.event,"event/i ");
  tEvents_->Branch("gtc",&theEvent_.gtc,"gtc/i");
  tEvents_->Branch("dtc",&theEvent_.dtc,"dtc/i");
  tEvents_->Branch("time",&theEvent_.time,"time/i ");
  tEvents_->Branch("tospill",&theEvent_.tospill,"tospill/D");
  tEvents_->Branch("tracklength",&theEvent_.tracklength,"tracklength/D");
  tEvents_->Branch("iclu",&theEvent_.iclu,"iclu/i");
  tEvents_->Branch("rclu",&theEvent_.rclu,"rclu/i");
  tEvents_->Branch("aplan",&theEvent_.aplan,"aplan/i");
  tEvents_->Branch("iplan",&theEvent_.iplan,"iplan/i");
  tEvents_->Branch("rplan",&theEvent_.rplan,"rplan/i");
  tEvents_->Branch("bsplan",&theEvent_.bsplan,"bsplan/l");
  tEvents_->Branch("bsiplan",&theEvent_.bsiplan,"bsiplan/l");
  tEvents_->Branch("bsrplan",&theEvent_.bsiplan,"bsrplan/l");


  tEvents_->Branch("npoint",&theEvent_.npoint,"npoint/i ");
  tEvents_->Branch("allpoints",&theEvent_.allpoints,"allpoints/i");
  tEvents_->Branch("ntrack",&theEvent_.ntrack,"ntrack/s");
  tEvents_->Branch("allshowers",&theEvent_.allshowers,"allshowers/s");
  tEvents_->Branch("showers",&theEvent_.showers,"showers/s");
  tEvents_->Branch("type",&theEvent_.type,"type/b");

  tEvents_->Branch("fpi",&theEvent_.fpi,"fpi/b");
  tEvents_->Branch("lpi",&theEvent_.lpi,"lpi/b");
  tEvents_->Branch("m0",&theEvent_.m0,"m0/s");
  tEvents_->Branch("m1",&theEvent_.m1,"m1/s");
  tEvents_->Branch("m2",&theEvent_.m2,"m2/s");
  tEvents_->Branch("e0",&theEvent_.e0,"e0/s");
  tEvents_->Branch("e1",&theEvent_.e1,"e1/s");
  tEvents_->Branch("e2",&theEvent_.e2,"e2/s");
  tEvents_->Branch("c0",&theEvent_.c0,"c0/s");
  tEvents_->Branch("c1",&theEvent_.c1,"c1/s");
  tEvents_->Branch("c2",&theEvent_.c2,"c2/s");
  tEvents_->Branch("i0",&theEvent_.i0,"i0/s");
  tEvents_->Branch("i1",&theEvent_.i1,"i1/s");
  tEvents_->Branch("i2",&theEvent_.i2,"i2/s");
  tEvents_->Branch("sm0",&theEvent_.sm0,"sm0/s");
  tEvents_->Branch("sm1",&theEvent_.sm1,"sm1/s");
  tEvents_->Branch("sm2",&theEvent_.sm2,"sm2/s");
  tEvents_->Branch("se0",&theEvent_.se0,"se0/s");
  tEvents_->Branch("se1",&theEvent_.se1,"se1/s");
  tEvents_->Branch("se2",&theEvent_.se2,"se2/s");
  tEvents_->Branch("sc0",&theEvent_.sc0,"sc0/s");
  tEvents_->Branch("sc1",&theEvent_.sc1,"sc1/s");
  tEvents_->Branch("sc2",&theEvent_.sc2,"sc2/s");
  tEvents_->Branch("si0",&theEvent_.si0,"si0/s");
  tEvents_->Branch("si1",&theEvent_.si1,"si1/s");
  tEvents_->Branch("si2",&theEvent_.si2,"si2/s");

#ifdef USE_SHOWERS_TREE
  tShowers_ = new TTree("showers","Showers");

  tShowers_->Branch("bcid",&theShower_.bcid,"bcid/l");
  tShowers_->Branch("xm",&theShower_.xm,"xm[3]/D ");
  tShowers_->Branch("lambda",&theShower_.lambda,"lambda[3]/D");
  tShowers_->Branch("v1",&theShower_.v1,"v1[3]/D");
  tShowers_->Branch("v2",&theShower_.v2,"v2[3]/D");
  tShowers_->Branch("v3",&theShower_.v3,"v3[3]/D");
  tShowers_->Branch("rxm",&theShower_.rxm,"rxm[3]/D ");
  tShowers_->Branch("rlambda",&theShower_.rlambda,"rlambda[3]/D");
  tShowers_->Branch("rv1",&theShower_.rv1,"rv1[3]/D");
  tShowers_->Branch("rv2",&theShower_.rv2,"rv2[3]/D");
  tShowers_->Branch("rv3",&theShower_.rv3,"rv3[3]/D");
  tShowers_->Branch("eventid",&theShower_.eventid,"eventid/I");
  tShowers_->Branch("gtc",&theShower_.gtc,"gtc/I");
  tShowers_->Branch("showers",&theShower_.showers,"showers/I");
  tShowers_->Branch("idx",&theShower_.idx,"idx/i");
  tShowers_->Branch("time",&theShower_.time,"time/i ");
  tShowers_->Branch("nhit",&theShower_.nhit,"nhit[3]/s");
  tShowers_->Branch("rnhit",&theShower_.rnhit,"rnhit[3]/s");
  tShowers_->Branch("rncor",&theShower_.rncor,"rncor[3]/D");


  tShowers_->Branch("xb1",&theShower_.xb1,"xb1/D");
  tShowers_->Branch("yb1",&theShower_.yb1,"yb1/D");
  tShowers_->Branch("maxlb1",&theShower_.maxlb1,"maxlb1/D");
  tShowers_->Branch("rbs",&theShower_.rbs,"rbs/D");
  tShowers_->Branch("rbt",&theShower_.rbt,"rbt/D");
  tShowers_->Branch("n9",&theShower_.n9,"n9/D");
  tShowers_->Branch("n25",&theShower_.n25,"n25/D");
  tShowers_->Branch("ib1",&theShower_.ib1,"ib1/l");


  tShowers_->Branch("plan0",&theShower_.plan0,"plan0[60]/s");
  tShowers_->Branch("plan1",&theShower_.plan1,"plan1[60]/s");
  tShowers_->Branch("plan2",&theShower_.plan2,"plan2[60]/s");
  tShowers_->Branch("firstplan",&theShower_.firstplan,"firstplan/b");
  tShowers_->Branch("lastplan",&theShower_.lastplan,"lastplan/b");
  tShowers_->Branch("np1",&theShower_.np1,"np1/b");
  tShowers_->Branch("fp1",&theShower_.fp1,"fp1/b");
  tShowers_->Branch("lp1",&theShower_.lp1,"lp1/b");


  tShowers_->Branch("fd",&theShower_.fd,"fd[4]/D");
  tShowers_->Branch("NH0",&theShower_.NH0,"NH0[8]/s");
  tShowers_->Branch("NH1",&theShower_.NH1,"NH1[8]/s");
  tShowers_->Branch("NH2",&theShower_.NH2,"NH2[8]/s");
  tShowers_->Branch("NH",&theShower_.NH,"NH[8]/s");


  tShowers_->Branch("nc",&theShower_.nc,"nc[3]/i");
  tShowers_->Branch("ne",&theShower_.ne,"ne[3]/i");
  tShowers_->Branch("nm",&theShower_.nm,"nm[3]/i");
  tShowers_->Branch("namas",&theShower_.namas,"namas/i");
  tShowers_->Branch("ngood",&theShower_.ngood,"ngood/i");
  tShowers_->Branch("nhitafter",&theShower_.nhitafterlast,"nhitafter/i");
  tShowers_->Branch("zfirst",&theShower_.zfirst,"zfirst/D");
  tShowers_->Branch("zlast",&theShower_.zlast,"zlast/D");
  tShowers_->Branch("tklen",&theShower_.tklen,"tklen/D");

  tShowers_->Branch("m0",&theShower_.m0,"m0/s");
  tShowers_->Branch("m1",&theShower_.m1,"m1/s");
  tShowers_->Branch("m2",&theShower_.m2,"m2/s");
  tShowers_->Branch("e0",&theShower_.e0,"e0/s");
  tShowers_->Branch("e1",&theShower_.e1,"e1/s");
  tShowers_->Branch("e2",&theShower_.e2,"e2/s");
  tShowers_->Branch("c0",&theShower_.c0,"c0/s");
  tShowers_->Branch("c1",&theShower_.c1,"c1/s");
  tShowers_->Branch("c2",&theShower_.c2,"c2/s");
  tShowers_->Branch("i0",&theShower_.i0,"i0/s");
  tShowers_->Branch("i1",&theShower_.i1,"i1/s");
  tShowers_->Branch("i2",&theShower_.i2,"i2/s");

#endif

#ifdef USE_TRACKS_TREE



  tTracks_ = new TTree("tracks","Tracks");
  theTrack_.idx=0;

  tTracks_->Branch("ax",&theTrack_.ax,"ax/F");
  tTracks_->Branch("ay",&theTrack_.ay,"ay/F");
  tTracks_->Branch("bx",&theTrack_.bx,"bx/F");
  tTracks_->Branch("by",&theTrack_.by,"by/F");
  tTracks_->Branch("chi2",&theTrack_.chi2,"chi2/F");
  tTracks_->Branch("npoint",&theTrack_.npoint,"npoint/I");
  tTracks_->Branch("idx",&theTrack_.idx,"idx/I");
  tTracks_->Branch("eventid",&theTrack_.eventid,"eventid/l");
  tTracks_->Branch("nhit0",&theTrack_.nhit0,"nhit0[61]/b");
  tTracks_->Branch("nhit1",&theTrack_.nhit1,"nhit1[61]/b");
  tTracks_->Branch("nhit2",&theTrack_.nhit2,"nhit2[61]/b");
  tTracks_->Branch("xhit",&theTrack_.xhit,"xhit[61]/D");
  tTracks_->Branch("yhit",&theTrack_.yhit,"yhit[61]/D");

#endif

  std::cout << " create Trees"<<std::endl;



}
void TrackAnalyzer::closeTrees()
{
  INFO_PRINT("CLOSING FILES");
  if (treeFile_!=NULL)
    {
  treeFile_->cd();
  tEvents_->BuildIndex("idx");
  //tShowers_->BuildIndex("idx","eventid");
  //tTracks_->BuildIndex("idx","eventid");
  tEvents_->Write();
  //tShowers_->Write();
  //tTracks_->Write();
  treeFile_->ls();
  treeFile_->Close();
    }
  if ( theNtupleFile_!=NULL)
    {
  theNtupleFile_->cd();
  theNtuple_->Write();
  theNtupleFile_->Close();
    }
}

void TrackAnalyzer::PointsBuilder(std::vector<RecoHit*> &vrh)
{
  std::vector<RECOCluster> vCluster;
  vCluster.clear();
  for (std::vector<RecoHit*>::iterator ih=vrh.begin();ih!=vrh.end();ih++)
    {
      bool append= false;
      //std::cout<<"Clusters "<<vCluster.size()<<std::endl;
      RecoHit* hit=(*ih);
      for (std::vector<RECOCluster>::iterator icl=vCluster.begin();icl!=vCluster.end();icl++)
	if (icl->Append(*hit))
	  {
	    append=true;
	    break;
	  }
      if (append) continue;
      RECOCluster cl(*hit);
		
      vCluster.push_back(cl);
      // std::cout<<"Apres push Clusters "<<vCluster.size()<<std::endl;
    }




  // Merge adjacent clusters
  //std::cout<<"Avant merged Clusters "<<vCluster.size()<<std::endl;
  bool merged=false;
  do
    {
      merged=false;
      std::vector<RECOCluster> vNew;
      vNew.clear();
      for (uint32_t i=0;i<vCluster.size();i++)
	{
	  if (!vCluster[i].isValid()) continue;
	  for (uint32_t j=i+1;j<vCluster.size();j++)
	    {
	      if (!vCluster[j].isValid()) continue;
	      if (vCluster[i].isAdjacent(vCluster[j]))
		{
		  RECOCluster c;
		  for (std::vector<RecoHit>::iterator iht=vCluster[i].getHits()->begin();iht!=vCluster[i].getHits()->end();iht++)
		    c.getHits()->push_back((*iht));
		  for (std::vector<RecoHit>::iterator jht=vCluster[j].getHits()->begin();jht!=vCluster[j].getHits()->end();jht++)
		    c.getHits()->push_back((*jht));
		  vCluster[i].setValidity(false);
		  vCluster[j].setValidity(false);
					
					
		  //DEBUG_PRINT("Merged cluster %d %d \n",i,j);
		  vNew.push_back(c);
		  merged=true;
		  break;
		}
				
	    }
	}
      if (merged)
	{
	  for (std::vector<RECOCluster>::iterator jc=vCluster.begin();jc!=vCluster.end();)
	    {
				
	      if (!jc->isValid())
		vCluster.erase(jc);
	      else
		{
					
		  ++jc;
		}
	    }
	  //DEBUG_PRINT(" vCluster Size %d\n",vCluster.size());
	  //DEBUG_PRINT(" New clusters found %d\n",vNew.size());
	  for (std::vector<RECOCluster>::iterator ic=vNew.begin();ic!=vNew.end();ic++)
	    vCluster.push_back((*ic));
	  //DEBUG_PRINT(" New clusters found %d\n",vCluster.size());
	}
    } while (merged);
  // std::cout<<"Apres merged Clusters "<<vCluster.size()<<std::endl;
  //std::cout<<"Apres clean Clusters "<<vCluster.size()<<std::endl;

  // Look for time +15 and time+16
  allpoints_.clear();
  uint32_t ptid=0;
  for (std::vector<RECOCluster>::iterator icl=vCluster.begin();icl!=vCluster.end();icl++)
    {
      RECOCluster& cl=*icl;
      // DEBUG_PRINT("%f %f %f \n",cl.X(),cl.Y(),cl.getHits()->begin()->Z());
      //if (icl->getHits()->begin()->chamber()==20) DEBUG_PRINT("nh = %d \n",icl->getHits()->size());
      //cl.Print();
      RecoPoint p(cl,cl.getHits()->begin()->chamber(),cl.X(),cl.Y(),cl.getHits()->begin()->Z(),posError,posError);
      p.setPointId(ptid++);
      allpoints_.push_back(p);
		
    }
  //std::cout<<"N points ="<<allpoints_.size()<<std::endl;
  // Now group points per chamber
  chamberPoints_.clear();

  for (std::vector<RecoPoint>::iterator icl=allpoints_.begin();icl!=allpoints_.end();icl++)
    {
      RECOCluster& c= (*icl).getCluster();
      RecoPoint* p =&(*icl);
      uint32_t ch=p->getChamberId();
		

      std::map<uint32_t,std::vector<RecoPoint*> >::iterator itch=chamberPoints_.find(p->getChamberId());
      if (itch!=chamberPoints_.end())
	{
	  itch->second.push_back(p);
	}
      else
	{
	  std::vector<RecoPoint*> v;
	  v.push_back(p);
	  std::pair<uint32_t,std::vector<RecoPoint*> > pa(ch,v);
	  chamberPoints_.insert(pa);
			
	}
    }
  // std::cout<<"N points ch ="<<chamberPoints_.size()<<std::endl;
  return;
}

uint32_t TrackAnalyzer::buildTracks(std::vector<RecoHit*> &vrh,std::string vdir)
{
  
  std::vector<RECOCluster*> clusters;
  std::vector<RECOCluster*> realc;
  std::vector<RECOCluster*> intc;
  clusters.clear();
  realc.clear();
  intc.clear();
  HoughCut cuts;
  float *h_x=(float *) malloc(4096*sizeof(float));
  float *h_y= (float *) malloc(4096*sizeof(float));
  float *h_z=(float *) malloc(4096*sizeof(float));
  unsigned int *h_layer=(unsigned int *) malloc(4096*sizeof(unsigned int));
  uint32_t nshower=0;
  //ComputerTrack ch(&cuts);
  //ch.DefaultCuts();

  for (std::vector<RecoHit*>::iterator ih=vrh.begin();ih<vrh.end();ih++)
    {
      // DEBUG_PRINT("Hit plan = %d %d \n",(*ih)->chamber(),(*ih)->plan());
      //      if ((*ih)->getFlag(RecoHit::CORE)==1) continue;
      bool merged=false;
      for (std::vector<RECOCluster*>::iterator ic=realc.begin();ic!=realc.end();ic++)
	{
	  if ((*ih)->chamber()!=(*ic)->chamber()) continue;
	  merged=(*ic)->Append((*(*ih)),2.); // avant 4 et normalement 2
	  if (merged) break;
	}
      if (merged) continue;
      RECOCluster* c= new RECOCluster((*(*ih)));
      realc.push_back(c);
      clusters.push_back(c);
    }
  
  DEBUG_PRINT(" Number of clusters %d REALC %d INTC %d Hits %d \n",clusters.size(),realc.size(),intc.size(),vrh.size());

  uint32_t nstub=0;
  for (std::vector<RECOCluster*>::iterator ic=realc.begin();ic!=realc.end();ic++)
    {

      ChamberPos& cp=reader_->getPosition((*ic)->chamber());
	  // DEBUG_PRINT(" %d (%f,%f,%f) (%f,%f,%f) (%d,%d) \n",
	  //	 cp.getId(),cp.getX0(),cp.getY0(),cp.getZ0(),cp.getX1(),cp.getY1(),cp.getZ1(),cp.getXsize(),cp.getYsize());
      double x,y,z;
      cp.calculateGlobal((*ic)->X(),(*ic)->Y(),x,y,z);

      h_x[nstub]=x;//(*ic)->X();
      h_y[nstub]=y;//(*ic)->Y();
      h_z[nstub]=z;//(*ic)->Z();
      h_layer[nstub]=(*ic)->plan();

      // DEBUG_PRINT("\t %d :  %d %f %f %f \n",nstub,h_layer[nstub],h_x[nstub],h_y[nstub],h_z[nstub]);
      nstub++;
    }
  //  theComputerTrack_->associate(nstub,h_x,h_y,h_z,h_layer);
  //theComputerTrack_->telescope(nstub,h_x,h_y,h_z,h_layer,NPLANS_USED);
  theComputerTrack_->muonFinder(nstub,h_x,h_y,h_z,h_layer);
 

  //  if (theComputerTrack_->getTracks().size()>0) theNbTracks_++;
  uint32_t nmip=0;
   for (unsigned int i=0;i<theComputerTrack_->getTracks().size();i++)
	{
	  TrackInfo& tk = theComputerTrack_->getTracks()[i];

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
	  //	  std::cout<<tk.planes_<<std::endl;
	  //getchar();
	  for (uint32_t ip=fch;ip<=lch;ip++)
	    {
	      
	      TrackInfo tex;
	      
	      tk.exclude_layer(ip,tex);
	      uint32_t npext=tex.size();
	      /*
	      if (npext<minChambersInTime_) continue; // Au moins 4 plans dans l'estrapolation touches 

	      if (ip>1 && !tex.plane(ip-1)) 
		if (ip>2 && !tex.plane(ip-2)) continue;

	      if (ip<lch && !tex.plane(ip+1))
		if (ip<(lch-1) && !tex.plane(ip+2)) continue;
	      */
	      if (npext<tkExtMinPoint_) continue;

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
	      //	      if (yext< 5 || yext>30 || xext<2 || xext>29) continue;
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
		  int nx=int(xa-xi)+1;
		  int ny=int(ya-yi)+1;

		  hext= rootHandler_->BookTH2(s.str()+"ext",nx,xi,xa,ny,yi,ya);
		  hfound= rootHandler_->BookTH2(s.str()+"found",nx,xi,xa,ny,yi,ya);
		  hnear= rootHandler_->BookTH2(s.str()+"near",nx,xi,xa,ny,yi,ya);
		  hfound1= rootHandler_->BookTH2(s.str()+"found1",nx,xi,xa,ny,yi,ya);
		  hfound2= rootHandler_->BookTH2(s.str()+"found2",nx,xi,xa,ny,yi,ya);
		  hmul= rootHandler_->BookTH2(s.str()+"mul",nx,xi,xa,ny,yi,ya);
		  hdx=  rootHandler_->BookTH1(s.str()+"dx",400,-4.,4.);
		  hdy=  rootHandler_->BookTH1(s.str()+"dy",400,-4.,4.);
		}
	   
	      hext->Fill(xext,yext);
	      //bool 
	      float dist=1E9;
	      bool th1=false,th2=false;
	      float dxi,dyi,xn,yn,nhi;
	      for (std::vector<RECOCluster*>::iterator ic=clusters.begin();ic!=clusters.end();ic++)
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
		      for (std::vector<RecoHit>::iterator ih=(*ic)->getHits()->begin();ih!=(*ic)->getHits()->end();ih++)
			{
			  if ((*ih).getFlag(RecoHit::THR1)!=0) th1=true;
			  if ((*ih).getFlag(RecoHit::THR2)!=0) th2=true;
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
	    }





	}
   DEBUG_PRINT("==> MIPS hit %d -> %.2f\n",nmip,nmip*100./vrh.size()); 
 
 
   for (std::vector<RECOCluster*>::iterator ic=clusters.begin();ic!=clusters.end();ic++)
     delete (*ic);
   free(h_x);
   free(h_y);
   free(h_z);
   free(h_layer);
   return nshower;
}

void TrackAnalyzer::clearClusters()
{
  npBuf_=0;
  /*
  memset(_x,0,4096*sizeof(float));
  memset(_y,0,4096*sizeof(float));
  memset(_z,0,4096*sizeof(float));
  memset(_layer,0,4096*sizeof(uint32_t));
  */
  
  //printf(" # clusters \n",allClusters_.size());  
  realClusters_.clear();
  interactionClusters_.clear();
  
  for (std::vector<PlaneCluster*>::iterator ic=allClusters_.begin();ic!=allClusters_.end();ic++)
    delete (*ic);
  
  allClusters_.clear();
  nPlansReal_.reset();
  nPlansInteraction_.reset();
  nPlansAll_.reset();
    
}
void TrackAnalyzer::fillPlaneClusters(std::vector<RecoHit*> vrh)
{
  //printf("DEBUT\n");
  
  clearClusters();
  
  //return;
  for (std::vector<RecoHit*>::iterator ih=vrh.begin();ih<vrh.end();ih++)
    {
      // if ((*ih)->getAmplitude()==2) nh0++;
      // if ((*ih)->getAmplitude()==1) nh1++;
      // if ((*ih)->getAmplitude()==3) nh2++;
      // Components* ch=(Components*) (*ih)->Components();
      // double w=0;
      // if (ch->l2!=0) w=sqrt((ch->l1)/ch->l2);
      // hwt->Fill(w);
      
      if ((*ih)->getFlag(RecoHit::CORE)==1) continue;
      bool merged=false;
      for (std::vector<PlaneCluster*>::iterator ic=realClusters_.begin();ic!=realClusters_.end();ic++)
	{
	  if ((*ih)->chamber()!=(*ic)->chamber()) continue;
	  merged=(*ic)->Append((*ih),2.); // avant 4 et normalement 2
	  if (merged) break;
	}
      if (merged) continue;
      PlaneCluster* c= new PlaneCluster((*ih));
      realClusters_.push_back(c);
      allClusters_.push_back(c);
    }
  
  //printf("OLA %d %d \n",allClusters_.size(),realClusters_.size());
  for (std::vector<RecoHit*>::iterator ih=vrh.begin();ih<vrh.end();ih++)
    {
      if ((*ih)->getFlag(RecoHit::CORE)!=1) continue;
      bool merged=false;
      for (std::vector<PlaneCluster*>::iterator ic=interactionClusters_.begin();ic!=interactionClusters_.end();ic++)
	{
	  if ((*ih)->chamber()!=(*ic)->chamber()) continue;
	  merged=(*ic)->Append((*ih),4.); // avant 4 et normalement 2
	  if (merged) break;
	}
      if (merged) continue;
      PlaneCluster* c= new PlaneCluster((*ih));
      interactionClusters_.push_back(c);
      allClusters_.push_back(c);
    }
  
  // Move small cluster from interactionClusters_ to realClusters_
  for (std::vector<PlaneCluster*>::iterator ic=interactionClusters_.begin();ic!=interactionClusters_.end();)
    {
      if ((*ic)->getHits()->size()>=5) 
	++ic;
      else
	{
	  realClusters_.push_back((*ic));
	  interactionClusters_.erase(ic);
	}
    }
  
  for (std::vector<PlaneCluster*>::iterator ic=realClusters_.begin();ic!=realClusters_.end();)
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
  for (std::vector<PlaneCluster*>::iterator ic=interactionClusters_.begin();ic!=interactionClusters_.end();ic++)
    {
      uint32_t ch=(*ic)->plan();
      nPlansInteraction_.set(ch,true);
      nPlansAll_.set(ch,true);
      if (lastInteractionPlane_<ch) lastInteractionPlane_=ch;
      if (firstInteractionPlane_>ch) firstInteractionPlane_=ch;
    }

  // Fill Position/layer buffer
  
  npBuf_=0;
  
  for (std::vector<PlaneCluster*>::iterator ic=realClusters_.begin();ic!=realClusters_.end();ic++)
    {

      ChamberPos& cp=reader_->getPosition((*ic)->chamber());
	  // DEBUG_PRINT(" %d (%f,%f,%f) (%f,%f,%f) (%d,%d) \n",
	  //	 cp.getId(),cp.getX0(),cp.getY0(),cp.getZ0(),cp.getX1(),cp.getY1(),cp.getZ1(),cp.getXsize(),cp.getYsize());
      if ((*ic)->plan()==0) continue;
      double x,y,z;
      cp.calculateGlobal((*ic)->X(),(*ic)->Y(),x,y,z);

      _x[npBuf_]=x;
      _y[npBuf_]=y;
      _z[npBuf_]=z;
      _layer[npBuf_]=(*ic)->plan();
      nPlansReal_.set((*ic)->plan(),true);
      nPlansAll_.set((*ic)->plan(),true);
      DEBUG_PRINT("\t %d :  %d %f %f %f \n",npBuf_,_layer[npBuf_],_x[npBuf_],_y[npBuf_],_z[npBuf_]);
      npBuf_++;
      if (npBuf_>=4096) break;
    }
  
  // printf("FINI\n");
}




double xmin[100];
double xmax[100];
double ymin[100];
double ymax[100];

uint32_t TrackAnalyzer::buildPrincipal(std::vector<RecoHit*> &vrh,std::string vdir)
{


#ifdef standalone_way  
  std::vector<RECOCluster*> realc;
  realc.clear();

  float *h_x=(float *) malloc(4096*sizeof(float));
  float *h_y= (float *) malloc(4096*sizeof(float));
  float *h_z=(float *) malloc(4096*sizeof(float));
  unsigned int *h_layer=(unsigned int *) malloc(4096*sizeof(unsigned int));
  uint32_t nshower=0;
  //ComputerTrack ch(&cuts);
  //ch.DefaultCuts();
  //INFO_PRINT("Avant Cluster \n");
  for (std::vector<RecoHit*>::iterator ih=vrh.begin();ih<vrh.end();ih++)
    {
      bool merged=false;
      for (std::vector<RECOCluster*>::iterator ic=realc.begin();ic!=realc.end();ic++)
	{
	  if ((*ih)->chamber()!=(*ic)->chamber()) continue;
	  merged=(*ic)->Append((*(*ih)),2.); // avant 4 et normalement 2
	  if (merged) break;
	}
      if (merged) continue;
      RECOCluster* c= new RECOCluster((*(*ih)));
      realc.push_back(c);

    }
  
  

  uint32_t nstub=0;
  for (std::vector<RECOCluster*>::iterator ic=realc.begin();ic!=realc.end();ic++)
    {

      ChamberPos& cp=reader_->getPosition((*ic)->chamber());
	  // DEBUG_PRINT(" %d (%f,%f,%f) (%f,%f,%f) (%d,%d) \n",
	  //	 cp.getId(),cp.getX0(),cp.getY0(),cp.getZ0(),cp.getX1(),cp.getY1(),cp.getZ1(),cp.getXsize(),cp.getYsize());
      double x,y,z;
      cp.calculateGlobal((*ic)->X(),(*ic)->Y(),x,y,z);

      h_x[nstub]=x;//(*ic)->X();
      h_y[nstub]=y;//(*ic)->Y();
      h_z[nstub]=z;//(*ic)->Z();
      h_layer[nstub]=(*ic)->plan();

      // DEBUG_PRINT("\t %d :  %d %f %f %f \n",nstub,h_layer[nstub],h_x[nstub],h_y[nstub],h_z[nstub]);
      nstub++;
    }

  //  theComputerTrack_->associate(nstub,h_x,h_y,h_z,h_layer);
  //theComputerTrack_->telescope(nstub,h_x,h_y,h_z,h_layer,NPLANS_USED);
  //INFO_PRINT("Avant Principal %d\n",nstub);
  ShowerParams isha;
  Shower::TPrincipalComponents(nstub,h_x,h_y,h_z,h_layer,(double*) &isha);
#else
  ShowerParams isha;
  //INFO_PRINT("Avant Principal %d\n",npBuf_);
  Shower::TPrincipalComponents(npBuf_,_x,_y,_z,_layer,(double*) &isha);

#endif

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
      for (std::vector<PlaneCluster*>::iterator ic=realClusters_.begin();ic!=realClusters_.end();ic++)
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
		  if ((*ih)->getFlag(RecoHit::THR1)!=0) th1=true;
		  if ((*ih)->getFlag(RecoHit::THR2)!=0) th2=true;
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





void TrackAnalyzer::draw(TrackInfo& t)
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
uint32_t TrackAnalyzer::fillVolume(uint32_t seed)
{
  std::map<uint32_t,std::vector<IMPL::RawCalorimeterHitImpl*> >::iterator iseed=reader_->getPhysicsEventMap().find(seed);
  if (iseed==reader_->getPhysicsEventMap().end()) 
    {
      DEBUG_PRINT("Impossible \n");
      return 0;
    }
  //memset(hitVolume_,0,60*96*96*sizeof(RecoHit)); Aie!!!

  //return 0;
  std::bitset<61> planes(0);
  
  theImage_.clear();
  uint32_t ncount=0;
  
  if (iseed->second.size()>4096) return 0;
  for (std::vector<IMPL::RawCalorimeterHitImpl*>::iterator ihit=iseed->second.begin();ihit!=iseed->second.end();ihit++)
    {
      IMPL::RawCalorimeterHitImpl* hit =(*ihit);
      unsigned int difid = hit->getCellID0()&0xFF;
      if (difid<1 || difid>255) continue;
      int asicid = (hit->getCellID0()&0xFF00)>>8;
      int channel= (hit->getCellID0()&0x3F0000)>>16;
      bool thr[3];
      //      DEBUG_PRINT("%x \n",hit->getCellID0());
      int ithr= hit->getAmplitude()&0x3;
      if (ithr==0)
	{
	  std::cout<<difid<<" had:"<<asicid<<":"<<channel<<":"<<ithr<<std::endl;
	  continue;
	}
      std::map<unsigned int,DifGeom>::iterator idg = reader_->getDifMap().find(difid);
      DifGeom& difgeom = idg->second;
      int x=0,y=0;
      uint32_t chid = idg->second.getChamberId();
      uint32_t hrtype=2;
      if (chid>50) hrtype=11;
      DifGeom::PadConvert(asicid,channel,x,y,hrtype);
      uint32_t I=difgeom.toGlobalX(x);
      if (I<1 || I>96) continue;
      uint32_t J=difgeom.toGlobalY(y);
      if (J<1 || J>96) continue;
      if (chid<1 || chid>60) continue;
		
      //planes.set(chid-1,true);
      std::map<unsigned int,ChamberGeom>::iterator icg = reader_->getChamberMap().find( chid);
      if (icg==reader_->getChamberMap().end()) continue;
      ChamberGeom& chgeom = icg->second;
      //printf("Hit beeing filled %d %d %d\n",chid-1,I-1,J-1);
      chgeom.setZ(reader_->getPosition(chid).getZ0());
      theImage_.setValue(chid-1,I-1,J-1,1);

      hitVolume_[chid-1][I-1][J-1].initialise(difgeom,chgeom,hit,hrtype);

      ncount++;
    }
  DEBUG_PRINT("Total number of Hit in buildVolume %d  %d \n",ncount,seed);
  return ncount;
}

void TrackAnalyzer::TagIsolated(uint32_t fpl,uint32_t lpl)
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
 
  for (uint32_t k=0;k<theImage_.getXSize();k++)
    {

      for (uint32_t i=0;i<theImage_.getYSize();i++)
	{
	  //if (image2x[k][i]>=-1 ) continue;
			
	  for (uint32_t j=0;j<theImage_.getZSize();j++)

	    {
	      if (theImage_.getValue(k,i,j)>0)
		if (theImage_.getValue(k,i,j)>0)
		  {
		    int izmin=-2;
		    int izmax=+2;
		    if (k<=fpl+2) {izmin=0;izmax=4;}
		    if (k>=lpl-2) {izmin=-4;izmax=0;}
		    uint32_t nv=0;
		    std::vector<RecoHit*> vnear_;
		    RecoHit* h0=&hitVolume_[k][i][j];
		    for (int z=izmin;z<=izmax;z++)
		      {
			if (z+k<0) continue;
			if (z+k>=theImage_.getXSize()) continue;
			for (int x=ixmin;x<=ixmax;x++)
			  {
			    if (x+i<0) continue;
			    if (x+i>=theImage_.getYSize()) continue;
			    for (int y=ixmin;y<=ixmax;y++)
			      {
				if (y+j<0) continue;
				if (y+j>=theImage_.getZSize()) continue;
				if (theImage_.getValue(k+z,i+x,j+y)<=0) continue;
				RecoHit* h1=&hitVolume_[k+z][i+x][j+y];
				float x0=h0->X(),y0=h0->Y(),z0=h0->Z(),x1=h1->X(),y1=h1->Y(),z1=h1->Z();
				float dist=sqrt((x1-x0)*(x1-x0)+(y1-y0)*(y1-y0)+(z1-z0)*(z1-z0));
				dist=abs(x1-x0)+abs(y1-y0)+abs(z1-z0)/2.;
				if (dist<4.) vnear_.push_back(h1); // was 4

				nv++;
			      }
			  }
		      }
		    if (nv>nmax) nmax=nv;
		    hitVolume_[k][i][j].calculateComponents(vnear_);

		    Components* c=(Components*) hitVolume_[k][i][j].Components();
		    double w=0;
		    if (c->l2!=0) w=sqrt((c->l1+c->l0)/c->l2);
		    if (c->l2==0 || vnear_.size()<=3)
		      {
			hitVolume_[k][i][j].setFlag(RecoHit::ISOLATED,true);niso++;
			hnv->Fill(vnear_.size()*1.);
		      }
		    else
		      {
			hweight->Fill(w);
			hweight2->Fill(vnear_.size()*1.,w);
			if (w<0.3 && vnear_.size()<20) // 0.3 before  
			  {hitVolume_[k][i][j].setFlag(RecoHit::EDGE,true);nedge++;}
			else
			  {hitVolume_[k][i][j].setFlag(RecoHit::CORE,true);ncore++;}
		      }
		  }

	    }
	}
	  
    }
  coreRatio_=ncore*1./(nedge+niso);
  return;
}


uint32_t TrackAnalyzer::fillVector(uint32_t seed)
{
  
  uint32_t nplans=0;
 
  //INFO_PRINT("%s-%d %d %f  \n",__PRETTY_FUNCTION__,__LINE__,reader_->getPositionMap().begin()->second.getPlan(),reader_->getPositionMap().begin()->second.getZ0());    
  uint32_t nhit=this->fillVolume(seed);
  //INFO_PRINT("%s-%d %d %f  \n",__PRETTY_FUNCTION__,__LINE__,reader_->getPositionMap().begin()->second.getPlan(),reader_->getPositionMap().begin()->second.getZ0());    
  if (nhit<minHitCount_) return 0;
  if (nhit>maxHitCount_) return 0;
  this->TagIsolated(1,48);

  theHitVector_.clear();
  theTkHitVector_.clear();
  for (uint32_t k=0;k<theImage_.getXSize();k++)
    {
      bool found=false;
      for (uint32_t i=0;i<theImage_.getYSize();i++)
	{
	  //if (image2x[k][i]>=-1 ) continue;
			
	  for (uint32_t j=0;j<theImage_.getZSize();j++)
	    {
	      if (theImage_.getValue(k,i,j)>0)
		{
		  if ((hitVolume_[k][i][j].getFlag(RecoHit::ISOLATED)!=1) )
		    theTkHitVector_.push_back(&hitVolume_[k][i][j]);
		  theHitVector_.push_back(&hitVolume_[k][i][j]);
		  found=true;
		}
	    }
	}
      if (found) nplans++;
    }

  this->fillPlaneClusters(theHitVector_);
  DEBUG_PRINT("Hits %d tk %d ===> %d clusters %d Real %d Interaction \n",theHitVector_.size(),theTkHitVector_.size(),allClusters_.size(),realClusters_.size(),interactionClusters_.size());
  
  this->tagMips();

  return nplans;
}
void TrackAnalyzer::tagMips()
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
      for (std::vector<PlaneCluster*>::iterator ic=realClusters_.begin();ic!=realClusters_.end();ic++)
	    {
	      float dx=tk.getXext((*ic)->Z())-(*ic)->X();
	      float dy=tk.getYext((*ic)->Z())-(*ic)->Y();
	      if (sqrt(dx*dx+dy*dy)<2.)
		{
		  // DEBUG_PRINT("Point (%f,%f,%f) dist %f %f -> %f  \n",(*ic)->X(),(*ic)->Y(),(*ic)->Z(),dx,dy,sqrt(dx*dx+dy*dy));
		for (std::vector<RecoHit*>::iterator ih=(*ic)->getHits()->begin();ih!=(*ic)->getHits()->end();ih++)
		  {
		    if ((*ih)->getFlag(RecoHit::MIP)==0)
		      {
			// DEBUG_PRINT("Point (%f,%f,%f) dist %f %f -> %f  \n",(*ic)->X(),(*ic)->Y(),(*ic)->Z(),dx,dy,sqrt(dx*dx+dy*dy));
			(*ih)->setFlag(RecoHit::MIP,true);
			hitVolume_[(*ih)->chamber()-1][(*ih)->I()-1][(*ih)->J()-1].setFlag(RecoHit::MIP,true);
			nmip++;
		      }
		  }
		}
	    }
	}
  
   DEBUG_PRINT("==> MIPS hit %d -> %.2f Length %f \n",nmip,nmip*100./theHitVector_.size(),theComputerTrack_->Length()); 

}
