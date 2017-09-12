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
#include <sys/time.h>

#include "TStripCluster.hh"


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
	  if(_geo->difInfo(d->getID()).type.compare("HR2BIF")!=0) continue;
      
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
          
		  uint32_t chid = _geo->difInfo(dc->getID()).chamber;
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
		      if (chid==57)
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

trackAnalysis::trackAnalysis() :trackIndex_(0),nAnalyzed_(0),clockSynchCut_(8), spillSize_(90000),maxHitCount_(500000),minHitCount_(2),
				tkMinPoint_(3),tkExtMinPoint_(3),tkBigClusterSize_(32),tkChi2Cut_(0.01),tkDistCut_(5.),tkExtChi2Cut_(0.01),tkExtDistCut_(10.),tkAngularCut_(20.),zLastAmas_(134.),
				findTracks_(true),dropFirstSpillEvent_(false),useSynchronised_(true),chamberEdge_(5.),rebuild_(false),oldAlgo_(true),collectionName_("DHCALRawHits1"),
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
	{
	  std::cout<<it->second.getStringValue()<<std::endl;
	  _geo=new jsonGeo(it->second.getStringValue());
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

void trackAnalysis::debora(std::vector<recoPoint*> vp)
{
  // Build map of plan clusters
  std::map<uint32_t,std::vector<recoPoint*> > plans;
  for (std::vector<recoPoint*>::iterator ip=vp.begin();ip!=vp.end();ip++)
    {
      (*ip)->setUse(false);
      std::map<uint32_t,std::vector<recoPoint*> >::iterator ipl=plans.find((*ip)->plan());
      if (ipl==plans.end())
	{
	  std::vector<recoPoint*> vp;
	  vp.push_back((*ip));
	  std::pair< uint32_t,std::vector<recoPoint*> > p((*ip)->plan(),vp);
	  plans.insert(p);
	}
      else
	ipl->second.push_back((*ip));
    }
  
  // Make tracks with 3 outer points 987 98 97 87
  std::vector<recoTrack> vseed;
  for (std::vector<recoPoint*>::iterator ip=vp.begin();ip!=vp.end();ip++)
    {
      if ((*ip)->plan()!=9) continue;
      if ((*ip)->isUsed()) continue;
      for (std::vector<recoPoint*>::iterator jp=vp.begin();jp!=vp.end();jp++)
	{
	  if ((*jp)->plan()!=8) continue;
	  if ((*jp)->isUsed()) continue;
	  for (std::vector<recoPoint*>::iterator kp=vp.begin();kp!=vp.end();kp++)
	    {
	      if ((*kp)->plan()!=7) continue;
	      if ((*kp)->isUsed()) continue;
	      recoTrack t;
	      t.addPoint((*ip));
	      t.addPoint((*jp));
	      t.addPoint((*kp));
	      t.regression();
	      t.getChi2(vp);
	      //std::cout<<" candidate "<<t.pchi2()<<std::endl;
	      if (t.pchi2()<1E-3) continue;
	      (*ip)->setUse(true);
	      (*jp)->setUse(true);
	      (*kp)->setUse(true);
	      vseed.push_back(t);
	    }
	  
	}
    }
  if (vseed.size()==0)
    {
      //9-7
      for (std::vector<recoPoint*>::iterator ip=vp.begin();ip!=vp.end();ip++)
	{
	  if ((*ip)->plan()!=9) continue;
	  if ((*ip)->isUsed()) continue;
	  for (std::vector<recoPoint*>::iterator jp=vp.begin();jp!=vp.end();jp++)
	    {
	      if ((*jp)->plan()!=7) continue;
	      if ((*jp)->isUsed()) continue;
	      recoTrack t;
	      t.addPoint((*ip));
	      t.addPoint((*jp));
	      t.regression();
	      //if (t.pchi2()<5E-3) continue;
	      (*ip)->setUse(true);
	      (*jp)->setUse(true);
	      vseed.push_back(t);

	  
	    }
	}

    }
  if (vseed.size()==0)
    {
      //9-8
      for (std::vector<recoPoint*>::iterator ip=vp.begin();ip!=vp.end();ip++)
	{
	  if ((*ip)->plan()!=9) continue;
	  if ((*ip)->isUsed()) continue;
	  for (std::vector<recoPoint*>::iterator jp=vp.begin();jp!=vp.end();jp++)
	    {
	      if ((*jp)->plan()!=8) continue;
	      if ((*jp)->isUsed()) continue;
	      recoTrack t;
	      t.addPoint((*ip));
	      t.addPoint((*jp));
	      t.regression();
	      //if (t.pchi2()<5E-3) continue;
	      (*ip)->setUse(true);
	      (*jp)->setUse(true);
	      vseed.push_back(t);

	  
	    }
	}

    }
  if (vseed.size()==0)
    {
      //8-7
      for (std::vector<recoPoint*>::iterator ip=vp.begin();ip!=vp.end();ip++)
	{
	  if ((*ip)->plan()!=8) continue;
	  if ((*ip)->isUsed()) continue;
	  for (std::vector<recoPoint*>::iterator jp=vp.begin();jp!=vp.end();jp++)
	    {
	      if ((*jp)->plan()!=7) continue;
	      if ((*jp)->isUsed()) continue;
	      recoTrack t;
	      t.addPoint((*ip));
	      t.addPoint((*jp));
	      t.regression();
	      //if (t.pchi2()<5E-3) continue;
	      (*ip)->setUse(true);
	      (*jp)->setUse(true);
	      vseed.push_back(t);

	  
	    }
	}

    }
  
  // Extrapolate to plan 3 and 2 , found nearest hit<10 cm or fill missing
  //std::cout<<"9-8-7 Seeds found "<<vp.size()<<" gives " <<vseed.size()<<std::endl;
  //getchar();
  if (vseed.size()!=1) return;
  recoTrack& p789=vseed[0];
  if (abs(p789.dir().X())>0.4) return;
  if (abs(p789.dir().Y())>0.4) return;
  //2-3
  std::vector<recoTrack> v23;
  for (std::vector<recoPoint*>::iterator ip=vp.begin();ip!=vp.end();ip++)
    {
      if ((*ip)->plan()!=3) continue;
      if ((*ip)->isUsed()) continue;
      for (std::vector<recoPoint*>::iterator jp=vp.begin();jp!=vp.end();jp++)
	{
	  if ((*jp)->plan()!=2) continue;
	  if ((*jp)->isUsed()) continue;
	  recoTrack t;
	  t.addPoint((*ip));
	  t.addPoint((*jp));
	  t.regression();
	  //if (t.pchi2()<5E-3) continue;
	  (*ip)->setUse(true);
	  (*jp)->setUse(true);
	  v23.push_back(t);
      
      
	}
    }

  recoTrack p23;
  if (v23.size()>=1)
    p23=v23[0];
  else
    return;

  
  TH1* hmin3= rootHandler_->GetTH1("/debora/dmin3");
  TH2* hdist3= rootHandler_->GetTH2("/debora/dist3");
  TH2* hdist2= rootHandler_->GetTH2("/debora/dist2");
  TH2* hmiss3= rootHandler_->GetTH2("/debora/miss3");
  TH2* hext3= rootHandler_->GetTH2("/debora/ext3");
  TH2* hprob= rootHandler_->GetTH2("/debora/probpos");
  TH2* hgood= rootHandler_->GetTH2("/debora/good");
  if (hmin3==NULL)
    {
      hmin3 =rootHandler_->BookTH1("/debora/dmin3",100,0.,20.);
      hdist3= rootHandler_->BookTH2("/debora/dist3",100,-10.,10.,100,-10.,10.);
      hdist2= rootHandler_->BookTH2("/debora/dist2",100,-10.,10.,100,-10.,10.);
      hmiss3= rootHandler_->BookTH2("/debora/miss3",60,0.,60.,50,0.,50.);
      hext3= rootHandler_->BookTH2("/debora/ext3",60,0.,60.,50,0.,50.);
      hprob= rootHandler_->BookTH2("/debora/probpos",60,0.,60.,50,0.,50.);
      hgood= rootHandler_->BookTH2("/debora/good",60,0.,60.,50,0.,50.);
    }
  Json::Value jc3=_geo->chamberGeo(3);
  Json::Value jc2=_geo->chamberGeo(2);
  Json::Value jc5=_geo->chamberGeo(5);
  Json::Value jc7=_geo->chamberGeo(7);

  std::map<uint32_t,std::vector<recoPoint*> >::iterator ipl3=plans.find(3);
  std::map<uint32_t,std::vector<recoPoint*> >::iterator ipl2=plans.find(2);


  for (std::vector<recoTrack>::iterator it=vseed.begin();it!=vseed.end();it++)
    {
      //if (it->size()<3) continue;

      ROOT::Math::XYZPoint po3=it->extrapolate(jc3["z0"].asFloat());
      ROOT::Math::XYZPoint po5=it->extrapolate(jc5["z0"].asFloat());
      ROOT::Math::XYZPoint po2=it->extrapolate(jc2["z0"].asFloat());


      if (ipl2!=plans.end())
      for (std::vector<recoPoint*>::iterator ip=ipl2->second.begin();ip!=ipl2->second.end();ip++)
	{
	  recoPoint& pi2=(*(*ip));
	  ROOT::Math::XYZVector d2=po2-pi2;
	  double rd2=sqrt(d2.Mag2());
	  hdist2->Fill(d2.X(),d2.Y());
	}


      double dmin=1E30;
      if (ipl3!=plans.end())
      for (std::vector<recoPoint*>::iterator ip=ipl3->second.begin();ip!=ipl3->second.end();ip++)
	{
	  recoPoint& pi3=(*(*ip));
	  ROOT::Math::XYZVector d3=po3-pi3;
	  double rd3=sqrt(d3.Mag2());
	  if (rd3 < dmin) dmin=rd3;
	  hdist3->Fill(d3.X(),d3.Y());
	}
      //std::cout<<"dmin"<<dmin<<" "<<hext3<<" "<<hmiss3<<" "<<v23.size()<<std::endl;
      if (dmin<15)
	hext3->Fill(po5.X(),po5.Y());
      // else
      // 	hmiss3->Fill(po5.X(),po5.Y());


      if (v23.size()==0) continue;

      double dist;
      ROOT::Math::XYZPoint p1,p2;
      it->cap(p23,dist,p1,p2);

      if (p1.Z()<=jc3["z0"].asFloat() || p1.Z()>=jc7["z0"].asFloat() ) continue;


      if (ipl3==plans.end()) {
	hmiss3->Fill(po3.X(),po3.Y());
	continue;
      }
      hmin3->Fill(dmin);
      if (dmin>15)
	{
	  hmiss3->Fill(po5.X(),po5.Y());
	}
      else
	if (dmin>1.4)
	  {
	    double prob=1.-erf(dmin/2./1.4);
	    std::cout<<"dmin "<<dmin<<" ->"<<prob<<" "<<abs(log(prob))<<std::endl;
	    if (prob<1E-20) prob=1E-20;
	    if (prob<0.1)
	      hprob->Fill(p1.X(),p1.Y(),abs(log(prob)));
	    else
	      hgood->Fill(p1.X(),p1.Y(),1);
	  }

    }

  
  //-> Fill  missing

  //-> fill Proba

  
}

void trackAnalysis::processSeed(IMPL::LCCollectionVec* rhcol,uint32_t seed)
{
  
  //printf("On entre %s\n",__PRETTY_FUNCTION__);
  //if (seed<10 || seed>20) return;
  ptime("Enter");
  _tcl.clear();
  for (std::vector<planeCluster*>::iterator ic=allClusters_.begin();ic!=allClusters_.end();ic++)
    delete (*ic);
  allClusters_.clear();
  currentTime_=seed;
  
  theAbsoluteTime_=theBCID_-currentTime_;
  if (theBCIDSpill_==0) theBCIDSpill_=theAbsoluteTime_;
  if (theAbsoluteTime_-theBCIDSpill_>theSpillLength_/2E-7) theBCIDSpill_=theAbsoluteTime_;
  
  int nhits=0;
  theNplans_=0;
  uint32_t tag=0;
  std::bitset<60> chhit(0);
  std::map<uint32_t,std::vector<IMPL::RawCalorimeterHitImpl*> >::iterator iseed=reader_->getPhysicsEventMap().find(seed);
  if (iseed==reader_->getPhysicsEventMap().end()) 
    {
      INFO_PRINT("Impossible \n");
      return ;
    }
  // Cerenkov Analysis ?
  if (_geo->cuts()["cerenkovAnalysis"].asUInt()!=0)
    {
      theCerenkovTag_=this->CerenkovTagger(3,seed);
      tag=theCerenkovTag_;
  
      // printf("Cerenkov %d %d \n",seed,theCerenkovTag_);
    }
  
  ptime("Init");
  theNplans_=this->fillVolume(seed);
  
  ptime("fillVolume");
  if (theNplans_<_geo->cuts()["minPlans"].asUInt()) return;
  
  //if (nPlansAll_.count()<_geo->cuts()["minPlans"].asUInt()) return;
  // Ask at least 3 Plans in the 5 first
  uint32_t npf=0,npl=0;
  for (int i=1;i<=5;i++)
    if (nPlansAll_[i]) npf++;
  for (int i=41;i<=45;i++)
    if (nPlansAll_[i]) npl++;
  //if (npf<3) return;
  //   std::cout<<nPlansAll_.to_string()<<std::endl;
  if (_geo->cuts()["showerAnalysis"].asUInt()!=0)  
    {
      this->TagIsolated(1,48);
      ptime("Tag iso");
      // if (_pMipCand<2E-2 &&_hits.size()<25) return;
      //if (_pMipCand<_geo->cuts()["mipRate"].asFloat()) return; // Muon selection
    }
  
  this->fillVector(seed);
  DEBUG_PRINT("Real clusters size %d \n",realClusters_.size());
  bool goodtrack=true;
  if (_geo->cuts()["clusterAnalysis"].asUInt()!=0)  
    {
      _monitor->clusterHistos(_tcl,allClusters_,"/Clusters");
    
    
   
      if (_geo->cuts()["trackAlign"].asUInt()!=0)
	{
	  this->align();
	  if ((_tcl.size()*1.0/_nStripPlanes)>1.34) goodtrack=false;
	  if ((allClusters_.size()*1.0/_nPadPlanes)>1.51) goodtrack=false;;
	}
    }
  if (_geo->cuts()["trackAnalysis"].asUInt()!=0 && goodtrack)  
    {
    
      std::vector<recoPoint*> vrc;
      for (std::vector<TricotCluster>::iterator it=_tcl.begin();it!=_tcl.end();it++)
	{
	  TricotCluster& c=(*it);
	  vrc.push_back(&c);
	}
      for (std::vector<planeCluster*>::iterator it=allClusters_.begin();it!=allClusters_.end();it++)
	{
	  vrc.push_back((*it));
	}

#undef DEBORA
#ifdef DEBORA
      this->debora(vrc);
#else
      recoTrack::combinePoint1(vrc,_geo,_vtk);
      //std::cout<<"Number of tracks :"<<_vtk.size()<<std::endl;
      _monitor->trackHistos(_vtk,vrc,"/Principal");
    
      if ((_geo->cuts()["clusterDisplay"].asUInt()&1)!=0 && _vtk.size()>=1 && _vtk[0]->size()>=4)
	{
	  this->drawCluster();
	  char c;c=getchar();putchar(c); if (c=='.') exit(0);
	}
#endif
    }
  ptime("fill vector");
#define CLUSTER_ANALYSIS
#ifdef CLUSTER_ANALYSIS
  //if (_hits.size()<30) return;
  //recoTrack::combine(realClusters_,_geo,_vtk);
  ptime("combine");
  this->tagMips();
  ptime("tagmip");
  //if (((_pMip>1E-5 && _pMip<0.4) || (_vtk.size()>0 || _hits.size()>30)) && (_geo->cuts()["clusterDisplay"].asUInt()&2)!=0)
    if (tag&1!=0 & false)
    {
      std::cout<<"ELECTRON "<<_hits.size()<<" hits "<<std::endl;
      this->drawHits();
      char c;c=getchar();putchar(c); if (c=='.') exit(0);
    }
    
    if (((_pMip>1E-5 && _pMip<0.4 && _hits.size()>100) ) && (_geo->cuts()["clusterDisplay"].asUInt()&2)!=0)
    {
      std::cout<<_hits.size()<<" hits "<<_pMipCand*100<<" % low weight "<<realClusters_.size()<<" clusters "<<_pMip*100<<" % Mip tagged"<<std::endl;
      this->drawHits();
      char c;c=getchar();putchar(c); if (c=='.') exit(0);
    }
  //   std::cout<<"Seed :"<<seed<<" plans :"<<theNplans_<<std::endl;
  /*
    if (_pMip>_geo->cuts()["mipRate"].asFloat())
    _monitor->trackHistos(_vtk,realClusters_,"/Principal");
    else
    //if (_vtk.size()<1 && _hits.size()<150.) return;
    //if (_pMip>0.05) return;
    //if (_pMip<=1E-5 && _hits.size()<55) return;
    //std::cout<<" Np lnas " <<theNplans_<<std::endl;
    if (_pMip>1E-6) {
    //this->drawHits();
    //char c;c=getchar();putchar(c); if (c=='.') exit(0);
    }
  */
  if (theNplans_<minChambersInTime_) return;
#endif    
  //printf("Fini %s\n",__PRETTY_FUNCTION__);
  return;
    

}

void trackAnalysis::processEvent()
{
  
  
  
  if (reader_->getEvent()==0) return;
  
  evt_=reader_->getEvent();
  //theSkip_=380;
  
  if (evt_->getEventNumber()<=_geo->cuts()["firstEvent"].asUInt()) return;
  
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
  rebuild_=true;
  collectionName_="DHCALRawHits1";
  useSynchronised_=false;
  if (rebuild_)
    {
    
      reader_->parseLevbdimEvent();
      INFO_PRINT("End of parseraw \n");
      //reader_->flagSynchronizedFrame();
      std::vector<uint32_t> seed;
      if (useSynchronised_ )
	{
	  INFO_PRINT("Calling FastFlag2\n");
      
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
      INFO_PRINT("Calling CreaetRaw %d\n",minChambersInTime_);
      //reader_->findDIFSeeds(minChambersInTime_);
      //rhcol=reader_->createRawCalorimeterHits(reader_->getDIFSeeds());
      if (collectionName_.compare("DHCALRawHits")!=0)
	{
	  rhcol=reader_->createRawCalorimeterHits(seed);
	  INFO_PRINT("Calling CreaetRaw %d\n", rhcol->getNumberOfElements());
	  evt_->addCollection(rhcol,collectionName_);
	  INFO_PRINT("Calling CreaetRaw %d\n",minChambersInTime_);
	  rhcoltransient=false;
	}
      else
	try {
	  rhcol=(IMPL::LCCollectionVec*) evt_->getCollection(collectionName_);
	}
	catch(...)
	  {
	    return;
	  }
    
    }
  else
    rhcol=(IMPL::LCCollectionVec*) evt_->getCollection(collectionName_);

  INFO_PRINT("After createraw %d\n",rhcol->getNumberOfElements());
  //  if (rhcol->getNumberOfElements()>1000) return;
  _info.processDIFs(reader_->getDIFList());
  if (_info.getReadoutTotalTime()>_geo->cuts()["maxTime"].asFloat())
    {
      std::stringstream s;
      s<<"/tmp/trackanalysis"<<evt_->getRunNumber()<<".root";
      rootHandler_->writeSQL(s.str());
      exit(0);

    }
  
  //INFO_PRINT("ProcessEvent CreateRaw %d \n",rhcol->getNumberOfElements());  
  if (rhcol->getNumberOfElements()>4E6) return;
  bool difs =_geo->cuts()["difAnalysis"].asUInt()!=0 && evt_->getEventNumber()%_geo->cuts()["difAnalysis"].asUInt()==0;
  if(_geo->cuts()["timeAnalysis"].asUInt()!=0)
    _monitor->FillTimeAsic(rhcol,difs);
  
  //  LCTOOLS::printParameters(rhcol->parameters());
  //DEBUG_PRINT("Time Stamp %d \n",evt_->getTimeStamp());
  if (rhcol==NULL) return;
  if (rhcol->getNumberOfElements()==0) return;
  DEBUG_PRINT("Calling decodeTrigger\n");
  // TESTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
  
  if (!decodeTrigger(rhcol,spillSize_) ) { if (rhcoltransient) delete rhcol;return;}  
  //if (isNewSpill_) return;
  DEBUG_PRINT("Apres decodeTrigger\n");
  if (evt_->getEventNumber()%100 ==0)
    {
      std::stringstream s;
      s<<"/tmp/tempanalysis"<<evt_->getRunNumber()<<"_"<<evt_->getEventNumber()%500<<".root";
      rootHandler_->writeSQL(s.str());
    }
  //    rootHandler_->writeXML(theMonitoringPath_);
  
  //PMAnalysis(3);
  uint32_t npmin=_geo->cuts()["minPlans"].asUInt();
  //INFO_PRINT("Apres PM %d\n",npmin);
  reader_->findDIFSeeds(npmin,collectionName_);
  DEBUG_PRINT("Apres timeseed\n");
  std::vector<uint32_t> vseeds =this->cleanMap(npmin);
  DEBUG_PRINT("Apres cleanmap\n");
  //std::vector<uint32_t> vseeds=reader_->getTimeSeeds();
  
  
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
  TH2* hpix = rootHandler_->GetTH2("ix");
  TH2* hpiy = rootHandler_->GetTH2("iy");
  TH1* hc2 = rootHandler_->GetTH1("chi2");
  TH1* hdx = rootHandler_->GetTH1("dx");
  TH1* hdy = rootHandler_->GetTH1("dy");
  
  if (hpx==NULL)
    {
      hdx =rootHandler_->BookTH1("dx",100,-3.,3.);
      hdy =rootHandler_->BookTH1("dy",100,-3.,3.);
      hc2 =rootHandler_->BookTH1("chi2",500,0.,1.);
      hpx =rootHandler_->BookTH2("realx",200,-10.,150.,120,-10.,120.);
      hpy =rootHandler_->BookTH2("realy",200,-10.,150.,120,-10.,120.);
      hpix =rootHandler_->BookTH2("ix",200,-10.,150.,120,-10.,120.);
      hpiy =rootHandler_->BookTH2("iy",200,-10.,150.,120,-10.,120.);
    
    }
  else
    {
      hpx->Reset();
      hpy->Reset();
      hpix->Reset();
      hpiy->Reset();
    }
  
  if (hcgposi!=0 )
    {
      hcgposi->Reset();
    
      for (std::vector<RecoHit>::iterator ih=_hits.begin();ih!=_hits.end();ih++)
	{
	  hcgposi->Fill(ih->Z(),ih->X(),ih->Y());//(*ih)->Z(),(*ih)->X(),(*ih)->Y());
	  if (ih->weight()>0.25)
	    {
	      hpix->Fill(ih->Z(),ih->X());//(*ih)->Z(),(*ih)->X(),(*ih)->Y());
	      hpiy->Fill(ih->Z(),ih->Y());//(*ih)->Z(),(*ih)->X(),(*ih)->Y());
	    }
      
	}
      for (std::vector<TricotCluster>::iterator ih=_tcl.begin();ih!=_tcl.end();ih++)
	{
	  hcgposi->Fill(ih->Z(),ih->X(),ih->Y());//(*ih)->Z(),(*ih)->X(),(*ih)->Y());
    
	  hpix->Fill(ih->Z(),ih->X());//(*ih)->Z(),(*ih)->X(),(*ih)->Y());
	  hpiy->Fill(ih->Z(),ih->Y());//(*ih)->Z(),(*ih)->X(),(*ih)->Y());
      
      
	}
      if (TCHits==NULL)
	{
	  TCHits=new TCanvas("TCHits","tChits1",900,900);
	  TCHits->Modified();
	  TCHits->Draw();
	  TCHits->Divide(2,2);
	}
      TCHits->cd(1);
      hcgposi->SetMarkerStyle(25);
      hcgposi->SetMarkerColor(kRed);
      hcgposi->Draw("P");
      /*
       *      if (ish!=0)
       *	{
       *	  TPolyLine3D *pl3d1 = new TPolyLine3D(2);
       *	  double* v=ish->l0;    
       *	  double* x=ish->xm;
       *	  double r=ish->lambda[0]*100;
       *	  pl3d1->SetPoint(0,x[2],x[0],x[1]);
       *	  pl3d1->SetPoint(1,x[2]+v[2]*r,x[0]+v[0]*r,x[1]+v[1]*r);
       *			
       *	  pl3d1->SetLineWidth(3);
       *	  pl3d1->SetLineColor(1);
       *	  pl3d1->Draw("SAME");
       *	  TPolyLine3D *pl3d2 = new TPolyLine3D(2);
       *	  v=ish->l1;    
       *	  r=ish->lambda[1]*100;
       *	  pl3d2->SetPoint(0,x[2],x[0],x[1]);
       *	  pl3d2->SetPoint(1,x[2]+v[2]*r,x[0]+v[0]*r,x[1]+v[1]*r);
       *			
       *	  pl3d2->SetLineWidth(3);
       *	  pl3d2->SetLineColor(2);
       *	  pl3d2->Draw("SAME");
       *	  TPolyLine3D *pl3d3 = new TPolyLine3D(2);
       *	  v=ish->l2;    
       *	  r=ish->lambda[2]*100;
       *	  pl3d3->SetPoint(0,x[2],x[0],x[1]);
       *	  pl3d3->SetPoint(1,x[2]+v[2]*r,x[0]+v[0]*r,x[1]+v[1]*r);
       * 
       * 
       * 
       *			
       *	  pl3d3->SetLineWidth(3);
       *	  pl3d3->SetLineColor(3);
       *	  pl3d3->Draw("SAME");
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
    
 
 
      /*
       *      for (std::vector<recoTrack>::iterator it=vtk.begin();it!=vtk.end();it++)
       *	{
       *	  std::cout<<(*it);
       *	  // for (std::vector<planeCluster*>::iterator kc=realClusters_.begin();kc!=realClusters_.end();kc++)
       *	  //   {
       *	  //     std::cout<<it->distance((*kc))<<std::endl;
       *	      
       *	  //   }
       *	  for (std::vector<ROOT::Math::XYZPoint*>::iterator kc=it->points().begin();kc!=it->points().end();kc++)
       *	    {
       *	      std::cout<<"\t Z:"<<(*kc)->Z()<<std::endl;
       }
  
  
       }
      */ 
    
      //for (int i=1;i<=15;i++)
      //	for (int j=1;j<15;j++)
      //  if (hdisp->GetBinContent(i,j)<12) hdisp->SetBinContent(i,j,0);
    
      hcgposi->SetMarkerStyle(25);
      hcgposi->SetMarkerColor(kRed);
      //      hdisp->Draw("COLZ");
      TH1* hweight= rootHandler_->GetTH1("/HitStudy/showerweight");
      //hweight->Reset();
      //for (std::vector<RecoHit>::iterator ih=_hits.begin();ih!=_hits.end();ih++)
      //	if (ih->weight()>1E-2)
      //	  hweight->Fill(ih->weight());
      if (hweight!=0)
	hweight->Draw();
      TCHits->Modified();
      TCHits->Draw();
    
      hpx->Reset();
      hpy->Reset();
      float paderr=1./sqrt(12.);
      for (std::vector<recoTrack*>::iterator it=_vtk.begin();it!=_vtk.end();it++)
	{
	  //std::cout<<(*(*it));
	  //(*it)->Dump();
	  float chi2=0;
	  for (std::vector<ROOT::Math::XYZPoint*>::iterator ip=(*it)->points().begin();ip!=(*it)->points().end();ip++)
	    {
	      float cont=(*it)->distance((*ip));
        
	      for (std::vector<planeCluster*>::iterator ic=realClusters_.begin();ic!=realClusters_.end();ic++)
		if ((ROOT::Math::XYZPoint*) (*ic)==(*ip))
		  {
		    float errx=sqrt((*ic)->hits().size())*paderr;
		    float erry=sqrt((*ic)->hits().size())*paderr;
		    float err=sqrt(errx*errx+erry*erry);
		    cont=cont*cont/err/err;
		    //std::cout<<" Cluster found"<<std::endl;
		    for (std::vector<RecoHit*>::iterator ih=(*ic)->hits().begin();ih!=(*ic)->hits().end();ih++)
		      {
			hpx->Fill((*ih)->Z(),(*ih)->X());
			hpy->Fill((*ih)->Z(),(*ih)->Y());
		      }
		    ROOT::Math::XYZPoint pex=(*it)->extrapolate((*ip)->Z());
		    ROOT::Math::XYZVector dex=pex-(*(*ip));
		    hdx->Fill(dex.X()/errx);
		    hdy->Fill(dex.Y()/erry);
		    break;
		  }
	      chi2+=cont;
	    }
    
	  //std::cout<<"chi2 "<<chi2<<" ndf"<<(*it)->points().size()*2-4<<" "<<TMath::Prob(chi2,(*it)->points().size()*2-4)<<std::endl;
	  hc2->Fill((*it)->pchi2());
	}
    
      TH1* hmip= rootHandler_->GetTH1("/HitStudy/mip");
      //if (hmip!=NULL)
      //	hmip->Draw();
      TH1* hwmip= rootHandler_->GetTH1("/HitStudy/weightmip");
      if (hwmip!=NULL)
	hwmip->Draw();
    
      TCHits->Modified();
      TCHits->Draw();
      TCHits->cd(3);
      /*
       * 
       *      hdx->Draw();
       *      TCHits->cd(4);
       *      hdy->Draw();
       * 
       */
      hpix->SetLineColor(kGreen);
      hpix->Draw("BOX");
    
    
      if (hpix->GetEntries()>0)
	hpx->Draw("BOXSAME");
      else
	hpx->Draw("BOX");
      for (std::vector<recoTrack*>::iterator it=_vtk.begin();it!=_vtk.end();it++)
	{
	  (*it)->linex()->Draw("SAME");
	}
      TCHits->cd(4);
    
    
      hpiy->SetLineColor(kGreen);
    
      hpiy->Draw("BOX");
    
      hpy->SetLineColor(kRed);
      if (hpiy->GetEntries()>0)
	hpy->Draw("BOXSAME");
      else
	hpy->Draw("BOX");
    
      for (std::vector<recoTrack*>::iterator it=_vtk.begin();it!=_vtk.end();it++)
	{
	  (*it)->liney()->Draw("SAME");
	}
    
      TCHits->Modified();
      TCHits->Draw();
      TCHits->Update();
    }
  
}
void trackAnalysis::align()
{
  nPlansAll_.reset();
  for (std::vector<planeCluster*>::iterator ih=allClusters_.begin();ih!=allClusters_.end();ih++)
    {
      nPlansAll_.set((*ih)->plan());    
    }
  for (std::vector<TricotCluster>::iterator ih=_tcl.begin();ih!=_tcl.end();ih++)
    {
      nPlansAll_.set((*ih).plan());
    }
  _nStripPlanes=0;
  for (int i=2;i<=6;i++)
    if (nPlansAll_[i]) _nStripPlanes++;
  _nPadPlanes=0;
  for (int i=7;i<=9;i++)
    if (nPlansAll_[i]) _nPadPlanes++;
  
  if (_nStripPlanes<3 || _nPadPlanes<2) return;
  if ((_tcl.size()*1.0/_nStripPlanes)>1.34) return;
  if ((allClusters_.size()*1.0/_nPadPlanes)>1.51) return;
  
  std::cout<<theAbsoluteTime_<<":"<<_tcl.size()<<" => Strips plans "<<_nStripPlanes<<" "<<nPlansAll_<<std::endl;
  std::cout<<theAbsoluteTime_<<":"<<allClusters_.size()<<" =>pads plans "<<_nPadPlanes<<" "<<nPlansAll_<<std::endl;
  
  //if (allClusters_.size()<3) return;
  //if (_tcl.size()<3) return;
  TProfile* hxpadtric= (TProfile*)rootHandler_->GetTH1("xtricvspad");
  TProfile* hypadtric= (TProfile*) rootHandler_->GetTH1("ytricvspad");
  TH2* hxpadtric2= (TH2*)rootHandler_->GetTH2("xtricvspad2");
  TH2* hypadtric2= (TH2*) rootHandler_->GetTH2("ytricvspad2");
  if (hxpadtric==NULL)
    {
    
      hxpadtric=rootHandler_->BookProfile("xtricvspad",30,-10.,40.,-40.,40.);
      hypadtric=rootHandler_->BookProfile("ytricvspad",30,-10.,40.,-40.,40.);
      hxpadtric2=rootHandler_->BookTH2("xtricvspad2",30,-10.,40.,30,-10.,40.);
      hypadtric2=rootHandler_->BookTH2("ytricvspad2",30,-10.,40.,30,-10.,40.);    
    }
  
  recoTrack tkpad,tktric;
  for (std::vector<planeCluster*>::iterator ih=allClusters_.begin();ih!=allClusters_.end();ih++)
    {
     
      tkpad.addPoint((*ih));
      //printf("pads %f %f %f \n",(*ih)->Z(),(*ih)->X(),(*ih)->Y());
   
    }
  for (std::vector<TricotCluster>::iterator ih=_tcl.begin();ih!=_tcl.end();ih++)
    {
      
      tktric.addPoint(&(*ih));
      //printf("strips %f %f %f \n",ih->Z(),ih->X(),ih->Y());
    }
  ROOT::Math::XYZPoint ppad=tkpad.extrapolate(60);
  ROOT::Math::XYZPoint ptric=tktric.extrapolate(60);
   
  //printf("Pads track %f %f %f \n",ppad.X(),ppad.Y(),ppad.Z());
  // printf("Tric track %f %f %f \n",ptric.X(),ptric.Y(),ptric.Z());
  hxpadtric->Fill(ppad.X(),ptric.X());
  hypadtric->Fill(ppad.Y(),ptric.Y());
  hxpadtric2->Fill(ppad.X(),ptric.X());
  hypadtric2->Fill(ppad.Y(),ptric.Y()); 
  
}

void trackAnalysis::drawCluster()
{
  
  TH3* hcgposi = rootHandler_->GetTH3("InstantClusterMap");
  
  if (hcgposi==NULL)
    {
      hcgposi =rootHandler_->BookTH3("InstantClusterMap",160,-20.,140.,160,-20.,120.,160,-20.,120.);
   
    
    }
  else
    {
      hcgposi->Reset();
    }
  
  
  if (hcgposi!=0 )
    {
      hcgposi->Reset();
      for (std::vector<planeCluster*>::iterator ih=allClusters_.begin();ih!=allClusters_.end();ih++)
	{
	  hcgposi->Fill((*ih)->Z(),(*ih)->X(),(*ih)->Y());
	  //printf("pads %f %f %f \n",(*ih)->Z(),(*ih)->X(),(*ih)->Y());
      
	}
      for (std::vector<TricotCluster>::iterator ih=_tcl.begin();ih!=_tcl.end();ih++)
	{
	  hcgposi->Fill(ih->Z(),ih->X(),ih->Y());//(*ih)->Z(),(*ih)->X(),(*ih)->Y());
	  //printf("strips %f %f %f \n",ih->Z(),ih->X(),ih->Y());
	}
    
      if (TCHits==NULL)
	{
	  TCHits=new TCanvas("TCHits","tChits1",600,600);
	  TCHits->Modified();
	  TCHits->Draw();
	  TCHits->Divide(2,2);
	}
      TCHits->cd(1);
      hcgposi->SetMarkerStyle(25);
      hcgposi->SetMarkerColor(kRed);
      hcgposi->Draw("P");
      printf("ok1\n");
      TCHits->cd(2);
      TProfile2D* hpx1=hcgposi->Project3DProfile("yx");
      //hpx1->SetLineColor(kGreen);
      hpx1->SetMarkerStyle(20);
      hpx1->SetMarkerColor(3);
      hpx1->Draw("P"); 
      //hxpadtric->Draw();
      printf("ok2\n");
      TCHits->Modified();
      TCHits->Draw();
      TCHits->cd(3);
      TProfile2D* hpx2=hcgposi->Project3DProfile("zx");
      hpx2->SetMarkerStyle(20);
      hpx2->SetMarkerColor(2);
      hpx2->Draw("P"); 
      //hypadtric->Draw();
    
      printf("ok3\n");
    
      TCHits->Modified();
      TCHits->Draw();
      TCHits->Update();
      printf("ok4\n");
    
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
void trackAnalysis::ptime( std::string s)
{
#undef DEBUG
#ifdef DEBUG
  
  gettimeofday(&endT, NULL); 
  
  timersub(&endT, &startT, &diffT);
  
  printf("%s **time taken = %ld %ld\n",s.c_str(),diffT.tv_sec, diffT.tv_usec);
  gettimeofday(&startT, NULL);
#endif
}
void trackAnalysis::fillPlaneClusters()
{
  //printf("DEBUT\n");
  
  clearClusters();
  
  
  for (std::vector<RecoHit>::iterator ih=_hits.begin();ih<_hits.end();ih++)
    {
    
      //if (ih->weight()>0.25) continue;
      if (ih->isUsed()) continue;
      bool merged=false;
      double z=(*ih).Z();
      for (std::vector<planeCluster*>::iterator ic=allClusters_.begin();ic!=allClusters_.end();ic++)
	{
	  if (abs((*ic)->Z()-z)>0.5) continue;
	  //if (ih->chamber()!=(*ic)->chamber()) continue;
	  merged=(*ic)->Append(&(*ih),2.); // avant 4 et normalement 2
	  if (merged) break;
	}
      if (merged) continue;
      planeCluster* c= new planeCluster(&(*ih));
      allClusters_.push_back(c);
    }
  
  for (std::vector<planeCluster*>::iterator ic=allClusters_.begin();ic!=allClusters_.end();ic++)
    {
      if ((*ic)->getHits()->size()<=4)
	realClusters_.push_back((*ic));
      else
	{
	  bool mipc=false;
	  if ((*ic)->getHits()->size()<7)
	    for (std::vector<RecoHit*>::iterator ih=(*ic)->getHits()->begin();ih!=(*ic)->getHits()->end();ih++)
	      if ((*ih)->weight()<0.25) {mipc=true;break;}
          if (mipc)
            realClusters_.push_back((*ic));
          else
            interactionClusters_.push_back((*ic));
	}
    }
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
  return;   
  
  
  
  //return;
  uint32_t nlow=0;
  for (std::vector<RecoHit>::iterator ih=_hits.begin();ih<_hits.end();ih++)
    {
    
      //if (ih->isTagged(RecoHit::CORE)==1) continue;
      if (ih->isUsed()) continue;
      //std::cout<<ih->weight()<<std::endl;
      if (ih->weight()<1E-3) continue;
      if (ih->weight()>25E-2) continue;
      nlow++;
      bool merged=false;
      for (std::vector<planeCluster*>::iterator ic=realClusters_.begin();ic!=realClusters_.end();ic++)
	{
	  if (ih->chamber()!=(*ic)->chamber()) continue;
	  merged=(*ic)->Append(&(*ih),4.); // avant 4 et normalement 2
	  if (merged) break;
	}
      if (merged) continue;
      planeCluster* c= new planeCluster(&(*ih));
      realClusters_.push_back(c);
      allClusters_.push_back(c);
    }
  
  // Add adjacent hist
  for (std::vector<RecoHit>::iterator ih=_hits.begin();ih<_hits.end();ih++)
    {
    
      //if (ih->isTagged(RecoHit::CORE)==1) continue;
      if (ih->isUsed()) continue;
      bool merged=false;
      for (std::vector<planeCluster*>::iterator ic=realClusters_.begin();ic!=realClusters_.end();ic++)
	{
	  if (ih->chamber()!=(*ic)->chamber()) continue;
	  merged=(*ic)->Append(&(*ih),4.); // avant 4 et normalement 2
	  if (merged) break;
	}
    }
  
  //printf("OLA Low %d all %d real %d \n",nlow,allClusters_.size(),realClusters_.size());
  for (std::vector<RecoHit>::iterator ih=_hits.begin();ih<_hits.end();ih++)
    {
      if (ih->isUsed()) continue;
      if (ih->weight()<25E-2) continue;
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
  //printf("OLI all %d interaction %d \n",allClusters_.size(),interactionClusters_.size());
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
  //printf("OLF real %d  interaction %d \n",realClusters_.size(),interactionClusters_.size());
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
  
  //std::bitset<61> planes(0);
  nPlansAll_.reset();
  
  uint32_t ncount=0;
  DEBUG_PRINT("Number of hits %d \n",iseed->second.size());
  if (iseed->second.size()>_geo->cuts()["maxSeedHits"].asUInt()) return 0;
  
  // Found fully filled DIF
  std::bitset<64> difc[255];
  for (int i=0;i<255;i++) difc[i].set(0);
  for (std::vector<IMPL::RawCalorimeterHitImpl*>::iterator ihit=iseed->second.begin();ihit!=iseed->second.end();ihit++)
    {
      IMPL::RawCalorimeterHitImpl* hit =(*ihit);
      uint32_t dif =hit->getCellID0()&0xFF;
      uint32_t asic = (0xFF & (hit->getCellID0()&0xFF00)>>8);
      difc[dif].set(asic,1);
    
    }
  for (int i=0;i<255;i++) if (difc[i].count()>24) {
      INFO_PRINT(" DIF %d Count %d  at seed %d \n",i,difc[i].count(),seed);
      return 0;
    }
  // Now fill _hits vector
  for (std::vector<IMPL::RawCalorimeterHitImpl*>::iterator ihit=iseed->second.begin();ihit!=iseed->second.end();ihit++)
    {
      IMPL::RawCalorimeterHitImpl* hit =(*ihit);
      // Check the type of the RawCalorimeterHit
      //std::cout<<(int) (hit->getCellID0()&0xFF)<<" type = "<<_geo->difGeo(hit->getCellID0()&0xFF)["type"].asString()<<std::endl;
    
      if (_geo->difInfo(hit->getCellID0()&0xFF).type.compare("TRICOT")==0) continue;
      RecoHit h(_geo,hit);
      nPlansAll_.set(h.plan());
      _hits.push_back(h);
      //_keys.push_back(makekey(I,J,chid));
    
      ncount++;
    }
  DEBUG_PRINT("Number of PAD hits %d \n",ncount);
  // Now build Strip clusters
  for (std::vector<TStripCluster*>::iterator its=_vts.begin();its!=_vts.end();its++) delete (*its);
  _vts.clear();
  for (std::vector<IMPL::RawCalorimeterHitImpl*>::iterator ihit=iseed->second.begin();ihit!=iseed->second.end();ihit++)
    {
      IMPL::RawCalorimeterHitImpl* hit =(*ihit);
      // Check the type of the RawCalorimeterHit
      //std::cout<<(hit->getCellID0()&0xFF)<<" _vts "<<_vts.size()<<std::endl;
    
      if (_geo->difInfo(hit->getCellID0()&0xFF).type.compare("TRICOT")!=0) continue;
      bool added=false;
      //Json::Value dif=_geo->difGeo(hit->getCellID0()&0xFF);
      //Json::Value ch=_geo->chamberGeo(dif["chamber"].asUInt());
      nPlansAll_.set(_geo->chamberInfo(_geo->difInfo(hit->getCellID0()&0xFF).chamber).plan);
      //std::cout<<(hit->getCellID0()&0xFF)<<" av "<<(0xFF & (hit->getCellID0()&0xFF00)>>8)<<" "<<(0xFF & (hit->getCellID0()&0x3F0000)>>16)<<std::endl;
      if (true)
	for (std::vector<TStripCluster*>::iterator its=_vts.begin();its!=_vts.end();its++)
	  {
	    if ((*its)->append((hit->getCellID0()&0xFF),(0xFF & (hit->getCellID0()&0xFF00)>>8),
			       (0xFF & (hit->getCellID0()&0x3F0000)>>16))) 
	      { added=true; break;}
      
	  }
      //std::cout<<(hit->getCellID0()&0xFF)<<" "<<(0xFF & (hit->getCellID0()&0xFF00)>>8)<<" "<<(0xFF & (hit->getCellID0()&0x3F0000)>>16)<<std::endl;
      if (!added)
	_vts.push_back(new TStripCluster((hit->getCellID0()&0xFF),(0xFF & (hit->getCellID0()&0xFF00)>>8),
					 (0xFF & (hit->getCellID0()&0x3F0000)>>16),_geo));
      //std::cout<<(hit->getCellID0()&0xFF)<<"ap "<<(0xFF & (hit->getCellID0()&0xFF00)>>8)<<" "<<(0xFF & (hit->getCellID0()&0x3F0000)>>16)<<std::endl;
    
      
    }
  //printf("STrip clusters %d  %s\n",_vts.size(),__PRETTY_FUNCTION__);
  
  // Now build TricotCluster
  _tcl.clear();
  for (int i=0;i<_vts.size();i++)
    {
      Json::Value dif=_geo->difGeo(_vts[i]->dif());
      Json::Value ch=_geo->chamberGeo(dif["chamber"].asUInt());
      //std::cout<<ch["plan"].asFloat()<<"Chamber Z"<<ch["z0"].asDouble()<<std::endl;
      for(int j=i+1;j<_vts.size();j++)
	{
	  if(_vts[i]->dif()!=_vts[j]->dif()) continue;
	  if(_vts[i]->asic()==_vts[j]->asic()) continue;
	  for(int k=j+1;k<_vts.size();k++)
	    {
	      if(_vts[i]->dif()!=_vts[k]->dif()) continue;
	      if(_vts[k]->dif()!=_vts[j]->dif()) continue;
	      if(_vts[i]->asic()==_vts[k]->asic()) continue;
	      if(_vts[j]->asic()==_vts[k]->asic()) continue;
        
        
	      float x1=(_vts[j]->b()-_vts[i]->b())/(_vts[i]->a()-_vts[j]->a());
	      float y1=_vts[i]->a()*x1+_vts[i]->b();
	      float x2=(_vts[k]->b()-_vts[i]->b())/(_vts[i]->a()-_vts[k]->a());
	      float y2=_vts[i]->a()*x2+_vts[i]->b();
	      float x3=(_vts[k]->b()-_vts[j]->b())/(_vts[j]->a()-_vts[k]->a());
	      float y3=_vts[j]->a()*x3+_vts[j]->b();
	      float x=(x1+x2+x3)/3.;
	      float y=(y1+y2+y3)/3.;
	      if (sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2))>2) continue;
	      if (sqrt((x1-x3)*(x1-x3)+(y1-y3)*(y1-y3))>2) continue;
	      if (sqrt((x3-x2)*(x3-x2)+(y3-y2)*(y3-y2))>2) continue;
	      _vts[i]->setUsed();
	      _vts[j]->setUsed();
	      _vts[k]->setUsed();
	      _tcl.push_back(TricotCluster(x+ch["x0"].asDouble(),y+ch["y0"].asDouble(),ch["z0"].asDouble(),(*_vts[i]),(*_vts[j]),(*_vts[k])));
	    }
	}
    }
  // 2 lines remaining
  for (int i=0;i<_vts.size();i++)
    {
      Json::Value dif=_geo->difGeo(_vts[i]->dif());
      Json::Value ch=_geo->chamberGeo(dif["chamber"].asUInt());
      
      if (_vts[i]->isUsed()) continue;
      for(int j=i+1;j<_vts.size();j++)
	{
	  if (_vts[j]->isUsed()) continue;
	  if(_vts[i]->dif()!=_vts[j]->dif()) continue;
	  if(_vts[i]->asic()==_vts[j]->asic()) continue;
	  float x=(_vts[j]->b()-_vts[i]->b())/(_vts[i]->a()-_vts[j]->a());
	  float y=_vts[i]->a()*x+_vts[i]->b();
	  _vts[i]->setUsed();
	  _vts[j]->setUsed();
	  _tcl.push_back(TricotCluster(x+ch["x0"].asDouble(),y+ch["y0"].asDouble(),ch["z0"].asDouble(),(*_vts[i]),(*_vts[j])));
	}
      
    }
  //printf("\n");
    
    
  DEBUG_PRINT(" Seed %d Number of strips founded %d Number of hit found %d \n",seed,_vts.size(),_hits.size());
    
  for (std::vector<TricotCluster>::iterator itc=_tcl.begin();itc!=_tcl.end();itc++)
    {
      Json::Value dif=_geo->difGeo(itc->dif());
      Json::Value ch=_geo->chamberGeo(dif["chamber"].asUInt());
      //printf("Strip %d pos %f %f %f \n",ch["plan"].asUInt(),itc->X(),itc->Y(),itc->Z());
    }
  DEBUG_PRINT("Total number of Hit in buildVolume %d  %d => planes %d \n",ncount,seed,planes.count());
  return nPlansAll_.count();
}

void trackAnalysis::TagIsolated(uint32_t fpl,uint32_t lpl)
{
  TH1* hweight= rootHandler_->GetTH1("/HitStudy/showerweight");
  TH1* hmipc= rootHandler_->GetTH1("/HitStudy/mipcand");
  TH2* hmipch= rootHandler_->GetTH2("/HitStudy/mipcandhit");
  TH1* hnv= rootHandler_->GetTH1("/HitStudy/nv");
  TH2* hweight2= rootHandler_->GetTH2("/HitStudy/showerweight2");
  if (hweight==NULL)
    {
      //hweight=(TH1F*) rootHandler_->BookTH1("showerweight",100,0.,2.);
      hweight= rootHandler_->BookTH1("/HitStudy/showerweight",160,-0.1,1.5);
      hmipc= rootHandler_->BookTH1("/HitStudy/mipcand",160,-0.1,1.5);
      hmipch= rootHandler_->BookTH2("/HitStudy/mipcandhit",300,0.,1800,160,-0.1,1.5);
      hnv= rootHandler_->BookTH1("/HitStudy/nv",150,0.,150.);
      hweight2= rootHandler_->BookTH2("/HitStudy/showerweight2",150,0.,150.,110,-0.1,0.99);
    }
  //hweight->Reset(); // commented by LM21_01_2015
  
  uint32_t nmax=0;
  uint32_t nedge=0,ncore=0,niso=0;
  int32_t ixmin=-6,ixmax=6; // 6 avant
  std::vector<RecoHit*> vnear_;
  float dcut2=47.; //36.
  dcut2=25.;
  int nmipc=0;
  for (std::vector<RecoHit>::iterator it=_hits.begin();it!=_hits.end();it++)
    {
      vnear_.clear();
      for (std::vector<RecoHit>::iterator jt=_hits.begin();jt!=_hits.end();jt++)
	{
	  if (jt==it) continue;
	  ROOT::Math::XYZVector d=(*it)-(*jt);
	  //	   if (abs(d.Z())<1) continue;
	  //float dist=2*(abs(d.X())+abs(d.y()))+abs(d.Z());
	  if (d.Mag2()<dcut2)
	    vnear_.push_back(&(*jt));
	}
      /*
       *       int izmin=-2;
       *       int izmax=+2;
       *       if (it->chamber()<=fpl+2) {izmin=0;izmax=4;}
       *       if (it->chamber()>=lpl-2) {izmin=-4;izmax=0;}
       *       std::vector<RecoHit*> vnear_;vnear_.clear();
       *       RecoHit* h0=&(*it);
       *       for (std::vector<RecoHit>::iterator jt=_hits.begin();jt!=_hits.end();jt++)
       *	 {
       *	   if ((*it)==(*jt)) continue;
       *	   if (jt->chamber()<it->chamber()+izmin) continue;
       *	   if (jt->chamber()>it->chamber()+izmax) continue;
       *	   if (jt->I()<it->I()+ixmin) continue;
       *	   if (jt->I()>it->I()+ixmax) continue;
       *	   if (jt->J()<it->J()+ixmin) continue;
       *	   if (jt->J()>it->J()+ixmax) continue;
       * 
       *	   RecoHit* h1=&(*jt);
       *	   
       *	   float x0=h0->X(),y0=h0->Y(),z0=h0->Z(),x1=h1->X(),y1=h1->Y(),z1=h1->Z();
       *	   float dist=sqrt((x1-x0)*(x1-x0)+(y1-y0)*(y1-y0)+(z1-z0)*(z1-z0));
       *	   dist=abs(x1-x0)+abs(y1-y0)+abs(z1-z0)/2.;
       *	   if (dist<4.) vnear_.push_back(h1); // was 4
       }
      */
      RecoHit* h0=&(*it);
      hnv->Fill(vnear_.size()*1.);
      pcaComponents c=RecoHit::calculateComponents<RecoHit>(vnear_);
      double w=0;
      if (c[5]!=0) w=sqrt((c[4]+c[3])/c[5]);
      h0->setWeight(w);
      if (w<2.5E-1) nmipc++;
      if (c[5]==0 || vnear_.size()<=3)
	{
	  h0->setTag(RecoHit::ISOLATED,true);niso++;
      
	}
      else
	{
	  hweight->Fill(w);
	  hweight2->Fill(vnear_.size()*1.,w);
	  if (w<0.2 && vnear_.size()<20) // 0.3 before  
	    {h0->setTag(RecoHit::EDGE,true);nedge++;}
	  else
	    {h0->setTag(RecoHit::CORE,true);ncore++;}
	}
    
    }
  coreRatio_=ncore*1./(nedge+niso);
  _pMipCand=nmipc*1./_hits.size();
  hmipc->Fill(_pMipCand);
  hmipch->Fill(_hits.size(),_pMipCand);
  return;
}


uint32_t trackAnalysis::fillVector(uint32_t seed)
{
  
  
  
  //INFO_PRINT("%s-%d %d %f  \n",__PRETTY_FUNCTION__,__LINE__,reader_->getPositionMap().begin()->second.getPlan(),reader_->getPositionMap().begin()->second.getZ0());    
  //  uint32_t nplans=this->fillVolume(seed);
  //if (nplans<7) return 0;
  uint32_t nhit=_hits.size();
  
  //INFO_PRINT("%s-%d %d %f  \n",__PRETTY_FUNCTION__,__LINE__,reader_->getPositionMap().begin()->second.getPlan(),reader_->getPositionMap().begin()->second.getZ0());    
  if (nhit<minHitCount_) return 0;
  if (nhit>maxHitCount_) return 0;
  //  STEP;
  
  theTkHitVector_.clear();
  for (std::vector<RecoHit>::iterator it=_hits.begin();it!=_hits.end();it++)
    {
      RecoHit h=(*it);
      if (h.isTagged(RecoHit::ISOLATED)!=1) 
	theTkHitVector_.push_back(&h);
    }
  ptime("after tkhit");
  this->fillPlaneClusters();
  std::sort(realClusters_.begin(),realClusters_.end(),PlaneCompare());
  //for (std::vector<planeCluster*>::iterator itc=allClusters_.begin();itc!=allClusters_.end();itc++)
  //{
  //  printf("Pads %d pos %f %f %f \n",(*itc)->chamber(),(*itc)->X(),(*itc)->Y(),(*itc)->Z());
  // }
  DEBUG_PRINT("Hits %d tk %d ===> %d clusters %d Real %d Interaction \n",_hits.size(),theTkHitVector_.size(),allClusters_.size(),realClusters_.size(),
              interactionClusters_.size());

  
  
  return nPlansAll_.count();
}

void trackAnalysis::tagMips()
{
  
  TH1* hmip= rootHandler_->GetTH1("/HitStudy/mip");
  TH1* hwmip= rootHandler_->GetTH1("/HitStudy/weightmip");
  TH2* hmiph= rootHandler_->GetTH2("/HitStudy/miphit");
  
  if (hmip==NULL)
    {
      //hweight=(TH1F*) rootHandler_->BookTH1("showerweight",100,0.,2.);
      hmip= rootHandler_->BookTH1("/HitStudy/mip",110,-0.05,1.05);
      hwmip= rootHandler_->BookTH1("/HitStudy/weightmip",210,-0.05,1.05);
      hmiph= rootHandler_->BookTH2("/HitStudy/miphit",300,0.,1800.,220,-0.05,1.05);
    }
  
  //hwmip->Reset();
  uint32_t nmip=0,nc=0;
  
  double paderr=100./96./sqrt(12.); 
  for (std::vector<recoTrack*>::iterator it=_vtk.begin();it!=_vtk.end();it++)
    {
      double chi2=0;
      for (std::vector<ROOT::Math::XYZPoint*>::iterator ip=(*it)->points().begin();ip!=(*it)->points().end();ip++)
	{
	  double cont=(*it)->distance((*ip));
	  double err=0;
	  bool found=false;   
	  for (std::vector<planeCluster*>::iterator ic=realClusters_.begin();ic!=realClusters_.end();ic++)
	    if ((ROOT::Math::XYZPoint*) (*ic)==(*ip))
	      {
		double errx=1./sqrt((*ic)->hits().size())*paderr;
		double erry=1./sqrt((*ic)->hits().size())*paderr;
		err=sqrt(errx*errx+erry*erry);
		for (std::vector<RecoHit*>::iterator ih=(*ic)->hits().begin();ih!=(*ic)->hits().end();ih++)
		  {
		    (*ih)->setTag(RecoHit::MIP,true);
		    hwmip->Fill((*ih)->weight());
		    nmip++;
		  }
		found=true;
		break;
	      }
        
	  if (!found)
	    { if (0) std::cout<<"Cluster not found!!!"<<std::endl;
	    }
	  else
	    {
	      chi2+=cont*cont/err/err;
	      nc++;
	    }
	}
      //std::cout<<"chi2 "<<chi2<<" ndf"<<(*it)->points().size()*2-4<<" "<<TMath::Prob(chi2,(*it)->points().size()*2-4)<<std::endl;
      (*it)->setChi2(chi2);
      //  std::cout<<(*it)->chi2()<<" prob "<<(*it)->pchi2()<<std::endl;
    }
  
  
  _pMip=nmip*1./_hits.size();
  hmip->Fill(_pMip);
  hmiph->Fill(_hits.size(),_pMip);
  DEBUG_PRINT("==> MIPS hit %d -> %.2f Length %d \n",nmip,nmip*100./_hits.size(),nc);
}


std::vector<uint32_t> trackAnalysis::cleanMap(uint32_t nchmin)
{
  std::vector<uint32_t> vs;vs.clear();
  std::map<uint32_t,std::vector<IMPL::RawCalorimeterHitImpl*> >& pmap=reader_->getPhysicsEventMap();
  //INFO_PRINT("Number of seeds %d \n",pmap.size());
  for (std::map<uint32_t,std::vector<IMPL::RawCalorimeterHitImpl*> >::iterator iseed=pmap.begin();iseed!=pmap.end();iseed++)
    {
      std::bitset<64> plans(0);
      for (std::vector<IMPL::RawCalorimeterHitImpl*>::iterator ih=iseed->second.begin();ih!=iseed->second.end();ih++)
	{
	  uint32_t difid=(*ih)->getCellID0()&0xFF;
	  //Json::Value dif=_geo->difGeo(difid);
	  plans.set(_geo->difInfo(difid).chamber,1);
      
	}
      //std::cout<<"Seed "<<iseed->first<<" "<<plans.count()<<" "<<plans<<std::endl;
      if (plans.count()>=nchmin)
	{
	  vs.push_back(iseed->first);
      
	}
    
    
    }
  //INFO_PRINT("Number of seeds after cut %d \n",vs.size());
  return vs;
}



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
