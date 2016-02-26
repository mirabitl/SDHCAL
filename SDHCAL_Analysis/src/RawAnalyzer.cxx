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
  _neff=0;_neff2=0;_neff3=0;_nall=0;
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
static TCanvas* TCPlot=NULL;
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
  
  
  if (TCPlot==NULL)
    {
      TCPlot=new TCanvas("TCPlotR","tcplotr1",1300,900);
      TCPlot->Modified();
      TCPlot->Draw();
      TCPlot->Divide(2,3);
    }
  memset(theCount_,0,255*49*sizeof(uint32_t));
  struct timespec tp0,tp1;
  clock_gettime(CLOCK_REALTIME,&tp0);
  int64_t tlast=0;
#define   ENBEAMTEST
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

      printf("DIF %d BCID %d %d frames \n",d->getID(),d->getBCID(),d->getNumberOfFrames());
      for (uint32_t i=0;i<d->getNumberOfFrames();i++)
      {
	if (d->getID()!=256)
	  {
	    //printf("%d ",d->getFrameTimeToTrigger(i));
	    int32_t idt=d->getBCID()-d->getFrameBCID(i);
	    if (idt<0)
	      printf(" error frame %d  FBCID %d  ASIC %d DIF %d  BCID %d\n",i,d->getFrameBCID(i),d->getFrameAsicHeader(i),d->getID(),d->getBCID());
	  }
	

	double t=d->getFrameTimeToTrigger(i)*2E-7;

	if (t>3.8) {
	  printf("Wrong Time DT  %f  FTTT %x BCID %lx  Frame BCID %lx \n",t,d->getFrameTimeToTrigger(i),d->getBCID(),d->getFrameBCID(i));
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
      printf("\n");
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
#ifdef OLDTRICOT
 int ipl=0;
 std::map<uint8_t,uint8_t> plid;
 std::map<uint8_t,float> pldx;
 std::map<uint8_t,float> pldy;
 std::map<uint8_t,float> plz;
 plid.insert(std::pair<uint8_t,uint8_t>(5,1));
 plid.insert(std::pair<uint8_t,uint8_t>(94,2));
 plid.insert(std::pair<uint8_t,uint8_t>(99,3));
 plid.insert(std::pair<uint8_t,uint8_t>(128,4));
  plid.insert(std::pair<uint8_t,uint8_t>(1,5));
 plz.insert(std::pair<uint8_t,uint8_t>(5,10.));
 plz.insert(std::pair<uint8_t,uint8_t>(94,29.));
 plz.insert(std::pair<uint8_t,uint8_t>(99,45.));
 plz.insert(std::pair<uint8_t,uint8_t>(128,61.));
 plz.insert(std::pair<uint8_t,uint8_t>(1,77.));
 pldx.insert(std::pair<uint8_t,uint8_t>(5,8.));
 pldx.insert(std::pair<uint8_t,uint8_t>(94,0.));
 pldx.insert(std::pair<uint8_t,uint8_t>(99,2.));
 pldx.insert(std::pair<uint8_t,uint8_t>(128,0.));
 pldx.insert(std::pair<uint8_t,uint8_t>(1,0.));
 pldy.insert(std::pair<uint8_t,uint8_t>(5,0.));
 pldy.insert(std::pair<uint8_t,uint8_t>(94,0.));
 pldy.insert(std::pair<uint8_t,uint8_t>(99,3.));
 pldy.insert(std::pair<uint8_t,uint8_t>(128,8.));
 pldy.insert(std::pair<uint8_t,uint8_t>(1,0.));
 TH2* hplx=rootHandler_->GetTH2("/plx");
 TH2* hply=rootHandler_->GetTH2("/ply");
 if (hplx==NULL)
   {
     hplx=rootHandler_->BookTH2("/plx",70,0.,70.,100,-10.,40.);
     hply=rootHandler_->BookTH2("/ply",70,0.,70.,100,-10.,40.);
   }
 else
   {
     hplx->Reset();
     hply->Reset();
   }

 std::bitset<64> plhit(0);

  for (std::vector<DIFPtr*>::iterator it = reader_->getDIFList().begin();it!=reader_->getDIFList().end();it++)
    {
      DIFPtr* d = (*it);
      if (d->getID()>255) continue;
      std::stringstream s1;
      s1<<"/Plot"<<d->getID();
      TH2* hpl=rootHandler_->GetTH2(s1.str());
      if (hpl==NULL)
	{
	  hpl=rootHandler_->BookTH2(s1.str(),40,-10.,30.,40,-10.,30.);
	}
      else
      	hpl->Reset();
   
      std::vector<float> va_,vb_;
      va_.clear();
      vb_.clear();
      std::vector<StripCluster*> _cl;

      
     // Loop on frames
      uint32_t dmin=1000;
      uint32_t npd=0;
      for (uint32_t i=0;i<d->getNumberOfFrames();i++)
      {
	double t=d->getFrameTimeToTrigger(i)*2E-7;
	if (d->getFrameTimeToTrigger(i)<dmin) 
	  {
	    dmin=d->getFrameTimeToTrigger(i);
	    npd=0;
	    for (uint32_t j=0;j<64;j++)
	      {
		if (!(d->getFrameLevel(i,j,0) || d->getFrameLevel(i,j,1))) continue;
		npd++;
	      }

	  }

	if (t>3.8) {
	  printf("Wrong Time %f %x %d %d %x %x \n",t,d->getFrameTimeToTrigger(i),d->getBCID(),d->getFrameBCID(i),d->getBCID(),d->getFrameBCID(i));
	  ipl++;
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

      
      std::stringstream s0;
      s0<<"/TIME/DIF"<<d->getID();
      TH1* hfratime=rootHandler_->GetTH1(s0.str()+"/FrameTime");
      TH1* hnpd=rootHandler_->GetTH1(s0.str()+"/Npad");
  

      TH2* hmit=rootHandler_->GetTH2(s0.str()+"/HitMapIT");
      if (hmit==NULL)
	{
	  hfratime =rootHandler_->BookTH1(s0.str()+ "/FrameTime",2000,0.,2000.);
	  hnpd =rootHandler_->BookTH1( s0.str()+"/Npad",64,0.1,64.1);
	  hmit=rootHandler_->BookTH2(s0.str()+"/HitMapIT",48,0.1,48.1,32,0.1,32.1);
	}

      hfratime->Fill(dmin*1.);
   

      if (dmin>13.5 && dmin<20.5)
	{
	  printf("DIF %d :",d->getID());
	for (uint32_t i=0;i<d->getNumberOfFrames();i++)
	  {

	    if (d->getFrameTimeToTrigger(i)==dmin) 
	      {
		npd=0;

		for (uint32_t j=0;j<64;j++)
		  {
		    if (!(d->getFrameLevel(i,j,0) || d->getFrameLevel(i,j,1))) continue;
		    int asic=d->getFrameAsicHeader(i),channel=j+1,x=0,y=0;

		    DifGeom::PadConvert(asic,channel,x,y,2);

		    hmit->Fill(x*1.,y*1.,1.);
		    npd++;
		  }
		hnpd->Fill(npd*1.);

	      }
	    if (d->getFrameTimeToTrigger(i)>13.5 &&  d->getFrameTimeToTrigger(i)<20.5)
	      
	      for (uint32_t j=0;j<64;j++)
		{
		  if (!(d->getFrameLevel(i,j,0) || d->getFrameLevel(i,j,1))) continue;
		  int asic=d->getFrameAsicHeader(i),channel=j+1;
		  printf("(%d,%d,%d)-",d->getFrameTimeToTrigger(i),asic,channel);
		  bool ap=false;
		  for (std::vector<StripCluster*>::iterator it=_cl.begin();it!=_cl.end();it++)
		    {ap=ap || (*it)->append(asic,channel); if (ap) break;}
		  if (!ap)
		    {
		      StripCluster* sc=new StripCluster(asic,channel);
		      _cl.push_back(sc);
		    }
	
		}
	  }
	
	printf("Strips hit %d \n",_cl.size());
	for (std::vector<StripCluster*>::iterator it=_cl.begin();it!=_cl.end();it++)
	  printf("(%d-%f)-",(*it)->asic(),(*it)->x());
	printf("\n");
	//hpl->Reset();
	uint32_t np=0;
	for (int i=0;i<_cl.size();i++)
	  for(int j=i+1;j<_cl.size();j++)
	    {
	      if(_cl[i]->asic()==_cl[j]->asic()) continue;
	      for(int k=j+1;k<_cl.size();k++)
		{
		  if(_cl[i]->asic()==_cl[k]->asic()) continue;
		  if(_cl[j]->asic()==_cl[k]->asic()) continue;
		  np++;
		}
	    }
	
	for (int i=0;i<_cl.size();i++)
	  for(int j=i+1;j<_cl.size();j++)
	    {
	      if(_cl[i]->asic()==_cl[j]->asic()) continue;
	      for(int k=j+1;k<_cl.size();k++)
		{
		  if(_cl[i]->asic()==_cl[k]->asic()) continue;
		  if(_cl[j]->asic()==_cl[k]->asic()) continue;
	      
	      
		  float x1=(_cl[j]->b()-_cl[i]->b())/(_cl[i]->a()-_cl[j]->a());
		  float y1=_cl[i]->a()*x1+_cl[i]->b();
		  float x2=(_cl[k]->b()-_cl[i]->b())/(_cl[i]->a()-_cl[k]->a());
		  float y2=_cl[i]->a()*x2+_cl[i]->b();
		  float x3=(_cl[k]->b()-_cl[j]->b())/(_cl[j]->a()-_cl[k]->a());
		  float y3=_cl[j]->a()*x3+_cl[j]->b();
		  float x=(x1+x2+x3)/3.;
		  float y=(y1+y2+y3)/3.;
		  if (sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2))>2) continue;
		  if (sqrt((x1-x3)*(x1-x3)+(y1-y3)*(y1-y3))>2) continue;
		  if (sqrt((x3-x2)*(x3-x2)+(y3-y2)*(y3-y2))>2) continue;
		  _cl[i]->setUsed();
		  _cl[j]->setUsed();
		  _cl[k]->setUsed();
		  x+=pldx[d->getID()];
		  y+=pldy[d->getID()];
		  printf("(%f,%f)-",x,y);
		  hpl->Fill(x,y);
		  hplx->Fill(plz[d->getID()],x);
		  hply->Fill(plz[d->getID()],y);
		  plhit.set(plid[d->getID()]);
		  
		}
	    }
	for (int i=0;i<_cl.size();i++)
	  {
	    if (_cl[i]->isUsed()) continue;
	  for(int j=i+1;j<_cl.size();j++)
	    {
	      if (_cl[j]->isUsed()) continue;
	      if(_cl[i]->asic()==_cl[j]->asic()) continue;
	      float x=(_cl[j]->b()-_cl[i]->b())/(_cl[i]->a()-_cl[j]->a());
	      float y=_cl[i]->a()*x+_cl[i]->b();
	      _cl[i]->setUsed();
	      _cl[j]->setUsed();
	      x+=pldx[d->getID()];
	      y+=pldy[d->getID()];

	      printf("(%f,%f)-",x,y);
	      plhit.set(plid[d->getID()]);
	      hpl->Fill(x,y);
	      hplx->Fill(plz[d->getID()],x);
	      hply->Fill(plz[d->getID()],y);
	    }
	    
	  }
	printf("\n");

	}
#define DRAWPLOT  
#ifdef DRAWPLOT
      TCPlot->cd(plid[d->getID()]);hpl->Draw("COLZ");ipl++;      
      TCPlot->Modified();
      TCPlot->Draw();
      TCPlot->Update();
#endif
      for (std::vector<StripCluster*>::iterator it=_cl.begin();it!=_cl.end();it++)
	delete (*it);
    }

#ifdef DRAWPLOT
  //  TCPlot->cd(5);hplx->Draw("COLZ");
  TCPlot->cd(6);hply->Draw("COLZ");      
      TCPlot->Modified();
      TCPlot->Draw();
      TCPlot->Update();
      getchar();
#endif


  if (plhit[1] || plhit[2] || plhit[3] ||plhit[4]) _nall++;
  if (plhit[1] && plhit[4])
    {
      _neff++;
      if (plhit[2]) _neff2++;
      if (plhit[3]) _neff3++;
    }
#endif
    hacqtime->Fill(theEventTotalTime_);
  theTotalTime_+=theEventTotalTime_;

  printf("Processing %d - %d GTC %d Total time %f Acquition time %f all %d [1-5] %d  [1-2-5]  %d [1-3-5] %d \n",evt_->getRunNumber(),evt_->getEventNumber(),dbase->getGTC(),(dbase->getAbsoluteBCID()-theStartBCID_)*2E-7,theTotalTime_,_nall,_neff,_neff2,_neff3);


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
