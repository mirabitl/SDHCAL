#define NX 36
#define NY 36

#include "ShowerAnalyzer.h"
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

using namespace boost; 
typedef boost::singleton_pool<RecoHit, sizeof(RecoHit)> RecoHitPool;
typedef std::vector<RecoHit*>::iterator recit;

#include <HoughLocal.h>

#include "ChamberAnalyzer.h"

#define posError 0.5

#ifdef USE_CULA
#include <cula_lapack.h>
#include <cublas.h>
#include <cublas_v2.h>
#include <cublas_api.h>
#endif


void ShowerAnalyzer::initHistograms()
{
  //  rootHandler_->BookTH1("/Clusters/EST1",100,0.,300.);
}

ShowerAnalyzer::ShowerAnalyzer(DHCalEventReader* r,DCHistogramHandler* h) :trackIndex_(0),nAnalyzed_(0),clockSynchCut_(8), spillSize_(90000),maxHitCount_(500000),
									     tkMinPoint_(3),tkExtMinPoint_(3),tkBigClusterSize_(32),tkChi2Cut_(0.01),tkDistCut_(5.),tkExtChi2Cut_(0.01),tkExtDistCut_(10.),tkAngularCut_(20.),zLastAmas_(134.),
									     findTracks_(true),dropFirstSpillEvent_(false),useSynchronised_(true),chamberEdge_(5.),rebuild_(false),oldAlgo_(true),collectionName_("DHCALRawHits"),
									     tkFirstChamber_(1),tkLastChamber_(61),useTk4_(false),offTimePrescale_(1),houghIndex_(0),theRhcolTime_(0.),theTimeSortTime_(0.),theTrackingTime_(0),
									   theHistoTime_(0),theSeuil_(0),draw_(false),theSkip_(0),theMonitoring_(r,h),theMonitoringPeriod_(0),theMonitoringPath_("/dev/shm/Monitoring"),ntkbetween(0),theBCIDSpill_(0),theLastBCID_(0)
{
  reader_=r;
  rootHandler_ =h;
  this->initialise();
  HoughCut cuts;

  theComputerHough_=new ComputerHough(&cuts);
  theComputerHough_->DefaultCuts();

}

void ShowerAnalyzer::initialise()
{
  headerWritten_=false;
  //  TVirtualFitter::SetDefaultFitter("Minuit"); 
  //  gPluginMgr->AddHandler("ROOT::Math::Minimizer", "Minuit", "TMinuitMinimizer", "Minuit", "TMinuitMinimizer(const char *)"); 
  integratedTime_=0;
  asicCount_.clear();
  DCBufferReader::setDAQ_BC_Period(0.2);
  // theHTx_=new HTImage(120,0,1*M_PI,150,-50.,100.);
  // theHTy_=new HTImage(120,0,1*M_PI,150,-50.,100.);
  theHTx_=new HTImage(120,0,1*M_PI,150,-50.,100.);
  theHTy_=new HTImage(120,0,1*M_PI,150,-50.,100.);
  theTime_.time=0;
  theTime_.millitm=0;
  theDb_=NULL;
  useSqlite_=false;
  useMysql_=false;
  theImage_.initialise(theImageBuffer_,60,96,96);
  theImageWeight_.initialise(theImageWeightBuffer_,60,96,96);

}
void ShowerAnalyzer::initJob(){presetParameters();}
void ShowerAnalyzer::endJob(){
  if (theMonitoringPeriod_!=0)
    {
      rootHandler_->writeXML(theMonitoringPath_);
      //char c;c=getchar();putchar(c); if (c=='.') exit(0);;
		
    }
#ifdef USE_CULA
  culaShutdown();
#endif
  closeTrees();
	
	
}
void ShowerAnalyzer::presetParameters()
{
  std::map<std::string,MarlinParameter> m=reader_->getMarlinParameterMap();
  std::map<std::string,MarlinParameter>::iterator it;
  try
    {
      if ((it=m.find("ClockSynchCut"))!=m.end()) clockSynchCut_=it->second.getIntValue();
      if ((it=m.find("SpillSize"))!=m.end()) spillSize_=it->second.getIntValue();
      if ((it=m.find("MaxHitCount"))!=m.end()) maxHitCount_=it->second.getIntValue();
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

      DEBUG_PRINT("Interactif %d \n",draw_);

      //getchar();

    }
  catch (std::string s)
    {
      std::cout<<__PRETTY_FUNCTION__<<" error "<<s<<std::endl;
    }
	
}
bool ShowerAnalyzer::decodeTrigger(LCCollection* rhcol, double tcut)
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

uint32_t ShowerAnalyzer::NoiseStudy(std::map<uint32_t,std::bitset<255> > timeDif,std::map<uint32_t,std::bitset<61> > timeChamber)
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


void ShowerAnalyzer::DIFStudy(IMPL::RawCalorimeterHitImpl* hit)
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

void ShowerAnalyzer::FillTimeAsic(IMPL::LCCollectionVec* rhcol)
{
  printf("On rentre \n");
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
double  ShowerAnalyzer::checkTime()
{

  ftime(&theCurrentTime_);  
  double dt=theCurrentTime_.time-theTime_.time+(theCurrentTime_.millitm-theTime_.millitm)*1E-3;
  theTime_.time= theCurrentTime_.time;  theTime_.millitm= theCurrentTime_.millitm;
  return dt;
}
void ShowerAnalyzer::findTimeSeeds( IMPL::LCCollectionVec* rhcol, int32_t nhit_min,std::vector<uint32_t>& candidate)
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

uint32_t ShowerAnalyzer::buildVolume(IMPL::LCCollectionVec* rhcol,uint32_t seed)
{
  std::bitset<61> planes(0);
  theImage_.clear();
  theImageWeight_.clear();

  memset(hitVolume_,0,60*96*96*sizeof(RecoHit));
  uint32_t ncount=0;
  float mindif=DBL_MAX;
  for (int i=0;i<rhcol->getNumberOfElements();i++)
    {
      IMPL::RawCalorimeterHitImpl* hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(i);
      if (hit==0) continue;
      int32_t bc = hit->getTimeStamp();
      int tdif=bc-seed;
      if (tdif<-5 || tdif>5) continue;
      if (abs(1.*(bc-seed))<mindif) mindif=abs(1.*(bc-seed));

      // Decode
      unsigned int difid = hit->getCellID0()&0xFF;
      if (difid<1 || difid>255) continue;
      int asicid = (hit->getCellID0()&0xFF00)>>8;
      int channel= (hit->getCellID0()&0x3F0000)>>16;
      bool thr[3];
      //      DEBUG_PRINT("%x \n",hit->getCellID0());
      int ithr= hit->getAmplitude()&0x3;
      if (ithr==0)
	{
	  std::cout<<difid<<" had:"<<asicid<<":"<<channel<<":"<<bc<<":"<<ithr<<std::endl;
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
      theImage_.setValue(chid-1,I-1,J-1,1);
      theImageWeight_.setValue(chid-1,I-1,J-1,(1<<ithr-1));
      std::map<unsigned int,ChamberGeom>::iterator icg = reader_->getChamberMap().find( chid);
      ChamberGeom& chgeom = icg->second;
      //printf("Hit beeing filled %d %d %d\n",chid-1,I-1,J-1);
      hitVolume_[chid-1][I-1][J-1].initialise(difgeom,chgeom,hit,hrtype);
      ncount++;


    }
  //uint32_t nplanes=0;
  //for (int i=0;i<61;i++)
  //	if (planes[i]!=0) nplanes++;
  //std::cout<<nplanes<<" ==>"<<planes<<std::endl;
  //intf("Minimal distance in time.... %f  %d \n",mindif,ncount);
  return ncount;

}
void ShowerAnalyzer::EdgeDetection(array3D<unsigned char> &imagev,array3D<unsigned char> &cores,array3D<unsigned char> &edges)
{


  memset(image3Buf_,0,imagev.getXSize()*imagev.getYSize()*imagev.getZSize()*sizeof(float));
  unsigned char adjBuf[imagev.getXSize()*imagev.getYSize()*imagev.getZSize()];
  image3_.initialise(&image3Buf_[0],imagev.getXSize(),imagev.getYSize(),imagev.getZSize());
  array3D<unsigned char> adj;adj.initialise(&adjBuf[0],imagev.getXSize(),imagev.getYSize(),imagev.getZSize());
  image3_.clear();
  cores.clear();
  edges.clear();
  adj.clear();

  sobel_volume(imagev,image3_);

  theNall_=0;theNedge_=0;theWeights_=0;
  for (uint32_t k=1;k<imagev.getXSize();k++)
    {
      for (uint32_t i=1;i<imagev.getYSize();i++)
	{
	  //if (image2x[k][i]>=-1 ) continue;
			
	  for (uint32_t j=1;j<imagev.getZSize();j++)
	    {
	      //if (image2y[k][j]>=-1 ) continue;
	      if (imagev.getValue(k,i,j)==1) {
		theNall_++;
		theWeights_+=image3_.getValue(k,i,j);
	      }
	      if (image3_.getValue(k,i,j)>=-20) // -32 si poids -44
		{
		  if (image3_.getValue(k,i,j)>=-2) continue;
		  if (imagev.getValue(k,i,j)!=0) cores.setValue(k,i,j,1);
					
		  continue;
		}
	      edges.setValue(k,i,j,1);
	      if (image3_.getValue(k,i,j)<-25)
	       edges.setValue(k,i,j,2);
	    }
	}
    }
  // Add adjacent hit to core
	
  for (uint32_t k=1;k<imagev.getXSize()-1;k++)
    {
      for (uint32_t i=1;i<imagev.getYSize()-1;i++)
	{
	  //if (image2x[k][i]>=-1 ) continue;
			
	  for (uint32_t j=1;j<imagev.getZSize()-1;j++)
	    {
	      if (edges.getValue(k,i,j)==0) continue;
				
	      for (uint32_t ks=k-1;ks<=k+1;ks++)
		{
		  for (uint32_t is=i-1;is<=i+1;is++)
		    {
		      for (uint32_t js=j-1;js<=j+1;js++)
			{
			  if (cores.getValue(ks,is,js)>0)
			    {
			      adj.setValue(ks,is,js,1);
			      edges.setValue(k,i,j,0);
			      break;
			    }
			}
		    }
		}
				
	    }
	}
    }

  for (uint32_t k=1;k<imagev.getXSize()-1;k++)
    {
      for (uint32_t i=1;i<imagev.getYSize()-1;i++)
	{
	  //if (image2x[k][i]>=-1 ) continue;
			
	  for (uint32_t j=1;j<imagev.getZSize()-1;j++)
	    {
	      if (adj.getValue(k,i,j)>0)
		cores.setValue(k,i,j,1);
	      if (edges.getValue(k,i,j)>0) theNedge_++;
	    }
	}
    }
  //Now loop on core hits
  theAmas_.clear();
  theEdge3_=theNedge_;
  theCore3_=theNall_-theNedge_;
  //Store the evnt information
  if (useSqlite_ || useMysql_) 
    fillEventTable(evt_->getRunNumber(),theDTC_,theBCID_,currentTime_,theEdge3_,theCore3_,theWeights_,theNplans_);

  if (theNedge_*100./theNall_>95) return;


  if (theNedge_*100./theNall_<95)
    DEBUG_PRINT("%d %d %f -> %f\n",theNall_,theNedge_,theNedge_*100./theNall_,theNedge_*50.+(theNall_-theNedge_)*100.);
  //DEBUG_PRINT("Number of cores hits %d \n",cores.size());
  TH1* eratio=rootHandler_->GetTH1("eratio");
  TH1* eratio1=rootHandler_->GetTH1("eratio1");
  TH1* eratio2=rootHandler_->GetTH1("eratio2");
  TH1* eratio3=rootHandler_->GetTH1("eratio3");
  if (eratio==NULL)
    {
      eratio=rootHandler_->BookTH1("eratio",50,0.,1.);
      eratio1=rootHandler_->BookTH1("eratio1",50,0.,1.);
      eratio2=rootHandler_->BookTH1("eratio2",400,-20000.,0.);
      eratio3=rootHandler_->BookTH1("eratio3",200,-5000.,0.);
    }
  eratio->Fill(theNedge_*1./theNall_); 
  eratio2->Fill(theWeights_*1.);
  eratio3->Fill(theWeights_*1./log(theNplans_));
  //std::cout<<theWeights_<<std::endl;
	
  // Now build amas and count hits
  theNall_=0;theNedge_=0;
  uint32_t ne[3],nc[3];
  memset(ne,0,3*sizeof(uint32_t));
  memset(nc,0,3*sizeof(uint32_t));
  std::vector<RecoHit*> vedges;
  vedges.clear();
  for (uint32_t k=0;k<60;k++)
    for (uint32_t i=0;i<96;i++)
      for (uint32_t j=0;j<96;j++)
	if (theImage_.getValue(k,i,j))
	  {
	    theNall_++;
	    //RecoHit& h = hitVolume_[k][i][j];
	    int ithr=hitVolume_[k][i][j].getAmplitude()&0x3;
	    if (cores.getValue(k,i/3,j/3))
	      {
		bool appended=false;
		for (std::vector<Amas>::iterator ia=theAmas_.begin();ia!=theAmas_.end();ia++)
		  {
		    appended=(ia->append(&hitVolume_[k][i][j],2));
		    if (appended) break;
		  }
		if (!appended)
		  {
		    Amas a(&hitVolume_[k][i][j]);
		    theAmas_.push_back(a);
		  }
		if (ithr==1) nc[1]++;
		if (ithr==2) nc[0]++;
		if (ithr==3) nc[2]++;
	      }
	    else
	      {
		theNedge_++;
		vedges.push_back(&hitVolume_[k][i][j]);
		if (ithr==1) ne[1]++;
		if (ithr==2) ne[0]++;
		if (ithr==3) ne[2]++;

	      }
	  }


  DEBUG_PRINT("Number of Amas %d \n",theAmas_.size());
  eratio1->Fill(theNedge_*1./theNall_); 
  bool electron=false;
  bool leak=false;
  bool leakt=false;
  double emax =-DBL_MAX;
  double zmax =-DBL_MAX;
  double zlast =-DBL_MAX;
  double zfirst=DBL_MAX;
  uint32_t ng=0;
  std::sort(theAmas_.rbegin(),theAmas_.rend());
  for (std::vector<Amas>::iterator ia=theAmas_.begin();ia!=theAmas_.end();ia++)
    {
      ia->compute();
      if (ia->size()<=4) continue;
      if (ia->size()>=10)ng++;
      for (uint32_t i=0;i<21;i++)
	DEBUG_PRINT("%6.3f ",ia->getComponents(i));
      DEBUG_PRINT(" Size %d\n",ia->size()); 
      if (ia->getComponents(2)>zmax) zmax=ia->getComponents(2);
      if (ia->getComponents(16)>zlast) zlast=ia->getComponents(16);
      if (ia->getComponents(15)<zfirst) zfirst=ia->getComponents(15);
      if (ia->size()>emax)
	{
	  emax=ia->size();
	  if (ia->getComponents(15)<=2.81 && (ia->size()*1./theNall_)>0.2 ) 
	    {
	      electron=true;
	    }


	}
      if (ia->getComponents(16)>=zLastAmas_ && (ia->size())>4)
	{
	  leak=true;
	}
      if (ia->getComponents(17)<4 && (ia->size())>4)
	{
	  leakt=true;
	}
      if (ia->getComponents(18)>93 && (ia->size())>4)
	{
	  leakt=true;
	}

      if (ia->getComponents(19)<4 && (ia->size())>4)
	{
	  leakt=true;
	}
      if (ia->getComponents(20)>93 && (ia->size())>4)
	{
	  leakt=true;
	}

      // Store Amas
      if (useSqlite_ || useMysql_)
	{
	  Amas& a=(*ia);
	  fillAmasTable(&a);
	}
    }
  uint32_t nafter=0;
  theTag_=0;
  theTag_|=(ng<<16);
  if (leak) DEBUG_PRINT("==================================================> L E A K A G E <==============================\n");
  if (leakt) DEBUG_PRINT("==================================================> T R A N S V E R S E  L E A K A G E <==============================\n");
  if (electron) DEBUG_PRINT("==================================================> E L E C T R O N <============================== %d %f %d \n",ng,zmax,nafter);



  std::bitset<61> planes(0);
  for (recit it=vedges.begin();it!=vedges.end();it++)
    {
      if ((*it)->Z()>zlast) {
	if ((*it)->chamber()<61)
	  planes.set((*it)->chamber(),true);
      }
    }
  for (uint32_t i=0;i<51;i++)
    if (planes[i]!=0) nafter++;
  theZMax_=zmax;
  theZFirst_=zfirst;
  theZLast_=zlast;
  thePlanAfter_=nafter;


  electron =electron && (ng<=3) && (zlast<50.) && nafter<10;

  // theAllHit_=(na2<<20)|(na1<<10)|na0;
  // theEdgeHit_=(ne2<<20)|(ne1<<10)|ne0;



  theTag_=0;
  if (electron) theTag_=1;
  if (leak) theTag_+=2;
  if (leakt) theTag_+=4;

  //Store Amas Summary
  if (useSqlite_ || useMysql_)
    {
      fillAmasSummaryTable(nc,ne,ng,nafter,theTag_,zlast);
    }



#ifdef AFAIRE 
  //DEBUG_PRINT("NAFTER %d \n",nafter);     
  //DEBUG_PRINT("%d / %d -  %d / %d - %d / %d \n",ne0,na0,ne1,na1,ne2,na2);

  if (useSqlite_ || useMysql_)
    {
      uint32_t cor[3];
      uint32_t edg[3];
      cor[0]=na0-ne0;
      cor[1]=na1-ne1;
      cor[2]=na2-ne2;
      edg[0]=ne0;
      edg[1]=ne1;
      edg[2]=ne2;
      fillAmasSummaryTable(cor,edg,ng,nafter,theTag_,zlast);
    }
  if (TCEdge!=NULL && draw_ && theNall_>100)
    {
      TH2* hiti= rootHandler_->GetTH2("hiti");
      TH2* hitis= rootHandler_->GetTH2("hitis");
      TH2* hitis1= rootHandler_->GetTH2("hitis1");
      TH2* hitj= rootHandler_->GetTH2("hitj");
      TH2* hitjs= rootHandler_->GetTH2("hitjs");
      TH2* hitjs1= rootHandler_->GetTH2("hitjs1");
      TH1* hw3=rootHandler_->GetTH1("hw3");
      if (hiti==NULL)
	{
	  //hit3= rootHandler_->BookTH3(sp.str(),52,-2.8,145.6,100,0.,100.,100,0.,100.);
	  hiti= rootHandler_->BookTH2("hiti",60,0,60.,96,0.,96.);
	  hitis= rootHandler_->BookTH2("hitis",60,0,60.,96,0.,96.);
	  hitis1= rootHandler_->BookTH2("hitis1",60,0,60.,96,0.,96.);
	  hitj= rootHandler_->BookTH2("hitj",60,0,60.,96,0.,96.);
	  hitjs= rootHandler_->BookTH2("hitjs",60,0,60.,96,0.,96.);
	  hitjs1= rootHandler_->BookTH2("hitjs1",60,0,60.,96,0.,96.);
	  hw3=rootHandler_->BookTH1("hw3",50,-49.,0.);
	}
      else
	{
	  hiti->Reset();
	  hitis->Reset();
	  hitis1->Reset();
	  hitj->Reset();
	  hitjs->Reset();
	  hitjs1->Reset();
	}
      double nx=0,ny=0;
      /* a l'ancienne     

	 for (uint32_t k=1;k<59;k++)
	 {
	 for (uint32_t i=1;i<95;i++)
	 {
		
	 //DEBUG_PRINT("Plan %d %d %f\n",k,i,image2[k][i]);
	 hiti->SetBinContent(k+1,i+1,imagex[k][i]*1.);
	 if (image2x[k][i]<-1 && image2x[k][i]>-8) {hitis->SetBinContent(k+1,i+1,1.);nx=nx+1;}
	 hitj->SetBinContent(k+1,i+1,imagey[k][i]*1.);
	 if (image2y[k][i]<-1 && image2y[k][i]>-8) {hitjs->SetBinContent(k+1,i+1,1.);ny=ny+1;}

	 }
	 }
      */
      for (recit it=edges.begin();it!=edges.end();it++)
	{
	  hitis->SetBinContent((*it)->chamber(),(*it)->I(),1.);
	  hitjs->SetBinContent((*it)->chamber(),(*it)->J(),1.);
	}
      for (recit it=cores.begin();it!=cores.end();it++)
	{
	  hiti->SetBinContent((*it)->chamber(),(*it)->I(),1.);
	  hitj->SetBinContent((*it)->chamber(),(*it)->J(),1.);
	}
      //DEBUG_PRINT("NT %d NX %f ny %f => %f %f \n",vrh.size(),nx,ny,nx/vrh.size(),ny/vrh.size());
      for (uint32_t k=1;k<59;k++)
	{
	  for (uint32_t i=1;i<95;i++)
	    {
	      //if (image2x[k][i]>=-1 ) continue;

	      for (uint32_t j=1;j<95;j++)
		{
		  //if (image2y[k][j]>=-1 ) continue;
		  if (image3[k][i][j]>=-2) continue;
		  hw3->Fill(image3[k][i][j]);
		  if (image3[k][i][j]<-32) continue;

		  //uint32_t key=((i+1)<<24)|((j+1)<<16)|(k+1);
		  //std::map<uint32_t,RecoHit*>::iterator it=mapij.find(key);
					
		  hitis1->SetBinContent(k+1,i+1,1.);
		  hitjs1->SetBinContent(k+1,j+1,1.);
		}
	    }
	}
    }
#endif
}



uint32_t ShowerAnalyzer::mergeAmas(array3D<unsigned char> &core,array3D<unsigned char> &edges)
{
  uint32_t nb=1;
  std::vector<RecoHit*> vedges;
  vedges.clear();
  //INFO_PRINT("Number of Amas %d \n",theAmas_.size());
  if (theAmas_.size()==0) return 0;
  theAmas_[0].copyTo(vedges);

  //if (theAmas_.size()<2) return 1;
  //DEBUG_PRINT("On a copie dans vedge\n");
  TH1* edsig=rootHandler_->GetTH1("edsig");

  if (edsig==NULL)
    {
      edsig=rootHandler_->BookTH1("edsig",100,0.,30.);

    }



  Amas& theBig=theAmas_[0];
  double* v3=&theBig.Components()[12];
  double* xm=&theBig.Components()[0];
  double* lm=&theBig.Components()[3];
  double bx=xm[0]-v3[0]/v3[2]*xm[2];
  double by=xm[1]-v3[1]/v3[2]*xm[2];

  DEBUG_PRINT("Big position ===> %f %f \n",bx,by);

  for (uint32_t i=1;i<theAmas_.size();i++)
    {
      if (theAmas_[i].size()<10) continue;
      double* ov3=&theAmas_[i].Components()[12];
      double* oxm=&theAmas_[i].Components()[0];
      double* olm=&theBig.Components()[3];
      double obx=oxm[0]-ov3[0]/ov3[2]*oxm[2];
      double oby=oxm[1]-ov3[1]/ov3[2]*oxm[2];
      double dist=sqrt((bx-obx)*(bx-obx)+(by-oby)*(by-oby));

      double cx=ov3[0]/ov3[2]*xm[2]+obx;
      double cy=ov3[1]/ov3[2]*xm[2]+oby;
      double cdist=sqrt((bx-cx)*(bx-cx)+(by-cy)*(by-cy));

      double err=sqrt(lm[1]+olm[1]);


      double dz=sqrt((xm[2]-oxm[2])*(xm[2]-oxm[2])/(lm[2]+olm[2]));
      double dxy=sqrt((xm[0]-oxm[0])*(xm[0]-oxm[0])+(xm[1]-oxm[1])*(xm[1]-oxm[1]));
      if (dxy/err<3)  
	{
	  theAmas_[i].copyTo(vedges);
	  continue;
	}
      if (cdist/err>5.) 
	nb++;
      else
	theAmas_[i].copyTo(vedges);
      DEBUG_PRINT("[%d]=%d  distances ===> %f %f %f sigma \n",i,theAmas_[i].size(),dz,dxy,dxy/err);

      DEBUG_PRINT("%d  position ===> %f %f %f ---------- %f %f %f at %f %f sigma \n",i,obx,oby,dist,cx,cy,cdist,err,cdist/err);
      edsig->Fill(cdist/err);
    }
  // Now build an Amas with vedges
  //INFO_PRINT("On a construit avec vedge\n");
  Amas shCand(vedges);
  //INFO_PRINT("On a construit avec vedge 1\n");
  vedges.clear();
  //DEBUG_PRINT("On a construit avec vedge 2 \n");
  for (uint32_t k=0;k<60;k++)
    for (uint32_t i=0;i<96;i++)
      for (uint32_t j=0;j<96;j++)
	if (theImage_.getValue(k,i,j))
	  {
		
#ifdef OLD_DISTANCE_CUT
	    if (edges.getValue(k,i/3,j/3))
	      {
		bool appended=shCand.append(&hitVolume_[k][i][j],2);
		//DEBUG_PRINT("On a construit avec vedge 3 \n");
		if (!appended)
		  {
		    RecoHit& h=hitVolume_[k][i][j];
		    for (std::vector<RecoHit*>::iterator iht=shCand.getHits().begin();iht!=shCand.getHits().end();iht++)
		      {	
			uint32_t iDist=abs((*iht)->chamber()-h.chamber())+2*(abs(h.I()-(*iht)->I())+abs(h.J()-(*iht)->J()));
			if (iDist<15)
			  {
			    shCand.add(&hitVolume_[k][i][j]);
			    //DEBUG_PRINT("On a construit avec vedge %d \n",iDist);
			    break;
			  }
		      }
		  }
	      }
#endif
		
	    RecoHit& h=hitVolume_[k][i][j];
	    if (std::find(shCand.getHits().begin(),shCand.getHits().end(),&h)!=shCand.getHits().end()) continue;
	    for (std::vector<RecoHit*>::iterator iht=shCand.getHits().begin();iht!=shCand.getHits().end();iht++)
	      {	
		uint32_t iDist=abs((*iht)->chamber()-h.chamber())+2*(abs(h.I()-(*iht)->I())+abs(h.J()-(*iht)->J()));
		if (iDist<15)
		  {
		    shCand.add(&hitVolume_[k][i][j]);
		    //DEBUG_PRINT("On a construit avec vedge %d \n",iDist);
		    break;
		  }
	      }
		
	  }
	
  //INFO_PRINT("On a copie dans vedge\n");
  shCand.copyTo(vedges);
  // INFO_PRINT("On a appele ImageBuilder All %d  Edge %d vsize %d \n",theNall_,theNedge_,vedges.size());
  // this->ImageBuilder(vedges);
  //INFO_PRINT("On a applele ShowerBuilder\n");
  this->ShowerBuilder(vedges);
  //INFO_PRINT("On a fini\n");
  return nb;
}

void ShowerAnalyzer::processEvent()
{

  checkTime();
  if (reader_->getEvent()==0) return;

  evt_=reader_->getEvent();
  if (evt_->getEventNumber()<=theSkip_) return;
  IMPL::LCCollectionVec* rhcol=NULL;
  bool rhcoltransient=false;
  if (rebuild_)
    {
      reader_->parseRawEvent();
      //      DEBUG_PRINT("End of parseraw \n");
      //reader_->flagSynchronizedFrame();
#if DU_DATA_FORMAT_VERSION <= 12
      if (useSynchronised_)
	{
	  //  DEBUG_PRINT("Calling FastFlag\n");
	  reader_->fastFlag(2,minChambersInTime_);
	}
      //
      // getchar();

      rhcol=reader_->createRawCalorimeterHits(useSynchronised_);
      rhcoltransient=true;
#else
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
      //DEBUG_PRINT("Calling CreaetRaw\n");

      rhcol=reader_->createRawCalorimeterHits(seed);
      rhcoltransient=true; 
      //DEBUG_PRINT("End of CreaetRaw %d \n",rhcol->getNumberOfElements());
#endif
    }
  else
    rhcol=(IMPL::LCCollectionVec*) evt_->getCollection(collectionName_);

  

  theMonitoring_.FillTimeAsic(rhcol);
  //  LCTOOLS::printParameters(rhcol->parameters());
  //DEBUG_PRINT("Time Stamp %d \n",evt_->getTimeStamp());
  if (rhcol==NULL) return;
  if (rhcol->getNumberOfElements()==0) return;
  //DEBUG_PRINT("Calling decodeTrigger\n");

  if (!decodeTrigger(rhcol,spillSize_)) { if (rhcoltransient) delete rhcol;return;}





  // unsigned char image[60][96][96];
  // unsigned char imagew[60][96][96];
  // unsigned char cores[60][96][96];
  // unsigned char edges[60][96][96];
  std::vector<uint32_t> vseeds;
  findTimeSeeds(rhcol,10,vseeds);
  std::sort(vseeds.begin(),vseeds.end(),std::greater<uint32_t>());
	
  printf("================>  %d  Number of seeds %d \n",evt_->getEventNumber(),(int) vseeds.size());
  if (evt_->getEventNumber()%100 == 0)
    rootHandler_->writeXML(theMonitoringPath_);
  if (vseeds.size()==0)  { if (rhcoltransient) delete rhcol;return;}
  // getchar();
  array3D<unsigned char> edges;
  edges.initialise(&theImageEdgeBuffer_[0],60,NX,NY);
  array3D<unsigned char> cores;
  cores.initialise(&theImageCoreBuffer_[0],60,NX,NY);
  unsigned char temp[60*NX*NY];
  array3D<unsigned char> tempim;
  tempim.initialise(temp,60,NX,NY);
  theNbShowers_=0;
  for (std::vector<uint32_t>::iterator is=vseeds.begin();is!=vseeds.end();is++)
    {
      currentTime_=(*is);

      theAbsoluteTime_=theBCID_-currentTime_;
      if (theBCIDSpill_==0) theBCIDSpill_=theAbsoluteTime_;
      if (theAbsoluteTime_-theBCIDSpill_>15/2E-7) theBCIDSpill_=theAbsoluteTime_;

      DEBUG_PRINT("GTC %d DTC %d BCID %llu Current Time %llu Time SPill %f Distance %f \n",theGTC_,theDTC_,theBCID_,currentTime_,theBCIDSpill_*2E-7,(theAbsoluteTime_-theBCIDSpill_)*2E-7);
      //      continue;
      
      tempim.clear();
      // DEBUG_PRINT("Building voulume for %d \n",(*is));
      uint32_t nhits=buildVolume(rhcol,(*is));
      // DEBUG_PRINT("Edge detection for %d \n",(*is));
      buildEdges();
      theNplans_=0;
      std::vector<RecoHit*> vrh;vrh.clear();
      for (uint32_t k=0;k<60;k++)
	{ bool found=false;
	  for (uint32_t i=0;i<96;i++)
	    for (uint32_t j=0;j<96;j++)
	      if (theImage_.getValue(k,i,j)>0) 
		{tempim.setValue(k,i/3,j/3,1);
		  //printf("%d%d %d %d \n",i,j,hitVolume_[k][i][j].getFlag(RecoHit::EDGE),hitVolume_[k][i][j].getFlag(RecoHit::ISOLATED));
		  //if ((hitVolume_[k][i][j].getFlag(RecoHit::EDGE)==1||hitVolume_[k][i][j].getFlag(RecoHit::ISOLATED)==1) )
		    vrh.push_back(&hitVolume_[k][i][j]);
		  found=true;}
	  if (found) theNplans_++;
	}
      
      if (theNplans_<minChambersInTime_) continue;

      
      ShowerParams ish;
      Shower::computePrincipalComponents(vrh,(double*) &ish);

      if (sqrt((ish.lambda[0]+ish.lambda[1])/ish.lambda[2])<0.1) continue;
      if (sqrt((ish.lambda[0])/ish.lambda[2])<0.05) continue;
      if (ish.xm[0]<5) continue;

      double* v=ish.l2;
      double p[3];
      for (uint32_t i=0;i<3;i++) p[i]=v[i]/ish.lambda[2];
      double* x=ish.xm;    
      RecoCandTk t;
      t.ax_ =p[0]/p[2];
      t.ay_ =p[1]/p[2];
      t.bx_=x[0]-t.ax_*x[2];
      t.by_=x[1]-t.ay_*x[2];
      if (t.bx_<5 || t.bx_>95) continue;
      if (t.by_<5 || t.by_>95) continue;
      if (!(abs(x[0]-55)<10&&abs(x[1]-49)<10)) continue;
      std::cout<<t.bx_<<std::endl;
      uint32_t nshower=buildClusters(vrh);
      // if (nshower>0) 
      // 	{
      // 	  printf("Angles %f %f \n",t.ax_,t.ay_);
      // 	  INFO_PRINT(" %d Hits Composantes principales %f %f %f => %.2f  and %.2f  \n",vrh.size(),ish.lambda[0],ish.lambda[1],ish.lambda[2], sqrt((ish.lambda[0]+ish.lambda[1])/ish.lambda[2]), sqrt((ish.lambda[0])/ish.lambda[2]));
      // 	  getchar();
      // 	}
      continue;

      std::vector<RecoHit*> vrh1;
      for (std::vector<RecoHit*>::iterator iht=vrh.begin();iht!=vrh.end();iht++)
	{
	  double xe=t.ax_*(*iht)->Z()+t.bx_;
	  double ye=t.ay_*(*iht)->Z()+t.by_;
	  double dist=sqrt((xe-(*iht)->X())*(xe-(*iht)->X())+(ye-(*iht)->Y())*(ye-(*iht)->Y()));
	  if (dist<5)
	    vrh1.push_back((*iht));
	}
      if (vrh1.size()>0.9*vrh.size())
	{
	  printf(" Une trace sans doute %ld hits near\n",vrh1.size());
	  continue;
	}
      Shower::computePrincipalComponents(vrh1,(double*) &ish);
      DEBUG_PRINT(" Second fit %d Composantes principales %f %f %f => %f \n",vrh1.size(),ish.lambda[0],ish.lambda[1],ish.lambda[2], sqrt((ish.lambda[0]+ish.lambda[1])/ish.lambda[2]));
      if (vrh1.size()>0.8*vrh.size() && sqrt((ish.lambda[0]+ish.lambda[1])/ish.lambda[2])<0.1) continue;


      //this->PointsBuilder(vrh);
      //this->draw(tkgood_,allpoints_);
      // char c;c=getchar();putchar(c); if (c=='.') exit(0);;
      if (nAnalyzed_ == 0)
	{
#ifdef USE_CULA
	  culaStatus status = culaInitialize();
	  printf("CULA is initialized %d \n",status);
	  benchSgesvd(96);
#endif
	  theNtupleFile_= new TFile("theNtuple.root","RECREATE");
	  theNtupleFile_->cd();
	  theNtuple_=new TNtuple("nt","Summary ntuple","nall:nedge:ncore1:ncore2:nedge3:ncore3:nbig:zm:zf:zl:pa:w:np");

	  for (uint32_t ipl=0;ipl<61;ipl++)
	    {
	      double* c = new double[64];
	      for (uint32_t ipad=0;ipad<63;ipad++) c[ipad]=1.;
	      std::pair<uint32_t,double*> p(ipl,c);
	      theCorreff_.insert(p);
				
	    }

	  try {
	    this->readCalibration(evt_->getRunNumber());
	  }
	  catch(std::string s)
	    {
	      std::cout<<s<<std::endl;
	    }

	  if (useSqlite_)
	    {
	      std::stringstream sb;
	      sb<<"/dev/shm/db"<<evt_->getRunNumber()<<".sl3";
	      openSqlite(sb.str());
	    }
	  if (useMysql_)
	    connect("mirabito/braze1@lyosdhcal11:BEAM_TEST_2012_E");

	  //this->createTrees("./toto.root");
	}
      nAnalyzed_++;
	

      this->ShowerBuilder(vrh);
      
      continue;
	
      EdgeDetection(tempim,cores,edges);
      vrh.clear();
      // Remove isolated hits
      for (uint32_t k=0;k<60;k++)
	for (uint32_t i=0;i<96;i++)
	  for (uint32_t j=0;j<96;j++)
	    if (theImage_.getValue(k,i,j))
	      {
	
		//		if (edges.getValue(k,i/3,j/3)==2)		  
		//  theImage_.setValue(k,i,j,10);
		//	else
		  {
		    vrh.push_back(&hitVolume_[k][i][j]);
		  }
	      }

      Shower::computePrincipalComponents(vrh,(double*) &ish);

      if (sqrt((ish.lambda[0]+ish.lambda[1])/ish.lambda[2])<0.1) continue;
      printf(" Second step %ld Hits Composantes principales %f %f %f => %f \n",vrh.size(),ish.lambda[0],ish.lambda[1],ish.lambda[2], sqrt((ish.lambda[0]+ish.lambda[1])/ish.lambda[2]));
      uint32_t ng=(theTag_>>16)&0xFFFF;
#define DRAW_DEBUG
      if (theNedge_==theNall_ || theAmas_.size()==0 )
	{
	  theEdge3_=0;theCore3_=0;
	  INFO_PRINT("%d Tracks!!!! Run %d Event %d Time %d Edge %d core %d Edge3 %d Core3 %d \n",nhits,evt_->getRunNumber(),evt_->getEventNumber(),currentTime_,theNedge_,theNall_,theEdge3_,theCore3_);
	  this->TracksBuilder();
	  theMonitoring_.trackHistos(tkgood_,allpoints_);
	  if (useMysql_) this->track2Db(tkgood_,allpoints_);
	  ntkbetween+=tkgood_.size();
#ifdef DRAW_DEBUG
	  if (tkgood_.size() == 10 && tkgood_[0].getList().size()<40 
	      && TMath::Abs(tkgood_[0].ax_)<0.1 
	      &&TMath::Abs(tkgood_[0].ay_)<0.1 
	      && TMath::Abs(tkgood_[0].bx_-50)<20 
	      && TMath::Abs(tkgood_[0].by_-50)<20)
	    {
	      this->draw(tkgood_,allpoints_);
	      char c;c=getchar();putchar(c); if (c=='.') exit(0);;
	    }
	  if (0)
	    {
	      this->draw(tkgood_,allpoints_);
	      char c;c=getchar();putchar(c); if (c=='.') exit(0);;
	    }
#endif
	  continue;
	}		
      if (theAmas_.size()==0) continue;
      uint32_t ncore=0,nedge=0,nall=0,ncore1=0,ncore2=0;
      for (uint32_t k=0;k<60;k++)
	for (uint32_t i=0;i<96;i++)
	  for (uint32_t j=0;j<96;j++)
	    if (theImage_.getValue(k,i,j))
	      {
		nall++;
		int ithr=hitVolume_[k][i][j].getAmplitude()&0x3;
		//if (ithr==1) 
		//if (ithr==2) {w=1;nh1[ch]++;}
		//if (ithr==3) {w=15;nh2[ch]++;}
		if (cores.getValue(k,i/3,j/3))
		  {
		    ncore++;
		    if (ithr==3) ncore2++;
		    if (ithr==1) ncore1++;
		  }
		else
		  nedge++;
	      }

      DEBUG_PRINT("%d amas ALL: %d  EDGE: %d CORE: %d %d %d =====================================> %f Energy: %f \n",ng,nall,nedge,ncore,ncore1,ncore2,nedge*100./nall,(nedge+ncore-ncore2-ncore1)*0.05+ncore1*0.15+ncore2*0.35);
		

      theBigCore_=mergeAmas(cores,edges);
      theCore1_=ncore1;
      theCore2_=ncore2;
      INFO_PRINT("%d Run %d Event %d Time %llu - %d Edge %d All %d Edge3 %d Core3 %d ==> %d\n",nhits,evt_->getRunNumber(),evt_->getEventNumber(),theBCID_,currentTime_,theNedge_,theNall_,theEdge3_,theCore3_,ntkbetween);
		
      if (theNedge_*1./theNall_>0.01 &&theNedge_*1./theNall_<0.8 )
	{
	  //newHT3(cores);
			
	  //DEBUG_PRINT("1\n");
	  theNtupleFile_->cd();
	  theNtuple_->Fill(1.*theNall_,1.*theNedge_,1.*theCore1_,1.*theCore2_,1.*theEdge3_,1.*theCore3_,1.*theBigCore_,theZMax_,theZFirst_,theZLast_,thePlanAfter_,theWeights_*1.,theNplans_*1.);
	  //DEBUG_PRINT("2\n");
	  DEBUG_PRINT("En 3x3 %d %d %d \n",theEdge3_,theCore3_,theWeights_);
	  if (theNedge_*1./theNall_<0.6  && theNall_>150)
	    ntkbetween=0;
#ifdef DRAW_DEBUG
	  if (nAnalyzed_%1!=0 || theBigCore_>=1)
	    {
	      printf("DRASWING!!!!!!!!!!!!!!!!!!!!!!! %d %d \n",nAnalyzed_,theBigCore_);
	      this->draw(theImage_,cores,edges);
	      char c;c=getchar();putchar(c); if (c=='.') exit(0);;
	    }
#endif
	  //theHTx_->Draw(rootHandler_);theHTy_->Draw(rootHandler_);
	}
      else 
	{
	  this->TracksBuilder();
	  theMonitoring_.trackHistos(tkgood_,allpoints_);
	  if (useMysql_) this->track2Db(tkgood_,allpoints_);
#ifdef DRAW_DEBUG
	  if (tkgood_.size() == 1 && tkgood_[0].getList().size()<40 
	      && TMath::Abs(tkgood_[0].ax_)<0.1 
	      &&TMath::Abs(tkgood_[0].ay_)<0.1 
	      && TMath::Abs(tkgood_[0].bx_-50)<20 
	      && TMath::Abs(tkgood_[0].by_-50)<20)
	    {
	      this->draw(tkgood_,allpoints_);
	      char c;c=getchar();putchar(c); if (c=='.') exit(0);;
	    }

	  this->draw(tkgood_,allpoints_);
	  char c;c=getchar();putchar(c); if (c=='.') exit(0);;
#endif
	  ntkbetween+=tkgood_.size();
	}
      if (theMonitoringPeriod_!=0)
	if (nAnalyzed_%theMonitoringPeriod_ ==0)	
	  {
	    rootHandler_->writeXML(theMonitoringPath_);
	    //char c;c=getchar();putchar(c); if (c=='.') exit(0);;
		
	  }
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

  theCountSpill_[theIdxSpill_%20] =  theNbShowers_;
  theTimeInSpill_[theIdxSpill_%20] = theMonitoring_.getEventIntegratedTime()*2E-7;

  // Integrated 10 last
  float nc=0;
  float tc=0;
  //printf("showers %uud ",theIdxSpill_ );
  for (int i=0;i<20;i++)
    {
      nc+=theCountSpill_[i];
      tc+=theTimeInSpill_[i];
      //printf("%f ",theCountSpill_[i]);
    }

  printf("\n %d Number of showers %d Event time %f -> Absolute bcid  %f-> Rate %f %f %f\n",evt_->getEventNumber(),theNbShowers_,theMonitoring_.getEventIntegratedTime()*2E-7,(theBCID_-theBCIDSpill_)*2E-7,nc,tc,nc/tc);
  theLastRate_=nc/tc;
  theLastBCID_=theBCID_;
  //etchar();

  if (rhcoltransient) delete rhcol;
  return;
  
  if (nAnalyzed_==0)
    {
      for (uint32_t ipl=0;ipl<61;ipl++)
	{
	  double* c = new double[64];
	  for (uint32_t ipad=0;ipad<63;ipad++) c[ipad]=1.;
	  std::pair<uint32_t,double*> p(ipl,c);
	  theCorreff_.insert(p);
			
	}

      try {
	this->readCalibration(evt_->getRunNumber());
      }
      catch(std::string s)
	{
	  std::cout<<s<<std::endl;
	}

      if (useSqlite_)
	{
	  std::stringstream sb;
	  sb<<"/dev/shm/db"<<evt_->getRunNumber()<<".sl3";
	  openSqlite(sb.str());
	}
      if (useMysql_)
	{
	  connect("mirabito/braze1@lyosdhcal11.in2p3.fr:BEAM_TEST_2012_B");
	}
    }
  // Event is accepted
  nAnalyzed_++;

  if (nAnalyzed_%20 ==0)
    {
      std::cout<<nAnalyzed_<<"====>"<<evt_->getRunNumber()<<":"<<evt_->getEventNumber()<<" found "<<trackIndex_<<" tracks "<<houghIndex_<<" HT candidate"<<std::endl;
      DEBUG_PRINT("Rhcol =%f TimeSort= %f Tracking= %f Histo= %f \n",theRhcolTime_,theTimeSortTime_,theTrackingTime_,theHistoTime_);
    }
  bool offtime = (nAnalyzed_%offTimePrescale_)==0;
  if (rhcol->getNumberOfElements()>maxHitCount_ && maxHitCount_!=0)
    {
      std::cout<<evt_->getRunNumber()<<":"<<evt_->getEventNumber()<<":"<<rhcol->getNumberOfElements()<<std::endl;
		
      { if (rhcoltransient) delete rhcol;return;}

    }
  if (evt_->getEventNumber()%1000==0)
    std::cout<<evt_->getEventNumber()<<" had:"<<rhcol->getNumberOfElements()<<std::endl;
  //Loop on hits and fill allrecs_
	
  hash_map<uint32_t,std::bitset<255> > timeDif;
  hash_map<uint32_t,std::bitset<61> > timeChamber;
	
  hash_map<uint32_t,std::vector<RecoHit*> > timeHits;
	
  timeDif.clear();
  timeChamber.clear();
  timeHits.clear();
  //hash_map<uint32_t,std::vector<RecoHit*> timeHits1.;
  //TH1* htdiff= rootHandler_->GetTH1("TimeDif");
  //  if (htdiff!=NULL)
  //  htdiff->Reset();
  //RecoHitPool::purge_memory();
  //RecoHitPool::release_memory();
  RecoHit* recotab=new RecoHit[rhcol->getNumberOfElements()];
  //  std::cout<<"Number of Hits "<<rhcol->getNumberOfElements()<<std::endl;
  //bool drawd=(rhcol->getNumberOfElements()>200000);
  bool drawd=(rhcol->getNumberOfElements()>2);

  // Essai de calcul des seed en temps

  hash_map<uint32_t,uint32_t> tcount;
  hash_map<uint32_t,int32_t> tedge;
  for (int i=0;i<rhcol->getNumberOfElements();i++)
    {
      IMPL::RawCalorimeterHitImpl* hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(i);
      if (hit==0) continue;
      uint32_t bc = hit->getTimeStamp();
      //      TH1* htdiff= rootHandler_->GetTH1("TimeDif");
      hash_map<uint32_t,uint32_t>::iterator it=tcount.find(bc);
      if (it!=tcount.end()) 
	it->second=it->second+1;
      else
	{
	  std::pair<uint32_t,uint32_t> p(bc,1);
	  tcount.insert(p);
	}
    }
  //std::cout<<tcount.size()<<" different times"<<std::endl;
  for (hash_map<uint32_t,uint32_t>::iterator it=tcount.begin();it!=tcount.end();it++)
    {
      hash_map<uint32_t,uint32_t>::iterator ita=tcount.find(it->first+1);
      hash_map<uint32_t,uint32_t>::iterator itb=tcount.find(it->first-1);
      int32_t c=-1*it->second;
      if (ita!=tcount.end()) c+=ita->second;
      if (itb!=tcount.end()) c+=itb->second;
      std::pair<uint32_t,int32_t> p(it->first,c);
      tedge.insert(p);
		
    }
  uint32_t nshti=0;
  for (hash_map<uint32_t,int32_t>::iterator it=tedge.begin();it!=tedge.end();)
    {

      if (it->second<-23)
	{
	  //std::cout<<it->first<<"====>"<<it->second<<" count="<<tcount[it->first]<<std::endl;	  
	  nshti++;
	  it++;
	}
      else
	tedge.erase(it++);
    }
  std::cout<<nshti<<" good showers "<< tedge.size()<<std::endl;
  if (nshti==0) return;
  //getchar();

  for (int i=0;i<rhcol->getNumberOfElements();i++)
    {
      IMPL::RawCalorimeterHitImpl* hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(i);
      if (hit==0) continue;
      if (offtime) DIFStudy(hit);
      // Decode
      unsigned int difid = hit->getCellID0()&0xFF;
      if (difid<1 || difid>255) continue;
      int asicid = (hit->getCellID0()&0xFF00)>>8;
      int channel= (hit->getCellID0()&0x3F0000)>>16;
      //streamlog_out(MESSAGE)<<"ch-"<<channel<<std::endl;
      unsigned int bc = hit->getTimeStamp();
      //      TH1* htdiff= rootHandler_->GetTH1("TimeDif");
      //      htdiff->Fill(bc*1.);
      bool intime=false;
      for (int32_t jt=-3;jt<=3;jt++)
	if (tedge.find(bc+jt) != tedge.end())
	  {
	    intime=true;
	    break;
	  }
      if (!intime) continue;
      if (bc>5E6)  continue;
      //DEBUG_PRINT("---------------------> %d %d \n",bc,difid);
      bool thr[3];
      //      DEBUG_PRINT("%x \n",hit->getCellID0());
      int ithr= hit->getAmplitude()&0x3;
      if ((hit->getAmplitude()&0x4)==0 && useSynchronised_) continue;
      //      std::cout<<difid<<" had:"<<asicid<<":"<<channel<<":"<<bc<<":"<<ithr<<std::endl;
      if (ithr==0)
	{
	  std::cout<<difid<<" had:"<<asicid<<":"<<channel<<":"<<bc<<":"<<ithr<<std::endl;
	  continue;
	}
      thr[0] = (ithr == 1);
      thr[1] = (ithr == 2);
      thr[2] = (ithr == 3);
      if (theSeuil_==1 && !thr[0] && !thr[2]) continue;
      if (theSeuil_==2 && !thr[2]) continue;

      std::map<unsigned int,DifGeom>::iterator idg = reader_->getDifMap().find(difid);
      DifGeom& difgeom = idg->second;
      uint32_t chid = idg->second.getChamberId();
      if (chid<1 || chid>61) continue;
      std::map<unsigned int,ChamberGeom>::iterator icg = reader_->getChamberMap().find( chid);
      ChamberGeom& chgeom = icg->second;
      RecoHit* ptrhit= &recotab[i];
      int32_t hrtype=2;
      if (chid>50) hrtype=11; // Hard coded MR
      ptrhit->initialise(difgeom,chgeom,hit,hrtype);
      //ptrhit->setIndices(i,0);
      ///RecoHit& rhit=recotab[i].initialise(difgeom,chgeom,hit);
      //RecoHit rhit(difgeom,chgeom,hit);
      //RecoHit& rhit=recotab[i];
      //std::cout<<difid<<" "<<chid<<std::endl;

      bool found=false;
      for (int32_t jt=-6;jt<=6;jt++)
	if (timeHits.find(bc+jt) != timeHits.end())
	  {
			
	    hash_map<uint32_t,std::bitset<255> >::iterator t=timeDif.find(bc+jt);
	    if (difid<255)
	      t->second.set(difid,true);  
	    hash_map<uint32_t,std::bitset<61> >::iterator tc=timeChamber.find(bc+jt);
	    if (chid<60)
	      tc->second.set(chid,true);
	    hash_map<uint32_t,std::vector<RecoHit*> >::iterator th=timeHits.find(bc+jt);
	    th->second.push_back(ptrhit);
	    found=true;
	    break;
	  }
      if (!found)
	{
			
	  std::bitset<255> sd(0);
	  if (difid<255)
	    sd.set(difid,true);
	  pair <uint32_t,std::bitset<255> > pd(bc,sd);
	  timeDif.insert(pd);
			
	  std::bitset<61> sc(0);
	  if (chid<60)
	    sc.set(chid,true);
	  pair <uint32_t,std::bitset<61> > pc(bc,sc);
	  timeChamber.insert(pc);
	  std::vector<RecoHit*> v;
	  //v.reserve(100);
	  v.push_back(ptrhit);
	  pair <uint32_t,std::vector<RecoHit*> > pv(bc,v);
	  timeHits.insert(pv);
	}
		

    }
  theRhcolTime_+=checkTime();
  std::vector<RECOCluster> vCluster(5000);
  vCluster.clear();
  if (offtime)
    {
      //uint32_t dtime=NoiseStudy(timeDif,timeChamber);
      FillTimeAsic(rhcol);
    }
#ifdef DEBUGDIF
  for (std::map<uint32_t,std::bitset<255> >::iterator it=timeDif.begin();it!=timeDif.end();it++)
    {
      uint32_t nchambers=0;
      for (unsigned int ib=0;ib<255;ib++)
	if (it->second[ib]!=0) nchambers++;
		
      // std::cout<<nchambers<<" "<<minChambersInTime_<<std::endl;
      if (nchambers<4) continue;
      std::cout<<"DIF pattern "<<it->first<<" "<<it->second<<std::endl;
    }
#endif  
  TH1* hnchint= (TH1F*) rootHandler_->GetTH1("ChambersInTime");
  TH1* hnpc= (TH1F*) rootHandler_->GetTH1("NumberOfPcs");
  if (hnchint==0)
    {
      hnchint=rootHandler_->BookTH1("ChambersInTime",110,-10.,100.);
      hnpc=rootHandler_->BookTH1("NumberOfPcs",1000,0.,1000.);
    }
  hnchint->Fill(-1);
  uint32_t nsynch=0;
  uint32_t ntk=0;
  uint32_t ntmax=0;
  for (hash_map<uint32_t,std::bitset<61> >::iterator it=timeChamber.begin();it!=timeChamber.end();it++)
    {
      uint32_t nchambers=0;
      for (unsigned int ib=0;ib<60;ib++)
	if (it->second[ib]!=0) nchambers++;
		
      // std::cout<<nchambers<<" "<<minChambersInTime_<<std::endl;
      if (nchambers<(uint32_t) minChambersInTime_) continue;
      if (it->first>ntmax) ntmax=it->first;
      hnchint->Fill(nchambers*1.);
      nsynch++;
      //std::cout<<"Chamber pattern "<<it->first<<" "<<it->second<<std::endl;
      //std::cout<<" Number of Hits "<<timeHits.find(it->first)->second.size()<<std::endl;
      //::sleep(1);
      //getchar();
      if (it->second[60]!=0) continue; // Already used
      std::vector<RecoHit*>& vrh= timeHits.find(it->first)->second;

      theEvent_.idx++;
      theEvent_.bcid=theBCID_;
      theEvent_.dtc=theDTC_;
      theEvent_.gtc=theGTC_;
      theEvent_.run=evt_->getRunNumber();
      theEvent_.event=evt_->getEventNumber();
      theEvent_.time=it->first;
      if (useSqlite_ || useMysql_) 
	fillEventTable(evt_->getRunNumber(),theDTC_,theBCID_,it->first,theEdge3_,theCore3_,theWeights_,theNplans_);
      allpoints_.clear();
      tkgood_.clear();
      //DEBUG_PRINT("NHit at %d is %d \n",it->first,vrh.size());
      if (vrh.size()>6000) continue;
      //DEBUG_PRINT("Event Time is %d \n",it->first);
      ImageBuilder(vrh);
      //if (theNedge_*1./theNall_>0.85) continue;
      ShowerBuilder(vrh);
      if (tkgood_.size()>0)
	{
	  trackHistos();
	  FillTrackTree();
	}
      /*
	else
	{
	if (theEvent_.showers==0)
	{
	PointsBuilder(vrh);
	HT();
	if (tkgood_.size()>0)
	{
	trackHistos();
	FillTrackTree();
	}
	}
	}
      */
      if (1) continue;
      // std::cout<<"Hit size" <<vrh.size()<<std::endl;
      // ::sleep(1);
      // getchar();
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
	  //std::cout<<"Apres push Clusters "<<vCluster.size()<<std::endl;
	}
      it->second.set(60,true); //Set this list of Hits used



      // Merge adjacent clusters
      //      std::cout<<"Avant merged Clusters "<<vCluster.size()<<std::endl;
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
	      // DEBUG_PRINT(" vCluster Size %d\n",vCluster.size());
	      //DEBUG_PRINT(" New clusters found %d\n",vNew.size());
	      for (std::vector<RECOCluster>::iterator ic=vNew.begin();ic!=vNew.end();ic++)
		vCluster.push_back((*ic));
	      //DEBUG_PRINT(" New clusters found %d\n",vCluster.size());
	    }
	} while (merged);
      //std::cout<<"Apres merged Clusters "<<vCluster.size()<<std::endl;
      //std::cout<<"Apres clean Clusters "<<vCluster.size()<<std::endl;

      // Look for time +15 and time+16
#ifdef TRYPLUS15
      if (timeHits.find(it->first+15)!=timeHits.end())
	{
	  timeChamber.find(it->first+15)->second.set(60,true);
	  vrh= timeHits.find(it->first+15)->second;

	  for (std::vector<RecoHit>::iterator ih=vrh.begin();ih!=vrh.end();ih++)
	    {
	      bool append= false;
	      //std::cout<<"Clusters "<<vCluster.size()<<std::endl;
	      RecoHit& hit=(*ih);
	      for (std::vector<RECOCluster>::iterator icl=vCluster.begin();icl!=vCluster.end();icl++)
		if (icl->Append(hit))
		  {
		    append=true;
		    break;
		  }
	      if (append) continue;
	      RECOCluster cl(hit);
	      //std::cout<<"Avant push Clusters "<<vCluster.size()<<std::endl;
	      vCluster.push_back(cl);
	      //std::cout<<"Apres push Clusters "<<vCluster.size()<<std::endl;
	    }
	}

      if (timeHits.find(it->first+16)!=timeHits.end())
	{
	  timeChamber.find(it->first+16)->second.set(60,true);
	  vrh= timeHits.find(it->first+16)->second;
	  for (std::vector<RecoHit>::iterator ih=vrh.begin();ih!=vrh.end();ih++)
	    {
	      bool append= false;
	      //std::cout<<"Clusters "<<vCluster.size()<<std::endl;
	      RecoHit& hit=(*ih);
	      for (std::vector<RECOCluster>::iterator icl=vCluster.begin();icl!=vCluster.end();icl++)
		if (icl->Append(hit))
		  {
		    append=true;
		    break;
		  }
	      if (append) continue;
	      RECOCluster cl(hit);
	      //std::cout<<"Avant push Clusters "<<vCluster.size()<<std::endl;
	      vCluster.push_back(cl);
	      //std::cout<<"Apres push Clusters "<<vCluster.size()<<std::endl;
	    }
	}
#endif
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
      currentTime_=it->first;
      // Now group points per chamber
      chamberPoints_.clear();
      bool micromegas=false;
      TH1* hmip= rootHandler_->GetTH1("Mips");
      TH1* hmip5= rootHandler_->GetTH1("Mips5");
      TH1* hnh0= rootHandler_->GetTH1("NumberOfHit0");
      TH1* hnh1= rootHandler_->GetTH1("NumberOfHit1");
      TH1* hnh2= rootHandler_->GetTH1("NumberOfHit2");
      TH1* hnht= rootHandler_->GetTH1("NumberOfHitTotal");
      TH1* henergy= rootHandler_->GetTH1("Energy");

      if (hmip==NULL)
	{
	  hmip =rootHandler_->BookTH1( "Mips",1000,0.,10000.);
	  hmip5 =rootHandler_->BookTH1( "Mips5",1000,0.,10000.);
	  hnh0 =rootHandler_->BookTH1( "NumberOfHit0",2000,0.,2000.);
	  hnh1 =rootHandler_->BookTH1( "NumberOfHit1",4000,0.,4000.);
	  hnh2 =rootHandler_->BookTH1( "NumberOfHit2",2000,0.,2000.);
	  hnht =rootHandler_->BookTH1( "NumberOfHitTotal",6000,0.,6000.);
	  henergy =rootHandler_->BookTH1( "Energy",15000,0.,150000.);

	}
      uint32_t nmip=0,nhit=0;
      uint16_t nh0[61],nh1[61],nh2[61],smip[61];
      memset(&nh0,0,61*sizeof(uint16_t));
      memset(&nh2,0,61*sizeof(uint16_t));
      memset(&nh1,0,61*sizeof(uint16_t));
      memset(&smip,0,61*sizeof(uint16_t));
      for (std::vector<RecoPoint>::iterator icl=allpoints_.begin();icl!=allpoints_.end();icl++)
	{
	  RECOCluster& c= (*icl).getCluster();
	  RecoPoint* p =&(*icl);
	  uint32_t ch=p->getChamberId();

	  uint32_t w=0;
	  for (std::vector<RecoHit>::iterator iht=c.getHits()->begin();iht!=c.getHits()->end();iht++)
	    {
	      int ithr= (*iht).getAmplitude()&0x3;
	      if (ithr==1) {w=3;nh0[ch]++;}
	      if (ithr==2) {w=1;nh1[ch]++;}
	      if (ithr==3) {w=15;nh2[ch]++;}
				
	      nmip+=w;
	      smip[ch]+=w;
	      nhit++;
				
	    }

	  micromegas|=ch>48;
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
      //
      hmip->Fill(nmip*1.);
      uint32_t fp=61,lp=1;
      for (uint32_t i=1;i<61;i++)
	{
	  if (smip[i]>25 && i<fp) fp=i;
	  if (smip[i]>25 && i>lp) lp=i;

	}
      if (fp>=2 && lp<46)
	{
	  uint32_t mt=0,mh1=0,mh2=0,mh0=0;
	  for (uint32_t i=fp-1;i<=lp+2;i++)
	    {
	      mt+=smip[i];
	      mh0+=nh0[i];
	      mh1+=nh1[i];
	      mh2+=nh2[i];
	    }
	  if (mt>40 && (lp-fp)>5 ) 
	    {
	      hmip5->Fill(nmip*1.);
	      hnh0->Fill(mh0*1.);
	      hnh1->Fill(mh1*1.);
	      hnh2->Fill(mh2*1.);
	      hnht->Fill((mh0+mh1+mh2)*1.);
	      henergy->Fill(mh0*69.83990544751262+mh1*70.06174985305178+mh2*69.71033820211227);
	    }
	}
      if (nmip>500)
	{
	  //DEBUG_PRINT("Pion  %d %d %d \n",evt_->getEventNumber(),currentTime_,nmip);
	  npi_++;
	}
      theTimeSortTime_+=checkTime();
      if (1) 
	{
	  //if ((externalTriggerTime_-lastSpill_)<0.9) continue;
	  tkgood_.clear();
	  if (nmip<25000)
	    {
				
	      HT();
	      if (nmip>350)
		HT3D();
	      //HTOld();
	      theTrackingTime_+=checkTime();
	      trackHistos();
	      theHistoTime_+=checkTime();
	      if (tkgood_.size()>0) ntk++;
	    }
	  // if (draw_ && nmip>250)
	  // 	{
	  // 	  this->drawDisplay();
			
	  // 	}
			
			
	  //       for (std::map<uint32_t,std::vector<RecoPoint*> >::iterator itch=chamberPoints_.begin();itch!=chamberPoints_.end();itch++)
	  // 	{
	  // 	  uint32_t np=itch->second.size();
			
	  // 	  std::cout<<np<<" points "<<std::endl;
	  // 	  if (np>40)
	  // 	    this->drawDisplay();
			
	  // 	}
	  //
	}
      else
	{
	  int npmax=0;
	  float npmean=0;
	  for (std::map<uint32_t,std::vector<RecoPoint*> >::iterator itch=chamberPoints_.begin();itch!=chamberPoints_.end();itch++)
	    {
	      uint32_t np=itch->second.size();
	      //std::cout<<itch->first<<" "<<itch->second.size()<<std::endl;
	      if (np>npmax) npmax=np;
	      npmean=npmean+np;
	    }
	  npmean=npmean/chamberPoints_.size();
	  if ( findTracks_ && npmean<2000002.5)//&& allpoints_.size()<60 && npmax<6 && npmean<2.5) 
	    {
	      if (oldAlgo_)
		findTracks();
	      else
		CosmicFinder();
				
				
	      //	if (tkgood_.size() )
	      //  std::cout<<tkgood_.size()<<" Tracks found "<<trackIndex_<<" "<<vCluster.size()<<" "<<allpoints_.size()<<std::endl;
	      trackHistos();
	      //       	if (tkgood_.size())
	      //this->drawDisplay();
	    }
	  else
	    {

	      tkgood_.clear();
	      std::cout<<"Shower "<<nchambers<<" "<<allpoints_.size()<<" "<<npmax<<" "<<npmean<<std::endl;
	      //this->drawDisplay();
	    }
	}
      // getchar();
    }
  hnpc->Fill(nsynch*1.);

  std::cout<<externalTriggerTime_<<" On sort  Event in synch "<<nsynch<<"  with tracks "<<ntk<<" Pions since Last spill "<<npi_<<"   max clock "<<ntmax<<std::endl;

  //for (uint32_t i=0;i<rhcol->getNumberOfElements();i++) delete &recotab[i];
  delete recotab;
  if (rhcoltransient) delete rhcol;
}

void ShowerAnalyzer::appendHits(RecoCandTk& tk)
{

  for (unsigned int ich=tk.lastChamber_+1;ich<61;ich++)
    {
      if (ich>60) break;
      std::map<uint32_t,std::vector<RecoPoint*> >::iterator ipch=chamberPoints_.find(ich);
      if (ipch!=chamberPoints_.end())
	{
	  // Loop on points and calculate min distance
	  float distmin=99999.; std::vector<RecoPoint*>::iterator ipmin;
	  for (std::vector<RecoPoint*>::iterator ip=ipch->second.begin();ip!=ipch->second.end();ip++)
	    {
	      float dist = tk.calculateDistance((**ip));
	      if (dist<distmin) {distmin=dist;ipmin=ip;}
	    }
	  if (distmin<tkDistCut_)
	    {


	      double chi2=distmin*distmin/((**ipmin).dX()*(**ipmin).dX()+(**ipmin).dY()*(**ipmin).dY());
	      if (chi2<10.)
		{
		  tk.addPoint((**ipmin));
		  tk.regression();
		  // std::cout<<" + adding "<<distmin<<" c2 "<<chi2<<" from "<<ich
		  // 	   <<" chi2 "<<tk.chi2_<<" # "<<tk.getList().size()
		  // 	   <<std::endl;
		}
	    }
	}
    }
  for (unsigned int ich=tk.firstChamber_-1;ich>0;ich--)
    {
      if (ich<1) break;
      std::map<uint32_t,std::vector<RecoPoint*> >::iterator ipch=chamberPoints_.find(ich);
      if (ipch!=chamberPoints_.end())
	{
	  // Loop on points and calculate min distance
	  float distmin=99999.; std::vector<RecoPoint*>::iterator ipmin;
	  for (std::vector<RecoPoint*>::iterator ip=ipch->second.begin();ip!=ipch->second.end();ip++)
	    {
	      float dist = tk.calculateDistance((**ip));
	      if (dist<distmin) {distmin=dist;ipmin=ip;}
	    }
	  if (distmin<tkDistCut_)
	    {

	      double chi2=distmin*distmin/((**ipmin).dX()*(**ipmin).dX()+(**ipmin).dY()*(**ipmin).dY());
	      if (chi2<10.)
		{
		  tk.addPoint((**ipmin));
		  tk.regression();
		  // std::cout<<" - adding "<<distmin<<" c2 "<<chi2<<" from "<<ich
		  // 	   <<" chi2 "<<tk.chi2_<<" # "<<tk.getList().size()
					
		  // <<std::endl;
		}


	      // std::cout<<" - adding "<<distmin<<" from "<<ich
	      // 	       <<" chi2 "<<tk.chi2_<<" # "<<tk.getList().size()
	      // 	       <<std::endl;	  
				
	    }
	}
    }
  //std::cout<<tk.getList().size()<<std::endl;
  // getchar();
  tk.regression();
}

void ShowerAnalyzer::findTracks()
{

  tkgood_.clear();
  tklist_.clear();
  for (unsigned int i=0;i<allpoints_.size();i++)
    {
      allpoints_[i].setUsed(false);
      //      if (allpoints_[i].Z()!=z1) continue;
      for (unsigned int j=i+1;j<allpoints_.size();j++)
	{
	  //  if (j==i) continue;
	  if (allpoints_[j].getChamberId()==allpoints_[i].getChamberId()) continue;
	  int dch=abs((int) (allpoints_[j].getChamberId()-allpoints_[i].getChamberId()));
	  //if (allpoints_[j].getChamberId()>allpoints_[i].getChamberId()+1) continue;
	  if (dch>2) continue;
	  RecoCandTk tk;
	  tk.addPoint(allpoints_[i]);
	  tk.addPoint(allpoints_[j]);
	  tk.regression();
	  for (unsigned int k=j+1;k<allpoints_.size();k++)
	    {
	      if (allpoints_[k].getChamberId()==allpoints_[i].getChamberId()) continue;
	      if (allpoints_[k].getChamberId()==allpoints_[j].getChamberId()) continue;
	      int dch1=abs((int) (allpoints_[j].getChamberId()-allpoints_[k].getChamberId()));
	      //if (allpoints_[k].getChamberId()>allpoints_[j].getChamberId()+1) continue;
	      if (dch1>2) continue;
	      if (tk.calculateDistance(allpoints_[k])>tkDistCut_) continue;
	      tk.addPoint(allpoints_[k]);

	      tk.regression();
	      tklist_.push_back(tk);
	      break;
	    }
	  //tk.Print();
	  //getchar();
	}
    }

  //  std::cout<<tklist_.size()<<" segments"<<std::endl;
  for (unsigned int itk=0;itk<tklist_.size();itk++)
    {
      RecoCandTk& tk = tklist_[itk];
      tk.addPoints(allpoints_,tkDistCut_);
      //      tk.addPoints(allpoints_,83.1,5.,2.);
      // tk.addPoints(allpoints_,131.7,5.,2.);
      //std::cout<<tk.getList().size()<<" points" << std::endl;
      if (tk.getList().size()>=(uint32_t)tkMinPoint_)
	{
	  bool found =false;
	  //	  DEBUG_PRINT("\t avant tkgood %d nb hit =%d  %f %f %f\n",itk,tk.getList().size(),tk.ax_,tk.ay_,tk.chi2_);
	  for (std::vector<RecoCandTk>::iterator jt=tkgood_.begin();jt!=tkgood_.end();jt++)
	    {
	      if (fabs(tk.ax_-jt->ax_)+fabs( tk.bx_-jt->bx_)+fabs(tk.ay_-jt->ay_)+fabs(tk.by_-jt->by_)<1E-4) {found=true;break;}
	    }
			
	  if (!found)
	    {
	      //std::cout<<tk.chi2_<<" "<<TMath::Prob(tk.chi2_,2*tk.getList().size()-4)<<" "<<tkChi2Cut_<<std::endl;

	      // Make a ROOT Fit
	      double theProb=1.;

#ifdef ROOT_FIT
	      TF1 fa("fa","[0]*x+[1]",0.,120.);  

	      float x[60],ex[60],y[60],ey[60];
	      for (uint32_t jj=0;jj<tk.getList().size();jj++)
		{
		  x[jj]=(tk.getList())[jj]->Z();
		  ex[jj]=0.1;
		  y[jj] = (tk.getList())[jj]->X();
		  ey[jj] = (tk.getList())[jj]->dX();
					
		}

	      TGraphErrors* g= new TGraphErrors(tk.getList().size(),x,y,ex,ey);

	      g->Fit(&fa,"Q");
	      // std::cout<<fa.GetProb()<<std::endl;
	      if (fa.GetProb()<theProb) theProb=fa.GetProb();
	      //pf.Get()->Print();
	      for (uint32_t jj=0;jj<tk.getList().size();jj++)
		{
					
		  y[jj] = (tk.getList())[jj]->Y();
		  ey[jj] = (tk.getList())[jj]->dY();
		}
	      TGraphErrors* g1= new TGraphErrors (tk.getList().size(),x,y,ex,ey);
	      g1->Fit(&fa,"Q");
	      if (fa.GetProb()<theProb) theProb=fa.GetProb();
	      delete g;
	      delete g1;
	      tk.prChi2_=theProb;
	      //pf1.Get()->Print();
#else
				
	      tk.prChi2_=TMath::Prob(tk.chi2_,2*tk.getList().size()-4);
#endif
	      //getchar();
	      //	      if (TMath::Prob(tk.chi2_,2*tk.getList().size()-4)>tkChi2Cut_) 
	      if (theProb>tkChi2Cut_)
		{
		  //std::cout<<trackIndex_<<" "<<theProb<<std::endl;

		  tkgood_.push_back(tk);
		  //		  std::cout<<"added"<<std::endl;
		}
	    }
	}
    }
  // std::cout<<tkgood_.size()<<" good segments"<<std::endl;
  // Tag points belonging to several tracks
  for (unsigned int itk=0;itk<tkgood_.size();itk++)
    {
      RecoCandTk& tk = tkgood_[itk];
      std::vector<double>& dist1=tk.getChi2();
      for (unsigned int ih=0;ih<tk.getList().size();ih++)
	{
	  //	   std:: cout<<ih<<"  ----->"<<dist1[ih]<<std::endl;

	  RecoPoint& pt= *(tk.getList())[ih];
	  for (unsigned jtk=itk+1;jtk<tkgood_.size();jtk++)
	    {
	      if (jtk==itk) continue;
	      RecoCandTk& tk2 = tkgood_[jtk];
	      std::vector<double>& dist2=tk2.getChi2();
	      for (unsigned int jh=0;jh<tk2.getList().size();jh++)
		{

		  RecoPoint& pt2= *(tk2.getList())[jh];
		  //		   std:: cout<<jh<<"  ----->"<<dist2[jh]<<std::endl;
		  // std::cout<<itk<<" "<<ih<<" "<<jtk<<" "<<jh<<std::endl;
		  if (pt.getPointId() == pt2.getPointId()  )
		    {

		      //DEBUG_PRINT("common hit  %f %f\n",dist1[ih],dist2[ih]);
		      if (dist1[ih]<dist2[jh])
			dist2[jh]=99999.;
		      else
			dist1[ih]=99999.;
		      //   		       pt.Print()

		      // getchar();
		    }
		}
	    }
	}
    }

  // Refit the track without bad points  
  for (unsigned int itk=0;itk<tkgood_.size();itk++)
    {
      RecoCandTk& tk = tkgood_[itk];
      //    DEBUG_PRINT("\t before %d nb hit =%d  %f %f %f\n",itk,tk.getList().size(),tk.ax_,tk.ay_,tk.chi2_);
      tk.clean();
      // DEBUG_PRINT("\t %d nb hit =%d \n",itk,tk.getList().size());
    }



  // remove candidate with less than 3 points
  for (std::vector<RecoCandTk>::iterator i=tkgood_.begin();i!=tkgood_.end();)
    {
      if (i->getList().size()<(uint32_t) tkMinPoint_)
	{
	  tkgood_.erase(i++);
	}
      else
	{
	  i->setValid(true);
	  i++;
	}
    }

  for (unsigned int itk=0;itk<tkgood_.size();itk++)
    {
      RecoCandTk& tk = tkgood_[itk];
      if (!tk.isValid())continue;
      //std::cout<<"TK BX "<<tk.bx_<<" "<<tk.by_<<std::endl;
      for (unsigned int jtk=itk+1;jtk<tkgood_.size();jtk++)
	{
	  RecoCandTk& tkj = tkgood_[jtk];
	  if (!tkj.isValid())continue;
	  //std::cout<<"TKJ BX "<<tkj.bx_<<" "<<tkj.by_<<std::endl;
	  if (fabs(tk.bx_-tkj.bx_)>2. && fabs(tk.by_-tkj.by_)>2.) continue;

	  if (tk.prChi2_>tkj.prChi2_)
	    {
	      std::cout<<"Removing tkj "<<tkj.by_<<std::endl;
	      for (unsigned int ih=0;ih<tkj.getList().size();ih++)
		{

		  RecoPoint& pt= *(tkj.getList())[ih];
		  tk.addPoint(pt);
		}
	      tk.regression();
	      tkj.setValid(false);
	    }
	  if (tkj.prChi2_>tk.prChi2_)
	    {
	      std::cout<<"Removing tk "<<tk.by_<<std::endl;
	      for (unsigned int ih=0;ih<tk.getList().size();ih++)
		{

		  RecoPoint& pt= *(tk.getList())[ih];
		  tkj.addPoint(pt);
		}
	      tkj.regression();
	      tk.setValid(false);
	    }

	}
    }

  for (std::vector<RecoCandTk>::iterator i=tkgood_.begin();i!=tkgood_.end();)
    {
      if (!i->isValid())
	{
	  tkgood_.erase(i++);
	}
      else
	i++;
    }





  for (unsigned int itk=0;itk<tkgood_.size();itk++)
    {
      RecoCandTk& tk = tkgood_[itk];
      for (unsigned int ih=0;ih<tk.getList().size();ih++)
	{

	  RecoPoint& pt= *(tk.getList())[ih];
	  pt.setUsed(true);
	  for (unsigned int i=0;i<allpoints_.size();i++)
	    {
	      if (pt.getPointId()==allpoints_[i].getPointId())
		{
		  allpoints_[i].setUsed(true);
		  break;
		}
	    }

	}
    }
  // std::vector<RecoCandTk> tkfinal_;
  // for (std::vector<RecoCandTk>::iterator i=tkgood_.begin();i!=tkgood_.end();i++)
  //   {
	
  //   }

  // DEBUG_PRINT("%d %d had: good tracks found \n",tklist_.size(),tkgood_.size());
  //  getchar();
}






void ShowerAnalyzer::trackHistos()
{
  TH1* htimedif = rootHandler_->GetTH1("TimeToTrigger");
  if (htimedif==NULL)
    {
      htimedif=rootHandler_->BookTH1( "TimeToTrigger",5000,0.,5000.);

    }
  htimedif->Fill(currentTime_*1.);
  std::string tkdir="/Tracking";
  if (currentTime_>=(uint32_t)clockSynchCut_) tkdir="/OtherTracking";

  for (unsigned int i=0;i<allpoints_.size();i++)
    {	 

      //if (allpoints_[i].getCluster().getHits()->size()>1) continue;
      std::stringstream namec("");


      namec<<tkdir+"/Plan"<<allpoints_[i].getChamberId();
      if (allpoints_[i].isUsed()) 
	namec<<"/OnTrack";
      else
	namec<<"/OffTrack";
      TH2* hpos = rootHandler_->GetTH2(namec.str()+"/XYPos");	   
      TH2* hcpos = rootHandler_->GetTH2(namec.str()+"/XYClusterPos");	   
      TH2* hposhit = rootHandler_->GetTH2(namec.str()+"/XYPosHit");	   
      TH1* hposmul = rootHandler_->GetTH1(namec.str()+"/Multiplicity");	   
      if (hpos==NULL)
	{
	  hpos=rootHandler_->BookTH2( namec.str()+"/XYPos",115,-10.1,110.1,115,-10.1,110.1);
	  hcpos=rootHandler_->BookTH2( namec.str()+"/XYClusterPos",100,0.,96.,100,0.,96.);
	  hposhit=rootHandler_->BookTH2( namec.str()+"/XYPosHit",96,0.,96.,96,0.,96.);
	  hposmul=rootHandler_->BookTH1( namec.str()+"/Multiplicity",50,0.,50.);
	}
      hpos->Fill(allpoints_[i].X(),allpoints_[i].Y());
      hcpos->Fill(allpoints_[i].getCluster().X(),allpoints_[i].getCluster().Y());
      hposmul->Fill(allpoints_[i].getCluster().getHits()->size()*1.);
      for (std::vector<RecoHit>::iterator ih=allpoints_[i].getCluster().getHits()->begin();ih!=allpoints_[i].getCluster().getHits()->end();ih++)
	hposhit->Fill(ih->X()*1.,ih->Y()*1.);	
    }

  TH1* hngood = rootHandler_->GetTH1(tkdir+"/NumberOfTracks");
  if (hngood==0)
    {
      hngood = rootHandler_->BookTH1(tkdir+"/NumberOfTracks",21,-0.1,20.9);
    }
  hngood->Fill(tkgood_.size()*1.);
  if (tkgood_.size()==0) return;
  for (unsigned int itk=0;itk<tkgood_.size();itk++)
    {
      trackIndex_++;
      RecoCandTk& tk = tkgood_[itk];
      //DEBUG_PRINT("Tk=%d Time=%d %f %f %f %f %f \n",trackIndex_,currentTime_,tk.ax_,tk.bx_,tk.ay_,tk.by_,tk.chi2_);
      TH1* htchi2 = rootHandler_->GetTH1(tkdir+"/Chi2");
      TH1* htpchi2 = rootHandler_->GetTH1(tkdir+"/ProbChi2");
      TH1* htnpoint = rootHandler_->GetTH1(tkdir+"/NumberOfPoints");
      TH1* htax = rootHandler_->GetTH1(tkdir+"/Ax");
      TH1* htay = rootHandler_->GetTH1(tkdir+"/Ay");
      TH1* htxh = rootHandler_->GetTH1(tkdir+"/xh");
      TH1* htyh = rootHandler_->GetTH1(tkdir+"/yh");

      if (htchi2==0)
	{
	  htchi2 = rootHandler_->BookTH1(tkdir+"/Chi2",500,0.,100.);
	  htpchi2 = rootHandler_->BookTH1(tkdir+"/ProbChi2",1000,0.,1.);
	  htnpoint = rootHandler_->BookTH1(tkdir+"/NumberOfPoints",60,0.,60.);
	  htax = rootHandler_->BookTH1(tkdir+"/Ax",200,-10.,10.);
	  htay = rootHandler_->BookTH1(tkdir+"/Ay",200,-10.,10.);
	  htxh = rootHandler_->BookTH1(tkdir+"/xh",6000,0.,6000.);
	  htyh = rootHandler_->BookTH1(tkdir+"/yh",6000,0.,6000.);

	}

      htchi2->Fill(tk.chi2_/(2*tk.getList().size()-4.));
      htpchi2->Fill(tk.prChi2_);
      //DEBUG_PRINT("track %f %d %f %f \n",tk.chi2_,2*tk.getList().size()-4,TMath::Prob(tk.chi2_,2*tk.getList().size()-4),tkChi2Cut_);
      //getchar();
      htnpoint->Fill(tk.getList().size()*1.);

      htax->Fill(tk.ax_);

      htay->Fill(tk.ay_);


      for (unsigned int ich=0;ich<61;ich++)
	{
	  uint32_t bintk=((trackIndex_-1)%100)*60+ich+1;
	  htxh->SetBinContent(bintk,0);
	  htyh->SetBinContent(bintk,0);
	}

      //       if (tkgood_.size()>1)
      //DEBUG_PRINT("\t %d good hits found \n",tk.getList().size());
      for (unsigned int i =0;i<tk.getList().size();i++)
	{
	  // if (tkgood_.size()>1)
	  //   DEBUG_PRINT("\t \t %f %f %f \n",tk.getList()[i].X(),tk.getList()[i].Y(),tk.getList()[i].Z());
	  //(tk.getList())[i].Print();
	  std::stringstream namec("");
	  namec<<tkdir+"/Plan"<<(tk.getList())[i]->getChamberId();
	  TH1* hposx = rootHandler_->GetTH1(namec.str()+"/XPos");	   
	  TH1* hposy = rootHandler_->GetTH1(namec.str()+"/YPos");	   
	  TH1* hpullx = rootHandler_->GetTH1(namec.str()+"/XDist");	   
	  TH1* hpully = rootHandler_->GetTH1(namec.str()+"/YDist");	   
	  TH1* hmult = rootHandler_->GetTH1(namec.str()+"/Multiplicity");	   

	  if (hposx==0)
	    {
				
	      hposx =rootHandler_->BookTH1( namec.str()+"/XPos",115,-10.,110.);
	      hposy =rootHandler_->BookTH1( namec.str()+"/YPos",115,-10.,110.);
	      hpullx =rootHandler_->BookTH1( namec.str()+"/XDist",200,-5.,5.);
	      hpully =rootHandler_->BookTH1( namec.str()+"/YDist",200,-5.,5.);
	      hmult =rootHandler_->BookTH1( namec.str()+"/Multiplicity",50,0.,50.);
				
	    }
	  uint32_t bintk=((trackIndex_-1)%100)*60+(tk.getList())[i]->getChamberId();
	  htxh->SetBinContent(bintk,(tk.getList())[i]->X());
	  htyh->SetBinContent(bintk,(tk.getList())[i]->Y());

	  hposx->Fill((tk.getList())[i]->X());
	  hposy->Fill((tk.getList())[i]->Y());
	  hpullx->Fill((tk.getList())[i]->X() -tk.getXext((tk.getList())[i]->Z()) );
	  hpully->Fill((tk.getList())[i]->Y() -tk.getYext((tk.getList())[i]->Z()) );
	  hmult->Fill(tk.getList()[i]->getCluster().getHits()->size()*1.);
	}


      if (fabs(tk.ax_)>tkAngularCut_ || fabs(tk.ay_)>tkAngularCut_) continue;
      if (tkgood_.size()!=1) continue;

      std::bitset<255> intrack;
      bool synch= (currentTime_< (uint32_t)clockSynchCut_);
      unsigned int s_shift=0;
      if (synch) 
	intrack.set(s_shift,true);
      else
	{
	  s_shift =100;
	  intrack.set(s_shift,true);

	}
      for (std::map<unsigned int,ChamberGeom>::iterator ip=reader_->getChamberMap().begin();ip!=reader_->getChamberMap().end();ip++)
	{
	  // std::cout<<ip->first<<" "<<ip->second.getZ()<<std::endl;
	  //getchar();
	  int chid = ip->first;
	  int32_t interval=3;
	  if (chid<=tkFirstChamber_+1) interval=5;
	  if (chid>=tkLastChamber_-1) interval=5;

	  int32_t tkFirstEx=((chid-interval)>tkFirstChamber_)?(chid-interval):tkFirstChamber_;
	  int32_t tkLastEx=((chid+interval)<tkLastChamber_)?(chid+interval):tkLastChamber_;
	  //std::cout<<chid<<" "<<tkFirstEx<<" "<<tkLastEx<<std::endl;

	  RecoCandTk tk0;
	  for (unsigned int j =0;j<tk.getList().size();j++)
	    {
	      //std::cout<<fabs(tk.getList()[j].Z()-zplane[ip])<<std::endl;
	      if (tk.getList()[j]->getChamberId()==chid) continue;
				
	      tk0.addPoint(*tk.getList()[j]);
	    }
	  //std::cout<<" extra "<<chid<<" " <<tk0.getList().size()<<std::endl;
	  float xext=-999999,yext=-999999;
	  if (useTk4_ && chid>=tkFirstChamber_ && chid<=tkLastChamber_)
	    {
	      RecoCandTk tk4;
	      for (unsigned int j =0;j<tk.getList().size();j++)
		{
		  //std::cout<<fabs(tk.getList()[j].Z()-zplane[ip])<<std::endl;
		  if (tk.getList()[j]->getChamberId()==chid) continue;
		  if (tk.getList()[j]->getChamberId()<tkFirstEx) continue;
		  if (tk.getList()[j]->getChamberId()>tkLastEx) continue;
		  tk4.addPoint(*tk.getList()[j]);

		  // if (chid==tkLastChamber_)
		  //   if (tk.getList()[j]->getChamberId()==(chid-3)) tk4.addPoint(*tk.getList()[j]);
		  // if (chid>=tkFirstChamber_+2)
		  //   if (tk.getList()[j]->getChamberId()==(chid-2)) tk4.addPoint(*tk.getList()[j]);
		  // if (chid>=tkFirstChamber_+1)
		  // if (tk.getList()[j]->getChamberId()==(chid-1)) tk4.addPoint(*tk.getList()[j]);

		  // if (chid<=tkLastChamber_-1)
		  //   if (tk.getList()[j]->getChamberId()==(chid+1)) tk4.addPoint(*tk.getList()[j]);
		  // if (chid<=tkLastChamber_-2)
		  //   if (tk.getList()[j]->getChamberId()==(chid+2)) tk4.addPoint(*tk.getList()[j]);
		  // if (chid==tkFirstChamber_)
		  //   if (tk.getList()[j]->getChamberId()==(chid+3)) tk4.addPoint(*tk.getList()[j]);
					

		}
	      tk4.regression();
	      if (tk4.getList().size()>=3 && TMath::Prob(tk4.chi2_,2*tk4.getList().size()-4)>tkExtChi2Cut_)
		{
		  xext=tk4.getXext(ip->second.getZ());
		  yext=tk4.getYext(ip->second.getZ());
		}
	      else 
		continue;

	    }
	  if (xext==-999999 || yext==-999999)
	    {
	      tk0.regression();
	      if (tk0.getList().size()<(uint32_t) tkExtMinPoint_ || TMath::Prob(tk0.chi2_,2*tk0.getList().size()-4)<tkExtChi2Cut_) continue;
	      xext=tk0.getXext(ip->second.getZ());
	      yext=tk0.getYext(ip->second.getZ());
	    }
	  double xchext = ip->second.toLocalX(xext);
	  double ychext = ip->second.toLocalY(yext);
	  double zch=ip->second.getZ();
	  //std::cout <<xext<<" " <<yext<<" "<<xchext<<" " <<ychext<< " "<<zch<<" "<<ip->second.getZ()<<std::endl;
	  ip->second.calculateLocal(xext,yext,0,xchext,ychext,zch);
	  //std::cout <<xext<<" " <<yext<<" "<<xchext<<" " <<ychext<< " "<<zch<<" "<<ip->second.getZ()<<std::endl;
	  //	  getchar();
	  if (xchext<chamberEdge_ || xchext >96-chamberEdge_) continue;
	  if (ychext<chamberEdge_ || ychext >96-chamberEdge_) continue;
	  if (s_shift+ip->first<255)
	    intrack.set(s_shift+ip->first,true);
	  std::stringstream namec("");
	  namec<<tkdir+"/Plan"<<ip->first;

	  TH2* hextpos = rootHandler_->GetTH2( namec.str()+"/LocalExtrapolationMap");
	  TH2* hfoundpos = rootHandler_->GetTH2( namec.str()+"/LocalFoundMap");
	  TH2* hnearpos = rootHandler_->GetTH2( namec.str()+"/LocalNearestMap");
	  TH2* hmispos = rootHandler_->GetTH2( namec.str()+"/LocalMissedMap");
	  TH1* hmisti = rootHandler_->GetTH1(namec.str()+"/MissedTime");	  		
	  if (hextpos == NULL)
	    {
	      hextpos =rootHandler_->BookTH2( namec.str()+"/LocalExtrapolationMap",96,0.,96.,96,0.,96.);
	      hfoundpos =rootHandler_->BookTH2( namec.str()+"/LocalFoundMap",96,0.,96.,96,0.,96.);
	      hnearpos =rootHandler_->BookTH2( namec.str()+"/LocalNearestMap",96,0.,96.,96,0.,96.);
	      hmispos =rootHandler_->BookTH2( namec.str()+"/LocalMissedMap",96,0.,96.,96,0.,96.);
	      hmisti =rootHandler_->BookTH1( namec.str()+"/MissedTime",500,0.,500.);

	    }
	  hextpos->Fill(xchext,ychext);
	  //std::cout<<xext<<" "<<yext<<std::endl;
	  unsigned int imin=999999;double distmin=9999999;
	  for (unsigned int irp=0;irp<allpoints_.size();irp++)
	    {
	      if (allpoints_[irp].getChamberId()!=chid) continue;
	      double dist =sqrt((allpoints_[irp].X()-xext)*(allpoints_[irp].X()-xext)+(allpoints_[irp].Y()-yext)*(allpoints_[irp].Y()-yext));
	      //std::cout<<chid<<" "<<dist<<std::endl;
	      if (dist<distmin)
		{
		  distmin=dist;
		  imin=irp;
		}
	    }
	  if (imin == 999999)
	    {
	      hmispos->Fill(xchext,ychext);
	      hmisti->Fill(currentTime_*1.);
	      //		std::cout<<" chamber "<<chid<<" not found ("<<xchext<<","<<ychext<<") "<<tk0.ax_<<":"<<tk0.ay_<<":"<<tk0.chi2_<<std::endl;
	    }
	  // else
	  //   std::cout<<chid<<" is found "<<std::endl;
	  if (distmin<tkExtDistCut_*1000)
	    {
	      std::stringstream namec("");
	      namec<<tkdir+"/Plan"<<ip->first;
	      TH1* hresol = rootHandler_->GetTH1(namec.str()+"/Resolution");	  
	      TH1* hresx = rootHandler_->GetTH1(namec.str()+"/XPull");	  
	      TH1* hresy = rootHandler_->GetTH1(namec.str()+"/YPull");	  
	      if (hresol==0)
		{
		  hresol =rootHandler_->BookTH1(namec.str()+"/Resolution",200,0.,50.);
		  hresx =rootHandler_->BookTH1( namec.str()+"/XPull",2000,-150.,150.);
		  hresy =rootHandler_->BookTH1( namec.str()+"/YPull",2000,-150.,150.);
		}
	      hresol->Fill(distmin);
	      hresx->Fill(allpoints_[imin].X()-xext);
	      hresy->Fill(allpoints_[imin].Y()-yext);
				

	      if (distmin<tkExtDistCut_)
		{
		  if (s_shift+ip->first+60<255)
		    intrack.set(s_shift+ip->first+60,true);
		  double xchnear = ip->second.toLocalX(allpoints_[imin].X());
		  double ychnear = ip->second.toLocalY(allpoints_[imin].Y());
		  double zch;
		  ip->second.calculateLocal(allpoints_[imin].X(),allpoints_[imin].Y(),0,xchnear,ychnear,zch);

		  hnearpos->Fill(xchnear,ychnear);
		  hfoundpos->Fill(xchext,ychext);
					
		}
	      else
		if (1<0)
		  DEBUG_PRINT("Dist = %f X =%f Y =%f \n",distmin,allpoints_[imin].X()-xext,allpoints_[imin].Y()-yext);
	    }
	}
      TH1* hintrack= rootHandler_->GetTH1("PlanInTrack");
      if (hintrack==NULL)
	{
	  hintrack =rootHandler_->BookTH1( "PlanInTrack",200,-0.1,199.9);
	}
      for (unsigned int ib=0;ib<200;ib++)
	if (intrack[ib]!=0) hintrack->Fill(ib*1.);
		
    }

}

static TCanvas* TCPlot=NULL;
static TCanvas* TCShower=NULL;
static TCanvas* TCEdge=NULL;
static TCanvas* TCHT=NULL;
static TCanvas* TCCluster=NULL;

void ShowerAnalyzer::drawDisplay()
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
	  TCPlot=new TCanvas("TCPlot","test1",1300,600);
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




void ShowerAnalyzer::findTracks1()
{

  tkgood_.clear();
  tklist_.clear();
  std::vector<RecoCandTk> tkseed_;
  tkseed_.clear();
  for (unsigned int i=0;i<allpoints_.size();i++)
    {
      allpoints_[i].setUsed(false);
      //      if (allpoints_[i].Z()!=z1) continue;
      for (unsigned int j=i+1;j<allpoints_.size();j++)
	{
	  //  if (j==i) continue;
	  if (allpoints_[j].getChamberId()==allpoints_[i].getChamberId()) continue;
	  if (allpoints_[j].getChamberId()>allpoints_[i].getChamberId()+10) continue;
	  RecoCandTk tk;
	  tk.addPoint(allpoints_[i]);
	  tk.addPoint(allpoints_[j]);
	  tk.regression();
	  for (unsigned int k=j+1;k<allpoints_.size();k++)
	    {
	      if (allpoints_[k].getChamberId()==allpoints_[i].getChamberId()) continue;
	      if (allpoints_[k].getChamberId()==allpoints_[j].getChamberId()) continue;
	      if (allpoints_[k].getChamberId()>allpoints_[j].getChamberId()+10) continue;

	      if (tk.calculateDistance(allpoints_[k])>tkDistCut_) continue;
	      tk.addPoint(allpoints_[k]);
	      tk.regression();
	      tkseed_.push_back(tk);
	      break;
	    }
	  //tk.Print();
	  //getchar();
	}
    }


  //std::cout<<tkseed_.size()<<" segments"<<std::endl;
  for (unsigned int itk=0;itk<tkseed_.size();itk++)
    {
      RecoCandTk& tk = tkseed_[itk];
      appendHits(tk);
      if (tk.getList().size()<tkMinPoint_) continue;
      // Make a ROOT Fit
      float x[60],ex[60],y[60],ey[60];
      for (uint32_t jj=0;jj<tk.getList().size();jj++)
	{
	  x[jj]=(tk.getList())[jj]->Z();
	  ex[jj]=0.1;
	  y[jj] = (tk.getList())[jj]->X();
	  ey[jj] = (tk.getList())[jj]->dX();
			
	}
      TF1 fa("fa","[0]*x+[1]",0.,120.);
      TGraphErrors g(tk.getList().size(),x,y,ex,ey);

      TFitResultPtr pf= g.Fit(&fa,"Q");
      double theProb=1.;
      // std::cout<<fa.GetProb()<<std::endl;
      if (fa.GetProb()<theProb) theProb=fa.GetProb();
      //pf.Get()->Print();
      for (uint32_t jj=0;jj<tk.getList().size();jj++)
	{
			
	  y[jj] = (tk.getList())[jj]->Y();
	  ey[jj] = (tk.getList())[jj]->dY();
	}
      TGraphErrors g1(tk.getList().size(),x,y,ex,ey);
      TFitResultPtr pf1= g1.Fit(&fa,"Q");
      if (fa.GetProb()<theProb) theProb=fa.GetProb();
		
      tk.prChi2_=theProb;
      //std::cout<<itk<<" "<<tk.getList().size()<<" "<<tk.prChi2_<<std::endl;
      if (tk.prChi2_<tkChi2Cut_) continue;
      tklist_.push_back(tk);
    }
  // if (tkseed_.size()>0)
  //   std::cout<<"Number of candidate "<<tkseed_.size()<<" "<<tklist_.size()<<std::endl
  ;
#ifdef OLDWAYOFREMOVING
  for (unsigned int itk=0;itk<tklist_.size();itk++)
    {
      RecoCandTk& tk = tklist_[itk];

		
      bool found =false;
      //	  DEBUG_PRINT("\t avant tkgood %d nb hit =%d  %f %f %f\n",itk,tk.getList().size(),tk.ax_,tk.ay_,tk.chi2_);
      for (std::vector<RecoCandTk>::iterator jt=tkgood_.begin();jt!=tkgood_.end();jt++)
	{
	  if (fabs(tk.ax_-jt->ax_)+fabs( tk.bx_-jt->bx_)+fabs(tk.ay_-jt->ay_)+fabs(tk.by_-jt->by_)<1E-4) {found=true;break;}
	}
		
      if (!found)
	{
	  //	  std::cout<<trackIndex_<<"  ====>chi2 : "<<tk.prChi2_<<std::endl;
	  //std::cout<<itk<<" "<<tk.prChi2_<<" "<<tk.getList().size()<<" "<<tk.bx_<<" "<<tk.by_;
	  //for (uint32_t jj=0;jj<tk.getList().size();jj++)
	  // std::cout<<" "<<(tk.getList())[jj]->getPointId();
			

	  tkgood_.push_back(tk);
	  //std::cout<<"added"<<tk.isValid()<<std::endl;

	}
    }
  if (tkgood_.size())
    std::cout<<tkgood_.size()<<" good segments"<<std::endl;
  // 
#endif
  for (unsigned int itk=0;itk<tklist_.size();itk++)
    {
      RecoCandTk& tk = tklist_[itk];
      if (!tk.isValid()) continue;
      for (unsigned int jtk=itk+1;jtk<tklist_.size();jtk++)
	{
	  RecoCandTk& tkj = tklist_[jtk];
	  if (!tkj.isValid()) continue;
	  if (tkj.getList().size()>tk.getList().size()) continue;
	  uint32_t common=0;
	  for (uint32_t ip=0;ip<tk.getList().size();ip++)
	    {
	      for (uint32_t jp=0;jp<tkj.getList().size();jp++)
		if ((tk.getList())[ip]->getPointId() == (tkj.getList())[jp]->getPointId() ){common++;break;}
	    }
	  if (common==tkj.getList().size()) 
	    {
	      tkj.setValid(false);
	      //std::cout<<" Invalidating "<<jtk<<std::endl;
	    }
	  else
	    // If more than 2 hits common, keep the best chi2
	    if (common>2 )
	      {
		if (tk.prChi2_> tkj.prChi2_)
		  tkj.setValid(false);
		else
		  {
		    tk.setValid(false);
		    break;
		  }
		//std::cout<<" Invalidating "<<jtk<<std::endl;
	      }

	}

    }
  for (unsigned int itk=0;itk<tklist_.size();itk++)
    {
      RecoCandTk& tk = tklist_[itk];
      if (!tk.isValid()) continue;
      tkgood_.push_back(tk);
    }

  if (tkgood_.size())
    std::cout<<tkgood_.size()<<" good segments"<<std::endl;
  // Tag points belonging to several tracks
  for (unsigned int itk=0;itk<tkgood_.size();itk++)
    {

      RecoCandTk& tk = tkgood_[itk];      
      std::vector<double>& dist1=tk.getChi2();
      for (unsigned int ih=0;ih<tk.getList().size();ih++)
	{
	  //	   std:: cout<<ih<<"  ----->"<<dist1[ih]<<std::endl;
			
	  RecoPoint& pt= *(tk.getList())[ih];
	  for (unsigned jtk=itk+1;jtk<tkgood_.size();jtk++)
	    {
	      if (jtk==itk) continue;
	      RecoCandTk& tk2 = tkgood_[jtk];
	      std::vector<double>& dist2=tk2.getChi2();
	      for (unsigned int jh=0;jh<tk2.getList().size();jh++)
		{

		  RecoPoint& pt2= *(tk2.getList())[jh];
		  //		   std:: cout<<jh<<"  ----->"<<dist2[jh]<<std::endl;
		  // std::cout<<itk<<" "<<ih<<" "<<jtk<<" "<<jh<<std::endl;
		  if (pt.getPointId() == pt2.getPointId()  )
		    {

		      //DEBUG_PRINT("common hit  %f %f\n",dist1[ih],dist2[ih]);
		      if (dist1[ih]<dist2[jh])
			dist2[jh]=99999.;
		      else
			dist1[ih]=99999.;
		      //   		       pt.Print()

		      // getchar();
		    }
		}
	    }
	}
    }

  // Refit the track without bad points  
  for (unsigned int itk=0;itk<tkgood_.size();itk++)
    {
      RecoCandTk& tk = tkgood_[itk];
      //    DEBUG_PRINT("\t before %d nb hit =%d  %f %f %f\n",itk,tk.getList().size(),tk.ax_,tk.ay_,tk.chi2_);
      tk.clean();
      // DEBUG_PRINT("\t %d nb hit =%d \n",itk,tk.getList().size());
    }



  // remove candidate with less than 3 points
  for (std::vector<RecoCandTk>::iterator i=tkgood_.begin();i!=tkgood_.end();)
    {
      if (i->getList().size()<(uint32_t) tkMinPoint_ || !i->isValid())
	{
	  tkgood_.erase(i++);
	}
      else
	i++;
    }

  for (unsigned int itk=0;itk<tkgood_.size();itk++)
    {
      RecoCandTk& tk = tkgood_[itk];
      for (unsigned int ih=0;ih<tk.getList().size();ih++)
	{

	  RecoPoint& pt= *(tk.getList())[ih];
	  pt.setUsed(true);
	  for (unsigned int i=0;i<allpoints_.size();i++)
	    {
	      if (pt.getPointId()==allpoints_[i].getPointId())
		{
		  allpoints_[i].setUsed(true);
		  break;
		}
	    }

	}
    }
  if (tkgood_.size())
    {
      DEBUG_PRINT("%d %d had: good tracks found \n",tklist_.size(),tkgood_.size());
      //getchar();
    }
}


void ShowerAnalyzer::CosmicFinder()
{

  tkgood_.clear();


  for (uint32_t i=tkFirstChamber_;i<tkLastChamber_;i++)
    {
      //std::cout<<"Chambre "<<i<<std::endl;
      std::map<uint32_t,std::vector<RecoPoint*> >::iterator it_ch = chamberPoints_.find(i);
      if (it_ch==chamberPoints_.end()) continue;
      //std::cout<<"Points trouve"<<std::endl;
      for (std::vector<RecoPoint*>::iterator ip=it_ch->second.begin();ip!=it_ch->second.end();ip++)
	{
	  //std::cout<<"Point"<<ip->X()<<std::endl;
	  if ((*ip)->isUsed()) continue;
	  RecoCandTk tk;
	  RecoPoint& pI=(**ip);
	  tk.addPoint(pI);
	  //	  std::cout<<"Point"<<ip->X()<<std::endl;
	  std::map<uint32_t,std::vector<RecoPoint*> >::iterator jt_ch = chamberPoints_.find((i+1));
	  if (jt_ch==chamberPoints_.end()) continue;
	  //std::cout<<"Second Points trouve"<<std::endl;
	  for (std::vector<RecoPoint*>::iterator jp=jt_ch->second.begin();jp!=jt_ch->second.end();jp++)
	    {
	      if ((*jp)->isUsed()) continue;
	      RecoPoint& pJ=(**jp);
	      tk.addPoint(pJ);

	      //tk.addPoint(*jp);
	      tk.regression();
	      //	std::cout<<i<<" "<<i+1<<" CHi2 0: "<<tk.chi2_<<std::endl;
	      //Now extrapolate to other plan
	      for (uint32_t k=i+2;k<=tkLastChamber_;k++)
		{
		  std::map<uint32_t,std::vector<RecoPoint*> >::iterator kt_ch = chamberPoints_.find(k);
		  if (kt_ch==chamberPoints_.end()) continue;
		  for (std::vector<RecoPoint*>::iterator kp=kt_ch->second.begin();kp!=kt_ch->second.end();kp++)
		    {
		      if ((*kp)->isUsed()) continue;
		      // std::cout<<k<<" Distance: "<<tk.calculateDistance(*kp)<<std::endl;
		      RecoPoint& pK=(**kp);

		      if (tk.calculateDistance(pK)>tkDistCut_) continue;
		      tk.addPoint(pK);
		      tk.regression();
		      break;
		    }
		}
	      // Now Check the track
	      if (tk.getList().size()>tkLastChamber_) 
		{
		  std::cout<<tk.getList().size()<<" "<<tk.chi2_<<std::endl;
		  continue;
		}
	      //getchar();
	      if (tk.getList().size()<tkMinPoint_) continue;
	      // Make a ROOT Fit
	      float x[60],ex[60],y[60],ey[60];
	      for (uint32_t jj=0;jj<tk.getList().size();jj++)
		{
		  x[jj]=(tk.getList())[jj]->Z();
		  ex[jj]=0.1;
		  y[jj] = (tk.getList())[jj]->X();
		  ey[jj] = (tk.getList())[jj]->dX();
		  ey[jj] = 0.25;
					
		}
	      TF1 fa("fa","[0]*x+[1]",0.,120.);
	      TGraphErrors g(tk.getList().size(),x,y,ex,ey);

	      TFitResultPtr pf= g.Fit(&fa,"Q");
	      double theProb=1.;
	      // std::cout<<fa.GetProb()<<std::endl;
	      if (fa.GetProb()<theProb) theProb=fa.GetProb();
	      //pf.Get()->Print();
	      for (uint32_t jj=0;jj<tk.getList().size();jj++)
		{
					
		  y[jj] = (tk.getList())[jj]->Y();
		  ey[jj] = (tk.getList())[jj]->dY();
		  ey[jj] = 0.25;
		}
	      TGraphErrors g1(tk.getList().size(),x,y,ex,ey);
	      TFitResultPtr pf1= g1.Fit(&fa,"Q");
	      if (fa.GetProb()<theProb) theProb=fa.GetProb();
				
	      tk.prChi2_=theProb;
	      //std::cout<<" "<<tk.getList().size()<<" "<<tk.prChi2_<<std::endl;
	      if (tk.prChi2_<tkChi2Cut_) continue;
	      tkgood_.push_back(tk);
	      for (uint32_t jj=0;jj<tk.getList().size();jj++)
		{
		  //std::cout<<(tk.getList())[jj]->X()<<":"<<(tk.getList())[jj]->Y()<<":"<<(tk.getList())[jj]->Z()<<" "<<(tk.getList())[jj]->isUsed()<<std::hex<<&(tk.getList())[jj]<<std::dec<<std::endl;
		  (tk.getList())[jj]->setUsed(true);
		  //std::cout<<(tk.getList())[jj].X()<<":"<<(tk.getList())[jj].Y()<<":"<<(tk.getList())[jj].Z()<<" "<<(tk.getList())[jj].isUsed()<<std::endl;

		}
	      //getchar();
	      break;
	    }
	}
    }

}

#define NBIN_RAD 24
#define NBIN_R 30
#define DRAW_HOUGH

void ShowerAnalyzer::HT()
{
  theHTx_->Clear();
  theHTy_->Clear();
  for (std::vector<RecoPoint>::iterator icl=allpoints_.begin();icl!=allpoints_.end();icl++)
    {
      theHTx_->addPixel(icl->Z()-50.,icl->X());
      theHTy_->addPixel(icl->Z()-50.,icl->Y());
    }
  uint32_t _mvx;float _thx,_rx;
  theHTx_->findMaximum(_mvx,_thx,_rx);

  uint32_t _mvy;float _thy,_ry;
  theHTy_->findMaximum(_mvy,_thy,_ry);


  TH1* hthx = rootHandler_->GetTH1("Thetax");
  TH1* hrx = rootHandler_->GetTH1("Rx");
  TH1* hthy = rootHandler_->GetTH1("Thetay");
  TH1* hry = rootHandler_->GetTH1("Ry");
  if (hthx==NULL)
    {
      hthx= rootHandler_->BookTH1("Thetax",60,0.,1*M_PI);
      hrx= rootHandler_->BookTH1("Rx",125,-150.,150.);
      hthy= rootHandler_->BookTH1("Thetay",60,0.,1*M_PI);
      hry= rootHandler_->BookTH1("Ry",125,-150.,150.);
    }

  hthx->Fill(_thx);
  hrx->Fill(_rx);
  hthy->Fill(_thy);
  hry->Fill(_ry);
  //std::cout<<i_m<<" "<<j_m<<" "<<m<<" "<<cc<<" "<<maxval<<std::endl;
  //std::cout<<"Avec ROOT "<<i_m2<<" "<<j_m2<<" "<<rx<<" "<<thetax<<" "<<m2<<" "<<cc2<<" "<<maxval2<<std::endl;

  //std::cout<<"Avec HTImage "<<_rx<<" "<<_thx<<" "<<_mvx<<std::endl;
  RecoCandTk tkht;
  tkht.ax_= -1./tan(_thx);
  tkht.bx_= _rx/sin(_thx)-50.*tkht.ax_;
  //std::cout<<i_my<<" "<<j_my<<" "<<my<<" "<<ccy<<" "<<maxvaly<<std::endl;
  //std::cout<<"Avec ROOT "<<i_my2<<" "<<j_my2<<" "<<ry<<" "<<thetay<<" "<<my2<<" "<<ccy2<<" "<<maxvaly2<<std::endl;

  // std::cout<<"Avec HTImage "<<_ry<<" "<<_thy<<" "<<_mvy<<std::endl;
  tkht.ay_= -1./tan(_thy);
  tkht.by_= _ry/sin(_thy)-50*tkht.ay_;


  RecoCandTk tk;
  tkgood_.clear();
  if (_mvx>=5 || _mvy>=5) 
    {
      //     std::cout<<i_m2<<" "<<j_m2<<" "<<rx<<" "<<thetax<<" "<<m2<<" "<<cc2<<" "<<maxval2<<std::endl;
      //std::cout<<i_my2<<" "<<j_my2<<" "<<ry<<" "<<thetay<<" "<<my2<<" "<<ccy2<<" "<<maxvaly2<<std::endl;


      for (uint32_t i=tkFirstChamber_;i<=tkLastChamber_;i++)
	{
	  //std::cout<<"Chambre "<<i<<std::endl;
	  std::map<uint32_t,std::vector<RecoPoint*> >::iterator it_ch = chamberPoints_.find(i);
	  if (it_ch==chamberPoints_.end()) continue;
	  //std::cout<<"Points trouve"<<std::endl;
	  float distmin=999999.;
	  RecoPoint* ipmin=0;
	  for (std::vector<RecoPoint*>::iterator ip=it_ch->second.begin();ip!=it_ch->second.end();ip++)
	    {
	      //std::cout<<"Point"<<ip->X()<<std::endl;
	      if ((*ip)->isUsed()) continue;
	      RecoPoint& pI=(**ip);
	      float dist=tkht.calculateDistance(pI);

	      // float xext=m2*(pI.Z()-50.)+cc2;
	      // float yext=my2*(pI.Z()-50.)+ccy2;
	      // float dist=sqrt((pI.X()-xext)*(pI.X()-xext)+(pI.Y()-yext)*(pI.Y()-yext));
	      // dist =TMath::Abs((pI.X()-xext));
	      // if (TMath::Abs((pI.Y()-yext))>dist) dist=TMath::Abs((pI.Y()-yext));

	      if (dist<distmin)
		{
		  distmin=dist;
		  ipmin=(*ip);
		}
	    }
	  //std::cout<<i<<" "<<distmin<<std::endl;
	  if (distmin<tkDistCut_)
	    {
	      tk.addPoint((*ipmin));
	      ipmin->setUsed(true);
	    }
			
	}

      tk.regression();
      //tkht.calculateChi2();
      //DEBUG_PRINT("\t  Candidat nb hit =%d  %f %f %f %f\n",tk.getList().size(),tk.ax_,tk.ay_,tk.chi2_,tk.prChi2_);
      //DEBUG_PRINT("\t  Candidat Hought nb hit =%d  %f %f %f %f\n",tkht.getList().size(),tkht.ax_,tkht.ay_,tkht.chi2_,tkht.prChi2_);
      //getchar();
      // for (uint32_t jj=0;jj<tk.getList().size();jj++)
      //   {
      // 	 float z=(tk.getList())[jj]->Z();
      // 	 float xext=tk.ax_*z+tk.bx_;
      // 	 float yext=tk.ay_*z+tk.by_;
      // 	 DEBUG_PRINT("\t \t %f : %f %f %f %f \n",z,xext-(tk.getList())[jj]->X(),(tk.getList())[jj]->dX(),yext-(tk.getList())[jj]->Y(),(tk.getList())[jj]->dY()); 
      //   }
      //getchar();
      if (tk.getList().size()>=(uint32_t)tkMinPoint_) 
	{


	  tk.prChi2_=TMath::Prob(tk.chi2_,2*tk.getList().size()-4);
	  houghIndex_++;
	  if (tk.prChi2_>tkChi2Cut_) {
				
	    //DEBUG_PRINT("\t HT %d nb hit =%d  %f %f %f %f\n",houghIndex_,tk.getList().size(),tk.ax_,tk.ay_,tk.chi2_,tk.prChi2_);


	    tkgood_.push_back(tk);
	  }
	}

      //     if (allpoints_.size()>(tk.getList().size()+5))
      // HTOld();
    }

}
#define NEWSEARCH
#ifdef NEWSEARCH



//static TCanvas* TChough=NULL;



void ShowerAnalyzer::HT2D()
{
  std::vector<HC> vX;
  std::vector<HC> vY;


  HTImage htx(120,0,1*M_PI,150,-50.,100.);
  HTImage hty(120,0,1*M_PI,150,-50.,100.);
  std::vector<std::vector<RecoPoint>::iterator> usedPoint;
  std::vector<std::vector<RecoPoint>::iterator > vCom;
  //  htx.Clear();
  //   hty.Clear();
  //   for (std::vector<RecoPoint>::iterator icl=allpoints_.begin();icl!=allpoints_.end();icl++)
  //    {
  //      if (icl->Z()>160.) continue;
  // #ifdef M_PIXEL
  //      RECOCluster& c= (*icl).getCluster();
  //      float w=0;
  //      for (std::vector<RecoHit>::iterator iht=c.getHits()->begin();iht!=c.getHits()->end();iht++)
  //        {
  // 	 int ithr= (*iht).getAmplitude()&0x3;
  // 	 if (ithr==1) w=1;
  // 	 if (ithr==2) w=2;
  // 	 if (ithr==3) w=10;
	
  // 	 htx.addPixel((*iht).Z()-50.,(*iht).X(),w);
  // 	 hty.addPixel((*iht).Z()-50.,(*iht).Y(),w);


  //        }

  // #endif
  //      htx.addPixel(icl->Z()-50.,icl->X());
  //      hty.addPixel(icl->Z()-50.,icl->Y());
  //    }
  //   //htx.findMaxima(vXmax,vXtheta,vXr);
  usedPoint.clear();

  uint32_t mvx;float thx,rx;
  do
    {
      htx.Clear();
      for (std::vector<RecoPoint>::iterator icl=allpoints_.begin();icl!=allpoints_.end();icl++)
	{
	  if (std::find(usedPoint.begin(), usedPoint.end(), icl)!=usedPoint.end()) continue;
	  htx.addPixel(icl->Z()-50.,icl->X());
	}

      htx.findMaximum(mvx,thx,rx);
      //intf("%d \n",mvx);
      if (mvx<4) break;
      HC cand(mvx,thx,rx);
      std::map<float,std::vector<RecoPoint>::iterator> mapDist;
      mapDist.clear();
      for (std::vector<RecoPoint>::iterator icl=allpoints_.begin();icl!=allpoints_.end();icl++)
	{
	  if (std::find(usedPoint.begin(), usedPoint.end(), icl)!=usedPoint.end()) continue;
	  float dist=TMath::Abs(icl->X()-cand.Pos(icl->Z()));
	  //if (dist>4.) continue;
	  mapDist[dist]=icl;
	}
      uint32_t nd=0;
      float dmin=DBL_MAX,dmax=-DBL_MAX,dmean=0;
      float zmin=99999.,zmax=-10.;
      for (std::map<float,std::vector<RecoPoint>::iterator>::iterator id=mapDist.begin();id!=mapDist.end();id++)
	{
	  nd++;
	  //DEBUG_PRINT("---> %d %f \n",nd,id->first);
	  if (nd>cand.m_) break;
	  float z=(*(id->second)).Z();
	  if (z<zmin) zmin=z;
	  if (z>zmax) zmax=z;
	  if (nd==1) dmin=id->first;
	  dmean+=id->first;

	  if (nd==cand.m_-1) dmax=id->first;
	  usedPoint.push_back(id->second);
	  cand.add(id->second);
	}
      //nd.Dump();
      DEBUG_PRINT("%d %f %f \n",cand.m_,dmean,dmax);
      dmean/=nd;
      if (dmean>3. || dmax>5.) continue;
      vX.push_back(cand);
      if (TCPlot!=NULL)
	{
	  TCPlot->cd(2);
	  TLine* l = new TLine(zmin,cand.Pos(zmin),zmax,cand.Pos(zmax));
	  l->SetLineColor(3);
	  l->Draw("SAME");
	  TCPlot->Modified();
	  TCPlot->Update();
	}
    } while (mvx>=4);
  usedPoint.clear();
  do
    {
      htx.Clear();
      for (std::vector<RecoPoint>::iterator icl=allpoints_.begin();icl!=allpoints_.end();icl++)
	{
	  if (std::find(usedPoint.begin(), usedPoint.end(), icl)!=usedPoint.end()) continue;
	  htx.addPixel(icl->Z()-50.,icl->Y());
	}

      htx.findMaximum(mvx,thx,rx);
      //rintf("%d \n",mvx);
      if (mvx<4) break;
      HC cand(mvx,thx,rx);
		
      std::map<float,std::vector<RecoPoint>::iterator> mapDist;
      mapDist.clear();
      for (std::vector<RecoPoint>::iterator icl=allpoints_.begin();icl!=allpoints_.end();icl++)
	{
	  if (std::find(usedPoint.begin(), usedPoint.end(), icl)!=usedPoint.end()) continue;
	  float dist=TMath::Abs(icl->Y()-cand.Pos(icl->Z()));
	  //if (dist>4.) continue;
	  mapDist[dist]=icl;
	}
      uint32_t nd=0;
      float dmin=DBL_MAX,dmax=-DBL_MAX,dmean=0;
      float zmin=99999.,zmax=-10.;

      for (std::map<float,std::vector<RecoPoint>::iterator>::iterator id=mapDist.begin();id!=mapDist.end();id++)
	{
	  nd++;
	  if (nd>cand.m_) break;
	  float z=(*(id->second)).Z();
	  if (z<zmin) zmin=z;
	  if (z>zmax) zmax=z;
	  if (nd==1) dmin=id->first;
	  dmean+=id->first;
	  if (nd==cand.m_-1) dmax=id->first;
	  usedPoint.push_back(id->second);
	  cand.add(id->second);
	}
      //and.Dump();
      dmean/=nd;
      if (dmean>3. || dmax>10.) continue;
      vY.push_back(cand);
      if (TCPlot!=NULL)
	{
	  TCPlot->cd(3);
	  TLine* l = new TLine(zmin,cand.Pos(zmin),zmax,cand.Pos(zmax));
	  l->SetLineColor(3);
	  l->Draw("SAME");
	  TCPlot->Modified();
	  TCPlot->Update();
	}
    } while (mvx>=4);
  //intf("Sizes   %d %d \n",vX.size(),vY.size());

  for (std::vector<HC>::iterator itx=vX.begin();itx!=vX.end();itx++)
    {
      //intf("Size ne X %d \n",(*itx).points_.size());
		
      for (std::vector<HC>::iterator ity=vY.begin();ity!=vY.end();ity++)
	{
	  vCom.clear();
	  uint32_t nc=(*itx).common((*ity),vCom);
	  if (nc>=3)
	    {
	      DEBUG_PRINT("New Candidate %d \n",nc);
	      (*itx).Dump();
	      (*ity).Dump();
	      RecoCandTk tk;
	      for (std::vector<std::vector<RecoPoint>::iterator >::iterator ip=vCom.begin();ip!=vCom.end();ip++)
		tk.addPoint((*(*ip)));
	      tk.regression();
	      if (TCPlot!=NULL)
		{

		  TPolyLine3D *pl3d1 = new TPolyLine3D(2);
		  TCPlot->cd(1);
		  pl3d1->SetPoint(0, tk.zmin_,tk.ax_*tk.zmin_+tk.bx_,tk.ay_*tk.zmin_+tk.by_);
		  pl3d1->SetPoint(1, tk.zmax_,tk.ax_*tk.zmax_+tk.bx_,tk.ay_*tk.zmax_+tk.by_);

		  pl3d1->SetLineWidth(4);
		  pl3d1->SetLineColor(1);
		  pl3d1->Draw("SAME");
		  TCPlot->cd(2);
		  TLine* l = new TLine(tk.zmin_,tk.ax_*tk.zmin_+tk.bx_,tk.zmax_,tk.ax_*tk.zmax_+tk.bx_);
		  l->SetLineColor(1);
		  l->SetLineWidth(4);
		  l->Draw("SAME");
		  TCPlot->cd(3);
		  TLine* l1 = new TLine(tk.zmin_,tk.ay_*tk.zmin_+tk.by_,tk.zmax_,tk.ay_*tk.zmax_+tk.by_);
		  l1->SetLineColor(1);
		  l1->SetLineWidth(4);
		  l1->Draw("SAME");

		  TCPlot->Modified();
		  TCPlot->Update();
		}
	    }
	}
    }
#ifdef OLDWAY_HT2D
  //uint32_t mvx;float thx,rx;
  vXmax.clear();
  vXtheta.clear();
  vXr.clear();
  usedPoint.clear();
  do
    {
      htx.Clear();
      for (std::vector<RecoPoint>::iterator icl=allpoints_.begin();icl!=allpoints_.end();icl++)
	{
	  if (std::find(usedPoint.begin(), usedPoint.end(), icl)!=usedPoint.end()) continue;
	  htx.addPixel(icl->Z()-50.,icl->Y());
	}

      htx.findMaximum(mvx,thx,rx);
      DEBUG_PRINT("%d \n",mvx);
      if (mvx<5) break;
      vXmax.push_back(mvx);
      vXtheta.push_back(thx);
      vXr.push_back(rx);
      float ax_= -1./tan(thx);
      float bx_= rx/sin(thx)-50.*ax_;

      std::map<float,std::vector<RecoPoint>::iterator> mapDist;
      mapDist.clear();
      for (std::vector<RecoPoint>::iterator icl=allpoints_.begin();icl!=allpoints_.end();icl++)
	{
	  if (std::find(usedPoint.begin(), usedPoint.end(), icl)!=usedPoint.end()) continue;
	  float dist=TMath::Abs(icl->Y()-(ax_*(icl->Z())+bx_));
	  //if (dist>4.) continue;
	  mapDist[dist]=icl;
	}
      uint32_t nd=0;
      float dmin,dmax,dmean=0;
      float zmin=99999.,zmax=-10.;
      for (std::map<float,std::vector<RecoPoint>::iterator>::iterator id=mapDist.begin();id!=mapDist.end();id++)
	{
	  nd++;
	  if (nd>mvx) break;
	  float z=(*(id->second)).Z();
	  if (z<zmin) zmin=z;
	  if (z>zmax) zmax=z;
	  if (nd==1) dmin=id->first;
	  dmean+=id->first;
	  if (nd==mvx-1) dmax=id->first;
	  usedPoint.push_back(id->second);
	}
      dmean/=nd;
      DEBUG_PRINT(" %f %d %f %f => %f %f %f %f\n",dmean,mvx,ax_,bx_,dmin,dmax,zmin,zmax);
      //      getchar();
    } while (mvx>=5);

  for (uint32_t iht=0;iht<vXmax.size();iht++)
    {
      float ax_= -1./tan(vXtheta[iht]);
      float bx_= vXr[iht]/sin(vXtheta[iht])-50.*ax_;
      std::map<float,std::vector<RecoPoint>::iterator> mapDist;
      mapDist.clear();
      for (std::vector<RecoPoint>::iterator icl=allpoints_.begin();icl!=allpoints_.end();icl++)
	{
	  if (icl->Z()>160.) continue;
	  float dist=TMath::Abs(icl->Y()-(ax_*(icl->Z())+bx_));
	  //if (dist>4.) continue;
	  mapDist[dist]=icl;
	}


      uint32_t nd=0;
      float dmin,dmax,dmean=0;
      float zmin=99999.,zmax=-10.;
      for (std::map<float,std::vector<RecoPoint>::iterator>::iterator id=mapDist.begin();id!=mapDist.end();id++)
	{
	  nd++;
	  if (nd>vXmax[iht]) break;
	  float z=(*(id->second)).Z();
	  if (z<zmin) zmin=z;
	  if (z>zmax) zmax=z;
	  if (nd==1) dmin=id->first;
	  dmean+=id->first;
	  if (nd==vXmax[iht]-1) dmax=id->first;

	}
      dmean/=nd;
      DEBUG_PRINT("%d %f %d %f %f => %f %f %f %f\n",iht,dmean,vXmax[iht],ax_,bx_,dmin,dmax,zmin,zmax);
      //getchar();


      if (dmean<3. && dmax<10.)
	{

	  // TProfile* hhtx = (TProfile*) rootHandler_->GetTH1("HoughTransformX");
	  // if (hhtx==NULL)
	  //   {
	  //     hhtx = rootHandler_->BookProfile("HoughTransformX",100,0.,100,-50.,50.);
	  //   }
	  // else
	  //   hhtx->Reset();
	  // for (std::vector<RecoPoint>::iterator icl=allpoints_.begin();icl!=allpoints_.end();icl++)
	  //   {

	  //     RECOCluster& c= (*icl).getCluster();
	  //     for (std::vector<RecoHit>::iterator iht=c.getHits()->begin();iht!=c.getHits()->end();iht++)
	  // 	      {
	  // 		int ithr= (*iht).getAmplitude()&0x3;
	  // 		float dist=TMath::Abs(iht->X()-(ax_*(iht->Z())+bx_));
	  // 		hhtx->Fill(iht->Z(),dist);
	  // 	      }
			
	  //   }

	  if (TCPlot!=NULL)
	    {
	      TCPlot->cd(3);
	      TLine* l = new TLine(zmin,ax_*zmin+bx_,zmax,ax_*zmax+bx_);
	      l->SetLineColor(3);
	      l->Draw("SAME");
	      TCPlot->Modified();
	      TCPlot->Update();
	    }
	  // if (TChough==NULL)
	  //    TChough=new TCanvas("hough","hough",600,600);
	  // TChough->cd();
	  // TChough->Draw();
	  // hhtx->Draw();
	  // TChough->Modified();
	  // TChough->Update();


			
	}

    }

#endif
  char c;c=getchar();putchar(c); if (c=='.') exit(0);;
}

#endif
void ShowerAnalyzer::HTOld()
{
#ifdef HOUGH_CARTESIAN
  TH2F* hhtx = (TH2F*) rootHandler_->GetTH2("HoughTransformX");
  TH2F* hhty = (TH2F*) rootHandler_->GetTH2("HoughTransformY");
#endif
  TH2F* hhtx2 = (TH2F*) rootHandler_->GetTH2("HoughTransformX2");
  TH2F* hhty2 = (TH2F*) rootHandler_->GetTH2("HoughTransformY2");
#ifdef DRAW_HOUGH
  TH2F* hzx = (TH2F*) rootHandler_->GetTH2("ZX");
  TH2F* hzy = (TH2F*) rootHandler_->GetTH2("ZY");
#endif
  if (hhtx2==NULL)
    {

      hhtx2 =(TH2F*)rootHandler_->BookTH2("HoughTransformX2",NBIN_RAD,0.,1*M_PI,NBIN_R,-100.,100.);
      hhty2 =(TH2F*)rootHandler_->BookTH2("HoughTransformY2",NBIN_RAD,0.,1*M_PI,NBIN_R,-100.,100.);
      //hhtx2 =(TH2F*)rootHandler_->BookTH2("HoughTransformX2",200,0.,1*M_PI,200,-100.,100.);
      //hhty2 =(TH2F*)rootHandler_->BookTH2("HoughTransformY2",200,0.,1*M_PI,200,-100.,100.);
#ifdef HOUGH_CARTESIAN
      hhtx =(TH2F*)rootHandler_->BookTH2("HoughTransformX",160,-8.,8.,160,-250,250.);
      hhty =(TH2F*)rootHandler_->BookTH2("HoughTransformY",160,-8.,8.,160,-250,250.);
#endif
#ifdef DRAW_HOUGH
      hzy =(TH2F*)rootHandler_->BookTH2("ZY",120,-50.,70.,120,-10.,110.);
      hzx =(TH2F*)rootHandler_->BookTH2("ZX",120,-50.,70.,120,-10.,110.);
#endif
    }
  else
    {

      hhtx2->Reset();
      hhty2->Reset();
#ifdef DRAW_HOUGH
      hzy->Reset();
      hzx->Reset();
#endif
#ifdef HOUGH_CARTESIAN
      hhtx->Reset();
      hhty->Reset();
#endif
    }

  theHTx_->Clear();
  theHTy_->Clear();
  for (std::vector<RecoPoint>::iterator icl=allpoints_.begin();icl!=allpoints_.end();icl++)
    {
      float w=0;
      RECOCluster& c= (*icl).getCluster();
      float z=icl->Z(),x=0,y=0,wt=0;
      for (std::vector<RecoHit>::iterator iht=c.getHits()->begin();iht!=c.getHits()->end();iht++)
	{
	  int ithr= (*iht).getAmplitude()&0x3;
	  if (ithr==1) w=3;
	  if (ithr==2) w=1;
	  if (ithr==3) w=15;
			
	  x+= (*iht).X()*w;
	  y+= (*iht).Y()*w;
	  wt+=w;
	}

      x/=wt;
      y/=wt;
      w=1.;
      theHTx_->addPixel(z-50.,x,w);
      theHTy_->addPixel(z-50.,y,w);
#ifdef DRAW_HOUGH
      hzx->Fill(z-50,x);
      hzy->Fill(z-50.,y);
#endif
#ifdef HOUGH_CARTESIAN

      for (uint32_t i=0;i<160;i++)
	{
	  float m= -8+16./160.*i;
	  float cx = icl->X()-m*(icl->Z()-50.);
	  float cy = icl->Y()-m*(icl->Z()-50.);
	  hhtx->Fill(m,cx);
	  hhty->Fill(m,cy);
	}
#endif
      for (uint32_t i=0;i<NBIN_RAD;i++)
	{
	  float theta=i*1*M_PI/NBIN_RAD;
	  float rx= cos(theta)*(z-50.)+ sin(theta)*x;
	  hhtx2->Fill(theta,rx,w);
	  float ry= cos(theta)*(z-50.)+ sin(theta)*y;
	  hhty2->Fill(theta,ry,w);

	}
		
    }
#ifdef HOUGH_CARTESIAN
  float maxval=-9999.;
  uint32_t i_m,j_m;
  for (uint32_t i=0;i<160;i++)
    for (uint32_t j=0;j<160;j++)
      if (hhtx->GetBinContent(i+1,j+1)>maxval)
	{
	  maxval = hhtx->GetBinContent(i+1,j+1);
	  i_m=i+1;j_m=j+1;
	}

  float m=-8.+16./160.*(i_m-1)+8./160.;
  float cc= -250.+500./160.*(j_m-1)+250./160.;
#endif
  float maxval2=-9999.;
  int32_t i_m2=0,j_m2=0;
  for (uint32_t i=0;i<NBIN_RAD;i++)
    for (uint32_t j=0;j<NBIN_R;j++)
      if (hhtx2->GetBinContent(i+1,j+1)>maxval2)
	{
	  maxval2 = hhtx2->GetBinContent(i+1,j+1);
	  i_m2=i+1;j_m2=j+1;
	}

  float thetax= (i_m2-0.5)*1*M_PI/NBIN_RAD;
  float rx = -100.+(j_m2-0.5)*200/NBIN_R;

  float m2= -1./tan(thetax);
  float cc2= rx/sin(thetax);


  float maxvaly2=-9999.;
  int32_t i_my2=0,j_my2=0;
  for (uint32_t i=0;i<NBIN_RAD;i++)
    for (uint32_t j=0;j<NBIN_R;j++)
      if (hhty2->GetBinContent(i+1,j+1)>maxvaly2)
	{
	  maxvaly2 = hhty2->GetBinContent(i+1,j+1);
	  i_my2=i+1;j_my2=j+1;
	}

  float thetay= (i_my2-0.5)*1*M_PI/NBIN_RAD;
  float ry = -100.+(j_my2-0.5)*200/NBIN_R;

  float my2= -1./tan(thetay);
  float ccy2= ry/sin(thetay);


#ifdef HOUGH_CARTESIAN
  float maxvaly=-9999.;
  uint32_t i_my,j_my;
  for (uint32_t i=0;i<160;i++)
    for (uint32_t j=0;j<160;j++)
      if (hhty->GetBinContent(i+1,j+1)>maxvaly)
	{
	  maxvaly = hhty->GetBinContent(i+1,j+1);
	  i_my=i+1;j_my=j+1;
	}

  float my=-8.+16./160.*(i_my-1)+8./160.;
  float ccy= -250.+500./160.*(j_my-1)+250./160.;
#endif

  //std::cout<<i_m<<" "<<j_m<<" "<<m<<" "<<cc<<" "<<maxval<<std::endl;
  //std::cout<<"Avec ROOT "<<i_m2<<" "<<j_m2<<" "<<rx<<" "<<thetax<<" "<<m2<<" "<<cc2<<" "<<maxval2<<std::endl;
  uint32_t _mvx;float _thx,_rx;
  theHTx_->findMaximum(_mvx,_thx,_rx);
  //std::cout<<"Avec HTImage "<<_rx<<" "<<_thx<<" "<<_mvx<<std::endl;
  thetax=_thx;
  rx=_rx;
  m2= -1./tan(thetax);
  cc2= rx/sin(thetax);
  //std::cout<<i_my<<" "<<j_my<<" "<<my<<" "<<ccy<<" "<<maxvaly<<std::endl;
  //std::cout<<"Avec ROOT "<<i_my2<<" "<<j_my2<<" "<<ry<<" "<<thetay<<" "<<my2<<" "<<ccy2<<" "<<maxvaly2<<std::endl;
  uint32_t _mvy;float _thy,_ry;
  theHTy_->findMaximum(_mvy,_thy,_ry);
  // std::cout<<"Avec HTImage "<<_ry<<" "<<_thy<<" "<<_mvy<<std::endl;
  thetay=_thy;
  ry=_ry;
  my2= -1./tan(thetay);
  ccy2= ry/sin(thetay);
  RecoCandTk tk;
  tkgood_.clear();
  if (maxval2>=5 || maxvaly2>=5) 
    {
      //     std::cout<<i_m2<<" "<<j_m2<<" "<<rx<<" "<<thetax<<" "<<m2<<" "<<cc2<<" "<<maxval2<<std::endl;
      //std::cout<<i_my2<<" "<<j_my2<<" "<<ry<<" "<<thetay<<" "<<my2<<" "<<ccy2<<" "<<maxvaly2<<std::endl;


      for (uint32_t i=tkFirstChamber_;i<tkLastChamber_;i++)
	{
	  //std::cout<<"Chambre "<<i<<std::endl;
	  std::map<uint32_t,std::vector<RecoPoint*> >::iterator it_ch = chamberPoints_.find(i);
	  if (it_ch==chamberPoints_.end()) continue;
	  //std::cout<<"Points trouve"<<std::endl;
	  float distmin=999999.;
	  RecoPoint* ipmin=0;
	  for (std::vector<RecoPoint*>::iterator ip=it_ch->second.begin();ip!=it_ch->second.end();ip++)
	    {
	      //std::cout<<"Point"<<ip->X()<<std::endl;
	      if ((*ip)->isUsed()) continue;
	      RecoPoint& pI=(**ip);
				
	      float xext=m2*(pI.Z()-50.)+cc2;
	      float yext=my2*(pI.Z()-50.)+ccy2;
	      float dist=sqrt((pI.X()-xext)*(pI.X()-xext)+(pI.Y()-yext)*(pI.Y()-yext));
	      if (dist<distmin)
		{
		  distmin=dist;
		  ipmin=(*ip);
		}
	    }
	  //std::cout<<i<<" "<<distmin<<std::endl;
	  if (distmin<5.)
	    {
	      tk.addPoint((*ipmin));
	      ipmin->setUsed(true);
	    }
	}

      // for (std::vector<RecoPoint>::iterator icl=allpoints_.begin();icl!=allpoints_.end();icl++)
      //   {
      // 	 float xext=m2*(icl->Z()-50.)+cc2;
      // 	 float yext=my2*(icl->Z()-50.)+ccy2;
      // 	 float dist=sqrt((icl->X()-xext)*(icl->X()-xext)+(icl->Y()-yext)*(icl->Y()-yext));
      // 	 if (dist<10.)
      // 	   {
      // 	     RecoPoint& pI=(*icl);
      // 	     tk.addPoint(pI);
      //   }
		
      //   }
      tk.regression();
      // DEBUG_PRINT("\t  Candidat nb hit =%d  %f %f %f %f\n",tk.getList().size(),tk.ax_,tk.ay_,tk.chi2_,tk.prChi2_);
      // for (uint32_t jj=0;jj<tk.getList().size();jj++)
      //   {
      // 	 float z=(tk.getList())[jj]->Z();
      // 	 float xext=tk.ax_*z+tk.bx_;
      // 	 float yext=tk.ay_*z+tk.by_;
      // 	 DEBUG_PRINT("\t \t %f : %f %f %f %f \n",z,xext-(tk.getList())[jj]->X(),(tk.getList())[jj]->dX(),yext-(tk.getList())[jj]->Y(),(tk.getList())[jj]->dY()); 
      //   }
      //getchar();
      if (tk.getList().size()>=(uint32_t)tkMinPoint_) 
	{


	  tk.prChi2_=TMath::Prob(tk.chi2_,2*tk.getList().size()-4);
	  //std::cout<<" Avec la regression "<<tk.getList().size()<<" "<<tk.prChi2_<<std::endl;
	  // Make a ROOT Fit
	  // 		 float x[50],ex[60],y[60],ey[60];
	  // 		 for (uint32_t jj=0;jj<tk.getList().size();jj++)
	  // 		   {
	  // 		     x[jj]=(tk.getList())[jj]->Z();
	  // 		     ex[jj]=0.1;
	  // 		     y[jj] = (tk.getList())[jj]->X();
	  // 		     ey[jj] = (tk.getList())[jj]->dX();
	  // 		     //ey[jj] = 0.25;
			
	  // 		   }
	  // 		 TF1 fa("fa","[0]*x+[1]",0.,120.);
	  // 		 TGraphErrors g(tk.getList().size(),x,y,ex,ey);

	  // 		 TFitResultPtr pf= g.Fit(&fa,"Q");
	  // 		 double theProb=1.;
	  // 		 std::cout<<fa.GetProb()<<std::endl;
	  // 		 if (fa.GetProb()<theProb) theProb=fa.GetProb();
	  // //pf.Get()->Print();
	  // 		 for (uint32_t jj=0;jj<tk.getList().size();jj++)
	  // 		   {
			
	  // 		     y[jj] = (tk.getList())[jj]->Y();
	  // 		     ey[jj] = (tk.getList())[jj]->dY();
	  // 		     //ey[jj] = 0.25;
	  // 		   }
	  // 		 TGraphErrors g1(tk.getList().size(),x,y,ex,ey);
	  // 		 TFitResultPtr pf1= g1.Fit(&fa,"Q");
	  // 		 if (fa.GetProb()<theProb) theProb=fa.GetProb();
	  // 		 std::cout<<fa.GetProb()<<std::endl;
	  // 		 tk.prChi2_=theProb;
	  // 		 std::cout<<"Proba "<<theProb<<std::endl;
	  //if (tk.prChi2_>=tkChi2Cut_) {
	  if (tk.prChi2_>tkChi2Cut_) {

				
	    houghIndex_++;
	    DEBUG_PRINT("\t HT %d nb hit =%d  %f %f %f %f\n",houghIndex_,tk.getList().size(),tk.ax_,tk.ay_,tk.chi2_,tk.prChi2_);


	    tkgood_.push_back(tk);
	  }
	}
    }
#ifdef DRAW_HOUGH
  TCanvas* c=new TCanvas("test1","test1",1400,800);
  c->cd();
  c->Draw();
  c->Modified();
  c->Update();

  // std::cout<<"On le voit ?"<<std::endl;
  c->Divide(2,2);
  c->cd(1);

  hzx->Draw("COLZ");

  TLine* l = new TLine(-50.,-50.*tk.ax_+tk.bx_+50*tk.ax_,70.,70.*tk.ax_+tk.bx_+50*tk.ax_);
  l->SetLineColor(4);
  l->Draw("SAME");

  TLine* l2 = new TLine(-50.,-50.*m2+cc2,70.,70.*m2+cc2);
  l2->SetLineColor(3);
  l2->Draw("SAME");


  c->cd(2);
  hhtx2->Draw("LEGO2");

  c->cd(3);

  hzy->Draw("COLZ");

  TLine* l1 = new TLine(-50.,-50.*tk.ay_+tk.by_+50*tk.ay_,70.,70.*tk.ay_+tk.by_+50*tk.ay_);
  l1->SetLineColor(4);
  l1->Draw("SAME");
  TLine* l21 = new TLine(-50.,-50.*my2+ccy2,70.,70.*my2+ccy2);
  l21->SetLineColor(3);
  l21->Draw("SAME");

  c->cd(4);
  hhty2->Draw("COLZ");




  c->Modified();
  c->Update();
  char ci;ci=getchar();putchar(ci); if (ci=='.') exit(0);;
  //::sleep(1);
#endif
}



void ShowerAnalyzer::findHoughCandidates3D(uint32_t stop,std::vector<RecoCandTk> &tkSeed)
{
  HTImage htx(120,0,1*M_PI,150,-150.,150.);
  std::vector<std::vector<RecoPoint>::iterator> usedPoint;
  uint32_t mvx;float thx,rx;
  uint32_t mvy;float thy,ry;

  usedPoint.clear();
  uint32_t nloop=0;
  do
    {
      nloop++;
      if (nloop>10) break;
      htx.Clear();
		
      for (std::vector<RecoPoint>::iterator icl=allpoints_.begin();icl!=allpoints_.end();icl++)
	{
	  //RECOCluster& c= (*icl).getCluster();

	  //if (icl->Charge()>10) continue;
	  RECOCluster& c= (*icl).getCluster();
	  if (c.getHits()->size()>4) continue;
	  if (std::find(usedPoint.begin(), usedPoint.end(), icl)!=usedPoint.end()) continue;


	  // 	 if (icl->getChamberId()>15) continue;
	  uint32_t w=0;
	  float wt=0;
	  for (std::vector<RecoHit>::iterator iht=c.getHits()->begin();iht!=c.getHits()->end();iht++)
	    {
	      int ithr= (*iht).getAmplitude()&0x3;
	      if (ithr==1) w=3;
	      if (ithr==2) w=1;
	      if (ithr==3) w=15;
	      wt+=w;
				
				
	    }








	  htx.addPixel(icl->Z()-50.,icl->X(),1);
	}
      htx.findMaximum(mvx,thx,rx);
      //     DEBUG_PRINT("Initial MVX %d \n",mvx);
      HC candx(mvx,thx,rx);
      if (mvx<stop) break;
		
      std::map<float,std::vector<RecoPoint>::iterator> mapDistx;
      mapDistx.clear();
      for (std::vector<RecoPoint>::iterator icl=allpoints_.begin();icl!=allpoints_.end();icl++)
	{
	  if (std::find(usedPoint.begin(), usedPoint.end(), icl)!=usedPoint.end()) continue;
	  float distx;

	  distx=TMath::Abs(icl->X()-candx.Pos(icl->Z()));
	  candx.setDmin(distx,icl->getChamberId());
	  //if (dist>4.) continue;
	  mapDistx[distx]=icl;
	}
      uint32_t ndx=0;
      float dminx=99999,dminy=9999;
      for (std::map<float,std::vector<RecoPoint>::iterator>::iterator idx=mapDistx.begin();idx!=mapDistx.end();idx++)
	{
	  //if (idx->first>1.5) break;
	  usedPoint.push_back(idx->second);
	  if (idx->first==candx.dmin_[idx->second->getChamberId()]) 
	    {
	      candx.add(idx->second);
	      ndx++;
	    }
	  if (ndx==mvx) break;
			
	}
      DEBUG_PRINT("Type X  Final Candidat Npoints %d\n",candx.points_.size());

      // Now Make a Hough transform in Y
      HTImage hty(120,0,1*M_PI,150,-150.,150.);
      hty.Clear();
      for (std::vector<std::vector<RecoPoint>::iterator>::iterator idx=candx.points_.begin();idx!=candx.points_.end();idx++)
	{
	  hty.addPixel((*idx)->Z()-50.,(*idx)->Y(),1.);
	}
      hty.findMaximum(mvy,thy,ry);
      DEBUG_PRINT("%d in Y \n",mvy);

      if (mvy>=5)
	{
	  HC candy(mvy,thy,ry);

			
	  std::map<float,std::vector<RecoPoint>::iterator> mapDisty;
	  mapDisty.clear();
	  for (std::vector<std::vector<RecoPoint>::iterator>::iterator idx=candx.points_.begin();idx!=candx.points_.end();idx++)
	    {
	      float disty;
				
	      disty=TMath::Abs((*idx)->Y()-candy.Pos((*idx)->Z()));
	      candy.setDmin(disty,(*idx)->getChamberId());
	      //if (dist>4.) continue;
	      mapDisty[disty]=(*idx);
	    }
	  uint32_t ndy=0;
	  float dminy=9999;
	  for (std::map<float,std::vector<RecoPoint>::iterator>::iterator idy=mapDisty.begin();idy!=mapDisty.end();idy++)
	    {
	      //if (idy->first>1.5) break;
	      usedPoint.push_back(idy->second);
	      if (idy->first==candy.dmin_[idy->second->getChamberId()]) 
		{
		  candy.add(idy->second);
		  ndy++;
		}
	      if (ndy==mvy) break;
				
	    }
	  DEBUG_PRINT("Type Y  Final Candidat Npoints %d\n",candy.points_.size());
	  RecoCandTk tkht;
	  tkht.ax_= candx.a_;
	  tkht.bx_= candy.b_;
	  //DEBUG_PRINT("Npx %d\n",itx->points_.size());
	  //std::cout<<i_my<<" "<<j_my<<" "<<my<<" "<<ccy<<" "<<maxvaly<<std::endl;
	  //std::cout<<"Avec ROOT "<<i_my2<<" "<<j_my2<<" "<<ry<<" "<<thetay<<" "<<my2<<" "<<ccy2<<" "<<maxvaly2<<std::endl;

	  // std::cout<<"Avec HTImage "<<_ry<<" "<<_thy<<" "<<_mvy<<std::endl;
			
	  tkht.ay_= candy.a_;
	  tkht.by_= candy.b_;
	  uint32_t nc=0;
	  //DEBUG_PRINT("Npy %d\n",ity->points_.size());
	  for (std::vector<std::vector<RecoPoint>::iterator>::iterator ipx=candx.points_.begin();ipx!=candx.points_.end();ipx++)
	    {
	      if (std::find(candy.points_.begin(), candy.points_.end(), *ipx)!=candy.points_.end()) 
		tkht.addNearestPoint((*(*ipx)));
	    }
	  tkht.removeDistantPoint(7.);
	  if (tkht.getList().size()>=5)
	    {
				
	      tkht.regression();
	      DEBUG_PRINT("Avant %f %f %f %f %d  Chi2 %f\n ",tkht.ax_,tkht.bx_,tkht.ay_,tkht.by_,tkht.getList().size(),tkht.chi2_);
	      // for (std::vector<double>::iterator ic= tkht.getChi2().begin();ic!=tkht.getChi2().end();ic++)
	      //   DEBUG_PRINT("%f ",(*ic));
	      // DEBUG_PRINT("\n");
				
	      RecoCandTk tref;
	      tkht.Refit(tref,7.);
	      if (tref.getList().size()>=4)
		{
		  DEBUG_PRINT("Apres %f %f %f %f %d  Chi2 %f\n",tref.ax_,tref.bx_,tref.ay_,tref.by_,tref.getList().size(),tref.chi2_);
		  tref.addChi2Points(allpoints_,3.,&usedPoint);
		  DEBUG_PRINT("Final %f %f %f %f %d  Chi2 %f\n",tref.ax_,tref.bx_,tref.ay_,tref.by_,tref.getList().size(),tref.chi2_);
		  // for (std::vector<double>::iterator ic= tref.getChi2().begin();ic!=tref.getChi2().end();ic++)
		  //   DEBUG_PRINT("%f ",(*ic));
		  // DEBUG_PRINT("\n");
		  // for (std::vector<RecoPoint*>::iterator ip=tref.getList().begin();ip!=tref.getList().end();ip++)
		  //   {
		  //     std::vector<RecoPoint>::iterator it= (std::vector<RecoPoint>::iterator) *ip;
		  //     if (std::find(usedPoint.begin(), usedPoint.end(), it)!=usedPoint.end()) continue;
		  //     usedPoint.push_back(it);

		  //   }
		  tkSeed.push_back(tref);
		}
	    }
			
			
	}
      //getchar();

      if (ndx<stop) break;

    } while (mvx>=stop );
  return;
}


void ShowerAnalyzer::findHoughCandidates(uint32_t type,uint32_t stop,std::vector<HC> &vX)
{
  HTImage htx(120,0,1*M_PI,150,-150.,150.);
  std::vector<std::vector<RecoPoint>::iterator> usedPoint;
  uint32_t mvx;float thx,rx;
  usedPoint.clear();
  do
    {
      htx.Clear();
		
      for (std::vector<RecoPoint>::iterator icl=allpoints_.begin();icl!=allpoints_.end();icl++)
	{
	  //RECOCluster& c= (*icl).getCluster();

	  if (icl->Charge()<10) continue;
	  if (std::find(usedPoint.begin(), usedPoint.end(), icl)!=usedPoint.end()) continue;

	  RECOCluster& c= (*icl).getCluster();
	  // 	 if (icl->getChamberId()>15) continue;
	  uint32_t w=0;
	  float wt=0;
	  for (std::vector<RecoHit>::iterator iht=c.getHits()->begin();iht!=c.getHits()->end();iht++)
	    {
	      int ithr= (*iht).getAmplitude()&0x3;
	      if (ithr==1) w=3;
	      if (ithr==2) w=1;
	      if (ithr==3) w=15;
	      wt+=w;
				
				
	    }








	  if (type==1)
	    htx.addPixel(icl->Z()-50.,icl->X(),icl->Charge());
	  else
	    htx.addPixel(icl->Z()-50.,icl->Y(),icl->Charge());
	}
      htx.findMaximum(mvx,thx,rx);
      //     DEBUG_PRINT("Initial MVX %d \n",mvx);
      HC candx(mvx,thx,rx);
      if (mvx<stop) break;
		
      std::map<float,std::vector<RecoPoint>::iterator> mapDistx;
      mapDistx.clear();
      for (std::vector<RecoPoint>::iterator icl=allpoints_.begin();icl!=allpoints_.end();icl++)
	{
	  if (std::find(usedPoint.begin(), usedPoint.end(), icl)!=usedPoint.end()) continue;
	  float distx;
	  if (type==1)
	    distx=TMath::Abs(icl->X()-candx.Pos(icl->Z()));
	  else
	    distx=TMath::Abs(icl->Y()-candx.Pos(icl->Z()));
	  candx.setDmin(distx,icl->getChamberId());
	  //if (dist>4.) continue;
	  mapDistx[distx]=icl;
	}
      uint32_t ndx=0;
      float dminx=99999,dminy=9999;
      for (std::map<float,std::vector<RecoPoint>::iterator>::iterator idx=mapDistx.begin();idx!=mapDistx.end();idx++)
	{
	  //if (idx->first>1.5) break;
	  usedPoint.push_back(idx->second);
	  if (idx->first==candx.dmin_[idx->second->getChamberId()]) 
	    {
	      candx.add(idx->second);
	      ndx++;
	    }
	  if (ndx==mvx) break;
			
	}
      DEBUG_PRINT("Type %d  Final Candidat Npoints %d\n",type,candx.points_.size());
      vX.push_back((candx));
      if (ndx<stop) break;

    } while (mvx>=stop );
  return;
}

void ShowerAnalyzer::mergeHoughCandidate(std::vector<HC> &vX,std::vector<HC> &vY,std::vector<RecoCandTk> &tkSeed)
{
  // Clear tracks seed
  //DEBUG_PRINT("1\n");
  tkSeed.clear();
  // Loop on X candidate
  for (std::vector<HC>::iterator itx=vX.begin();itx!=vX.end();itx++)
    {
      RecoCandTk tkht;
      tkht.ax_= itx->a_;
      tkht.bx_= itx->b_;
      //DEBUG_PRINT("Npx %d\n",itx->points_.size());
      //std::cout<<i_my<<" "<<j_my<<" "<<my<<" "<<ccy<<" "<<maxvaly<<std::endl;
      //std::cout<<"Avec ROOT "<<i_my2<<" "<<j_my2<<" "<<ry<<" "<<thetay<<" "<<my2<<" "<<ccy2<<" "<<maxvaly2<<std::endl;

      // std::cout<<"Avec HTImage "<<_ry<<" "<<_thy<<" "<<_mvy<<std::endl;
		
      for (std::vector<HC>::iterator ity=vY.begin();ity!=vY.end();)
	// Loop on hits
	{
	  tkht.ay_= ity->a_;
	  tkht.by_= ity->b_;
	  uint32_t nc=0;
	  //DEBUG_PRINT("Npy %d\n",ity->points_.size());
	  for (std::vector<std::vector<RecoPoint>::iterator>::iterator ipx=itx->points_.begin();ipx!=itx->points_.end();ipx++)
	    {
	      if (std::find(ity->points_.begin(), ity->points_.end(), *ipx)!=ity->points_.end()) 
		{
		  tkht.addPoint((*(*ipx)));
		}
	    }
	  // DEBUG_PRINT("Size %d\n",tkht.getList().size());
	  if (tkht.getList().size()>=4)
	    {
	      vY.erase(ity);
	      break;
	    }
	  else
	    ++ity;
	}

      // DEBUG_PRINT("3\n");
		
      if (tkht.getList().size()>=4)
	{
			
	  tkht.regression();
	  //	  DEBUG_PRINT("Avant %f %f %f %f %d  Chi2 %f\n \t \t ",tkht.ax_,tkht.bx_,tkht.ay_,tkht.by_,tkht.getList().size(),tkht.chi2_);
	  // for (std::vector<double>::iterator ic= tkht.getChi2().begin();ic!=tkht.getChi2().end();ic++)
	  //   DEBUG_PRINT("%f ",(*ic));
	  // DEBUG_PRINT("\n");
	  RecoCandTk tref;
	  tkht.Refit(tref,500.);
	  if (tref.getList().size()>=4)
	    {
	      //  DEBUG_PRINT("Apres %f %f %f %f %d  Chi2 %f\n",tref.ax_,tref.bx_,tref.ay_,tref.by_,tref.getList().size(),tref.chi2_);
	      // for (std::vector<double>::iterator ic= tref.getChi2().begin();ic!=tref.getChi2().end();ic++)
	      //   DEBUG_PRINT("%f ",(*ic));
	      // DEBUG_PRINT("\n");
				

	      tkSeed.push_back(tref);
	    }
	}

    }
  //DEBUG_PRINT("=> %d \n",tkSeed.size());
  return;
}

void ShowerAnalyzer::HT3D()
{
  std::vector<HC> vX;
  std::vector<HC> vY;



  std::vector<std::vector<RecoPoint>::iterator> usedPoint;
  std::vector<std::vector<RecoPoint>::iterator > vCom;

  usedPoint.clear();

  uint32_t mvx=0;float thx,rx;
  uint32_t mvy=0;float thy,ry;
  bool first=true,found=false;
  uint32_t nmip=0,nhit=0;
  uint16_t nh0[61],nh1[61],nh2[61],smip[61];
  memset(&nh0,0,61*sizeof(uint16_t));
  memset(&nh2,0,61*sizeof(uint16_t));
  memset(&nh1,0,61*sizeof(uint16_t));
  memset(&smip,0,61*sizeof(uint16_t));
  TH1* hhnh0= rootHandler_->GetTH1("HNumberOfHit0");
  TH1* hhnh1= rootHandler_->GetTH1("HNumberOfHit1");
  TH1* hhnh2= rootHandler_->GetTH1("HNumberOfHit2");
  TH1* hhnht= rootHandler_->GetTH1("HNumberOfHitTotal");
  TH1* hhenergy= rootHandler_->GetTH1("HEnergy");
  TH1* hhmip= rootHandler_->GetTH1("HMip");

  if (hhnh0==NULL)
    {
      hhnh0 =rootHandler_->BookTH1( "HNumberOfHit0",2000,0.,2000.);
      hhnh1 =rootHandler_->BookTH1( "HNumberOfHit1",4000,0.,4000.);
      hhnh2 =rootHandler_->BookTH1( "HNumberOfHit2",2000,0.,2000.);
      hhnht =rootHandler_->BookTH1( "HNumberOfHitTotal",6000,0.,6000.);
      hhenergy =rootHandler_->BookTH1( "HEnergy",750,0.,150000.);
      hhmip =rootHandler_->BookTH1( "HMip",1000,0.,10000.);
		
    }
  bool showerFound=false;
  bool tkFound=false;
  TH3* hcgposin = rootHandler_->GetTH3("InstantClusterMapIn");
  uint32_t shcol=6,spcol=6;
  if (hcgposin==NULL)
    {
      hcgposin =rootHandler_->BookTH3("InstantClusterMapIn",52,-2.8,145.6,100,0.,100.,100,0.,100.);
		
    }
  else
    {
      if (TCPlot!=NULL) hcgposin->Reset();
    }
  do
    {

      //findHoughCandidates(1,7,vX);
      //findHoughCandidates(2,7,vY);
		
      // htx.Clear();
      // hty.Clear();
		
      // for (std::vector<RecoPoint>::iterator icl=allpoints_.begin();icl!=allpoints_.end();icl++)
      //   {
      // 	 RECOCluster& c= (*icl).getCluster();
      // 	 if (icl->getChamberId()>15) continue;
      // 	 uint32_t w=0;
      // 	 float wt=0;
      // 	 for (std::vector<RecoHit>::iterator iht=c.getHits()->begin();iht!=c.getHits()->end();iht++)
      // 	   {
      // 	     int ithr= (*iht).getAmplitude()&0x3;
      // 	     if (ithr==1) w=1;
      // 	     if (ithr==2) w=3;
      // 	     if (ithr==3) w=15;
      // 	     wt+=w;
      // 	     if (first)
      // 	       {
      // 		 nmip+=w;
      // 		 nhit++;
      // 		 smip[icl->getChamberId()] +=w;
		
      // 	       }
		
      // 	   }
		
		
      // 	 if (std::find(usedPoint.begin(), usedPoint.end(), icl)!=usedPoint.end()) continue;
      // 	 htx.addPixel(icl->Z()-50.,icl->X(),1);
      // 	 hty.addPixel(icl->Z()-50.,icl->Y(),1);
      //   }
      // first=false;
      // std::vector<HC> vX;
      // std::vector<HC> vY;
      // do
      //   {
      // 	 htx.findMaximum(mvx,thx,rx);
      // 	 HC candx(mvx,thx,rx);
      // 	 DEBUG_PRINT("X %d %f %f \n",mvx,thx,rx);
      // 	 vX.push_back(candx);
      //   } while (mvx>5);

      // do
      //   {
      // 	 hty.findMaximum(mvy,thy,ry);
      // 	 HC candy(mvy,thy,ry);
      // 	 DEBUG_PRINT("X %d %f %f \n",mvy,thy,ry);

      // 	 vY.push_back(candy);
      //   } while (mvy>5);


      std::vector<RecoCandTk> tkSeed;
      //     DEBUG_PRINT("MV x et Y %d  %d \n ",vX.size(),vY.size());


      //mergeHoughCandidate(vX,vY,tkSeed);
      DEBUG_PRINT("Number of seed %d  avant \n",tkSeed.size());
      findHoughCandidates3D(5,tkSeed);
      DEBUG_PRINT("Number of seed %d \n",tkSeed.size());
      //getchar();
#ifdef DIST_METHODE
      for (std::vector<HC>::iterator ihx=vX.begin();ihx!=vX.end();ihx++)
	{
	  RecoCandTk tkht;
	  tkht.ax_= ihx->a_;
	  tkht.bx_= ihx->b_;
	  for (std::vector<HC>::iterator ihy=vY.begin();ihy!=vY.end();)
	    {

	      //std::cout<<i_my<<" "<<j_my<<" "<<my<<" "<<ccy<<" "<<maxvaly<<std::endl;
	      //std::cout<<"Avec ROOT "<<i_my2<<" "<<j_my2<<" "<<ry<<" "<<thetay<<" "<<my2<<" "<<ccy2<<" "<<maxvaly2<<std::endl;

	      // std::cout<<"Avec HTImage "<<_ry<<" "<<_thy<<" "<<_mvy<<std::endl;
	      tkht.ay_= ihy->a_;
	      tkht.by_= ihy->b_;
				
	      for (std::vector<RecoPoint>::iterator icl=allpoints_.begin();icl!=allpoints_.end();icl++)
		{
		  if (tkht.calculateDistance((*icl))<2.){tkht.addNearestPoint((*icl));}
		}
	      if (tkht.getList().size()>=4)
		{
		  vY.erase(ihy++);
		  break;
		}
	      else
		ihy++;
	    }
	  if (tkht.getList().size()>=4)
	    {

	      tkht.regression();
	      DEBUG_PRINT("%f %f %f %f %d  Chi2 %f\n",tkht.ax_,tkht.bx_,tkht.ay_,tkht.by_,tkht.getList().size(),tkht.chi2_);
	      for (std::vector<double>::iterator ic= tkht.getChi2().begin();ic!=tkht.getChi2().end();ic++)
		DEBUG_PRINT("%f ",(*ic));
	      DEBUG_PRINT("\n");
	      tkSeed.push_back(tkht);
	    }
			

	}
      DEBUG_PRINT("=> %d \n",tkSeed.size());
#endif
#ifdef OLDTK_LOOP
      if (TCPlot!=NULL)
	{
	  TCPlot->cd(1);
	  for (std::vector<RecoCandTk>::iterator itk=tkSeed.begin();itk!=tkSeed.end();itk++)
	    {
	      RecoCandTk& tk=(*itk);
	      TPolyLine3D *pl3d1 = new TPolyLine3D(2);
				
	      pl3d1->SetPoint(0, tk.zmin_,tk.ax_*tk.zmin_+tk.bx_,tk.ay_*tk.zmin_+tk.by_);
	      pl3d1->SetPoint(1, tk.zmax_,tk.ax_*tk.zmax_+tk.bx_,tk.ay_*tk.zmax_+tk.by_);
				
	      pl3d1->SetLineWidth(2);
	      pl3d1->SetLineColor(6);
	      pl3d1->Draw("SAME");
	    }
	  TCPlot->Modified();
	  TCPlot->Update();

	}


      char c;c=getchar();putchar(c); if (c=='.') exit(0);;
		
      if (1>0) break;

      if (mvx<5 && mvy<5) break;
      HC candx(mvx,thx,rx);
      HC candy(mvy,thy,ry);
      std::map<float,std::vector<RecoPoint>::iterator> mapDistx;
      mapDistx.clear();
      std::map<float,std::vector<RecoPoint>::iterator> mapDisty;
      mapDisty.clear();
      for (std::vector<RecoPoint>::iterator icl=allpoints_.begin();icl!=allpoints_.end();icl++)
	{
	  if (std::find(usedPoint.begin(), usedPoint.end(), icl)!=usedPoint.end()) continue;
	  float distx=TMath::Abs(icl->X()-candx.Pos(icl->Z()));
	  float disty=TMath::Abs(icl->Y()-candy.Pos(icl->Z()));
	  //if (dist>4.) continue;
	  mapDistx[distx]=icl;
	  mapDisty[disty]=icl;
	}


      // Now loop on mvx first reco points and check the mvy first reco also
      vCom.clear();
      uint32_t ndx=0;
      float dminx=99999,dminy=9999;
      for (std::map<float,std::vector<RecoPoint>::iterator>::iterator idx=mapDistx.begin();idx!=mapDistx.end();idx++)
	{
	  if (idx->first<dminx) dminx=idx->first;
	  if (idx->first>2) break;
	  ndx++;
	  uint32_t ndy=0;
	  for (std::map<float,std::vector<RecoPoint>::iterator>::iterator idy=mapDisty.begin();idy!=mapDisty.end();idy++)
	    {
	      if (idx->first<dminy) dminy=idy->first;
	      if (idy->first>2) break;
	      if (sqrt(idx->first*idx->first+idy->first*idy->first)>tkDistCut_) continue;
	      ndy++;
	      if (idx->second == idy->second) vCom.push_back(idx->second);

	    }
	}
      //if (dminx> || dminy>5.) break;



      DEBUG_PRINT(" Common points %d \n",vCom.size());
      // Now remove Hits from vCom where zmin>7.
      //std::vector<std::vector<RecoPoint>::iterator > vGood;
      RecoCandTk tk;
      for (std::vector<std::vector<RecoPoint>::iterator >::iterator ip=vCom.begin();ip!=vCom.end();ip++)
	{
	  float zmin=150.;
	  for (std::vector<std::vector<RecoPoint>::iterator >::iterator jp=vCom.begin();jp!=vCom.end();jp++)
	    {

	      float dz=TMath::Abs((*ip)->Z()-(*jp)->Z());
	      if (dz<zmin && dz!=0) zmin=dz;
	    }
	  if (zmin>7) continue;
	  tk.addPoint((*(*ip)));
			
	}
      if (tk.getList().size()<3) break; //At least 3 points
      tk.regression();
#else
      DEBUG_PRINT("Avant drawdisplay \n");
      if (draw_ ) this->drawDisplay();
      char c;c=getchar();putchar(c); if (c=='.') exit(0);;
		
      for (std::vector<RecoCandTk>::iterator itk=tkSeed.begin();itk!=tkSeed.end();itk++)
	{
	  RecoCandTk &tk=(*itk);
	  tkFound=true;
	  showerFound=false;
	  //Add all points < 5 cm
	  std::vector<std::vector<RecoPoint>::iterator > vAdj;
	  for (std::vector<RecoPoint>::iterator icl=allpoints_.begin();icl!=allpoints_.end();icl++)
	    {
	      RECOCluster& c= (*icl).getCluster();
	      //if (icl->getChamberId()==20) DEBUG_PRINT("Size of cluster %d in chamber %d %f %f  \n",c.getHits()->size(),icl->getChamberId(),c.X(),c.Y());
	      if (std::find(usedPoint.begin(), usedPoint.end(), icl)!=usedPoint.end()) continue;
	      if (tk.calculateDistance((*icl)) <5)  {vAdj.push_back(icl);continue;}
	      // float distx=TMath::Abs(icl->X()-candx.Pos(icl->Z()));
	      // float disty=TMath::Abs(icl->Y()-candy.Pos(icl->Z()));
	      // if (sqrt(distx*distx+disty*disty)<5) 
	      //   {vAdj.push_back(icl);continue;}
	    }
	  uint32_t nadj=vAdj.size();
	  do
	    {
	      nadj=vAdj.size();
	      for (std::vector<RecoPoint>::iterator icl=allpoints_.begin();icl!=allpoints_.end();icl++)
		{
		  if (std::find(usedPoint.begin(), usedPoint.end(), icl)!=usedPoint.end()) continue;
		  if (std::find(vAdj.begin(), vAdj.end(), icl)!=vAdj.end()) continue;
		  RECOCluster& c= (*icl).getCluster();
		  bool closeby=false;

		  for (std::vector<RecoHit>::iterator iht=c.getHits()->begin();iht!=c.getHits()->end();iht++)
		    {
		      for (std::vector<std::vector<RecoPoint>::iterator >::iterator ip=vAdj.begin();ip!=vAdj.end();ip++)
			{
							
			  RECOCluster& c1= (*(*ip)).getCluster();
							
			  for (std::vector<RecoHit>::iterator iht1=c1.getHits()->begin();iht1!=c1.getHits()->end();iht1++)
			    {
			      uint32_t iDist=abs(iht->chamber()-iht1->chamber())+5*(abs(iht1->I()-iht->I())+abs(iht1->J()-iht->J()));
			      if (iDist<15)
				{closeby=true;break;}
			      // if (iht->Z()==iht1->Z())
			      //   {
			      // 	 if (sqrt((iht->X()-iht1->X())*(iht->X()-iht1->X())+(iht->Y()-iht1->Y())*(iht->Y()-iht1->Y()))<2.)
			      // 	   {closeby=true;break;}
			      //   }
			      // else
			      //   if (sqrt((iht->X()-iht1->X())*(iht->X()-iht1->X())+(iht->Y()-iht1->Y())*(iht->Y()-iht1->Y())+(iht->Z()-iht1->Z())*(iht->Z()-iht1->Z()))<5.)
			      // 	 {closeby=true;break;}
			    }
			  if (closeby)
			    {break;}
			}
		      if (closeby)
			{vAdj.push_back(icl);break;}
		    }
		}
	    }
	  while (vAdj.size()>nadj);
	  uint32_t nhitadj=0,mh0=0,mh1=0,mh2=0;
	  memset(&nh0,0,61*sizeof(uint16_t));
	  memset(&nh2,0,61*sizeof(uint16_t));
	  memset(&nh1,0,61*sizeof(uint16_t));
	  // Now tag all used hits
	  for (std::vector<std::vector<RecoPoint>::iterator >::iterator ip=vAdj.begin();ip!=vAdj.end();ip++)
	    {
	      usedPoint.push_back(*ip);
	      RECOCluster& c1= (*(*ip)).getCluster();
	      for (std::vector<RecoHit>::iterator iht1=c1.getHits()->begin();iht1!=c1.getHits()->end();iht1++)
		{
		  nhitadj++;
		  int ithr= (*iht1).getAmplitude()&0x3;
		  if (ithr==2) {nh0[(*(*ip)).getChamberId()] +=1;mh0++;}
		  if (ithr==1) {nh1[(*(*ip)).getChamberId()] +=1;mh1++;}
		  if (ithr==3) {nh2[(*(*ip)).getChamberId()] +=1;mh2++;}
		}
	    }
			
	  uint32_t fp=61,lp=1;
	  for (uint32_t i=1;i<61;i++)
	    {
	      if ((nh0[i]+nh1[i]+nh2[i])>2 && i<fp) fp=i;
	      if ( (nh0[i]+nh1[i]+nh2[i])>2 && i>lp) lp=i;
				
	    }
	  int dl=lp-fp;
	  //     float energy=(mh0*69.83990544751262+mh1*70.06174985305178+mh2*69.71033820211227);
	  float energy=(mh0*45+mh1*148+mh2*553.);
			
	  // DEBUG_PRINT("Candidate  In %d All %d E %f  First %d  Last %d  Length %d  HT size %d \n",vAdj.size(),allpoints_.size(),energy,fp,lp,lp-fp,vCom.size());
	  // for (int32_t i=1;i<=48;i++)
	  //   DEBUG_PRINT("[%d] %d %d %d =>%d \n",i,nh0[i],nh1[i],nh2[i],nh0[i]+nh1[i]+nh2[i]);
	  // getchar();
	  if (fp>=1 && lp<=48 && dl>4 )
	    {
	      hhnh0->Fill(mh0*1.);
	      hhnh1->Fill(mh1*1.);
	      hhnh2->Fill(mh2*1.);
	      hhnht->Fill((mh0+mh1+mh2)*1.);
	      hhmip->Fill((mh0+3.*mh1+15*mh2));
	      hhenergy->Fill(energy);
	      showerFound=true;
	      DEBUG_PRINT("New Shower In %d All %d E %f  First %d  Last %d  Length %d  HT size %d \n",vAdj.size(),allpoints_.size(),energy,fp,lp,lp-fp,vCom.size());
	    }
			
	  found=true;
	  DEBUG_PRINT("Shower %d Track %d Adj %d \n",showerFound,tkFound,vAdj.size());
	  if (TCPlot!=NULL && (showerFound||(tkFound && vAdj.size()>3)))
	    {
	      std::stringstream s("");
	      s<<"InstantClusterMapIn"<<shcol;
	      TH3* hcgposin = rootHandler_->GetTH3(s.str());
	      std::stringstream sp("");
	      sp<<"Hit20"<<spcol;
	      TH3* hch20= rootHandler_->GetTH3(sp.str());
	      if (hcgposin==NULL)
		{
		  hcgposin =rootHandler_->BookTH3(s.str(),52,-2.8,145.6,100,0.,100.,100,0.,100.);
		  hch20= rootHandler_->BookTH3(sp.str(),52,-2.8,145.6,100,0.,100.,100,0.,100.);
		}
	      else
		{
		  if (TCPlot!=NULL) hcgposin->Reset();
		  hch20->Reset();
		}
				
	      for (std::vector<RecoPoint*>::iterator ip=tk.getList().begin();ip!=tk.getList().end();ip++)
		{
		  hch20->Fill((*(*ip)).Z()*1.,(*(*ip)).X(),(*(*ip)).Y());
		}
	      for (std::vector<std::vector<RecoPoint>::iterator >::iterator ip=vAdj.begin();ip!=vAdj.end();ip++)
		{
		  RECOCluster& c= (*(*ip)).getCluster();
					
		  for (std::vector<RecoHit>::iterator iht=c.getHits()->begin();iht!=c.getHits()->end();iht++)
		    {
		      hcgposin->Fill((*iht).Z(),(*iht).X(),(*iht).Y());	 
						
		    }
		}

	      // for (std::vector<RecoPoint>::iterator ip=allpoints_.begin();ip!=allpoints_.end();ip++)
	      //   {
	      //     RECOCluster& c= (*ip).getCluster();

	      //     for (std::vector<RecoHit>::iterator iht=c.getHits()->begin();iht!=c.getHits()->end();iht++)
	      //       {
	      // 	 hcgposin->Fill((*iht).Z(),(*iht).X(),(*iht).Y());	 
	      // 	 if ((*ip).getChamberId()==20) hch20->Fill((*iht).I()*1.,(*iht).J()*1.);
	      //       }
	      // }
	      TPolyLine3D *pl3d1 = new TPolyLine3D(2);
	      TCPlot->cd(1);
	      pl3d1->SetPoint(0, tk.zmin_,tk.ax_*tk.zmin_+tk.bx_,tk.ay_*tk.zmin_+tk.by_);
	      pl3d1->SetPoint(1, tk.zmax_,tk.ax_*tk.zmax_+tk.bx_,tk.ay_*tk.zmax_+tk.by_);
				
	      pl3d1->SetLineWidth(2);
	      pl3d1->SetLineColor(1);
	      pl3d1->Draw("SAME");
	      TCPlot->cd(2);
	      TLine* l = new TLine(tk.zmin_,tk.ax_*tk.zmin_+tk.bx_,tk.zmax_,tk.ax_*tk.zmax_+tk.bx_);
	      l->SetLineColor(1);
	      l->SetLineWidth(2);
	      l->Draw("SAME");
	      TCPlot->cd(3);
	      // TLine* l1 = new TLine(tk.zmin_,tk.ay_*tk.zmin_+tk.by_,tk.zmax_,tk.ay_*tk.zmax_+tk.by_);
	      // l1->SetLineColor(1);
	      // l1->SetLineWidth(2);
	      // l1->Draw("SAME");
	      hch20->SetMarkerStyle(25);
	      hch20->SetMarkerSize(.2);
	      hch20->SetMarkerColor(shcol);
	      if (spcol==6)
		{
		  hch20->Draw("P");
		}
	      else
		hch20->Draw("PSAME");
	      spcol++;
	      pl3d1->SetLineWidth(1);
	      pl3d1->SetLineColor(1);
	      pl3d1->Draw("SAME");

				
	      TCPlot->cd(4);
	      //hmip->Draw();
	      hcgposin->SetMarkerStyle(25);
	      hcgposin->SetMarkerSize(.2);
	      hcgposin->SetMarkerColor(shcol);

	      if (shcol==6)
		hcgposin->Draw("p");
	      else
		hcgposin->Draw("psame");
	      shcol++;
	      TCPlot->Modified();
	      TCPlot->Update();
				

	    }
	}
#endif
      // if (vCom.size()>=5)
      //   {
      // 	 RecoCandTk tk;
      // 	 for (std::vector<std::vector<RecoPoint>::iterator >::iterator ip=vCom.begin();ip!=vCom.end();ip++)
      // 	   {
      // 	     tk.addPoint((*(*ip)));
      // 	     usedPoint.push_back(*ip);
      // 	   }
      // 	 tk.regression();
		
      //   }
      // else
      //   {
      // 	 uint32_t nd=0;
      // 	 for (std::map<float,std::vector<RecoPoint>::iterator>::iterator id=mapDistx.begin();id!=mapDistx.end();id++)
      // 	   {
      // 	     nd++;
      // 	     //if (nd>candx.m_) break;
      // 	     if (id->first>3 ) break;
      // 	     usedPoint.push_back(id->second);
      // 	     candx.add(id->second);
      // 	   }
      // 	 vX.push_back(candx);
      // 	 nd=0;
      // 	 for (std::map<float,std::vector<RecoPoint>::iterator>::iterator id=mapDisty.begin();id!=mapDisty.end();id++)
      // 	   {
      // 	     nd++;
      // 	     //if (nd>candy.m_) break;
      // 	     if (id->first>3 ) break;
      // 	     usedPoint.push_back(id->second);
      // 	     candy.add(id->second);
      // 	   }
      // 	 vY.push_back(candy);
      //   }
      if (1>0) break;
    } while (mvx>=7 || mvy>=7);


  //DEBUG_PRINT("Sizes   %d %d \n",vX.size(),vY.size());
#ifdef ADDITIONAL_TRIES
  vCom.clear(); 
  for (std::vector<HC>::iterator itx=vX.begin();itx!=vX.end();itx++)
    {
      //intf("Size ne X %d \n",(*itx).points_.size());
		
      for (std::vector<HC>::iterator ity=vY.begin();ity!=vY.end();ity++)
	{

	  uint32_t nc=(*itx).common((*ity),vCom);
	  if (nc>=7)
	    {
	      //DEBUG_PRINT("New Candidate %d \n",nc);
	      //(*itx).Dump();
	      //(*ity).Dump();
	      RecoCandTk tk;
	      for (std::vector<std::vector<RecoPoint>::iterator >::iterator ip=vCom.begin();ip!=vCom.end();ip++)
		tk.addPoint((*(*ip)));
	      tk.regression();
	      found=true;
	      if (TCPlot!=NULL)
		{

		  TPolyLine3D *pl3d1 = new TPolyLine3D(2);
		  TCPlot->cd(1);
		  pl3d1->SetPoint(0, tk.zmin_,tk.ax_*tk.zmin_+tk.bx_,tk.ay_*tk.zmin_+tk.by_);
		  pl3d1->SetPoint(1, tk.zmax_,tk.ax_*tk.zmax_+tk.bx_,tk.ay_*tk.zmax_+tk.by_);

		  pl3d1->SetLineWidth(1);
		  pl3d1->SetLineColor(12);
		  pl3d1->Draw("SAME");
		  TCPlot->cd(2);
		  TLine* l = new TLine(tk.zmin_,tk.ax_*tk.zmin_+tk.bx_,tk.zmax_,tk.ax_*tk.zmax_+tk.bx_);
		  l->SetLineColor(12);
		  l->SetLineWidth(1);
		  l->Draw("SAME");
		  TCPlot->cd(3);
		  TLine* l1 = new TLine(tk.zmin_,tk.ay_*tk.zmin_+tk.by_,tk.zmax_,tk.ay_*tk.zmax_+tk.by_);
		  l1->SetLineColor(12);
		  l1->SetLineWidth(1);
		  l1->Draw("SAME");
					
		  TCPlot->Modified();
		  TCPlot->Update();
		}
	    }
	}
    }
#endif
  if (showerFound || tkFound)
    {
		
      if (draw_) 
	{
	  DEBUG_PRINT("===============> Number of MIP %d %d %f \n",nmip,nhit,nmip*1./nhit);
	  std::stringstream ss("");
	  ss<<"/tmp/Display_"<<evt_->getRunNumber()<<"_"<<evt_->getEventNumber()<<"_"<<currentTime_<<".png";
	  TCPlot->SaveAs(ss.str().c_str());
	  // for (uint32_t i=0;i<60;i++)
	  // 	   {
	  // 	     DEBUG_PRINT("%d %d %d %d %d \n",i+1,nh0[i+1],nh1[i+1],nh2[i+1],smip[i+1]);
	  // 	   }
	  char c;c=getchar();putchar(c); if (c=='.') exit(0);;
	}
    }
}

void ShowerAnalyzer::ShowerBuilder(std::vector<RecoHit*> vreco)
{
  // BUild Plan Hit list
  std::map<uint32_t,std::vector<RecoHit> > Plans;

  for (std::vector<RecoHit*>::iterator ihp=vreco.begin();ihp!=vreco.end();ihp++)
    {
      uint32_t ch=(*ihp)->chamber();
      std::map<uint32_t,std::vector<RecoHit> >::iterator ipl=Plans.find(ch);
      if (ipl!=Plans.end())
	ipl->second.push_back((*(*ihp)));
      else
	{
	  std::vector<RecoHit> v;
	  v.push_back((*(*ihp)));
	  std::pair<uint32_t, std::vector<RecoHit> >  p(ch,v);
	  Plans.insert(p);
	}
    }

  std::vector<Shower> showerList;
  // Now Loop on plans

  uint32_t ninp=0;
  for (uint32_t ipl=1;ipl<61;ipl++)
    {

      std::map<uint32_t,std::vector<RecoHit> >::iterator lp=Plans.find(ipl);
      if (lp==Plans.end()) continue;
      // Loop on plan hits
      for (std::vector<RecoHit>::iterator ih=lp->second.begin();ih!=lp->second.end();ih++)
	{
	  ninp++;
	  // attach the Hit to existing shower or create a new one
	  bool found=false;
	  for (std::vector<Shower>::iterator ish=showerList.begin();ish!=showerList.end();ish++)
	    if (ish->append((*ih),15.)) {found=true;break;}
	  if (found) continue;
	  Shower sh((*ih));
	  showerList.push_back(sh);
	}
    }

  uint32_t n_as=0;


  // for (std::vector<Shower>::iterator ish=showerList.begin();ish!=showerList.end();ish++)
  //   {
	
	

  //     for (std::map<uint32_t,std::vector<RecoHit> >::iterator ipl=ish->getPlans().begin();ipl!=ish->getPlans().end();ipl++)
  // 	 {
	
	
  // 	   for (std::vector<RecoHit>::iterator iht=ipl->second.begin();iht!=ipl->second.end();iht++)
	
  // 	     n_as++;
  // 	   }
  //   }
  //if (draw_) this->drawDisplay();
  //DEBUG_PRINT("Number of showers %d and hits %d  %d %d\n",showerList.size(),vreco.size(),ninp,n_as);
  uint32_t nshower=0;
  uint32_t shcol=2;
  uint32_t nmips=0;

  TH1* hmip= rootHandler_->GetTH1("Mips");

  TH1* hnh0= rootHandler_->GetTH1("NumberOfHit0");
  TH1* hnh1= rootHandler_->GetTH1("NumberOfHit1");
  TH1* hnh2= rootHandler_->GetTH1("NumberOfHit2");
  TH1* hnht= rootHandler_->GetTH1("NumberOfHitTotal");
  TH1* hratio23= rootHandler_->GetTH1("ratio23");
  TH1* hl3= rootHandler_->GetTH1("l3");

  if (hmip==NULL)
    {
      hmip =rootHandler_->BookTH1( "Mips",1000,0.,10000.);
      hnh0 =rootHandler_->BookTH1( "NumberOfHit0",2000,0.,2000.);
      hnh1 =rootHandler_->BookTH1( "NumberOfHit1",4000,0.,4000.);
      hnh2 =rootHandler_->BookTH1( "NumberOfHit2",2000,0.,2000.);
      hnht =rootHandler_->BookTH1( "NumberOfHitTotal",6000,0.,6000.);
      hratio23 =rootHandler_->BookTH1( "ratio23",1000,0.,1.);
      hl3 =rootHandler_->BookTH1( "l3",1000,0.,1000.);

    }

  if (TCShower==NULL && draw_)
    {
      TCShower=new TCanvas("TCShower","test1",800,800);
      TCShower->Divide(2,2);
      TCShower->Modified();
      TCShower->Draw();

    }
  for (std::vector<Shower>::iterator ish=showerList.begin();ish!=showerList.end();ish++)
    {
      uint32_t nh0=ish->getNumberOfHits(0);
      uint32_t nh1=ish->getNumberOfHits(1);
      uint32_t nh2=ish->getNumberOfHits(2);
      uint32_t nht=nh0+nh1+nh2;
      uint32_t nmip=nh0+nh1+nh2*2;


      //printf("Shower %d %d %d %d %d \n",nh0,nh1,nh2,nht,nmip);
      ish->setSelected(false);
      if (nht<15) continue;
      
      ish->PlayMatrix();
      double ratio=sqrt((ish->getl1()+ish->getl2())/ish->getl3());
		
      //printf("\t Shower %d %d %f\n",ish->getLastPlan(),ish->getFirstPlan(),ratio);		
      //if (sqrt(ish->getl3())<7) continue;
      if ((ish->getLastPlan()-ish->getFirstPlan())<4) continue;

      if (ratio<0.01) continue;
		
      ish->setSelected(true);
    }


  // Unassociated bad shower hits
  for (std::vector<Shower>::iterator ish=showerList.begin();ish!=showerList.end();)
    {
      if (ish->isSelected()) {ish++;continue;}
      double dist=99999.;
      std::vector<Shower>::iterator jfound=showerList.end();
      for (std::map<uint32_t,std::vector<RecoHit> >::iterator ipl=ish->getPlans().begin();ipl!=ish->getPlans().end();ipl++)
	{
			
	  for (std::vector<RecoHit>::iterator iht=ipl->second.begin();iht!=ipl->second.end();iht++)
	    {
	      // Loop on selected shower and found nearest one
				
	      for (std::vector<Shower>::iterator jsh=showerList.begin();jsh!=showerList.end();jsh++)
		{
		  if (!jsh->isSelected()) continue; 
		  double dh=jsh->Distance((*iht));
		  if (dh<dist)
		    {
		      dist=dh;
		      jfound=jsh;
		    }
		}
	      // if (dist<15.)
	      // 	jfound->Add((*iht));
	    }

	}
      if (jfound!=showerList.end() && 
	  (dist<20 || (ish->getNumberOfHits(0)<20 && dist<40)) 
	  )
	{
	  //DEBUG_PRINT("Dist %f  && %d \n",dist,ish->getNumberOfHits(0));
	  for (std::map<uint32_t,std::vector<RecoHit> >::iterator ipl=ish->getPlans().begin();ipl!=ish->getPlans().end();ipl++)
	    {
				
	      for (std::vector<RecoHit>::iterator iht=ipl->second.begin();iht!=ipl->second.end();iht++)
		{
		  // Loop on selected shower and found nearest one
					
		  jfound->Add((*iht));
		}
				
	    }

	  // Remove the shower
	  ish=showerList.erase(ish);
	  continue;

	}
      // if (jfound!=showerList.end() && dist>=15. && ish->getNumberOfHits(0)>=20)
      // 	{
      // 	  DEBUG_PRINT("Dist %f is too large \n",dist);
      // 	}
      ++ish;
    }


  theEvent_.allshowers=showerList.size();
  theEvent_.showers=0;
  theShower_.eventid=theEvent_.run;
  theShower_.gtc=theEvent_.gtc;
  theShower_.idx=0;
  for (std::vector<Shower>::iterator ish=showerList.begin();ish!=showerList.end();ish++)
    {
      if (!ish->isSelected()) continue;
      theEvent_.showers=theEvent_.showers+1;
      ish->setSelected(ish->EdgeDetection());
      std::cout<<ish->getEdge3()<<" "<<ish->getAll3()<<std::endl;
      if (ish->getEdge3()*100/ish->getAll3()>95) ish->setSelected(false);
    }
  //if (theEvent_.showers>1) return;

  uint32_t indx=0;
  for (std::vector<Shower>::iterator ish=showerList.begin();ish!=showerList.end();ish++)
    {

      if (!ish->isSelected()) continue;
      printf("Shower %d %d %d \n",ish->getNumberOfHits(0),ish->getNumberOfHits(1),ish->getNumberOfHits(2));
      //printf("\t --> is selected \n");
      memset(&theShower_,0,sizeof(shower_t));
      theShower_.eventid=theEvent_.run;
      theShower_.gtc=theEvent_.gtc;
      indx++;
      theShower_.idx=(theEvent_.showers<<16)||indx;
      double* xm=ish->getxm();
      double* v1=ish->getv1();
      double* v2=ish->getv2();
      double* v3=ish->getv3();
      //theShower_.idx++;
      theShower_.xm[0]=xm[0];
      theShower_.xm[1]=xm[1];
      theShower_.xm[2]=xm[2];

      theShower_.lambda[0]=ish->getl1();
      theShower_.lambda[1]=ish->getl2();
      theShower_.lambda[2]=ish->getl3();

      theShower_.v1[0]=v1[0];
      theShower_.v1[1]=v1[1];
      theShower_.v1[2]=v1[2];
      theShower_.v2[0]=v2[0];
      theShower_.v2[1]=v2[1];
      theShower_.v2[2]=v2[2];
      theShower_.v3[0]=v3[0];
      theShower_.v3[1]=v3[1];
      theShower_.v3[2]=v3[2];
		
      theShower_.nhit[0]=ish->getNumberOfHits(0);
      theShower_.nhit[1]=ish->getNumberOfHits(1);
      theShower_.nhit[2]=ish->getNumberOfHits(2);

      theShower_.firstplan=ish->getFirstPlan();
      theShower_.lastplan = ish->getLastPlan();


      uint8_t np1=0,fp1=99,lp1=0;
      theShower_.ib1=0;
      theShower_.maxlb1=0;
      theShower_.n9=0;
      theShower_.n25=0;
      if (theShower_.nhit[0]+theShower_.nhit[1]+theShower_.nhit[2]> 3  )
	{
			
	  for (uint32_t ipl=ish->getFirstPlan();ipl<=ish->getLastPlan();ipl++)
	    {
	      uint32_t nh;double xb;double yb;double l0;double l1;double v0[2];double v1[2];double n9=0,n25=0;
	      ish->transverseProfile(ipl,nh,xb,yb,l0,l1,v0,v1,n9,n25);
	      theShower_.n9+=n9;
	      theShower_.n25+=n25;
	      if (sqrt(l0+l1)>theShower_.maxlb1) theShower_.maxlb1=sqrt(l0+l1);
	      if (sqrt(l0+l1)>1.5 || nh>=5) 
		{
		  //DEBUG_PRINT("plan %d = %d hit %f %f Pos %f %f size ==>  %f\n",ipl,nh,xb,yb,sqrt(l0),sqrt(l1),sqrt(l0+l1));
		  np1++;
		  theShower_.ib1 |=(1<<ipl);
		  if (ipl<fp1) {
		    fp1=ipl;
		    theShower_.xb1=xb;
		    theShower_.yb1=yb;
		  }
		  if (ipl>lp1) lp1=ipl;
		}
	    }

	  //getchar();
	}   

      if (np1<0) continue;
      theShower_.rncor[0]=0;
      theShower_.rncor[1]=0;
      theShower_.rncor[2]=0;
      theShower_.rbs=0;
      theShower_.rbt=0;
      double n0_5=0;
      double n15_46=0;
      double n41_46=0;
      double n0_29=0;
      for (uint32_t i=0;i<60;i++)
	{
	  theShower_.plan0[i]=ish->getNumberOfHits(i+1,0);
	  theShower_.plan1[i]=ish->getNumberOfHits(i+1,1);
	  theShower_.plan2[i]=ish->getNumberOfHits(i+1,2);
	  if (i>=0 && i<=5) n0_5+=theShower_.plan0[i]+theShower_.plan1[i]+theShower_.plan2[i];
	  if (i>=15 && i<=46) n15_46+=theShower_.plan0[i]+theShower_.plan1[i]+theShower_.plan2[i];
	  if (i>=41 && i<=46) n41_46+=theShower_.plan0[i]+theShower_.plan1[i]+theShower_.plan2[i];
	  if (i>=0 && i<=29) n0_29+=theShower_.plan0[i]+theShower_.plan1[i]+theShower_.plan2[i];
	  theShower_.corr0[i]=ish->getCorrectedNumberOfHits(i+1,0,theCorreff_);
	  theShower_.corr1[i]=ish->getCorrectedNumberOfHits(i+1,1,theCorreff_);
	  theShower_.corr2[i]=ish->getCorrectedNumberOfHits(i+1,2,theCorreff_);
	  if ((i+1)<fp1) continue;
	  if ((i+1)>lp1) continue;
	  theShower_.rncor[0]+=theShower_.corr0[i];
	  theShower_.rncor[1]+=theShower_.corr1[i];
	  theShower_.rncor[2]+=theShower_.corr2[i];

	}

      theShower_.rbs=n15_46/(n0_5+1E-5);
      theShower_.rbt=n41_46/(n0_29+1E-5);
		


		





		
      //      ish->PlayMatrix(ish->getFirstPlan(),ish->getLastPlan());
      ish->PlayMatrix(fp1,lp1);
      ish->setSelected(false);
      uint32_t nh0=ish->getNumberOfHits(0);
      uint32_t nh1=ish->getNumberOfHits(1);
      uint32_t nh2=ish->getNumberOfHits(2);
      uint32_t nht=nh0+nh1+nh2;
      uint32_t nmip=nh0+nh1*3+nh2*15;


      double ratio=sqrt((ish->getl1()+ish->getl2())/ish->getl3());
      hratio23->Fill(ratio);
      hl3->Fill(ish->getl3());
      if (ratio<0.12) 
	TracksBuilder((*ish),vreco);


      //      else
      ///	{
      //	  this->PointsBuilder(vreco);
      //	  HT3D();
      //	}
      //if (ish->getl3()<20) continue;
      if (ratio<0.01) continue;
      //std::cout<<ratio<<" "<<ish->getl2()<<" "<<ish->getl3()<<" "<<ish->getl2()/ish->getl3()<<" "<<sqrt(ish->getl2()/ish->getl3())<<std::endl;
      xm=ish->getxm();
      v1=ish->getv1();
      v2=ish->getv2();
      v3=ish->getv3();
      theShower_.rxm[0]=xm[0];
      theShower_.rxm[1]=xm[1];
      theShower_.rxm[2]=xm[2];

      theShower_.rlambda[0]=ish->getl1();
      theShower_.rlambda[1]=ish->getl2();
      theShower_.rlambda[2]=ish->getl3();

      theShower_.rv1[0]=v1[0];
      theShower_.rv1[1]=v1[1];
      theShower_.rv1[2]=v1[2];
      theShower_.rv2[0]=v2[0];
      theShower_.rv2[1]=v2[1];
      theShower_.rv2[2]=v2[2];
      theShower_.rv3[0]=v3[0];
      theShower_.rv3[1]=v3[1];
      theShower_.rv3[2]=v3[2];
		
      theShower_.rnhit[0]=ish->getReduceNumberOfHits(0,fp1,lp1);
      theShower_.rnhit[1]=ish->getReduceNumberOfHits(1,fp1,lp1);
      theShower_.rnhit[2]=ish->getReduceNumberOfHits(2,fp1,lp1);
      // DEBUG_PRINT("0 \n");
      uint32_t v[8];
      double rnmean=0.;

      double lsiz[8];
      lsiz[0]=10.;
      lsiz[1]=20.;
      lsiz[2]=30.;
      lsiz[3]=40.;
      lsiz[4]=60.;
      lsiz[5]=80.;
      lsiz[6]=120.;
      lsiz[7]=160.;
      memset(theShower_.NH,0,8*sizeof(uint16_t));
      rnmean=0.;
      //DEBUG_PRINT("1 \n");
      ish->getFDHitsN(v,0);
      for (uint32_t is=0;is<8;is++)
	{
	  theShower_.NH0[is]=v[is];
	  theShower_.NH[is]+=v[is];
	  if (is==0 || v[0]==0) continue;
	  rnmean +=log(v[0]*1./v[is])/log(lsiz[is]);
	}
      theShower_.fd[0]=rnmean/7.;
      //DEBUG_PRINT("2 \n");
      rnmean=0.;
      ish->getFDHitsN(v,1);
		
      for (uint32_t is=0;is<8;is++)
	{
	  theShower_.NH1[is]=v[is];
	  theShower_.NH[is]+=v[is];
	  if (is==0 || v[0]==0) continue;
	  rnmean +=log(v[0]*1./v[is])/log(lsiz[is]);
	}
      theShower_.fd[1]=rnmean/7.;
      //DEBUG_PRINT("3 \n");
      rnmean=0.;
      ish->getFDHitsN(v,2);
      for (uint32_t is=0;is<8;is++)
	{
	  theShower_.NH2[is]=v[is];
	  theShower_.NH[is]+=v[is];
	  if (is==0 || v[0]==0 ) continue;
	  rnmean +=log(v[0]*1./v[is])/log(lsiz[is]);
	}
      theShower_.fd[2]=rnmean/7.;


      rnmean=0.;
      for (uint32_t is=0;is<8;is++)
	{
	  if (is==0 || theShower_.NH[0]==0 ) continue;
	  rnmean +=log(theShower_.NH[0]*1./theShower_.NH[is])/log(lsiz[is]);
	}
      theShower_.fd[3]=rnmean/7.;


      //DEBUG_PRINT("4 \n");
      // v.clear();
      // double rn10=1.;
      // double rn20=log(v[0]*1./v[1])/log(20);
      // double rn30=log(v[0]*1./v[2])/log(30);
      // double rn40=log(v[0]*1./v[3])/log(40);
      // double rn60=log(v[0]*1./v[4])/log(60);
      // double rn80=log(v[0]*1./v[5])/log(80);
      // double rn120=log(v[0]*1./v[6])/log(120);
      // double rn150=log(v[0]*1./v[7])/log(150);
      // double rnmean=(rn20+rn30+rn40+rn60+rn80+rn120+rn150)/7.;
      // theShower_.fd=rnmean;
      // for (uint32_t is=0;is<8;is++)
      // 	theShower_.NH[is]=v[is];
      hmip->Fill(nmip*1.);
      hnh0->Fill(nh0*1.);
      hnh1->Fill(nh1*1.);
      hnh2->Fill(nh2*1.);
      hnht->Fill(nht*1.);
      ish->setSelected(true);



      theShower_.rncor[0]=theNall_;
      theShower_.rncor[1]=(theAbsoluteTime_-theBCIDSpill_)*2E-7;
      theShower_.rncor[2]=theTag_;

      theShower_.np1=np1;
      theShower_.fp1=fp1;
      theShower_.lp1=lp1;


      theShower_.ne[0]=ish->getEdge(0);
      theShower_.ne[1]=ish->getEdge(1);
      theShower_.ne[2]=ish->getEdge(2);
      theShower_.nc[0]=ish->getCore(0);
      theShower_.nc[1]=ish->getCore(1);
      theShower_.nc[2]=ish->getCore(2);
      theShower_.rncor[0]=theShower_.ne[0]+theShower_.ne[1]+theShower_.ne[2]+theShower_.nc[0]+theShower_.nc[1]+theShower_.nc[2];
      theShower_.ngood=ish->getNGood();
      theShower_.namas=ish->getAmas().size();
      theShower_.nhitafterlast=ish->getNAfter();
      theShower_.zfirst=ish->getZFirstAmas();
      theShower_.zlast=ish->getZLastAmas();


      if (tEvents_!=NULL)
	{
	  if (useSqlite_ || useMysql_)
	    fillShowerTable(theShower_);
	  else
	    {
	      treeFile_->cd();
	      theShower_.bcid= theBCID_;
	      theShower_.time= currentTime_;

	      tShowers_->Fill();
	    }
	  if ( np1>10 && sqrt((theShower_.lambda[0]+theShower_.lambda[1])/theShower_.lambda[2])>0.1) npi_++;
			
#undef PRINT_SHOWER	  
#ifdef PRINT_SHOWER
	  DEBUG_PRINT(" Event %d %d %d %d \n",theEvent_.idx,theEvent_.gtc,theEvent_.bcid,theEvent_.time);
	  DEBUG_PRINT("Number of plan %d First Plan %d %d ------------> debut %f fin %f th %f \n",np1,fp1,lp1,fp1*2.8,lp1*2.8,theShower_.rxm[2]+3*sqrt(theShower_.rlambda[2]));
	  DEBUG_PRINT("  %d =========================>  %f %f  %5.2f %d %d %d %f %f %f  \n",theShower_.NH[0],ish->getl3(),ish->getl2(),ratio,ish->getFirstPlan(),ish->getLastPlan(),ish->getLastPlan()-ish->getFirstPlan(),theShower_.fd[0],theShower_.fd[1],theShower_.fd[2]);
#endif
			
	  //DEBUG_PRINT(" %f %f %f %f %f => %f \n",rn20,rn30,rn60,rn120,rn150,rnmean);
	  // for (std::vector<uint32_t>::iterator it=v.begin();it!=v.end();it++)
	  //   DEBUG_PRINT("====>>>>>>>>>>>>>>>>> %d\n",(*it)); 

	}

		

		
      nshower++;

      nmips+=nmip;
      //printf("Hi all \n");
      draw_=false;
      if (TCShower!=NULL && draw_ )
	{
	  std::stringstream sp("");
	  sp<<"Hit3"<<shcol;
	  TH3* hit3= rootHandler_->GetTH3(sp.str());
	  if (hit3==NULL)
	    {
	      //hit3= rootHandler_->BookTH3(sp.str(),52,-2.8,145.6,100,0.,100.,100,0.,100.);
	      hit3= rootHandler_->BookTH3(sp.str(),80,-2.8,345.6,100,0.,100.,100,0.,100.);
	    }
	  else
	    {
	      hit3->Reset();
	    }
	  for (std::map<uint32_t,std::vector<RecoHit> >::iterator ipl=ish->getPlans().begin();ipl!=ish->getPlans().end();ipl++)
	    {
	      //DEBUG_PRINT("Plan %d nb %d\n",ipl->first,ipl->second.size());

	      for (std::vector<RecoHit>::iterator iht=ipl->second.begin();iht!=ipl->second.end();iht++)
		hit3->Fill(iht->Z()*1.,iht->X(),iht->Y());
	    }
	  TCShower->cd();
	  // TLine* l1 = new TLine(tk.zmin_,tk.ay_*tk.zmin_+tk.by_,tk.zmax_,tk.ay_*tk.zmax_+tk.by_);
	  // l1->SetLineColor(1);
	  // l1->SetLineWidth(2);
	  // l1->Draw("SAME");
	  hit3->SetMarkerStyle(25);
	  hit3->SetMarkerSize(.2);
	  hit3->SetMarkerColor(shcol);
	  TCShower->cd(1);
	  if (shcol==2)
	    {
	      hit3->Draw("P");
	    }
	  else
	    hit3->Draw("PSAME");
			
	  TPolyLine3D *pl3d1 = new TPolyLine3D(2);
	  double* v=ish->getv1();    
	  double* x=ish->getxm();    
	  pl3d1->SetPoint(0,x[2],x[0],x[1]);
	  pl3d1->SetPoint(1,x[2]+v[2],x[0]+v[0],x[1]+v[1]);
			
	  pl3d1->SetLineWidth(3);
	  pl3d1->SetLineColor(1);
	  pl3d1->Draw("SAME");
	  TPolyLine3D *pl3d2 = new TPolyLine3D(2);
	  v=ish->getv2();    
	  pl3d2->SetPoint(0,x[2],x[0],x[1]);
	  pl3d2->SetPoint(1,x[2]+v[2],x[0]+v[0],x[1]+v[1]);
			
	  pl3d2->SetLineWidth(3);
	  pl3d2->SetLineColor(1);
	  pl3d2->Draw("SAME");
	  TPolyLine3D *pl3d3 = new TPolyLine3D(2);
	  v=ish->getv3();    
	  pl3d3->SetPoint(0,x[2],x[0],x[1]);
	  pl3d3->SetPoint(1,x[2]+v[2],x[0]+v[0],x[1]+v[1]);



			
	  pl3d3->SetLineWidth(3);
	  pl3d3->SetLineColor(1);
	  pl3d3->Draw("SAME");

			

	  TCShower->cd(2);
	  TProfile2D* hpx1=hit3->Project3DProfile("yx");
	  hpx1->SetLineColor(kGreen);
	  hpx1->Draw("box");
	  TCShower->cd(3);
	  TProfile2D* hpx2=hit3->Project3DProfile("zx");
	  hpx2->SetLineColor(kGreen);
	  hpx2->Draw("box");

	  shcol++;
			
	  TCShower->Modified();
	  TCShower->Update();
	}

    }
  //  DEBUG_PRINT("Number of good showers %d ========================> %d \n",nshower,nmips);
      //printf("Hi all 1\n");
  if (TCShower!=NULL && (nshower>0))
    {
      std::stringstream sp("");
      sp<<"Hit300";
      TH3* hit3= rootHandler_->GetTH3(sp.str());
      if (hit3==NULL)
	{
	  hit3= rootHandler_->BookTH3(sp.str(),82,-2.8,345.6,100,0.,100.,100,0.,100.);
	}
      else
	{
	  hit3->Reset();
	}
      uint32_t n_unas=0;
      uint32_t n_as=0;

      for (std::vector<Shower>::iterator ish=showerList.begin();ish!=showerList.end();ish++)
	{
	  if (ish->isSelected()) 
	    {
	      for (std::map<uint32_t,std::vector<RecoHit> >::iterator ipl=ish->getPlans().begin();ipl!=ish->getPlans().end();ipl++)
		{
					
		  n_as+=ipl->second.size();
		}

	      continue;
	    }

	  for (std::map<uint32_t,std::vector<RecoHit> >::iterator ipl=ish->getPlans().begin();ipl!=ish->getPlans().end();ipl++)
	    {
				

	      for (std::vector<RecoHit>::iterator iht=ipl->second.begin();iht!=ipl->second.end();iht++)
		{hit3->Fill(iht->Z()*1.,iht->X(),iht->Y());
		  n_unas++;
		}
	    }
	}
      if (n_unas>0)
	{
	  hit3->SetMarkerStyle(25);
	  hit3->SetMarkerSize(.2);
	  hit3->SetMarkerColor(23);
	  hit3->Draw("PSAME");
	  TCShower->Modified();
	  TCShower->Update();
	  DEBUG_PRINT("In Shower %d Not in shower %d \n",n_as,n_unas);

	}
    }

  // for (std::vector<Shower>::iterator ish=showerList.begin();ish!=showerList.end();ish++)
  //   {
  //     if (!ish->isSelected()) continue;
  //     DEBUG_PRINT("Shower axis %f\n",ish->getl3());
  //     for (std::vector<Shower>::iterator jsh=showerList.begin();jsh!=showerList.end();jsh++)
  //       {
  // 	if (!jsh->isSelected()) continue;
  // 	if (jsh==ish) continue;
  // 	DEBUG_PRINT(" ----> %f  vs %f\n",ish->closestDistance((*jsh)),3*sqrt(ish->getl2()));
  //       }

  //   }
  //printf("Hi all 3\n");
  if (nshower>0 && TCShower!=NULL && draw_)
    {
      std::stringstream ss("");
      ss<<"/tmp/Display_"<<evt_->getRunNumber()<<"_"<<evt_->getEventNumber()<<"_"<<currentTime_<<".png";
      TCShower->SaveAs(ss.str().c_str());
      char c;c=getchar();putchar(c); if (c=='.') exit(0);;
    }
      //printf("Hi all 4\n");
  if (tEvents_!=NULL)
    {
      treeFile_->cd();

		
      tEvents_->Fill();
    }
  //printf("Hi all 5 \n");
}

void ShowerAnalyzer::createTrees(std::string s)
{

  treeFile_ = new TFile(s.c_str(),"recreate");
  treeFile_->cd();

  tEvents_ = new TTree("events","Events");

  theEvent_.idx=0;

  tEvents_->Branch("bcid",&theEvent_.bcid,"bcid/l");
  tEvents_->Branch("idx",&theEvent_.idx,"idx/I");

  tEvents_->Branch("energy",&theEvent_.energy,"energy/D");
  tEvents_->Branch("run",&theEvent_.run,"run/i");
  tEvents_->Branch("event",&theEvent_.event,"event/i ");
  tEvents_->Branch("gtc",&theEvent_.gtc,"gtc/i");
  tEvents_->Branch("dtc",&theEvent_.dtc,"dtc/i");
  tEvents_->Branch("time",&theEvent_.time,"time/i ");
  tEvents_->Branch("npoint",&theEvent_.npoint,"npoint/i ");
  tEvents_->Branch("allpoints",&theEvent_.allpoints,"allpoints/i");
  tEvents_->Branch("ntrack",&theEvent_.ntrack,"ntrack/s");
  tEvents_->Branch("allshowers",&theEvent_.allshowers,"allshowers/s");
  tEvents_->Branch("showers",&theEvent_.showers,"showers/s");
  tEvents_->Branch("type",&theEvent_.type,"type/b");




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
  tShowers_->Branch("namas",&theShower_.namas,"namas/i");
  tShowers_->Branch("ngood",&theShower_.ngood,"ngood/i");
  tShowers_->Branch("nhitafter",&theShower_.nhitafterlast,"nhitafter/i");
  tShowers_->Branch("zfirst",&theShower_.zfirst,"zfirst/D");
  tShowers_->Branch("zlast",&theShower_.zlast,"zlast/D");







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



  std::cout << " create Trees"<<std::endl;



}
void ShowerAnalyzer::closeTrees()
{
  treeFile_->cd();
  tEvents_->BuildIndex("idx");
  tShowers_->BuildIndex("idx","eventid");
  tTracks_->BuildIndex("idx","eventid");
  tEvents_->Write();
  tShowers_->Write();
  tTracks_->Write();
  treeFile_->ls();
  treeFile_->Close();
  theNtupleFile_->cd();
  theNtuple_->Write();
  theNtupleFile_->Close();

}
void ShowerAnalyzer::TracksBuilder()
{
  allpoints_.clear();
  // Build a vector of RecoHit
  std::vector<RecoHit*> vrh;
  for (uint32_t k=0;k<60;k++)
    {
      for (uint32_t i=0;i<96;i++)
	for (uint32_t j=0;j<96;j++)
	  if (theImage_.getValue(k,i,j)!=0 )
	    {
	      //if(image3_.getValue(k,i/3,j/3)<=-17 && image3_.getValue(k,i/3,j/3)>-25) 
	      vrh.push_back(&hitVolume_[k][i][j]);
	    }
    }
  ShowerParams ish;
  Shower::computePrincipalComponents(vrh,(double*) &ish);
  tkgood_.clear();
  this->PointsBuilder(vrh);
  // Use the shower to find initial track 
  // Direction u3/sqrt(l3), point xm
  double* v=ish.l2;
  double p[3];
  for (uint32_t i=0;i<3;i++) p[i]=v[i]/ish.lambda[2];
  double* x=ish.xm;    
  RecoCandTk t;
  t.ax_ =p[0]/p[2];
  t.ay_ =p[1]/p[2];
  t.bx_=x[0]-t.ax_*x[2];
  t.by_=x[1]-t.ay_*x[2];
  for (std::vector<RecoPoint>::iterator icl=allpoints_.begin();icl!=allpoints_.end();icl++)
    {
      if (t.calculateDistance((*icl))<tkDistCut_)
	{
	  t.addNearestPoint((*icl));
	  (*icl).setUsed(true);
	  if (t.getList().size()>=4) t.regression();
	}
    }
  if (t.getList().size()>=5) tkgood_.push_back(t);	
}

void ShowerAnalyzer::TracksBuilder(Shower& ish,std::vector<RecoHit*> &vrh)
{
  // Build all points
  if (allpoints_.size()==0)
    {
      tkgood_.clear();
      this->PointsBuilder(vrh);
    }
  // Use the shower to find initial track 
  // Direction u3/sqrt(l3), point xm
  double* v=ish.getv3();
  double p[3];
  for (uint32_t i=0;i<3;i++) p[i]=v[i]/ish.getl3();
  double* x=ish.getxm();    
  RecoCandTk t;
  t.ax_ =p[0]/p[2];
  t.ay_ =p[1]/p[2];
  t.bx_=x[0]-t.ax_*x[2];
  t.by_=x[1]-t.ay_*x[2];
  for (std::vector<RecoPoint>::iterator icl=allpoints_.begin();icl!=allpoints_.end();icl++)
    {
      if (t.calculateDistance((*icl))<tkDistCut_)
	{
	  t.addNearestPoint((*icl));
	  (*icl).setUsed(true);
	  if (t.getList().size()>=4) t.regression();
	}
    }
  if (t.getList().size()>=5) tkgood_.push_back(t);
}
void ShowerAnalyzer::FillTrackTree()
{

  if (tEvents_!=NULL)
    {
      treeFile_->cd();
      for (std::vector<RecoCandTk>::iterator it=tkgood_.begin();it!=tkgood_.end();it++)
	{
	  theTrack_.idx++;
	  theTrack_.eventid=theEvent_.idx;
	  theTrack_.ax=it->ax_;
	  theTrack_.ay=it->ay_;
	  theTrack_.bx=it->bx_;
	  theTrack_.by=it->by_;
	  theTrack_.chi2=it->chi2_;
	  theTrack_.npoint=it->getList().size();
	  memset(theTrack_.nhit0,0,61*sizeof(uint8_t));
	  memset(theTrack_.nhit1,0,61*sizeof(uint8_t));
	  memset(theTrack_.nhit2,0,61*sizeof(uint8_t));
	  for (std::vector<RecoPoint*>::iterator ip=it->getList().begin();ip!=it->getList().end();ip++)
	    {
	      uint32_t chid= (*ip)->getChamberId();
	      theTrack_.xhit[chid] =(*ip)->X();
	      theTrack_.yhit[chid] =(*ip)->Y();
	      theTrack_.nhit0[chid]=0;
	      theTrack_.nhit1[chid]=0;
	      theTrack_.nhit2[chid]=0;
	      for (std::vector<RecoHit>::iterator iht=(*ip)->getCluster().getHits()->begin();iht!=(*ip)->getCluster().getHits()->end();iht++)
		{
		  int ithr= (*iht).getAmplitude()&0x3;
		  if (ithr==1) {theTrack_.nhit0[chid]++;}
		  if (ithr==2) {theTrack_.nhit1[chid]++;}
		  if (ithr==3) {theTrack_.nhit2[chid]++;}
		}
	      //DEBUG_PRINT("%d %d %d %f %f \n",theTrack_.id,chid,theTrack_.nhit[chid],theTrack_.xhit[chid],theTrack_.yhit[chid]);
	    }
	}

      tTracks_->Fill();
      //      getchar();
    }


}

void ShowerAnalyzer::PointsBuilder(std::vector<RecoHit*> &vrh)
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
void ShowerAnalyzer::readCalibration(uint32_t run) throw (std::string)
{
  std::stringstream name("");
  name<<"~/Analysis_21Mai2012/efficiency_"<<run<<".fit";
  std::ifstream inFile;
  inFile.open(name.str().c_str());
  if (inFile.fail()) {
    std::cerr << "unable to open file input.dat for reading" << std::endl;
    std::string s="unable to open file input.dat for reading: "+name.str();
    throw s;
  }
  uint32_t ich,ipad,jpad;
  double nev,cor,mul;
  while (inFile>>ich>>ipad>>jpad>>nev>>cor>>mul)
    {
      std::map<uint32_t,double*>::iterator itch=theCorreff_.find(ich);
      if (itch!=theCorreff_.end())
	{
	  double* c= (itch->second);
	  c[(ipad-1)*8+(jpad-1)]=cor;
	}
      //DEBUG_PRINT("%d %f %f \n",ival,m,r);
    }
  inFile.close();
  return;

}
void ShowerAnalyzer::ImageBuilder(std::vector<RecoHit*> &vrh)
{
  unsigned char imagex[60][96];
  unsigned char imagey[60][96];
  unsigned char imagev[60][96][96];
  float image2x[60][96];
  float image2y[60][96];
  float image3[60][96][96];
  memset(imagex,0,60*96);
  memset(imagey,0,60*96);
  memset(imagev,0,60*96*96);
  memset(image2x,0,60*96*sizeof(float));
  memset(image2y,0,60*96*sizeof(float));
  memset(image3,0,60*96*96*sizeof(float));
  std::map<uint32_t,RecoHit*> mapij;
  uint32_t na1=0,na2=0,na0=0;
  uint32_t ne1=0,ne2=0,ne0=0;
  if (TCEdge==NULL && draw_)
    {
      TCEdge=new TCanvas("TCEdge","test2",800,800);
      TCEdge->Divide(2,2);
      TCEdge->Modified();
      TCEdge->Draw();

    }
  for (std::vector<RecoHit*>::iterator ih=vrh.begin();ih!=vrh.end();ih++)
    {
      bool append= false;
      //std::cout<<"Clusters "<<vCluster.size()<<std::endl;
      RecoHit* hit=(*ih);
      unsigned char w=1;
      int ithr= hit->getAmplitude()&0x3;
      if (ithr==1) {w=5;na1++;}
      if (ithr==2) {w=1;na0++;}
      if (ithr==3) {w=15;na2++;}
      if (hit->I()>96) continue;
      if (hit->J()>96) continue;
      if (hit->chamber()>60) continue;
      if (hit->I()<1) continue;
      if (hit->J()<1) continue;
      if (hit->chamber()<1) continue;

      imagex[hit->chamber()-1][hit->I()-1]=1;
      imagey[hit->chamber()-1][hit->J()-1]=1;
      imagev[hit->chamber()-1][hit->I()-1][hit->J()-1]=1;

      uint32_t key=(hit->I()<<24)|(hit->J()<<16)|(hit->chamber());
      std::map<uint32_t,RecoHit*>::iterator it=mapij.find(key);
      if (it==mapij.end())
	{
	  std::pair<uint32_t,RecoHit*> p(key,hit);
	  mapij.insert(p);
	}

    }
  // sobel_filtering( imagex, image2x);
  //sobel_filtering( imagey, image2y);
  sobel_volume(imagev,image3);
  std::vector<RecoHit*> cores;
  std::vector<RecoHit*> edges;

  theNall_=0;theNedge_=0;
  for (uint32_t k=1;k<59;k++)
    {
      for (uint32_t i=1;i<95;i++)
	{
	  //if (image2x[k][i]>=-1 ) continue;
			
	  for (uint32_t j=1;j<95;j++)
	    {
	      //if (image2y[k][j]>=-1 ) continue;
	      if (imagev[k][i][j]==1) theNall_++;
	      if (image3[k][i][j]>=-32) 
		{
		  if (image3[k][i][j]>=-2) continue;
		  uint32_t key=((i+1)<<24)|((j+1)<<16)|(k+1);
		  //std::cout<<key<<std::endl;
		  std::map<uint32_t,RecoHit*>::iterator it=mapij.find(key);
		  if (it!=mapij.end())
		    {
						
		      //if (image2x[k][i]<=-8 && image2y[k][j]<=-8) continue;
		      RecoHit* hit=((it->second));
		      cores.push_back(hit);
		    }

		  continue;
		}
	      theNedge_++;
	      //DEBUG_PRINT("%d %d %d %f \n",k,i,j,image3[k][i][j]);
	      //if (image3[k][i][j]<-21) continue;
				
	      uint32_t key=((i+1)<<24)|((j+1)<<16)|(k+1);
	      std::map<uint32_t,RecoHit*>::iterator it=mapij.find(key);
	      if (it!=mapij.end())
		{
					
		  //if (image2x[k][i]<=-8 && image2y[k][j]<=-8) continue;
					
		  edges.push_back(it->second);
		  //hitis1->SetBinContent(k+1,i+1,1.);
		  // hitjs1->SetBinContent(k+1,j+1,1.);
		}
				
				
	    }
	}
    }
  // Add adjacent hit to core
  std::vector<RecoHit*> adj;

  for (recit ie=edges.begin();ie!=edges.end();)
    {
      bool adjacent=false;
      for (recit ic=cores.begin();ic!=cores.end();ic++)
	{
	  if (abs((*ie)->chamber()-(*ic)->chamber())>1) continue;
	  if (abs((*ie)->I()-(*ic)->I())>1) continue;
	  if (abs((*ie)->J()-(*ic)->J())>1) continue;
	  adjacent=true;
	  break;
	}
      if (adjacent)
	{
	  adj.push_back((*ie));
	  edges.erase(ie);
	}
      else
	++ie;
    }
  for (recit ia=adj.begin();ia!=adj.end();ia++)
    {
      cores.push_back((*ia));
    }
  theNedge_=edges.size();
  theNall_=vrh.size();
  //Now loop on core hits
  //DEBUG_PRINT("%d %d %f -> %f\n",theNall_,theNedge_,theNedge_*100./theNall_,theNedge_*80.+(theNall_-theNedge_)*50.);
  //DEBUG_PRINT("Number of cores hits %d \n",cores.size());
  std::vector<Amas> vamas;
  for (recit it=cores.begin();it!=cores.end();it++)
    {
      bool appended=false;
      for (std::vector<Amas>::iterator ia=vamas.begin();ia!=vamas.end();ia++)
	{
	  appended=(ia->append(*it));
	  if (appended) break;
	}
      if (!appended)
	{
	  Amas a(*it);
	  vamas.push_back(a);
	}
    }
  //DEBUG_PRINT("Number of Amas %d \n",vamas.size());
  bool electron=false;
  bool leak=false;
  bool leakt=false;
  double emax =-DBL_MAX;
  double zmax =-DBL_MAX;
  double zlast =-DBL_MAX;
  uint32_t ng=0;
  for (std::vector<Amas>::iterator ia=vamas.begin();ia!=vamas.end();ia++)
    {
      ia->compute();
      if (ia->size()<=4) continue;
      ng++;
      //for (uint32_t i=0;i<21;i++)
      //	DEBUG_PRINT("%6.3f ",ia->getComponents(i));
      //DEBUG_PRINT(" Size %d\n",ia->size()); 
      if (ia->getComponents(2)>zmax) zmax=ia->getComponents(2);
      if (ia->getComponents(16)>zlast) zlast=ia->getComponents(16);
      if (ia->size()>emax)
	{
	  emax=ia->size();
	  if (ia->getComponents(15)<=2.81 && (ia->size()*1./theNall_)>0.2 ) 
	    {
	      electron=true;
	    }


	}
      if (ia->getComponents(16)>=zLastAmas_ && (ia->size())>4)
	{
	  leak=true;
	}
      if (ia->getComponents(17)<4 && (ia->size())>4)
	{
	  leakt=true;
	}
      if (ia->getComponents(18)>93 && (ia->size())>4)
	{
	  leakt=true;
	}

      if (ia->getComponents(19)<4 && (ia->size())>4)
	{
	  leakt=true;
	}
      if (ia->getComponents(20)>93 && (ia->size())>4)
	{
	  leakt=true;
	}
      if (useSqlite_ || useMysql_)
	{
	  Amas& a=(*ia);
	  fillAmasTable(&a);
	}
    }
  uint32_t nafter=0;
  std::bitset<61> planes(0);
  for (recit it=edges.begin();it!=edges.end();it++)
    {
      if ((*it)->Z()>zlast) {
	if ((*it)->chamber()<60)
	  planes.set((*it)->chamber(),true);
      }
      RecoHit* hit=(*it);
      int ithr= hit->getAmplitude()&0x3;
      if (ithr==1) {ne1++;}
      if (ithr==2) {ne0++;}
      if (ithr==3) {ne2++;}

    }
  for (uint32_t i=0;i<51;i++)
    if (planes[i]!=0) nafter++;
  electron =electron && (ng<=5) && (zlast<50.) && nafter<10;

  // theAllHit_=(na2<<20)|(na1<<10)|na0;
  // theEdgeHit_=(ne2<<20)|(ne1<<10)|ne0;


  /*
    if (leak) DEBUG_PRINT("==================================================> L E A K A G E <==============================\n");
    if (leakt) DEBUG_PRINT("==================================================> T R A N S V E R S E  L E A K A G E <==============================\n");
    if (electron) DEBUG_PRINT("==================================================> E L E C T R O N <============================== %d %f %d \n",ng,zmax,nafter);
  */
  theTag_=0;
  if (electron) theTag_=1;
  if (leak) theTag_+=2;
  if (leakt) theTag_+=4;
  //DEBUG_PRINT("NAFTER %d \n",nafter);     
  //DEBUG_PRINT("%d / %d -  %d / %d - %d / %d \n",ne0,na0,ne1,na1,ne2,na2);

  if (useSqlite_ || useMysql_)
    {
      uint32_t cor[3];
      uint32_t edg[3];
      cor[0]=na0-ne0;
      cor[1]=na1-ne1;
      cor[2]=na2-ne2;
      edg[0]=ne0;
      edg[1]=ne1;
      edg[2]=ne2;
      fillAmasSummaryTable(cor,edg,ng,nafter,theTag_,zlast);
    }
  if (TCEdge!=NULL && draw_ && theNall_>100)
    {
      TH2* hiti= rootHandler_->GetTH2("hiti");
      TH2* hitis= rootHandler_->GetTH2("hitis");
      TH2* hitis1= rootHandler_->GetTH2("hitis1");
      TH2* hitj= rootHandler_->GetTH2("hitj");
      TH2* hitjs= rootHandler_->GetTH2("hitjs");
      TH2* hitjs1= rootHandler_->GetTH2("hitjs1");
      TH1* hw3=rootHandler_->GetTH1("hw3");
      if (hiti==NULL)
	{
	  //hit3= rootHandler_->BookTH3(sp.str(),52,-2.8,145.6,100,0.,100.,100,0.,100.);
	  hiti= rootHandler_->BookTH2("hiti",60,0,60.,96,0.,96.);
	  hitis= rootHandler_->BookTH2("hitis",60,0,60.,96,0.,96.);
	  hitis1= rootHandler_->BookTH2("hitis1",60,0,60.,96,0.,96.);
	  hitj= rootHandler_->BookTH2("hitj",60,0,60.,96,0.,96.);
	  hitjs= rootHandler_->BookTH2("hitjs",60,0,60.,96,0.,96.);
	  hitjs1= rootHandler_->BookTH2("hitjs1",60,0,60.,96,0.,96.);
	  hw3=rootHandler_->BookTH1("hw3",50,-49.,0.);
	}
      else
	{
	  hiti->Reset();
	  hitis->Reset();
	  hitis1->Reset();
	  hitj->Reset();
	  hitjs->Reset();
	  hitjs1->Reset();
	}
      double nx=0,ny=0;
      /* a l'ancienne     

	 for (uint32_t k=1;k<59;k++)
	 {
	 for (uint32_t i=1;i<95;i++)
	 {
		
	 //DEBUG_PRINT("Plan %d %d %f\n",k,i,image2[k][i]);
	 hiti->SetBinContent(k+1,i+1,imagex[k][i]*1.);
	 if (image2x[k][i]<-1 && image2x[k][i]>-8) {hitis->SetBinContent(k+1,i+1,1.);nx=nx+1;}
	 hitj->SetBinContent(k+1,i+1,imagey[k][i]*1.);
	 if (image2y[k][i]<-1 && image2y[k][i]>-8) {hitjs->SetBinContent(k+1,i+1,1.);ny=ny+1;}

	 }
	 }
      */
      for (recit it=edges.begin();it!=edges.end();it++)
	{
	  hitis->SetBinContent((*it)->chamber(),(*it)->I(),1.);
	  hitjs->SetBinContent((*it)->chamber(),(*it)->J(),1.);
	}
      for (recit it=cores.begin();it!=cores.end();it++)
	{
	  hiti->SetBinContent((*it)->chamber(),(*it)->I(),1.);
	  hitj->SetBinContent((*it)->chamber(),(*it)->J(),1.);
	}
      //DEBUG_PRINT("NT %d NX %f ny %f => %f %f \n",vrh.size(),nx,ny,nx/vrh.size(),ny/vrh.size());
      for (uint32_t k=1;k<59;k++)
	{
	  for (uint32_t i=1;i<95;i++)
	    {
	      //if (image2x[k][i]>=-1 ) continue;

	      for (uint32_t j=1;j<95;j++)
		{
		  //if (image2y[k][j]>=-1 ) continue;
		  if (image3[k][i][j]>=-2) continue;
		  hw3->Fill(image3[k][i][j]);
		  if (image3[k][i][j]<-32) continue;

		  //uint32_t key=((i+1)<<24)|((j+1)<<16)|(k+1);
		  //std::map<uint32_t,RecoHit*>::iterator it=mapij.find(key);
					
		  hitis1->SetBinContent(k+1,i+1,1.);
		  hitjs1->SetBinContent(k+1,j+1,1.);
		}
	    }
	}
      if (TCEdge!=NULL && draw_)
	{
	  TCEdge->cd();

	  TCEdge->cd(1);

	  hiti->SetLineColor(kGreen);
	  hiti->Draw("box");
	  TCEdge->cd(2);
	  hitis1->SetLineColor(kRed);
	  hiti->Draw("box");
	  hitis->Draw("boxsame");

	  TCEdge->cd(3);
	  hitj->SetLineColor(kGreen);
	  hitj->Draw("box");
	  TCEdge->cd(4);
	  hitjs1->SetLineColor(kRed);
	  hitj->Draw("box");
	  hitjs->Draw("boxsame");
			
	  TCEdge->Modified();
	  TCEdge->Update();

	  std::stringstream ss("");
	  ss<<"/tmp/Sobel_"<<evt_->getRunNumber()<<"_"<<evt_->getEventNumber()<<"_"<<currentTime_<<".png";
	  TCEdge->SaveAs(ss.str().c_str());
	  char c;c=getchar();putchar(c); if (c=='.') exit(0);;
	} 
    }
}

void ShowerAnalyzer::sobel_filtering(unsigned char image1[60][96],float image2[60][96] )
/* Spatial filtering of image data */
/* Sobel filter (horizontal differentiation */
/* Input: image1[y][x] ---- Outout: image2[y][x] */
{
  /* Definition of Sobel filter in horizontal direction */
  int32_t weight[3][3] = {{ -1,  0,  1 },
			  { -2,  0,  2 },
			  { -1,  0,  1 }};

  /*
    int32_t weight2[3][3] = {{ -1,  -2,  -1 },
    { 0,  0,  0 },
    { 1,  2,  1 }};

  */
  int weight3[3][3] = {{ 1,  1,  1 },
		       { 1,  -8,  1 },
		       { 1,  1,  1 }};

  uint32_t x_size1=96;
  uint32_t y_size1=60;
  //uint32_t MAX_BRIGHTNESS=1;
  double pixel_value;
  double min, max;
  uint32_t x, y, i, j;  /* Loop variable */

  /* Maximum values calculation after filtering*/
  //DEBUG_PRINT("Now, filtering of input image is performed\n\n");
  min = DBL_MAX;
  max = -DBL_MAX;
  for (y = 1; y < y_size1 - 1; y++) {
    for (x = 1; x < x_size1 - 1; x++) {
      pixel_value = 0.0;
      for (j = -1; j <= 1; j++) {
	for (i = -1; i <= 1; i++) {
	  pixel_value += weight3[j + 1][i + 1] * image1[y + j][x + i];
	}
      }
      if (pixel_value < min) min = pixel_value;
      if (pixel_value > max) max = pixel_value;
    }
  }
  //DEBUG_PRINT("Min %f Max %f \n",min,max);
  if ((int)(max - min) == 0) {
    DEBUG_PRINT("Nothing exists!!!\n\n");
    exit(1);
  }

  /* Initialization of image2[y][x] */
  uint32_t x_size2 = x_size1;
  uint32_t y_size2 = y_size1;
  for (y = 0; y < y_size2; y++) {
    for (x = 0; x < x_size2; x++) {
      image2[y][x] = 0;
    }
  }
  /* Generation of image2 after linear transformtion */
  for (y = 1; y < y_size1 - 1; y++) {
    for (x = 1; x < x_size1 - 1; x++) {
      pixel_value = 0.0;
      for (j = -1; j <= 1; j++) {
	for (i = -1; i <= 1; i++) {
	  //pixel_value += weight[j + 1][i + 1] * image1[y + j][x + i];
	  pixel_value += weight3[j + 1][i + 1] * image1[y + j][x + i];
	}
      }
      //pixel_value = MAX_BRIGHTNESS * (pixel_value - min) / (max - min);
      image2[y][x] = pixel_value;
    }
  }
  return;
}
void ShowerAnalyzer::sobel_volume(unsigned char image1[60][96][96],float image2[60][96][96] )
/* Spatial filtering of image data */
/* Sobel filter (horizontal differentiation */
/* Input: image1[y][x] ---- Outout: image2[y][x] */
{
  /* Definition of Sobel filter in horizontal direction */
  int32_t weight[3][3][3];
  uint32_t x_size1=96;
  uint32_t y_size1=96;
  uint32_t z_size1=60;
  //uint32_t MAX_BRIGHTNESS=1;
  double pixel_value;
  double min, max;
  uint32_t x, y,z;int32_t i, j,k;  /* Loop variable */

  for (i=0;i<3;i++)
    for (j=0;j<3;j++)
      for (k=0;k<3;k++)
	weight[i][j][k]=1;
  weight[1][1][1]=-44;
  for (i=0;i<3;i++)
    for (j=0;j<3;j++)
      {
	weight[0][i][j]=2;
	weight[2][i][j]=2;
      }
  /* Maximum values calculation after filtering*/
  //DEBUG_PRINT("Now, filtering of input image is performed\n\n");
  min = DBL_MAX;
  max = -DBL_MAX;
  /* Generation of image2 after linear transformtion */
  for (y = 1; y < y_size1 - 1; y++) 
    {
      for (x = 1; x < x_size1 - 1; x++) 
	{
	  for (z = 1; z < z_size1 - 1; z++) 
	    {

	      pixel_value = 0.0;
	      for (j = -1; j <= 1; j++) {
		for (i = -1; i <= 1; i++) {
		  for (k=-1;k<=1;k++){
		    //pixel_value += weight[j + 1][i + 1] * image1[y + j][x + i];
		    pixel_value += weight[j + 1][i + 1][k+1] * image1[z+k][y + j][x + i];
		  }
		}
	      }
	      //pixel_value = MAX_BRIGHTNESS * (pixel_value - min) / (max - min);
	      image2[z][y][x] = pixel_value;
	      if (pixel_value<min) min=pixel_value;
	      if (pixel_value>max) max=pixel_value;
	      //DEBUG_PRINT("%d %d %d %f \n",z,y,x,pixel_value);
	    }
	}
    }
  //DEBUG_PRINT("MIn %f %f \n",min,max);
  //getchar();
  return;
}



void ShowerAnalyzer::sobel_volume(array3D<unsigned char> &im1,array3D<float> &im2)
/* Spatial filtering of image data */
/* Sobel filter (horizontal differentiation */
/* Input: image1[y][x] ---- Outout: image2[y][x] */
{

  /* Definition of Sobel filter in horizontal direction */
  int32_t weight[3][3][3];

  //  uint32_t MAX_BRIGHTNESS=1;
  double pixel_value;
  double min, max;
  uint32_t x, y,z; 
  int32_t i, j,k;  /* Loop variable */

  for (i=0;i<3;i++)
    for (j=0;j<3;j++)
      for (k=0;k<3;k++)
	weight[i][j][k]=1;
  weight[1][1][1]=-26;
  // for (i=0;i<3;i++)
  //   for (j=0;j<3;j++)
  //     {
  // 	weight[0][i][j]=1;
  // 	weight[2][i][j]=1;
  //     }
  /* Maximum values calculation after filtering*/
  //DEBUG_PRINT("Now, filtering of input image is performed\n\n");
  min = DBL_MAX;
  max = -DBL_MAX;
  /* Generation of image2 after linear transformtion */
  for (z = 1; z < im1.getXSize() - 1; z++) 
    {
      for (x = 1; x < im1.getYSize() - 1; x++) 
	{
	  for (y = 1; y < im1.getZSize() - 1; y++) 
	    {

	      pixel_value = 0.0;
	      im2.setValue(z,x,y,pixel_value);
	      if (im1.getValue((z),(x ),(y ))<=0) continue;
		for (i = -1; i <= 1; i++) {
		  for (j = -1; j <= 1; j++) {
		    for (k=-1;k<=1;k++){
		    //pixel_value += weight[j + 1][i + 1] * image1[y + j][x + i];
		      if (im1.getValue((z+i),(x + j),(y + i))>0)
			pixel_value += weight[i + 1][j + 1][k+1];
		  }
		}
	      }
	      //pixel_value = MAX_BRIGHTNESS * (pixel_value - min) / (max - min);
	      im2.setValue(z,x,y,pixel_value);
	      if (pixel_value<min) min=pixel_value;
	      if (pixel_value>max) max=pixel_value;
	      //DEBUG_PRINT("%d %d %d %f \n",z,y,x,pixel_value);
	    }
	}
    }
  //DEBUG_PRINT("MIn %f %f \n",min,max);
  //getchar();

  return;
}


void ShowerAnalyzer::openSqlite(std::string fileName)
{
  int32_t rc = sqlite3_open(fileName.c_str(), &theDb_);
  if( rc ){
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(theDb_));
    sqlite3_close(theDb_);
    exit(1);
  }
}
int32_t ShowerAnalyzer::fillEventTable(uint32_t run, uint32_t event,uint64_t bcid,uint32_t ti,uint32_t e3,uint32_t c3,int32_t w,uint32_t np)
{
  std::string start="(?,";
  if (useMysql_) start="(NULL,";
  //    char cmd[2048];
  //sprintf(cmd,"INSERT INTO EVENT ('RUN','EVENT','BCID','TIME') VALUES('%d','%d','%ld','%d')",run,event,bcid,time);
  std::stringstream s;
  s<<"INSERT INTO EVENT VALUES "<<start<<"'"<<run<<"','"<<event<<"','"<<bcid<<"','"<<ti<<"','"<<e3<<"','"<<c3<<"','"<<w<<"','"<<np<<"')";
  std::cout<<s.str()<<std::endl;
  int32_t retval =executeQuery(s.str());
  if (retval==0)
    theEventRowId_=getLastInsertId();
  return retval;
}
int32_t ShowerAnalyzer::fillComponentsTable(uint32_t *nh,double *components)
{
  std::string start="(?,";
  if (useMysql_) start="(NULL,";
  std::stringstream s;
  s<<"INSERT INTO COMPONENTS VALUES "<<start<<"'"<<theEventRowId_<<"',";
  for (uint32_t i=0;i<3;i++)
    s<<"'"<<nh[i]<<"',";
  for (uint32_t i=0;i<20;i++)
    s<<"'"<<components[i]<<"',";
  s<<"'"<<components[20]<<"')";

	
  int32_t retval =executeQuery(s.str());
  int32_t cid=-1;
  if (retval==0)
    cid=getLastInsertId();
  return cid;
}

int32_t ShowerAnalyzer::fillTrackTable(uint32_t npoint,double ax,double bx,double ay,double by,double chi2)
{
  std::string start="(?,";
  if (useMysql_) start="(NULL,";
  std::stringstream s;
  s<<"INSERT INTO TRACK VALUES "<<start<<"'"<<theEventRowId_<<"',"
   <<"'"<<npoint<<"',"
   <<"'"<<ax<<"',"
   <<"'"<<bx<<"',"
   <<"'"<<ay<<"',"
   <<"'"<<by<<"',"
   <<"'"<<chi2<<"')";
	

	
  int32_t retval =executeQuery(s.str());
  int32_t cid=-1;
  if (retval==0)
    cid=getLastInsertId();
  return cid;
}
int32_t ShowerAnalyzer::fillPointTable(uint32_t trackid,uint32_t plan,double z,double xext,double yext,double xclus,double yclus)
{
  std::string start="(?,";
  if (useMysql_) start="(NULL,";
  std::stringstream s;
  s<<"INSERT INTO POINT VALUES "<<start<<"'"<<trackid<<"',"
   <<"'"<<plan<<"',"
   <<"'"<<z<<"',"
   <<"'"<<xext<<"',";
  if (xclus!=999.)
    {
      s<<"'"<<yext<<"',"
       <<"'"<<xclus<<"',"
       <<"'"<<yclus<<"')";
    }
  else
    s<<"'"<<yext<<"')";
	

	
  int32_t retval =executeQuery(s.str());
  int32_t cid=-1;
  if (retval==0)
    cid=getLastInsertId();
  return cid;
}
int32_t ShowerAnalyzer::fillAmasTable(Amas* a)
{
  std::string start="(?,";
  if (useMysql_) start="(NULL,";
  uint64_t cid=this->fillComponentsTable(a->Hits(),a->Components());
  std::stringstream s;
  s<<"INSERT INTO AMAS VALUES "<<start<<"'"<<theEventRowId_<<"','"<<cid<<"')";
  //int32_t retval = sqlite3_exec(theDb_,s.str().c_str(),0,0,0);

  //std::cout<<s.str().c_str()<<std::endl;
  //getchar();
  int32_t retval =executeQuery(s.str());

  return retval;

}

int32_t ShowerAnalyzer::fillAmasSummaryTable(uint32_t core[3],uint32_t edge[3],uint32_t namas,uint32_t nafter,uint32_t tag,double zlast)
{
  std::string start="(?,";
  if (useMysql_) start="(NULL,";
  std::stringstream s;
  s<<"INSERT INTO AMAS_SUMMARY VALUES "<<start<<"'"<<theEventRowId_<<"',";
  for (uint32_t i=0;i<3;i++)
    s<<"'"<<core[i]<<"',";
  for (uint32_t i=0;i<3;i++)
    s<<"'"<<edge[i]<<"',";
  s<<"'"<<namas<<"',";
  s<<"'"<<nafter<<"',";
  s<<"'"<<tag<<"',";
  s<<"'"<<zlast<<"')";
  //int32_t retval = sqlite3_exec(theDb_,s.str().c_str(),0,0,0);
  int32_t retval =executeQuery(s.str());

  return retval;
}
int32_t ShowerAnalyzer::fillShowerTable(shower_t &s)
{
  std::string start="(?,";
  if (useMysql_) start="(NULL,";
  uint32_t nh[3];
  double res[21];
  // Fill components table
  for (uint32_t i=0;i<3;i++)
    nh[i]=s.nhit[i];
  uint32_t ix=0;
  memset(res,0,21*sizeof(double));
  res[ix++]=s.xm[0];
  res[ix++]=s.xm[1];
  res[ix++]=s.xm[2];
  res[ix++]=s.lambda[0];
  res[ix++]=s.lambda[1];
  res[ix++]=s.lambda[2];
  res[ix++]=s.v1[0];
  res[ix++]=s.v1[1];
  res[ix++]=s.v1[2];
  res[ix++]=s.v2[0];
  res[ix++]=s.v2[1];
  res[ix++]=s.v2[2];
  res[ix++]=s.v3[0];
  res[ix++]=s.v3[1];
  res[ix++]=s.v3[2];
  res[ix++]=s.firstplan;
  res[ix++]=s.lastplan;
  uint32_t cid=this->fillComponentsTable(nh,res);
  for (uint32_t i=0;i<3;i++)
    nh[i]=s.rnhit[i];
  ix=0;
  memset(res,0,21*sizeof(double));
  res[ix++]=s.rxm[0];
  res[ix++]=s.rxm[1];
  res[ix++]=s.rxm[2];
  res[ix++]=s.rlambda[0];
  res[ix++]=s.rlambda[1];
  res[ix++]=s.rlambda[2];
  res[ix++]=s.rv1[0];
  res[ix++]=s.rv1[1];
  res[ix++]=s.rv1[2];
  res[ix++]=s.rv2[0];
  res[ix++]=s.rv2[1];
  res[ix++]=s.rv2[2];
  res[ix++]=s.rv3[0];
  res[ix++]=s.rv3[1];
  res[ix++]=s.rv3[2];
  res[ix++]=s.fp1;
  res[ix++]=s.lp1;
  uint32_t rcid=this->fillComponentsTable(nh,res);

  std::stringstream sts;
  sts<<"INSERT INTO SHOWER VALUES "<<start<<"'"<<theEventRowId_<<"',";
  sts<<"'"<<cid<<"',";
  sts<<"'"<<rcid<<"',";
  sts<<"'"<<s.n9<<"',";
  sts<<"'"<<s.n25<<"',";
  sts<<"'"<<(uint32_t) s.firstplan<<"',";
  sts<<"'"<<(uint32_t)s.lastplan<<"',";
  sts<<"'"<<(uint32_t)s.fp1<<"',";
  sts<<"'"<<(uint32_t)s.lp1<<"',";
  sts<<"'"<<(uint32_t)s.np1<<"',";
  sts<<"'"<<s.fd[0]<<"',";
  sts<<"'"<<s.fd[1]<<"',";
  sts<<"'"<<s.fd[2]<<"',";
  sts<<"'"<<s.fd[3]<<"',";
  sts<<"'"<<s.rbs<<"',";

  sts<<"'"<<s.rbt<<"')";
  //std::cout<<sts.str()<<std::endl;
  //getchar();
  //int32_t retval = sqlite3_exec(theDb_,sts.str().c_str(),0,0,0);

  int32_t retval =executeQuery(sts.str());

	
  return retval;
}

int32_t ShowerAnalyzer::executeQuery(std::string stmt)
{
  int32_t retval =-1;
  if (useSqlite_)
    {
      do 
	{
	  retval=sqlite3_exec(theDb_,stmt.c_str(),0,0,0);
	  if (retval!=SQLITE_OK)
	    {std::cout<<"SQLITE ERROR =>"<<stmt<<std::endl;getchar();}
	} while (retval!=SQLITE_OK);
    }
  if (useMysql_)
    {
      do {
	retval=mysql_query (&theMysql_,stmt.c_str());
	if (retval!=0)
	  {
	    fprintf(stderr, "Error during query: Command %s Error: %s\n",
		    stmt.c_str(),mysql_error(&theMysql_));
	    char c;c=getchar();putchar(c); if (c=='.') exit(0);;
	  } 
      }
      while (retval!=0);
      // get Last Id
      //ID_=mysql_insert_id(&mysql_);



    }
	
  return retval;
}

uint32_t ShowerAnalyzer::getLastInsertId()
{
  uint32_t id=0;
  if (useMysql_)
    id=mysql_insert_id(&theMysql_);
  if (useSqlite_)
    id=sqlite3_last_insert_rowid(theDb_);
  return id;
}

void ShowerAnalyzer::decodeAccount(std::string account)
{
  // "root/monpasswd@localhost:DHCAL_TEST"
  int ipass = account.find("/");
  int ipath = account.find("@");
  int idb = account.find(":");
  myName_.clear();
  myName_=account.substr(0,ipass); 
  myPwd_.clear();
  myPwd_=account.substr(ipass+1,ipath-ipass-1); 
  myHost_.clear();
  myHost_=account.substr(ipath+1,idb-ipath-1); 
  myDatabase_.clear();
  myDatabase_=account.substr(idb+1,account.size()-idb); 
  std::cout<<myName_<<std::endl;
  std::cout<<myPwd_<<std::endl;
  std::cout<<myHost_<<std::endl;
  std::cout<<myDatabase_<<std::endl;

}
void ShowerAnalyzer::connect(std::string account)
{
  std::cout<<"Connecting to :"<<account<<std::endl;
  decodeAccount(account);
  mysql_init (&theMysql_);

  if (!mysql_real_connect(&theMysql_,myHost_.c_str(),myName_.c_str(),myPwd_.c_str(),myDatabase_.c_str(),0,NULL,0))
    {
      fprintf(stderr, "Failed to connect to database: Error: %s\n",
	      mysql_error(&theMysql_));
      exit(0);
    }
}

void ShowerAnalyzer::draw(array3D<unsigned char> &all,array3D<unsigned char> &core,array3D<unsigned char> &edge)
{



  // const unsigned char color[] = { 255,128,64 };
  //   CImg<unsigned char> img(all.getXSize(),all.getYSize(),all.getZSize());
  //    CImgDisplay disp(512,512,"Edge Explorer");
  //    unsigned char* idata;
  //    for (uint32_t i=0;i<all.getXSize();i++)
  //        {
  //    	for (uint32_t j=0;j<all.getYSize();j++)
  //    	  for (uint32_t k=0;k<all.getZSize();k++)
  //    	  {
	
  //    	    idata=img.data(i,j,k);
  // 	    if(all.getValue(i,j,k)!=0) 
  // 	      img.draw_point(i,j,k,color);
  // 	      //*idata=0;
  //   // 	    else
  //   // 	      *idata=255;
  //    	  }
  //        }
  //    img.display(disp);

  //    getchar();
  TH3* hallmap = rootHandler_->GetTH3("AllMap");
  TH3* hvecmap = rootHandler_->GetTH3("VecMap");
  TH3* hcoremap = rootHandler_->GetTH3("CoreMap");
  TH3* hedgemap = rootHandler_->GetTH3("EdgeMap");


  if (hallmap==NULL)
    {
      hallmap =rootHandler_->BookTH3("AllMap",all.getXSize(),0.,all.getXSize()*2.8,all.getYSize(),0.,all.getYSize()*1.,all.getZSize(),0.,all.getZSize()*1.);
      hvecmap =rootHandler_->BookTH3("VecMap",all.getXSize(),0.,all.getXSize()*1.,all.getYSize(),0.,all.getYSize()*1.,all.getZSize(),0.,all.getZSize()*1.);
      hcoremap =rootHandler_->BookTH3("CoreMap",core.getXSize(),0.,core.getXSize()*1.,core.getYSize(),0.,core.getYSize()*1.,core.getZSize(),0.,core.getZSize()*1.);
      hedgemap =rootHandler_->BookTH3("EdgeMap",edge.getXSize(),0.,edge.getXSize()*1.,edge.getYSize(),0.,edge.getYSize()*1.,edge.getZSize(),0.,edge.getZSize()*1.);
    }
  else
    {
      hvecmap->Reset();
      hallmap->Reset();
      hcoremap->Reset();
      hedgemap->Reset();

    }

  if (hallmap!=0 )
    {
      for (uint32_t i=0;i<all.getXSize();i++)
	for (uint32_t j=0;j<all.getYSize();j++)
	  for (uint32_t k=0;k<all.getZSize();k++)
	    if (all.getValue(i,j,k))
	      hallmap->SetBinContent(i+1,j+1,k+1,1);
    }
  if (hcoremap!=0 )
    {
      for (uint32_t i=0;i<core.getXSize();i++)
	for (uint32_t j=0;j<core.getYSize();j++)
	  for (uint32_t k=0;k<core.getZSize();k++)
	    if (core.getValue(i,j,k))
	      hcoremap->SetBinContent(i+1,j+1,k+1,1);
    }
  if (hedgemap!=0 )
    {
      for (uint32_t i=0;i<edge.getXSize();i++)
	for (uint32_t j=0;j<edge.getYSize();j++)
	  for (uint32_t k=0;k<edge.getZSize();k++)
	    if (edge.getValue(i,j,k))
	      hedgemap->SetBinContent(i+1,j+1,k+1,1);
    }
  if (TCPlot==NULL)
    {
      TCPlot=new TCanvas("TCPlot","test1",1200,800);
      TCPlot->Modified();
      TCPlot->Draw();
      /*
	TCPlot->Divide(1,3);
	TVirtualPad* p3=TCPlot->cd(3);
	p3->Divide(2,1);
      */
    }
  TCPlot->cd();
  TCPlot->cd(1);
  hallmap->SetMarkerStyle(25);
  hcoremap->SetMarkerStyle(25);
  hedgemap->SetMarkerStyle(25);
  hvecmap->SetMarkerStyle(25);
  hallmap->SetMarkerSize(.2);
  hcoremap->SetMarkerSize(.2);
  hedgemap->SetMarkerSize(.2);
  hvecmap->SetMarkerSize(.2);
  hallmap->SetMarkerColor(29);
  hvecmap->SetMarkerColor(kBlack);
	
  hcoremap->SetMarkerColor(kBlue);
  hedgemap->SetMarkerColor(kRed);
  hallmap->Draw("p");
  uint32_t igood=0;
	
  for (std::vector<Amas>::iterator ia=theAmas_.begin();ia!=theAmas_.end();ia++)
    {
      //ia->compute();
      if (ia->size()<=4) continue;
      std::stringstream hn("");
      hn<<"vecmap"<<igood;
      TH3* ham = rootHandler_->GetTH3(hn.str());


      if (ham==NULL)
	{
	  ham =rootHandler_->BookTH3(hn.str(),all.getXSize(),0.,all.getXSize()*1.,all.getYSize(),0.,all.getYSize()*1.,all.getZSize(),0.,all.getZSize()*1.);
	  ham->SetMarkerStyle(25);
	  ham->SetMarkerSize(.2);

	}
      else
	ham->Reset();
      for (std::vector<RecoHit*>::iterator ih=ia->getHits().begin();ih!=ia->getHits().end();ih++)
	{
	  hvecmap->SetBinContent((*ih)->chamber(),(*ih)->I(),(*ih)->J(),1.);
	  ham->SetBinContent((*ih)->chamber(),(*ih)->I(),(*ih)->J(),1.);
	}
      ham->SetMarkerColor(igood+2);
      igood++;
      ham->Draw("pSAME");
      double* cop=ia->Components();

		
      TPolyLine3D *pl3d1=new TPolyLine3D(2);
      double* v=&cop[6];    
      double* x=&cop[0]; 
      x[2]=x[2]/2.8;

      pl3d1->SetPoint(0,x[2],x[0],x[1]);
      pl3d1->SetPoint(1,x[2]+v[2],x[0]+v[0],x[1]+v[1]);
		
      pl3d1->SetLineWidth(3);
      pl3d1->SetLineColor(1);
      pl3d1->Draw("SAME");
      TPolyLine3D* pl3d2= new TPolyLine3D(2);
      v=&cop[9];    
      pl3d2->SetPoint(0,x[2],x[0],x[1]);
      pl3d2->SetPoint(1,x[2]+v[2],x[0]+v[0],x[1]+v[1]);
		
      pl3d2->SetLineWidth(3);
      pl3d2->SetLineColor(1);
      pl3d2->Draw("SAME");
      TPolyLine3D* pl3d3= new TPolyLine3D(2);
      v=&cop[12];    
      pl3d3->SetPoint(0,x[2],x[0],x[1]);
      pl3d3->SetPoint(1,x[2]+v[2],x[0]+v[0],x[1]+v[1]);
		
		
		
		
      pl3d3->SetLineWidth(3);
      pl3d3->SetLineColor(1);
      pl3d3->Draw("SAME");







    }
  //hvecmap->Draw("pSAME");
  /*
    TCPlot->cd(2);
    hcoremap->Draw("BOX");
    hedgemap->Draw("pSAME");
    TVirtualPad* p3=TCPlot->cd(3);
    p3->cd(1);
    TH1* eratio=rootHandler_->GetTH1("eratio2");
    if (eratio!=0) eratio->Draw();
    p3->cd(2);
    TH1* edsig=rootHandler_->GetTH1("eratio3");
    if (edsig!=0) edsig->Draw();
  */
  TCPlot->Modified();
  TCPlot->Draw();
  TCPlot->Update();
	
}

void ShowerAnalyzer::draw(std::vector<RecoCandTk> &tracks,std::vector<RecoPoint> &points)
{


  TH3* hcgposi = rootHandler_->GetTH3("InstantClusterMap");
  TH3* hcgposj = rootHandler_->GetTH3("HitClusterMap");

  if (hcgposi==NULL)
    {
      hcgposi =rootHandler_->BookTH3("InstantClusterMap",52,-2.8,145.6,100,0.,100.,100,0.,100.);
      hcgposj =rootHandler_->BookTH3("HitClusterMap",52,-2.8,145.6,100,0.,100.,100,0.,100.);

    }
  else
    {
      hcgposi->Reset();
      hcgposj->Reset();
		
    }
	
  if (hcgposi!=0 )
    {
      hcgposi->Reset();
      for (unsigned int i =0;i<points.size();i++)
	{
	  //if (allpoints_[i].Charge()<7) continue;
	  hcgposi->Fill(points[i].Z(),points[i].X(),points[i].Y());
	}
      for (uint32_t k=0;k<60;k++)
	for (uint32_t i=0;i<96;i++)
	  for (uint32_t j=0;j<96;j++)
	    if (theImage_.getValue(k,i,j)!=0 )
	      {
		hcgposj->Fill(hitVolume_[k][i][j].Z(),hitVolume_[k][i][j].X(),hitVolume_[k][i][j].Y());
							
							
	      }
    }
  if (TCPlot==NULL)
    {
      TCPlot=new TCanvas("TCPlot","test1",1200,800);
      TCPlot->Modified();
      TCPlot->Draw();
      /*
	TCPlot->Divide(1,3);
	TVirtualPad* p3=TCPlot->cd(3);
	p3->Divide(2,1);
      */
    }
	
  if (TCPlot!=NULL)
    {
      TCPlot->cd();
      hcgposi->SetMarkerStyle(25);
	
      hcgposi->SetMarkerSize(.2);
	
      hcgposi->SetMarkerColor(kRed);
	
      hcgposi->Draw("p");
      hcgposj->SetMarkerStyle(25);
	
      hcgposj->SetMarkerSize(.2);
	
      hcgposj->SetMarkerColor(kBlue);
	
      hcgposj->Draw("psame");
      for (std::vector<RecoCandTk>::iterator it=tracks.begin();it!=tracks.end();it++)
	{
	  RecoCandTk &tk=(*it);
	  TPolyLine3D *pl3d1 = new TPolyLine3D(2);
	  TCPlot->cd(1);
	  pl3d1->SetPoint(0, tk.zmin_,tk.ax_*tk.zmin_+tk.bx_,tk.ay_*tk.zmin_+tk.by_);
	  pl3d1->SetPoint(1, tk.zmax_,tk.ax_*tk.zmax_+tk.bx_,tk.ay_*tk.zmax_+tk.by_);

	  pl3d1->SetLineWidth(1);
	  pl3d1->SetLineColor(3);
	  pl3d1->Draw("SAME");
			

	  TCPlot->Modified();
	  TCPlot->Update();
	}
    }
	

	
	
	
}

#ifdef USE_CIMG
#include "CImg.h"

using namespace cimg_library;
#endif
void ShowerAnalyzer::newHT(array3D<unsigned char> &cores)
{
#ifdef USE_CIMG
  CImg <unsigned char>src(840,480);
  src.fill(255);
  const unsigned char
    red[3] = { 255,0,0 },          //
    black[3] = { 0,0,0 };          // Defining the colors we need for drawing.

    const double alpha=1.5;
    const double sigma=0.5;



    for (uint32_t k=0;k<60;k++)
      for (uint32_t i=0;i<96;i++)
	for (uint32_t j=0;j<96;j++)
	  if (theImage_.getValue(k,i,j)!=0 
	      && cores.getValue(k,i/3,j/3)==0)  
	    {

	      src.draw_point(k*14,j*5,black);
	    }



    CImg<> vote(360,300,1,1,0), img = src.get_norm().normalize(0,255).resize(-100,-100,1,2,2);

    CImgDisplay disp(src,"Image"), dispvote(vote,"Hough Transform");
    const unsigned char col1[3]={255,255,255}, col2[3]={0,0,0};
    double  rhomax = std::sqrt((double)(img.width()*img.width()+img.height()*img.height()))/2;

    double    thetamax = 2*M_PI;

    // while (!disp.is_closed() && !disp.is_keyQ() && !disp.is_keyESC()) {
    //   disp.wait(100);
    //   if (disp.is_resized()) {disp.resize();}
    // }

	
    while (!disp.is_closed() && !dispvote.is_closed() &&
	   !disp.is_keyQ() && !dispvote.is_keyQ() && !disp.is_keyESC() && !dispvote.is_keyESC()) {

      CImgDisplay::wait(disp,dispvote);

      // When pressing space bar, the vote is performed from the image gradients.
      if (dispvote.is_keySPACE() || disp.is_keySPACE()) {
	CImgList<> grad = img.get_gradient();
	cimglist_for(grad,l) grad[l].blur((float)alpha);
	vote.fill(0);
	cimg_forXY(img,x,y) {
	  const double
	    X = (double)x - img.width()/2,
	    Y = (double)y - img.height()/2,
	    gx = grad[0](x,y),
	    gy = grad[1](x,y);
	  double
	    theta = std::atan2(gy,gx),
	    rho   = std::sqrt(X*X+Y*Y)*std::cos(std::atan2(Y,X)-theta);
	  if (rho<0) { rho=-rho; theta+=M_PI; }
	  theta = cimg::mod(theta,thetamax);
	  vote((int)(theta*dispvote.width()/thetamax),(int)(rho*dispvote.height()/rhomax))+=(float)std::sqrt(gx*gx+gy*gy);
	}
	vote.blur((float)sigma);
	CImg<> vote2(vote); cimg_forXY(vote2,x,y) vote2(x,y) = (float)std::log(1+vote(x,y)); vote2.display(dispvote);
      }

      // When clicking on the vote window.
      if (dispvote.button()) {
	printf("%d %d %f \n",dispvote.mouse_x(),dispvote.mouse_y(),vote.atXY(dispvote.mouse_x(),dispvote.mouse_y()));
	const double
	  rho   = dispvote.mouse_y()*rhomax/dispvote.height(),
	  theta = dispvote.mouse_x()*thetamax/dispvote.width(),
	  x = img.width()/2  + rho*std::cos(theta),
	  y = img.height()/2 + rho*std::sin(theta);
	const int
	  x0 = (int)(x+1000*std::sin(theta)),
	  y0 = (int)(y-1000*std::cos(theta)),
	  x1 = (int)(x-1000*std::sin(theta)),
	  y1 = (int)(y+1000*std::cos(theta));
	CImg<unsigned char>(src).
	  draw_line(x0,y0,x1,y1,col1,1.0f,0xF0F0F0F0).draw_line(x0,y0,x1,y1,col2,1.0f,0x0F0F0F0F).
	  draw_line(x0+1,y0,x1+1,y1,col1,1.0f,0xF0F0F0F0).draw_line(x0+1,y0,x1+1,y1,col2,1.0f,0x0F0F0F0F).
	  draw_line(x0,y0+1,x1,y1+1,col1,1.0f,0xF0F0F0F0).draw_line(x0,y0+1,x1,y1+1,col2,1.0f,0x0F0F0F0F).
	  display(disp);
      }

      // When clicking on the image.
      if (disp.button() && disp.mouse_x()>=0) {
	double step=thetamax/vote.width()/2;
	cimg_forXY(img,x,y) {
	  const double
	    x0 = (double)x-disp.width()/2,
	    y0 = (double)y-disp.height()/2,
	    rho0 = std::sqrt(x0*x0+y0*y0),
	    theta0 = std::atan2(y0,x0);

	  //printf("%d %d %d \n",x,y,src.atXY(x,y));
	  if (img.atXY(x,y)==0)
	    for (double t=0; t<thetamax; t+=step) {
	      double theta = t, rho = rho0*std::cos(theta0-t);
	      if (rho<0) { rho=-rho; theta=cimg::mod(theta+M_PI,thetamax); }
	      vote((int)(theta*vote.width()/thetamax),(int)(rho*vote.height()/rhomax))+=1;
	    }
	}
	//vote.blur(1.5);
	CImg<> vote2(vote); cimg_forXY(vote2,x,y) vote2(x,y) = (float)std::log(1+vote(x,y)); vote2.display(dispvote);
	printf("Median %f Max %f \n",vote2.median(),vote2.max());
	double vmax=vote2.max();
	/* cimg_forXY(vote2,x,y) {

	   if (vote2.atXY(x,y)>0.9* vmax && std::abs(x-dispvote.width()/2)>4)
	   {
	   printf("%d , %d, %f %f \n",x,y,vote2.atXY(x,y),vote.atXY(x,y));
	   const double
	   rho   = y*rhomax/dispvote.height(),
	   theta = x*thetamax/dispvote.width(),
	   x = img.width()/2  + rho*std::cos(theta),
	   y = img.height()/2 + rho*std::sin(theta);
	   const int
	   x0 = (int)(x+1000*std::sin(theta)),
	   y0 = (int)(y-1000*std::cos(theta)),
	   x1 = (int)(x-1000*std::sin(theta)),
	   y1 = (int)(y+1000*std::cos(theta));
	   CImg<unsigned char>(src).
	   draw_line(x0,y0,x1,y1,col1,1.0f,0xF0F0F0F0).draw_line(x0,y0,x1,y1,col2,1.0f,0x0F0F0F0F).
	   draw_line(x0+1,y0,x1+1,y1,col1,1.0f,0xF0F0F0F0).draw_line(x0+1,y0,x1+1,y1,col2,1.0f,0x0F0F0F0F).
	   draw_line(x0,y0+1,x1,y1+1,col1,1.0f,0xF0F0F0F0).draw_line(x0,y0+1,x1,y1+1,col2,1.0f,0x0F0F0F0F).
	   display(disp);
	   sleep((unsigned int) 1);

	   }

	   }
	*/

      }
      dispvote.resize(dispvote);
      if (disp.is_resized()) {disp.resize();}
    }

#endif
}
#include <cmath>
#include <iostream>

void ShowerAnalyzer::newHT3(array3D<unsigned char> &cores)
{
  TH1F* hw = (TH1F*) rootHandler_->GetTH1("weights");
  if (hw==NULL)
    {
      //hhtx =(TH2F*)rootHandler_->BookTH2("HoughTransform",nhtheta,0,TWOM_PI,nhr,rmin,rmax);
      hw =(TH1F*)rootHandler_->BookTH1("weights",50,-50.,0.);
    }
  else
    hw->Reset();
  const uint32_t nbz=60;
  const uint32_t nbx=96;

  const double zmin=0;
  const double zmax=60*2.8;
  const double xmin=0;
  const double xmax=100.;
  const double zbin=(zmax-zmin)/nbz;
  const double xbin=(xmax-xmin)/nbx;
  const double TWOPI=2*M_PI;
	
  std::vector<RecoHit*> vrh;
  vrh.clear();
  printf("mask created \n");
  int32_t filter[3][3]={{1,1,1},{1,-8,1},{1,1,1}};
  uint8_t orig[nbz][nbx];
  memset(orig,0,nbz*nbx);
  const uint32_t nhtheta=8;
  const uint32_t nhr=8;
  uint16_t ht[nhtheta][nhr];
  memset(ht,0,nhtheta*nhr*sizeof(uint16_t));
	
  const double rmax=100;
  const double rmin=0;
  const double rbin=(rmax-rmin)/nhr;
  for (uint32_t k=0;k<60;k++)
    {
      //int32_t weighted[96][96];
      for (uint32_t i=0;i<96;i++)
	for (uint32_t j=0;j<96;j++)
	  if (theImage_.getValue(k,i,j)!=0 && image3_.getValue(k,i/3,j/3)<0)
	    {
	      hw->Fill(image3_.getValue(k,i/3,j/3));
	      if(image3_.getValue(k,i/3,j/3)<=-17 && image3_.getValue(k,i/3,j/3)>-25) //&& cores.getValue(k,i/3,j/3)==0)
		vrh.push_back(&hitVolume_[k][i][j]);
	      //orig[k][j]=1;
							
							
	    }
			
			

    }
  /*
    for (uint32_t k=0;k<60;k++)	
    for (uint32_t j=0;j<96;j++)
    {
    if (orig[k][j]==0) continue;

    if (k==0 || k==60 || j==0 || j==96) continue;
		
    float w=0;
    for (int ks=-1;ks<=1;ks++)
    for (int js=-1;js<=1;js++)
    {
    w+=1.*filter[ks+1][js+1]*orig[k+ks][j+js];
    //printf("=====> %d %d %f \n",filter[ks+1][js+1],(int) theImage_.getValue(k+ks,i,j+js),w);
    }
    if (w<=-8 || w>=-2) orig[k][j]=0;
    if (orig[k][j]==0) continue;
    for (uint32_t ith=0;ith<nhtheta;ith++)
    {
    float theta=(ith)*M_PI/nhtheta;
    float r=cos(theta)*(zmin+k*zbin)+sin(theta)*(xmin+j*xbin);
    if (r>0 && r<rmax)
    {
    uint32_t ir=int(round((r-rmin)/rbin));
    ht[ith][ir]+=1;
    }
    }
    }
  */
  // Un essai

  allpoints_.clear();
  this->PointsBuilder(vrh);

  HoughLocal htl(0,M_PI,-150.,150,8,8);
  htl.clear();
  htl.fill(&allpoints_);
  std::vector < std::pair<uint32_t,uint32_t> > hbins; hbins.clear();
  htl.findMaxima(hbins,5);
  for (std::vector < std::pair<uint32_t,uint32_t> >::iterator ihb=hbins.begin();ihb<hbins.end();ihb++)
    {
      printf("Bin %d %d => %d \n",(*ihb).first,(*ihb).second,htl.getValue((*ihb).first,(*ihb).second));
    }
  htl.draw(rootHandler_);
  for (std::vector < std::pair<uint32_t,uint32_t> >::iterator ihb=hbins.begin();ihb<hbins.end();ihb++)
    {
      uint32_t ith=(*ihb).first;
      uint32_t ir=(*ihb).second;
      printf("Bin  studied %d %d => %d \n",ith,ir,htl.getValue(ith,ir));
      HoughLocal htp(htl.getTheta(ith),htl.getTheta(ith+1),htl.getR(ir),htl.getR(ir+1),16,64);
      htp.clear();
      htp.fill(&allpoints_);
      std::vector < std::pair<uint32_t,uint32_t> > hpbins; hpbins.clear();
      htp.findMaxima(hpbins,5);
      for (std::vector < std::pair<uint32_t,uint32_t> >::iterator ihbp=hpbins.begin();ihbp<hpbins.end();ihbp++)
	{
	  printf("Bin %d %d => %d \n",(*ihbp).first,(*ihbp).second,htp.getValue((*ihbp).first,(*ihbp).second));
	}
      htp.draw(rootHandler_,&hpbins);
    }
  return;
	
	
	
  for (std::vector<RecoPoint>::iterator ip=allpoints_.begin();ip!=allpoints_.end();ip++)
    {
      float z=(*ip).Z();
      float x=(*ip).Y();
      uint32_t iz=int(round((z-zmin)/zbin));
      uint32_t ix=int(round((x-xmin)/xbin));
      if (z<zmin || z>zmax) continue;
      if (x<xmin || x>xmax) continue;
	
      orig[iz][ix]=1;
		
      for (uint32_t ith=0;ith<nhtheta;ith++)
	{
	  float theta=(ith)*M_PI/nhtheta;
	  float r=cos(theta)*z+sin(theta)*x;
	  if (r>0 && r<rmax)
	    {
	      uint32_t ir=int(round((r-rmin)/rbin));
	      ht[ith][ir]+=1;
	    }
	}
		
    }
  allpoints_.clear();
	
  if (TCHT==NULL)
    {
      TCHT=new TCanvas("TCHT","hugh",800,900);
      TCHT->Modified();
      TCHT->Draw();
      TCHT->Divide(1,2);
    }
  TCHT->cd();

  TH2F* hhtx = (TH2F*) rootHandler_->GetTH2("HoughTransformX");
  TH2F* hx = (TH2F*) rootHandler_->GetTH2("HOriginalX");
  if (hhtx==NULL)
    {
      hhtx =(TH2F*)rootHandler_->BookTH2("HoughTransform",nhtheta,0,M_PI,nhr,rmin,rmax);
      hx =(TH2F*)rootHandler_->BookTH2("HOriginalX",nbz,0.,zmax,nbx,0.,xmax);
    }
  else
    {
      hhtx->Reset();
      hx->Reset();
    }

  for (uint32_t i=0;i<nbz;i++)
    for (uint32_t j=0;j<nbx;j++)
      hx->SetBinContent(i+1,j+1,orig[i][j]*1.);


  for (uint32_t i=0;i<nhtheta;i++)
    for (uint32_t j=0;j<nhr;j++)
      {
	hhtx->SetBinContent(i+1,j+1,ht[i][j]*1.);

		
      }
	
  TCHT->cd(1);
  hhtx->Draw("LEGO2");
  //hw->Draw();
  TCHT->cd(2);
  hx->SetFillColor(3);
  hx->Draw("BOX");
  //hw->Draw();
  // TLine l(0.,candx.Pos(0),50.*2.8,candx.Pos(50*2.8));
  // l.SetLineColor(2);
  // l.Draw("SAME");
  std::vector<TLine*> vline;
  std::map<float,std::pair<float,float>,greater<float> > mline;
  for (uint32_t ith=0;ith<nhtheta;ith++)
    {
      for (uint32_t ir=0;ir<nhr;ir++)
	{
	  if (ht[ith][ir]<5) continue;
	  if (ith>1 && ht[ith-1][ir]>ht[ith][ir]) continue;
	  if (ith>1 && ir>1 && ht[ith-1][ir-1]>ht[ith][ir]) continue;
	  if (ith>1 && ir<nhr-1 && ht[ith-1][ir+1]>ht[ith][ir]) continue;

	  if (ith<nhtheta-1 && ht[ith+1][ir]>ht[ith][ir]) continue;
	  if (ith<nhtheta-1 && ir>1 && ht[ith+1][ir-1]>ht[ith][ir]) continue;
	  if (ith<nhtheta-1 && ir<nhr-1 &&ht[ith+1][ir+1]>ht[ith][ir]) continue;
	  if (ir>1 && ht[ith][ir-1]>ht[ith][ir]) continue;
	  if (ir<nhr-1 && ht[ith][ir+1]>ht[ith][ir]) continue;

	  float th=0;
	  float wt=0;
	  float rb=0;
				
	  int nb=0;
	  if (ith>0 && ir>0 && ith<nhtheta-1 && ir <nhr-1)
	    {
	      for (int ts=-1;ts<=1;ts++)
		for (int rs=-1;rs<=1;rs++)
		  {
		    if (ht[ith+ts][ir+rs]<5) continue;
		    th+=(ith+ts)*ht[ith+ts][ir+rs];
		    rb+=(ir+rs)*ht[ith+ts][ir+rs];
		    wt+=ht[ith+ts][ir+rs];
		    nb++;
		  }
	      th/=wt;
	      rb/=wt;
	    }
	  else
	    {
	      rb=ir+0.5;
	      th=ith+0.5;
	      wt=ht[ith][ir];
	      nb=1;
	    }
	  printf("Line found %f %f (%f) --->%d \n",th,rb,wt,ht[ith][ir]);
	  float theta=(th)*M_PI/nhtheta;
	  float r=rb*rbin+rmin;
	  float a=-1./tan(theta);
	  float b=r/sin(theta);
	  if ((theta<20./180*M_PI) || (theta>160./180.*M_PI)) continue;
	  printf("%f %f ===> %f %f \n",theta,r,a,b);
	  std::pair<float,float> p(a,b);
	  //std::pair<float,std::pair<float,float> > p1(rand()*0.5/RAND_MAX+ht[ith][ir],p);
	  std::pair<float,std::pair<float,float> > p1(wt/nb,p);
	  mline.insert(p1);
				
	}
    }
  uint32_t ngood=0;
  for (std::map<float,std::pair<float,float> >::iterator im=mline.begin();im!=mline.end();im++)
    {
      ngood++;
      if (ngood==15) break;
      printf("%f %f %f \n",im->first,im->second.first,im->second.second);
      TLine* l =new TLine(0.,im->second.second,zmax,im->second.first*zmax+im->second.second);
      l->SetLineColor(2);
      l->Draw("SAME");
      vline.push_back(l);
    }
  TCHT->Modified();
  TCHT->Draw();
  TCHT->Update();

	
	
	
  char c;c=getchar();putchar(c); if (c=='.') exit(0);
  for (std::vector<TLine*>::iterator il=vline.begin();il!=vline.end();il++) delete (*il);
	
	
}



#ifdef USE_OPENCV
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/gpu/gpu.hpp"

using namespace std;
using namespace cv;
using namespace cv::gpu;
#endif
void ShowerAnalyzer::newHT2(array3D<unsigned char> &cores)
{
#ifdef USE_OPENCV
  const uint32_t nbz=60;
  const uint32_t nbx=96;

  const double zmin=0;
  const double zmax=60*2.8;
  const double xmin=0;
  const double xmax=100.;
  const double zbin=(zmax-zmin)/nbz;
  const double xbin=(xmax-xmin)/nbx;

  Mat mask(nbx,nbz,CV_8UC1,Scalar::all(0));
  //std::vector<RecoHit*> vrh;
  //vrh.clear();
  printf("mask created \n");
  uint32_t filter[3][3]={{1,1,1},{1,-8,1},{1,1,1}};
  for (uint32_t k=0;k<60;k++)
    {
      //int32_t weighted[96][96];
      for (uint32_t i=0;i<96;i++)
	for (uint32_t j=0;j<96;j++)
	  if (theImage_.getValue(k,i,j)!=0 && cores.getValue(k,i/3,j/3)==0)
	    {

	      //vrh.push_back(&(hitVolume_[k][i][j]));
						
	      mask.at<uint8_t>(k,j)=255;
	    }
      //getchar();

    }
  // Un essai
  /*
    allpoints_.clear();
    this->PointsBuilder(vrh);
    for (std::vector<RecoPoint>::iterator ip=allpoints_.begin();ip!=allpoints_.end();ip++)
    {
    float z=(*ip).Z();
    float x=(*ip).Y();
    uint32_t iz=int((z-zmin)/zbin);
    uint32_t ix=int((x-xmin)/xbin);
    mask.at<uint8_t>(ix,iz)=255;
    }
    allpoints_.clear();
  */
  Mat dst_cpu;
  cvtColor(mask, dst_cpu, CV_GRAY2BGR);
  Mat dst_gpu = dst_cpu.clone();

  printf("mask filled \n");

  GpuMat d_src(mask);
  printf("mask transfered to device \n");

  GpuMat d_lines;
  HoughLinesBuf d_buf;
  {
    const int64 start = getTickCount();
    printf("Calling HoughLinesP \n");
#define HoughP

#ifdef HoughP
    gpu::HoughLinesP(d_src, d_lines, d_buf, (2*(nbz+nbx)+1)/150.0f, (float) (1*CV_PI / 120.0f),4,10000,8);
		
#else
    //gpu::HoughLines(d_src, d_lines, d_buf, 5.0f, (float) (5*CV_PI / 180.0f),5);
    gpu::HoughLines(d_src, d_lines, d_buf, 1.0f, (float) (1.*CV_PI / 180.0f),7);
#endif
    const double timeSec = (getTickCount() - start) / getTickFrequency();
    cout << "GPU Time : " << timeSec * 1000 << " ms" << endl;
    cout << "GPU Found : " << d_lines.cols << endl;
  }
	
#ifdef HoughP
  vector<Vec4i> lines_gpu;
	
  if (!d_lines.empty())
    {
      lines_gpu.resize(d_lines.cols);
      Mat h_lines(1, d_lines.cols, CV_32SC4, &lines_gpu[0]);
      d_lines.download(h_lines);
    }
  //gpu::HoughLinesDownload(d_lines,lines_gpu);

  for (size_t i = 0; i < lines_gpu.size(); ++i)
    {
      Vec4i l = lines_gpu[i];
      Point a(l[0], l[1]);
      Point b(l[2], l[3]);
      printf("Line %d : %d %d %d %d \n",(int) i,a.x,a.y,b.x,b.y);
		
      line(dst_gpu, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 1, CV_AA);
    }
#else
	
  //Mat h_votes_;
  Vector<Vec2f> h_lines_;
  Vector<int> h_votes_;
  if (!d_lines.empty())
    {
      h_lines_.resize(d_lines.cols);
      Mat m_lines_(1,d_lines.cols,CV_32FC2,&h_lines_[0]);
      h_votes_.resize(d_lines.cols);
      Mat m_votes_(1,d_lines.cols,CV_32SC1,&h_votes_[0]);

      gpu::HoughLinesDownload(d_lines,m_lines_,m_votes_);
    }
  /*
    vector<Vec3f> lines_gpu;
	
    if (!d_lines.empty())
    {
    lines_gpu.resize(d_lines.cols);
    Mat h_lines(2, d_lines.cols, CV_32FC2, &lines_gpu[0]);
    d_lines.download(h_lines);
    }
  */
  for( size_t i = 0; i < h_lines_.size(); i++ )
    {
      float rho = h_lines_[i][0], theta = h_lines_[i][1];
      int vote=h_votes_[i];
      printf("rho %f thehta %f  vote %d \n",rho,theta,vote);
      Point pt1, pt2;
      double a = cos(theta), b = sin(theta);
      double x0 = a*rho, y0 = b*rho;
      pt1.x = cvRound(x0 + 1000*(-b));
      pt1.y = cvRound(y0 + 1000*(a));
      pt2.x = cvRound(x0 - 1000*(-b));
      pt2.y = cvRound(y0 - 1000*(a));
      std::cout<<pt1.x<<" "<<pt1.y<<" "<<pt2.x<<" "<<pt2.y<<" "<<std::endl;
      line( dst_gpu, pt1, pt2, Scalar(0,0,255), 1, CV_AA);
    }
	
#ifdef NOGPU
  vector<Vec2f> lines;
  HoughLines( mask, lines, 1, 1*CV_PI/180, 5 );

  for( size_t i = 0; i < lines.size(); i++ )
    {
      float rho = lines[i][0];
      float theta = lines[i][1];
      printf("rho %f thehta %f \n",rho,theta);
      double a = cos(theta), b = sin(theta);
      double x0 = a*rho, y0 = b*rho;
      Point pt1(cvRound(x0 + 1000*(-b)),
		cvRound(y0 + 1000*(a)));
      Point pt2(cvRound(x0 - 1000*(-b)),
		cvRound(y0 - 1000*(a)));
      line( dst_gpu, pt1, pt2, Scalar(0,0,255), 1, CV_AA );
    }
#endif	
#endif
  //imshow("detected lines [GPU]", dst_gpu);
  //waitKey();
  imshow("detected lines [GPU]", dst_gpu);
  waitKey();

#endif

}

void ShowerAnalyzer::track2Db(std::vector<RecoCandTk> &tracks,std::vector<RecoPoint> &points)
{

	
  if (tracks.size()==0) return;
  for (unsigned int itk=0;itk<tracks.size();itk++)
    {
      RecoCandTk& tk = tracks[itk];
      //DEBUG_PRINT("Tk=%d Time=%d %f %f %f %f %f \n",theTrackIndex_,currentTime_,tk.ax_,tk.bx_,tk.ay_,tk.by_,tk.chi2_);
      uint32_t trackid=this->fillTrackTable(tk.getList().size(),tk.ax_,tk.bx_,tk.ay_,tk.by_,tk.chi2_);

		
      for (std::map<unsigned int,ChamberGeom>::iterator ip=reader_->getChamberMap().begin();ip!=reader_->getChamberMap().end();ip++)
	{
	  // std::cout<<ip->first<<" "<<ip->second.getZ()<<std::endl;
	  //getchar();
	  int chid = ip->first;
	  int32_t interval=3;
	  if (chid<=tkFirstChamber_+1) interval=5;
	  if (chid>=tkLastChamber_-1) interval=5;

	  int32_t tkFirstEx=((chid-interval)>tkFirstChamber_)?(chid-interval):tkFirstChamber_;
	  int32_t tkLastEx=((chid+interval)<tkLastChamber_)?(chid+interval):tkLastChamber_;
	  //std::cout<<chid<<" "<<tkFirstEx<<" "<<tkLastEx<<std::endl;

	  RecoCandTk tk0;
	  for (unsigned int j =0;j<tk.getList().size();j++)
	    {
	      if (tk.getList()[j]->getChamberId()==chid) continue;
				
	      tk0.addPoint(*tk.getList()[j]);
	    }
	  float xext=-999999,yext=-999999;
	  if (useTk4_ && chid>=tkFirstChamber_ && chid<=tkLastChamber_)
	    {
	      RecoCandTk tk4;
	      for (unsigned int j =0;j<tk.getList().size();j++)
		{
		  if (tk.getList()[j]->getChamberId()==chid) continue;
		  if (tk.getList()[j]->getChamberId()<tkFirstEx) continue;
		  if (tk.getList()[j]->getChamberId()>tkLastEx) continue;
		  tk4.addPoint(*tk.getList()[j]);

					
					

		}
	      tk4.regression();
	      if (tk4.getList().size()>=3 && TMath::Prob(tk4.chi2_,2*tk4.getList().size()-4)>tkExtChi2Cut_)
		{
		  xext=tk4.getXext(ip->second.getZ());
		  yext=tk4.getYext(ip->second.getZ());
		}
	      else 
		{
		  xext=-999999;
		  yext=-999999;
		}

	    }
	  if (xext==-999999 || yext==-999999)
	    {
	      tk0.regression();
	      if (tk0.getList().size()<(uint32_t) tkExtMinPoint_ || TMath::Prob(tk0.chi2_,2*tk0.getList().size()-4)<tkExtChi2Cut_) continue;
	      xext=tk0.getXext(ip->second.getZ());
	      yext=tk0.getYext(ip->second.getZ());
	    }
	  double xchext = ip->second.toLocalX(xext);
	  double ychext = ip->second.toLocalY(yext);
	  double zch=ip->second.getZ();
	  //std::cout <<xext<<" " <<yext<<" "<<xchext<<" " <<ychext<< " "<<zch<<" "<<ip->second.getZ()<<std::endl;
	  ip->second.calculateLocal(xext,yext,0,xchext,ychext,zch);
			
	  //std::cout<<xext<<" "<<yext<<std::endl;
	  unsigned int imin=999999;double distmin=9999999;
	  for (unsigned int irp=0;irp<points.size();irp++)
	    {
	      if (points[irp].getChamberId()!=chid) continue;
	      double dist =sqrt((points[irp].X()-xext)*(points[irp].X()-xext)+(points[irp].Y()-yext)*(points[irp].Y()-yext));
	      //std::cout<<chid<<" "<<dist<<std::endl;
	      if (dist<distmin)
		{
		  distmin=dist;
		  imin=irp;
		}
	    }
	  if (imin == 999999 || TMath::Abs(distmin)>5)
	    {
	      if (xext>4 && xext<96 && yext>4 && yext<96)
		this->fillPointTable(trackid,chid,zch,xext,yext,-999.,-999.);
	    }
	  //else
	  //this->fillPointTable(trackid,chid,zch,xext,yext,points[imin].X(),points[imin].Y());
			
	}
    }
		

}

uint32_t ShowerAnalyzer::buildClusters(std::vector<RecoHit*> vrh)
{
  
  std::vector<RECOCluster*> clusters;
  std::vector<RECOCluster*> realc;
  std::vector<RECOCluster*> intc;
  clusters.clear();
  realc.clear();
  intc.clear();
  HoughCut cuts;
  float *h_x=(float *) malloc(1024*sizeof(float));
  float *h_y= (float *) malloc(1024*sizeof(float));
  float *h_z=(float *) malloc(1024*sizeof(float));
  unsigned int *h_layer=(unsigned int *) malloc(1024*sizeof(unsigned int));
  uint32_t nshower=0;
  //ComputerHough ch(&cuts);
  //ch.DefaultCuts();
#ifdef CLUSTER_SIZE_METHOD
  for (std::vector<RecoHit*>::iterator ih=vrh.begin();ih<vrh.end();ih++)
    {
      bool merged=false;
      for (std::vector<RECOCluster*>::iterator ic=clusters.begin();ic!=clusters.end();ic++)
	{
	  if ((*ih)->chamber()!=(*ic)->chamber()) continue;
	  merged=(*ic)->Append((*(*ih)),4.); // avant 4 et normalement 2
	  if (merged) break;
	}
      if (merged) continue;
      RECOCluster* c= new RECOCluster((*(*ih)));
      clusters.push_back(c);
    }
  // Now clear clusters >6
  uint32_t fpi=99,lpi=0;
   for (std::vector<RECOCluster*>::iterator ic=clusters.begin();ic!=clusters.end();ic++)
	{
	  if ((*ic)->getHits()->size()>5) //6 avant 
	    {
	      uint32_t ch=(*ic)->chamber();
	      if (lpi<ch) lpi=ch;
	      if (fpi>ch) fpi=ch;
	      intc.push_back((*ic));
	      continue;
	    }
	  realc.push_back((*ic));
	}
#endif
#define CORE_EDGE_METHOD
#ifdef CORE_EDGE_METHOD
  for (std::vector<RecoHit*>::iterator ih=vrh.begin();ih<vrh.end();ih++)
    {
      if ((*ih)->getFlag(RecoHit::CORE)==1) continue;
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
  uint32_t fpi=99,lpi=0;
  for (std::vector<RecoHit*>::iterator ih=vrh.begin();ih<vrh.end();ih++)
    {
      if ((*ih)->getFlag(RecoHit::CORE)!=1) continue;
      bool merged=false;
      for (std::vector<RECOCluster*>::iterator ic=intc.begin();ic!=intc.end();ic++)
	{
	  if ((*ih)->chamber()!=(*ic)->chamber()) continue;
	  merged=(*ic)->Append((*(*ih)),4.); // avant 4 et normalement 2
	  if (merged) break;
	}
      if (merged) continue;
      RECOCluster* c= new RECOCluster((*(*ih)));
      intc.push_back(c);
      clusters.push_back(c);
    }

  // Move small cluster from intc to realc
  for (std::vector<RECOCluster*>::iterator ic=intc.begin();ic!=intc.end();)
    {
      if ((*ic)->getHits()->size()>=5) 
	++ic;
      else
	{
	  realc.push_back((*ic));
	  intc.erase(ic);
	}
    }
  for (std::vector<RECOCluster*>::iterator ic=realc.begin();ic!=realc.end();)
    {
      if ((*ic)->getHits()->size()<5) 
	++ic;
      else
	{
	  intc.push_back((*ic));
	  realc.erase(ic);
	}
    }
  // Now clear clusters >6

   for (std::vector<RECOCluster*>::iterator ic=intc.begin();ic!=intc.end();ic++)
	{
	      uint32_t ch=(*ic)->chamber();
	      if (lpi<ch) lpi=ch;
	      if (fpi>ch) fpi=ch;
	}

#endif
   
   if (false && (realc.size()*100./vrh.size()>40 || realc.size()*100./clusters.size()<95.) )
     {
       
       for (std::vector<RECOCluster*>::iterator ic=clusters.begin();ic!=clusters.end();ic++)
	 delete (*ic);
       free(h_x);
       free(h_y);
       free(h_z);
       free(h_layer);
       return nshower;
     }
   
   // 2D draw of Hits and real clusters
#define DRAW_HISTOS
#ifdef  DRAW_HISTOS
#define BOOK_HISTOS
#define FILL_HISTOS
#endif
   //#define BOOK_HISTOS

#ifdef BOOK_HISTOS
  
  TH2F* hzx = (TH2F*) rootHandler_->GetTH2("/Clusters/HZXPOS");

  TH2F* hzy = (TH2F*) rootHandler_->GetTH2("/Clusters/HZY");
  TH2F* czx = (TH2F*) rootHandler_->GetTH2("/Clusters/CZX");
  TH2F* czy = (TH2F*) rootHandler_->GetTH2("/Clusters/CZY");
  TH2F* mzx = (TH2F*) rootHandler_->GetTH2("/Clusters/MZX");
  TH2F* mzy = (TH2F*) rootHandler_->GetTH2("/Clusters/MZY");
  TH2F* izx = (TH2F*) rootHandler_->GetTH2("/Clusters/IZX");
  TH2F* izy = (TH2F*) rootHandler_->GetTH2("/Clusters/IZY");
  
  TH1* hest = rootHandler_->GetTH1("/Clusters/EST");
  
  TH1* hest1 = rootHandler_->GetTH1("/Clusters/EST1");
  TH1* hest2 = rootHandler_->GetTH1("/Clusters/EST2");
  TH1* hest3 = rootHandler_->GetTH1("/Clusters/EST3");
  TProfile* hrate=(TProfile*) rootHandler_->GetTH1("/Clusters/RATE");
  TProfile* hspill=(TProfile*) rootHandler_->GetTH1("/Clusters/SPILL");
  if (hest1==NULL)
    {
      printf("Booking\n");
      hest1 =rootHandler_->BookTH1("/Clusters/EST1",1000,0.,3000.);
      hest2 =rootHandler_->BookTH1("/Clusters/EST2",1000,0.,3000.);
      hest3 =rootHandler_->BookTH1("/Clusters/EST3",1000,0.,3000.);
      hrate =rootHandler_->BookProfile("/Clusters/RATE",20,0.,100.,0.,2000.);
      hspill =rootHandler_->BookProfile("/Clusters/SPILL",50,0.,10.,0.,2000.);
  
      hzx =(TH2F*)rootHandler_->BookTH2("/Clusters/HZXPOS",120,0.,170.,120,-10.,110.);


      hzy =(TH2F*)rootHandler_->BookTH2("/Clusters/HZY",120,0.,170.,120,-10.,110.);
    
      czy =(TH2F*)rootHandler_->BookTH2("/Clusters/CZY",120,0.,170.,120,-10.,110.);
      czx =(TH2F*)rootHandler_->BookTH2("/Clusters/CZX",120,0.,170.,120,-10.,110.);
      mzy =(TH2F*)rootHandler_->BookTH2("/Clusters/MZY",120,0.,170.,120,-10.,110.);
      mzx =(TH2F*)rootHandler_->BookTH2("/Clusters/MZX",120,0.,170.,120,-10.,110.);
      izy =(TH2F*)rootHandler_->BookTH2("/Clusters/IZY",120,0.,170.,120,-10.,110.);
      izx =(TH2F*)rootHandler_->BookTH2("/Clusters/IZX",120,0.,170.,120,-10.,110.);
      hest = rootHandler_->BookTH1("/Clusters/EST",120,-0.1,1.1);

      
    }
  
  else
    {
      
  

     
      hzx->Reset();
     hzy->Reset();
     czy->Reset();
      czx->Reset();
      izy->Reset();
      izx->Reset();
      mzy->Reset();
      mzx->Reset();
     
    }
   
#endif
#ifdef FILL_HISTOS 
  for (std::vector<RecoHit*>::iterator ih=vrh.begin();ih<vrh.end();ih++)
    {
      hzx->Fill((*ih)->Z(),(*ih)->X());
      hzy->Fill((*ih)->Z(),(*ih)->Y());
    }
#endif

  uint32_t nstub=0;
  for (std::vector<RECOCluster*>::iterator ic=realc.begin();ic!=realc.end();ic++)
    {
#ifdef FILL_HISTOS
      czx->Fill((*ic)->Z(),(*ic)->X());
      czy->Fill((*ic)->Z(),(*ic)->Y());
#endif
      h_x[nstub]=(*ic)->X();
      h_y[nstub]=(*ic)->Y();
      h_z[nstub]=(*ic)->Z();
      h_layer[nstub]=(*ic)->chamber();
      nstub++;
    }
  theComputerHough_->associate(nstub,h_x,h_y,h_z,h_layer);
 
  uint32_t nmip=0;
   for (unsigned int i=0;i<theComputerHough_->getCandidates().size();i++)
	{
	  RecoCandTk& tk = theComputerHough_->getCandidates()[i];
	  tk.ax_/=1.04125;
	  tk.bx_/=1.04125;
	  tk.ay_/=1.04125;
	  tk.by_/=1.04125;
	  //printf("Track cand tk: %f %f %f %f \n",tk.ax_,tk.bx_,tk.ay_,tk.by_);

	  for (std::vector<RECOCluster*>::iterator ic=realc.begin();ic!=realc.end();ic++)
	    {
	      float dx=tk.getXext((*ic)->Z())-(*ic)->X();
	      float dy=tk.getYext((*ic)->Z())-(*ic)->Y();
	      if (sqrt(dx*dx+dy*dy)<2.)
		{
		  //printf("Point (%f,%f,%f) dist %f %f -> %f  \n",(*ic)->X(),(*ic)->Y(),(*ic)->Z(),dx,dy,sqrt(dx*dx+dy*dy));
		for (std::vector<RecoHit>::iterator ih=(*ic)->getHits()->begin();ih!=(*ic)->getHits()->end();ih++)
		  {
		    if ((*ih).getFlag(RecoHit::MIP)==0)
		      {
			(*ih).setFlag(RecoHit::MIP,true);
			mzx->Fill(ih->Z(),ih->X());
			mzy->Fill(ih->Z(),ih->Y());
			nmip++;
		      }
		  }
		}
	    }
	}
   printf("==> MIPS hit %d -> %.2f\n",nmip,nmip*100./vrh.size()); 
   bool electron=(theComputerHough_->Length()<5. || nmip*100./vrh.size()<1);
   bool muon=nmip*100./vrh.size()>60.;
   if (electron || muon) 
    {
       for (std::vector<RECOCluster*>::iterator ic=clusters.begin();ic!=clusters.end();ic++)
	 delete (*ic);
       free(h_x);
       free(h_y);
       free(h_z);
       free(h_layer);
       return nshower;
    }
  //#undef DRAW_HISTOS  
  /*
  ch.ComputeOneShot(nstub,h_x,h_y,h_z,h_layer);
  std::vector<RecoCandTk> &v=ch.getCandidates();
  for (std::vector<RecoCandTk>::iterator it=v.begin();it!=v.end();it++)
    printf("%f %f %f %f \n",it->ax_,it->bx_,it->ay_,it->by_);
  */
  //drawph(ch.getPh());
  std::vector<Amas> theAmas_;
  theAmas_.clear();

  for (std::vector<RECOCluster*>::iterator ic=intc.begin();ic!=intc.end();ic++)
    {
      for (std::vector<RecoHit>::iterator ih=(*ic)->getHits()->begin();ih!=(*ic)->getHits()->end();ih++)
	{
#ifdef FILL_HISTOS
	  izx->Fill(ih->Z(),ih->X());
	  izy->Fill(ih->Z(),ih->Y());
#endif
	  RecoHit& h=(*ih);
	  bool appended=false;
	  for (std::vector<Amas>::iterator ia=theAmas_.begin();ia!=theAmas_.end();ia++)
	    {
	      appended=(ia->append(&h,2));
	      if (appended) break;
	    }
	  if (!appended)
	    {
	      Amas a(&h);
	      theAmas_.push_back(a);
	    }
	}
    }

#ifdef DRAW_HISTOS
  bool doPlot=int(hest1->GetEntries())%30==40;
  doPlot=false;
  if (doPlot)
    {
  if (TCCluster==NULL)
    {
      TCCluster=new TCanvas("TCCluster","hugh",1000,900);
      TCCluster->Modified();
      TCCluster->Draw();
      TCCluster->Divide(3,2);
    }
  hzx->SetLineColor(kRed);
  hzy->SetLineColor(kRed);
  czx->SetLineColor(kBlue);
  czy->SetLineColor(kBlue);
  czx->SetLineColor(kGreen);
  czy->SetLineColor(kGreen);
  mzx->SetMarkerColor(kBlack);
  mzy->SetMarkerColor(kBlack);
  mzx->SetMarkerStyle(21);
  mzy->SetMarkerStyle(21);

  TCCluster->cd(1);
  hzx->Draw("BOX");
  TCCluster->cd(2);
  czx->Draw("BOX");
  TCCluster->cd(3);
  izx->Draw("BOX");
  mzx->Draw("BOXSAME");
  TCCluster->cd(4);
  hzy->Draw("BOX");
  TCCluster->cd(5);
  czy->Draw("BOX");
  TCCluster->cd(6);
  izy->Draw("BOX");
  mzy->Draw("BOXSAME");
    }
#endif
  std::sort(theAmas_.rbegin(),theAmas_.rend());
  uint32_t nag=0;
  TH2F* amx[100];
  TH2F* amy[100];
  Components* c;
  uint32_t ifi=9999,ili=0;
  for (std::vector<Amas>::iterator ia=theAmas_.begin();ia!=theAmas_.end();ia++)
    {
      ia->compute();
      if (ia->getVolume()==0) continue;
      c=(Components*) ia->Components();
      printf("%f %f %d %d \n",c->zmin,c->zmax,int(c->zmin/2.8),int(c->zmax/2.8));
      if (int(c->zmin/2.8)+1<ifi) ifi =int(c->zmin/2.8)+1;
      if (int(c->zmax/2.8)+1>ili) ili =int(c->zmax/2.8)+1;
#ifdef DRAW_HISTOS
      std::stringstream sn;
      sn<<"amasxz"<<nag;
      amx[nag]=new TH2F(sn.str().c_str(),sn.str().c_str(),120,0.,170.,120,-10.,110.);
      for (std::vector<RecoHit*>::iterator iha=ia->getHits().begin(); iha!=ia->getHits().end();iha++)
	{
	  amx[nag]->Fill((*iha)->Z(),(*iha)->X());
	}
      sn<<"y";
      amy[nag]=new TH2F(sn.str().c_str(),sn.str().c_str(),120,0.,170.,120,-10.,110.);
      for (std::vector<RecoHit*>::iterator iha=ia->getHits().begin(); iha!=ia->getHits().end();iha++)
	{
	  amy[nag]->Fill((*iha)->Z(),(*iha)->Y());
	}
      if (doPlot)
	{
      amx[nag]->SetLineColor(nag+1);
      amy[nag]->SetLineColor(nag+1);
      TCCluster->cd(3);
      if (nag==0)
	amx[nag]->Draw("BOX");
      else
	amx[nag]->Draw("BOXSAME");
      TCCluster->cd(6);
      if (nag==0)
	amy[nag]->Draw("BOX");
      else
	amy[nag]->Draw("BOXSAME");
	}
      nag++;
#endif
      // ia->Print();
    }
#ifdef DRAW_HISTOS
  TLine* l[100];
  TLine* l1[100];
  uint32_t nline=0;
  if (doPlot)
    {
  TH1F* hweight=(TH1F*) rootHandler_->GetTH1("showerweight");
  TH1F* hweight2=(TH1F*) rootHandler_->GetTH1("showerweight2");
  
  TCCluster->cd(6);
  //mzy->Draw("BOX");
  //if (hest1!=0) hest1->Draw();
  // if (hweight2!=NULL) hweight2->Draw("text");


  
  
  for (unsigned int i=0;i<theComputerHough_->getCandidates().size();i++)
	{
	  RecoCandTk& tk = theComputerHough_->getCandidates()[i];
	  DEBUG_PRINT("%f %d \n",tk.chi2_,tk.getList().size());
	  TCCluster->cd(3);
	  l[i] = new TLine(tk.zmin_,tk.getXext(tk.zmin_),tk.zmax_,tk.getXext(tk.zmax_));
	  l[i]->SetLineColor(i+1);
	  l[i]->SetLineWidth(2);
	  l[i]->Draw("SAME");
	  TCCluster->cd(6);
	  l1[i] = new TLine(tk.zmin_,tk.getYext(tk.zmin_),tk.zmax_,tk.getYext(tk.zmax_));
	  l1[i]->SetLineColor(i+1);
	  l1[i]->SetLineWidth(2);
	  l1[i]->Draw("SAME");
	  nline++;
	}
  TCCluster->cd(3);
  mzx->SetMarkerSize(0.5);
  mzy->SetMarkerSize(0.5);
  mzx->Draw("SAME");
  TCCluster->cd(6);
  mzy->Draw("SAME");

    }
  if (hest!=NULL) hest->Fill(nmip*1./vrh.size());
  hest1->Fill(1.*vrh.size());
  hrate->Fill(theLastRate_,1.*vrh.size());
  hspill->Fill((theBCID_-currentTime_-theBCIDSpill_)*2E-7,1.*vrh.size());
  if (theLastRate_<20.) 
    hest2->Fill(1.*vrh.size());
  else
    hest3->Fill(1.*vrh.size());
  theNbShowers_++;
  nshower++;
#endif

  //  hest1->Draw();
  INFO_PRINT("Hits %ld  Clusters (%ld,%.2f)  REAL (%ld,%.2f,%.2f) FP %d %d LP %d %d \n",vrh.size(),clusters.size(),clusters.size()*100./vrh.size(),realc.size(),realc.size()*170/vrh.size(),realc.size()*100./clusters.size(),fpi,ifi,lpi,ili);
#ifdef DRAW_HISTOS
  if (doPlot)
    {
      TCCluster->Modified();
      TCCluster->Update();
      //TCCluster->WaitPrimitive();
      char ci;ci=getchar();putchar(ci); if (ci=='.') exit(0);
    }
#endif
   for (std::vector<RECOCluster*>::iterator ic=clusters.begin();ic!=clusters.end();ic++)
     delete (*ic);
#ifdef DRAW_HISTOS
   
   for (unsigned int i=0;i<nline;i++)
	{
	  if (doPlot)
	    {
	      delete l[i];
	      delete l1[i];
	    }
	}
   for (uint32_t i=0;i<nag;i++)
     {
       delete amx[i];
       delete amy[i];

     }
#endif
   free(h_x);
   free(h_y);
   free(h_z);
   free(h_layer);
   return nshower;
}
static TCanvas* CanvasGPU=NULL;
void ShowerAnalyzer::drawph(houghParam* p)
{

  unsigned int h_hough[p->ntheta*p->nrho];

  copyHoughImageCPU(p,h_hough);

  //TH2F* g_hough=(TH2F*) rootHandler_->GetTH2("GPUHough");
  printf("drawph==> %d %f %f %d %f %f \n",p->ntheta,p->thetamin,p->thetamax,p->nrho,p->rmin,p->rmax);
  TH2F* g_hough=new TH2F("GPUHough","GPUHough",p->ntheta,p->thetamin,p->thetamax,p->nrho,p->rmin,p->rmax);
  for (int ith=0;ith<p->ntheta;ith++)
    for (int ir=0;ir<p->nrho;ir++)
      if (h_hough[ith*p->nrho+ir]!=0)
	g_hough->SetBinContent(ith+1,ir+1,h_hough[ith*p->nrho+ir]*1.);
  if (CanvasGPU==NULL)
    {
      CanvasGPU=new TCanvas("CanvasGPU","hough",800,900);
      CanvasGPU->Modified();
      CanvasGPU->Draw();
    }
  CanvasGPU->cd();
  g_hough->Draw("COLZ");
  CanvasGPU->Modified();
  CanvasGPU->Draw();
  //CanvasGPU->WaitPrimitive();
  
  CanvasGPU->Update();
  //CanvasGPU->WaitPrimitive();
  char c;c=getchar();putchar(c); if (c=='.') exit(0);
  delete g_hough;
}
#define BINFACT 3
#define BINEDGE 96/BINFACT
void ShowerAnalyzer::buildEdges()
{
#ifdef DRAW_HISTOS  
  TH1* hweight= rootHandler_->GetTH1("showerweight");
  TH1* hweight2= rootHandler_->GetTH1("showerweight2");
  if (hweight==NULL)
    {
      //hweight=(TH1F*) rootHandler_->BookTH1("showerweight",100,0.,2.);
      hweight= rootHandler_->BookTH1("showerweight",100,-0.1,0.9);
      hweight2= rootHandler_->BookTH1("showerweight2",120,-0.1,1.1);
    }
  hweight->Reset();
#endif
  uint32_t nmax=0;
  uint32_t nedge=0,ncore=0,niso=0;
  for (uint32_t k=0;k<theImage_.getXSize();k++)
    {

      for (uint32_t i=0;i<theImage_.getYSize();i++)
	{
	  //if (image2x[k][i]>=-1 ) continue;
			
	  for (uint32_t j=0;j<theImage_.getZSize();j++)
	    {
	      if (theImage_.getValue(k,i,j)>0)
		{
		  int izmin=-2;
		  int izmax=+2;
		  if (k==0) {izmin=0;izmax=4;}
		  if (k==1) {izmin=0;izmax=4;}
		  uint32_t nv=0;
		  std::vector<RecoHit*> vnear_;
		  RecoHit* h0=&hitVolume_[k][i][j];
		  for (int z=izmin;z<=izmax;z++)
		    {
		      if (z+k<0) continue;
		      if (z+k>=theImage_.getXSize()) continue;
		      for (int x=-6;x<=6;x++)
			{
			  if (x+i<0) continue;
			  if (x+i>=theImage_.getYSize()) continue;
			for (int y=-6;y<=6;y++)
			  {
			    if (y+j<0) continue;
			    if (y+j>=theImage_.getZSize()) continue;
			    if (theImage_.getValue(k+z,i+x,j+y)<=0) continue;
			    RecoHit* h1=&hitVolume_[k+z][i+x][j+y];
			    float x0=h0->X(),y0=h0->Y(),z0=h0->Z(),x1=h1->X(),y1=h1->Y(),z1=h1->Z();
			    float dist=sqrt((x1-x0)*(x1-x0)+(y1-y0)*(y1-y0)+(z1-z0)*(z1-z0));
			    if (dist<6.) vnear_.push_back(h1);

			    nv++;
			  }
			}
		    }
		  if (nv>nmax) nmax=nv;
		  hitVolume_[k][i][j].calculateComponents(vnear_);

		  Components* c=(Components*) hitVolume_[k][i][j].Components();
		  double w=0;
		  if (c->l2!=0) w=sqrt((c->l1)/c->l2);
#ifdef DRAW_HISTOS
		  hweight->Fill(w);
#endif
		  if (c->l2==0)
		    {hitVolume_[k][i][j].setFlag(RecoHit::ISOLATED,true);niso++;}
		  else 
		    if (w<0.3) // 0.4 before  
		      {hitVolume_[k][i][j].setFlag(RecoHit::EDGE,true);nedge++;}
		    else
		      {hitVolume_[k][i][j].setFlag(RecoHit::CORE,true);ncore++;}
		}

	    }
	}
	  
    }
#ifdef DRAW_HISTOS
  if (nedge+ncore!=0)
    hweight2->Fill(nedge*1./(nedge+ncore));
#endif
  
  //printf("Max neigh %d Hit summary Isolated %d Edge %d Core %d => %.2f\n",nmax,niso,nedge,ncore,(nedge*100./(nedge+ncore)));
  return;

#ifdef OLD_EDGE_METHOD
  // Initialisation
  unsigned char bufv[60*BINEDGE*BINEDGE];
  array3D<unsigned char> imagev;
  imagev.initialise(bufv,60,BINEDGE,BINEDGE); imagev.clear();
  //printf("A\n");
  float bufi[60*BINEDGE*BINEDGE];
  array3D<float> image3;
  image3.initialise(bufi,60,BINEDGE,BINEDGE); image3.clear();
  //printf("B\n");
  unsigned char bufc[60*BINEDGE*BINEDGE];
  array3D<unsigned char> cores;
  cores.initialise(bufc,60,BINEDGE,BINEDGE); cores.clear();
  //printf("C\n");
  unsigned char bufe[60*BINEDGE*BINEDGE];
  array3D<unsigned char> edges;
  edges.initialise(bufe,60,BINEDGE,BINEDGE); edges.clear();
  //printf("D\n");
  unsigned char adjBuf[60*BINEDGE*BINEDGE];
  array3D<unsigned char> adj;
  adj.initialise(adjBuf,60,BINEDGE,BINEDGE);adj.clear();
  //  printf("E %d %d %d \n",image3.getXSize(),image3.getYSize(),image3.getZSize());
  // Fill table imagev

  for (uint32_t k=0;k<theImage_.getXSize();k++)
    {
      for (uint32_t i=0;i<theImage_.getYSize();i++)
	{
	  //if (image2x[k][i]>=-1 ) continue;
			
	  for (uint32_t j=0;j<theImage_.getZSize();j++)
	    {
	      if (theImage_.getValue(k,i,j)>0)
		{
		  int I=i/BINFACT,J=j/BINFACT;

		  imagev.setValue(k,i/BINFACT,j/BINFACT,1);
		  //printf("Setting image I,J %d,%d %d \n",I,J,imagev.getValue(k,I,J));
		}
	      image3.setValue(k,i/BINFACT,j/BINFACT,0);
	    }
	}
	  
    }
  //printf("F\n");
  ShowerAnalyzer::sobel_volume(imagev,image3);
  //printf("G\n");


  //uint32_t 

  for (uint32_t k=0;k<theImage_.getXSize();k++)
    {
      for (uint32_t i=0;i<theImage_.getYSize();i++)
	{
	  //if (image2x[k][i]>=-1 ) continue;
			
	  for (uint32_t j=0;j<theImage_.getZSize();j++)
	    {
	      //if (image2y[k][j]>=-1 ) continue;
	      //if (theImage_.getValue(k,i,j)==0) continue;
	      int I=i/BINFACT,J=j/BINFACT;
	      //	      printf("Compressed image I,J %d,%d %d \n",I,J,imagev.getValue(k,I,J));
	      if (imagev.getValue(k,I,J)==0) continue;
	      if (image3.getValue(k,I,J)<=-20)
		{
		  cores.setValue(k,I,J,0);
		  edges.setValue(k,I,J,1);
	      
		  if (image3.getValue(k,I,J)<-25)
		    edges.setValue(k,I,J,2);

		}
	      else
		{
		  //if (image3.getValue(k,I,J)==0) printf("%d %d %d ZEROOOOOOOOOOOOOOOOOOO\n",k,i,j);
		    cores.setValue(k,I,J,1);
		    edges.setValue(k,I,J,0);
		}
	      
	    }
	}
    }
  //getchar();
  //#define ADDAGE
#ifdef ADDAGE
  for (uint32_t k=1;k<imagev.getXSize()-1;k++)
    {
      for (uint32_t i=1;i<imagev.getYSize()-1;i++)
	{
	  //if (image2x[k][i]>=-1 ) continue;
			
	  for (uint32_t j=1;j<imagev.getZSize()-1;j++)
	    {
	      if (edges.getValue(k,i,j)==0) continue;
				
	      for (uint32_t ks=k-1;ks<=k+1;ks++)
		{
		  for (uint32_t is=i-1;is<=i+1;is++)
		    {
		      for (uint32_t js=j-1;js<=j+1;js++)
			{
			  if (cores.getValue(ks,is,js)>0)
			    {
			      /*adj.setValue(ks,is,js,1);
				edges.setValue(k,i,j,0);*/

			      adj.setValue(k,i,j,1);
			      edges.setValue(k,i,j,0);
			      
			      break;
			    }
			}
		    }
		}
				
	    }
	}
    }
  // Merge the adjacents

  for (uint32_t k=1;k<imagev.getXSize()-1;k++)
    {
      for (uint32_t i=1;i<imagev.getYSize()-1;i++)
	{
	  //if (image2x[k][i]>=-1 ) continue;
			
	  for (uint32_t j=1;j<imagev.getZSize()-1;j++)
	    {
	      if (adj.getValue(k,i,j)>0)
		cores.setValue(k,i,j,1);
	    
	    }
	}
    }

#endif
  // Now tag the hits_
   
  for (uint32_t k=1;k<theImage_.getXSize();k++)
    {
      for (uint32_t i=0;i<theImage_.getYSize();i++)
	{
	  //if (image2x[k][i]>=-1 ) continue;
			
	  for (uint32_t j=0;j<theImage_.getZSize();j++)
	    {
	      if (theImage_.getValue(k,i,j)<=0) continue;
	      hweight->Fill(image3.getValue(k,i/BINFACT,j/BINFACT));
	      hitVolume_[k][i][j].setFlag(RecoHit::CORE,false);
	      hitVolume_[k][i][j].setFlag(RecoHit::EDGE,false);
	      hitVolume_[k][i][j].setFlag(RecoHit::ISOLATED,false);
	      if (cores.getValue(k,i/BINFACT,j/BINFACT)==1)
		hitVolume_[k][i][j].setFlag(RecoHit::CORE,true);
	      else
		{
		  hitVolume_[k][i][j].setFlag(RecoHit::EDGE,true);
		  if (edges.getValue(k,i/BINFACT,j/BINFACT)==2) hitVolume_[k][i][j].setFlag(RecoHit::ISOLATED,true);
		}
	      //printf("(%d,%d,%d) weigth %f CORE ? %d EDGE %d \n",k,i,j,image3.getValue(k,i/BINFACT,j/BINFACT),hitVolume_[k][i][j].getFlag(RecoHit::CORE),hitVolume_[k][i][j].getFlag(RecoHit::EDGE));
	    }
	}
	  
    }
#endif
}
