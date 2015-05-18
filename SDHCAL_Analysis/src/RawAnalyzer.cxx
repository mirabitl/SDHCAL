#define NX 36
#define NY 36

#include "RawAnalyzer.h"
#include "DIFUnpacker.h"
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
//#include <lapacke.h>
#include "DifGeom.h"
#include "DIFSnapShot.h"
#include <time.h>



RawAnalyzer::RawAnalyzer() :nAnalyzed_(0),theMonitoringPeriod_(0),theMonitoringPath_("/dev/shm/Monitoring"),theSkip_(0),draw_(false)
{
  reader_=DHCalEventReader::instance();
  rootHandler_ =DCHistogramHandler::instance();
  this->initialise();
}



void RawAnalyzer::initialise()
{

  theTotalTime_=0.;
  memset(theTotalCount_,0,255*49*sizeof(uint32_t));
  theStartBCID_=0;  
}
void RawAnalyzer::initJob(){presetParameters();}
void RawAnalyzer::endJob(){
  if (theMonitoringPeriod_!=0)
    {
      rootHandler_->writeXML(theMonitoringPath_);
      //char c;c=getchar();putchar(c); if (c=='.') exit(0);;
		
    }
	
}
void RawAnalyzer::presetParameters()
{
  std::map<std::string,MarlinParameter> m=reader_->getMarlinParameterMap();
  std::map<std::string,MarlinParameter>::iterator it;
  try
    {
      if ((it=m.find("Interactif"))!=m.end()) draw_=it->second.getBoolValue();
      if ((it=m.find("SkipEvents"))!=m.end()) theSkip_=it->second.getIntValue();
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

static int nst=0;
static float allt=0.;
void RawAnalyzer::processEvent()
{
  
  if (reader_->getEvent()==0) return;
  evt_=reader_->getEvent();
  if (reader_->getRunHeader()!=0)
    std::cout<<reader_->getRunHeader()->getParameters().getStringVal("Setup")<<std::endl;
  
  if (evt_->getEventNumber()<=theSkip_) return;
  TH1* hacqtime=rootHandler_->GetTH1("AcquisitionTime");
  TH2* hfr2=rootHandler_->GetTH2("HitFrequency");
  if (hacqtime==NULL)
    {
      hacqtime =rootHandler_->BookTH1( "AcquisitionTime",2000,0.,2.);

      hfr2=rootHandler_->BookTH2("HitFrequency",255,0.1,255.1,48,0.1,48.1);
    }
  printf("4\n");
   reader_->parseRawEvent();
  std::vector<DIFPtr*>::iterator itb =reader_->getDIFList().begin();
  if (itb==reader_->getDIFList().end()) return;
				      
  DIFPtr* dbase= (*itb);
  
  if ( theStartBCID_==0) theStartBCID_=dbase->getAbsoluteBCID();
  theEventTotalTime_=0; 
 
  std::cout<<"Event  "<<dbase->getAbsoluteBCID()<<std::endl;
  
  
  memset(theCount_,0,255*49*sizeof(uint32_t));
  struct timespec tp0,tp1;
  clock_gettime(CLOCK_REALTIME,&tp0);
  int64_t tlast=0;
#ifdef ENBEAMTEST  
  // reader_->findTimeSeeds(5);
#define MAPSEARCH
  std::vector<uint32_t> seeds=reader_->getTimeSeeds();
#ifdef MAPSEARCH

  std::map<uint32_t,std::bitset<64> > chti;
  chti.clear();
  for (std::vector<DIFPtr*>::iterator it = reader_->getDIFList().begin();it!=reader_->getDIFList().end();it++)
    {
      DIFPtr* d = (*it);
      if (d->getID()>255) continue;
     // Loop on frames
      std::map<unsigned int,DifGeom>::iterator idg = reader_->getDifMap().find(d->getID());
      uint32_t chid = idg->second.getChamberId();
      
      for (uint32_t i=0;i<d->getNumberOfFrames();i++)
      {
	double t=d->getFrameTimeToTrigger(i)*2E-7;

	if (t>3.8) {
	  printf("Wrong Time %f %x \n",t,d->getFrameTimeToTrigger(i));
	  continue;
	}
	std::map<uint32_t,std::bitset<64> >::iterator ich=chti.find(d->getFrameTimeToTrigger(i));
	if (ich==chti.end())
	  {
	    std::bitset<64> k;k.set(chid);
	    std::pair<uint32_t,std::bitset<64> > p(d->getFrameTimeToTrigger(i),k);
	    chti.insert(p);
	  }
	else
	  {
	    ich->second.set(chid);
	  }
      }
    }
  uint32_t nseeds=0;
  std::vector<uint32_t> seedm;
  seedm.clear();
  for (std::map<uint32_t,std::bitset<64> >::iterator ich=chti.begin();ich!=chti.end();)
    {
      
      if (ich->second.count()>=5) {
	//std::cout<<  "     seed " <<ich->first<<" "<<std::setw(5)<<ich->second.count()<<" "<<ich->second<<std::endl;
	std::map<uint32_t,std::bitset<64> >::iterator ichn=chti.find(ich->first+1);
      if (ichn==chti.end())
	{
	  nseeds++;
	  //std::cout<<"Good seed " <<ich->first<<" "<<std::setw(5)<<ich->second.count()<<" "<<ich->second<<std::endl;
	  if (ich->first>tlast) tlast=ich->first;
	  seedm.push_back(ich->first);
	  ich++;
	}
      else
	{
	  if (ichn->second.count()>=3)
	    {
	      //std::cout<<"Mer0 seed " <<ich->first<<" "<<std::setw(5)<<ich->second.count()<<" "<<ich->second<<std::endl;
	      //std::cout<<"Mer1 seed " <<ichn->first<<" "<<std::setw(5)<<ichn->second.count()<<" "<<ichn->second<<std::endl;
	      ichn->second|=ich->second;
	      chti.erase(ich++);
	    }
	  else
	    {
	      nseeds++;
	      // std::cout<<"Good seed " <<ich->first<<" "<<std::setw(5)<<ich->second.count()<<" "<<ich->second<<std::endl;
	      if (ich->first>tlast) tlast=ich->first;
	      seedm.push_back(ich->first);
	      ich++;
	    }
	}
      }
     else
       ich++;
  
    }
//for (std::vector<uint32_t>::iterator is=seeds.begin();is!=seeds.end();is++)
// {
//   std::map<uint32_t,std::bitset<64> >::iterator ichn=chti.find((*is));
//   if (ichn!=chti.end())
//     std::cout<<"find seed " <<ichn->first<<" "<<std::setw(5)<<ichn->second.count()<<" "<<ichn->second<<std::endl;
	
// }
//getchar();
  #endif
  
//std::vector<uint32_t> seeds=reader_->getTimeSeeds();
DIFSnapShot ds;
for (std::vector<uint32_t>::iterator is=seedm.begin();is!=seedm.end();is++)
  ds.fill(reader_->getDIFList(),(*is));
  clock_gettime(CLOCK_REALTIME,&tp1);
  nst+=seedm.size();
  allt+=(tp1.tv_sec+1E-9*tp1.tv_nsec-tp0.tv_sec+1E-9*tp0.tv_nsec);
printf("Time %d Number of seed %d, DT %f , frequency %f, all seeds %d int Freq %f T tot %f N Evt %d\n",seedm.size(),seedm.size(),(tp1.tv_sec+1E-9*tp1.tv_nsec-tp0.tv_sec+1E-9*tp0.tv_nsec),
       seedm.size()/(tp1.tv_sec+1E-9*tp1.tv_nsec-tp0.tv_sec+1E-9*tp0.tv_nsec),nst,nst/allt,allt,evt_->getEventNumber()); 
//return;
//getchar();
#endif
  for (std::vector<DIFPtr*>::iterator it = reader_->getDIFList().begin();it!=reader_->getDIFList().end();it++)
    {
      DIFPtr* d = (*it);
      if (d->getID()>255) continue;
     // Loop on frames
      for (uint32_t i=0;i<d->getNumberOfFrames();i++)
      {
	double t=d->getFrameTimeToTrigger(i)*2E-7;

	if (t>3.8) {
	  printf("Wrong Time %f %x \n",t,d->getFrameTimeToTrigger(i));
	  continue;
	}
	if (dbase->getDTC()==17 && d->getFrameTimeToTrigger(i)>80385 && d->getFrameTimeToTrigger(i)<80395 )
	  {
	    std::cout<<d->getID()<<" "<<d->getAbsoluteBCID()<<" "<<d->getFrameTimeToTrigger(i);
	    //getchar();
	  }
	//printf("%d %d \n",d->getID(),d->getFrameAsicHeader(i));
	if (d->getFrameAsicHeader(i)>48) continue;
	theCount_[d->getID()-1][0]++;
	theCount_[d->getID()-1][d->getFrameAsicHeader(i)]++;
	if (t>theEventTotalTime_) theEventTotalTime_=t;
	// Fill ASICs histogram
	//if (theTotalTime_<1.) continue;
	//if ((theTotalCount_[d->getID()-1][d->getFrameAsicHeader(i)]/theTotalTime_)<700.) continue;
	std::stringstream s;
	s<<"/RAW/DIF"<<d->getID()<<"/Asic"<<d->getFrameAsicHeader(i);
	TH1* han=rootHandler_->GetTH1(s.str()+"/Hits");
	TH1* han20=rootHandler_->GetTH1(s.str()+"/Hits20");
	TH1* hfr=rootHandler_->GetTH1(s.str()+"/Frequency");
	TH1* hframetime=rootHandler_->GetTH1(s.str()+"/FrameTime");
	TProfile* hmul=(TProfile*) rootHandler_->GetTH1(s.str()+"/Multiplicity");
	if (han==NULL)
	  {
	    printf("booking %s \n",s.str().c_str());
	    han =rootHandler_->BookTH1(s.str()+"/Hits",64,0.1,64.1);
	    han20 =rootHandler_->BookTH1(s.str()+"/Hits20",64,0.1,64.1);
	    hfr =rootHandler_->BookTH1(s.str()+"/Frequency",64,0.1,64.1);
	    hframetime =rootHandler_->BookTH1(s.str()+"/FrameTime",2000,0.,2000.);
	    hmul =rootHandler_->BookProfile(s.str()+"/Multiplicity",64,0.1,64.1,0.5,5.);
	  }
	hframetime->Fill(d->getFrameTimeToTrigger(i)*1.);
	uint32_t npad=0;
	for (uint32_t j=0;j<64;j++)
	   {
	     if (!(d->getFrameLevel(i,j,0) || d->getFrameLevel(i,j,1))) continue;
	     npad++;
	   }
	 for (uint32_t j=0;j<64;j++)
	   {
	     if (!(d->getFrameLevel(i,j,0) || d->getFrameLevel(i,j,1))) continue;
	     han->Fill(j*1.);
	     hmul->Fill(j*1.,npad*1.);
	     if (d->getFrameTimeToTrigger(i)<20)
	       han20->Fill(j*1.);
	   }
      }
    }
  hacqtime->Fill(theEventTotalTime_);
  theTotalTime_+=theEventTotalTime_;
  printf("Processing %d - %d GTC %d Total time %f Acquition time %f\n",evt_->getRunNumber(),evt_->getEventNumber(),dbase->getGTC(),(dbase->getAbsoluteBCID()-theStartBCID_)*2E-7,theTotalTime_);
  if (evt_->getEventNumber()%100 ==0)
    rootHandler_->writeSQL();

  // Now loop on DIF
  for (uint32_t i=0;i<255;i++)
    {
      if (!theCount_[i][0]) continue;
      for (uint32_t j=0;j<49;j++)
	theTotalCount_[i][j]+=theCount_[i][j];
      uint32_t difid=i+1;
    }
  for (uint32_t i=0;i<255;i++)
    {
      if (!theTotalCount_[i][0]) continue;
      for (uint32_t j=1;j<49;j++)
	{
	  hfr2->SetBinContent(i+1,j,theTotalCount_[i][j]/theTotalTime_);
	  if (theTotalTime_<1.) continue;
	  if ((theTotalCount_[i][j]/theTotalTime_)<0.) continue;
	  std::stringstream s;
	  s<<"/RAW/DIF"<<i+1<<"/Asic"<<j;
	  TH1* han=rootHandler_->GetTH1(s.str()+"/Hits");
	  TH1* hfr=rootHandler_->GetTH1(s.str()+"/Frequency");
	  if (han==NULL) continue;
	  for (uint32_t k=0;k<64;k++)
	     hfr->SetBinContent(k+1,han->GetBinContent(k+1)/theTotalTime_);


	  
	}
      
    }
  
  /*
  std::map<unsigned int,DifGeom>::iterator idg = reader_->getDifMap().find(d->getID());
  DifGeom& difgeom = idg->second;
  uint32_t chid = idg->second.getChamberId();
  */
  //  LCTOOLS::printParameters(rhcol->parameters());
  //DEBUG_PRINT("Time Stamp %d \n",evt_->getTimeStamp());

}
