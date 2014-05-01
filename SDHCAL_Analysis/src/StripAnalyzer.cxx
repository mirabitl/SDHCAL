#define NX 36
#define NY 36

#include "StripAnalyzer.h"
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




StripAnalyzer::StripAnalyzer() :nAnalyzed_(0),theMonitoringPeriod_(0),theMonitoringPath_("/dev/shm/Monitoring"),theSkip_(0),draw_(false)
{
  reader_=DHCalEventReader::instance();
  rootHandler_ =DCHistogramHandler::instance();
  this->initialise();
}



void StripAnalyzer::initialise()
{

  theTotalTime_=0.;
  memset(theTotalCount_,0,255*49*sizeof(uint32_t));
  theStartBCID_=0;  
}
void StripAnalyzer::initJob(){presetParameters();}
void StripAnalyzer::endJob(){
  if (theMonitoringPeriod_!=0)
    {
      rootHandler_->writeXML(theMonitoringPath_);
      //char c;c=getchar();putchar(c); if (c=='.') exit(0);;
		
    }
	
}
void StripAnalyzer::presetParameters()
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
#define PITCH 2.5E-1
class StripCluster
{
public:
  StripCluster(int is){strips_.clear();strips_.push_back(is);calcpos();}
  bool addStrip(int is)
  {
    bool merged=false;
    for (int i=0;i<strips_.size();i++) 
      {merged=((is-strips_[i])<2 &&(is-strips_[i])>-2)  ;if (merged) break;} 
    if (merged) {strips_.push_back(is);calcpos();}
    return merged;
  } 
  float getXLocal(){return xlocal_;}
  float getXGeom(){return xgeom_;}
  uint8_t getSize(){return strips_.size();}
private:
  void calcpos()
  {
    xlocal_=0;
    for (int i=0;i<strips_.size();i++) {xlocal_+=strips_[i];}
    xlocal_*=1./strips_.size();
    xgeom_=xlocal_*PITCH;
  }
  std::vector<int8_t> strips_;
  float xlocal_,xgeom_;
};

class StripGeom 
{
public:
  uint16_t difid[2];
  uint16_t asicid[2];
  uint16_t strips[128];
  uint16_t timeid[2];
  uint16_t asictime[2];
  uint32_t coarse[2];
  uint32_t time0_0[2];
  uint32_t time0_1[2];
  std::vector<StripCluster> clusters;
  void buildClusters()
  {
    for (uint8_t i=0;i<128;i++)
      {
	if (strips[i]==0) continue;
	bool merged=false;
	for ( std::vector<StripCluster>::iterator it=clusters.begin();it!=clusters.end();it++)
	  {

	    bool added=it->addStrip(i);
	    merged|=added;
	    if (merged) break;
	  }
	if (merged) continue;
	StripCluster s(i);
	clusters.push_back(s);
      }
  }
};



void StripAnalyzer::processEvent()
{
  
  StripGeom chambers[2];
  chambers[0].difid[0]=125;
  chambers[0].asicid[0]=1;
  chambers[0].difid[1]=94;
  chambers[0].asicid[1]=1;
  chambers[0].timeid[0]=6;
  chambers[0].asictime[0]=1;
  chambers[0].timeid[1]=30;
  chambers[0].asictime[1]=1;
  chambers[1].difid[0]=125;
  chambers[1].asicid[0]=2;
  chambers[1].difid[1]=94;
  chambers[1].asicid[1]=2;
  chambers[1].timeid[0]=6;
  chambers[1].asictime[0]=2;
  chambers[1].timeid[1]=30;
  chambers[1].asictime[1]=2;
  if (reader_->getEvent()==0) return;
  evt_=reader_->getEvent();
  if (evt_->getEventNumber()<=theSkip_) return;
  /*
  TH1* hacqtime=rootHandler_->GetTH1("AcquisitionTime");
  TH2* hfr2=rootHandler_->GetTH2("HitFrequency");
  if (hacqtime==NULL)
    {
      hacqtime =rootHandler_->BookTH1( "AcquisitionTime",2000,0.,2.);

      hfr2=rootHandler_->BookTH2("HitFrequency",255,0.1,255.1,48,0.1,48.1);
    }
  printf("4\n");
   reader_->parseRawEvent();
  */
  std::vector<DIFPtr*>::iterator itb =reader_->getDIFList().begin();
  if (itb==reader_->getDIFList().end()) return;
				      
  DIFPtr* dbase= (*itb);
  
  if ( theStartBCID_==0) theStartBCID_=dbase->getAbsoluteBCID();
  theEventTotalTime_=0; 
 float CalibT0_0 = 7.14;
float CalibDeltaT_0 =0.053;
float CalibT0_1 = 7.142;
float CalibDeltaT_1 =0.0996;
 
  
  memset(theCount_,0,255*49*sizeof(uint32_t));
   for (int ich=0;ich<2;ich++)
     memset(chambers[ich].strips,0,128*sizeof(uint16_t));
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
	//printf("%d %d \n",d->getID(),d->getFrameAsicHeader(i));
	if (d->getFrameAsicHeader(i)>48) continue;
	if (d->getFrameTimeToTrigger(i)<20)
	  {
	    
	    for (int ich=0;ich<2;ich++)
	      {
		for (int iasic=0;iasic<2;iasic++)
		  {
		  if ((d->getID() == chambers[ich].difid[iasic]) &&  (d->getFrameAsicHeader(i) == chambers[ich].asicid[iasic]))
		    {
		      for (uint32_t j=0;j<64;j++)
			{
			  if (!(d->getFrameLevel(i,j,0) || d->getFrameLevel(i,j,1))) continue;
			  int istrip=0;
			  if (iasic==0)
			    istrip=(64-(j+1))*2+1;
			  else
			    istrip=(j+1)*2;
				    
			  //printf("DIF %d ASIC %d Time %d  pad %d strip %d\n",d->getID(),d->getFrameAsicHeader(i),d->getFrameTimeToTrigger(i),j,istrip);
			  if (ich==1 && istrip==128) continue ; //noisy
			  if (d->getFrameLevel(i,j,0)) chambers[ich].strips[istrip-1]= 2;
			  if (d->getFrameLevel(i,j,1)) chambers[ich].strips[istrip-1]+= 1;
			}
		    }
		if ((d->getID() == chambers[ich].timeid[iasic]) &&  (d->getFrameAsicHeader(i) == chambers[ich].asictime[iasic]))
		    {
		      printf("DIF %d ASIC %d Time %d  \n",d->getID(),d->getFrameAsicHeader(i),d->getFrameTimeToTrigger(i));
		      int32_t i1=(d->getFrameData(i,18)) |(d->getFrameData(i,17)<<8) ;
		      int32_t i0=(d->getFrameData(i,16)) |(d->getFrameData(i,15)<<8) ;
		      int32_t c0=(d->getFrameData(i,19)) &0x7;
		      for (int iw=0;iw<22;iw++)
			printf("%.2x ",d->getFrameData(i,iw));

		      if (iasic==0)
			{
			  float Time0_6 =       (i0 -i1)*CalibT0_0+i1*CalibDeltaT_0;
			  printf("=> %d %d %d-> %f %f ns\n",c0,i0,i1,Time0_6,c0*25+Time0_6);			  
			}
		      else
			{
			  float Time0_6 =       (i0 -i1)*CalibT0_1+i1*CalibDeltaT_1;
			  printf("=> %d %d %d-> %f %f ns\n",c0,i0,i1,Time0_6,c0*25+Time0_6);			  
			}
		    }
		  }
	      }
	  }

	// theCount_[d->getID()-1][0]++;
	// theCount_[d->getID()-1][d->getFrameAsicHeader(i)]++;
	// if (t>theEventTotalTime_) theEventTotalTime_=t;
	// // Fill ASICs histogram
	// //if (theTotalTime_<1.) continue;
	// //if ((theTotalCount_[d->getID()-1][d->getFrameAsicHeader(i)]/theTotalTime_)<700.) continue;
	// std::stringstream s;
	// s<<"/DIF"<<d->getID()<<"/Asic"<<d->getFrameAsicHeader(i);
	// TH1* han=rootHandler_->GetTH1(s.str()+"/Hits");
	// TH1* han20=rootHandler_->GetTH1(s.str()+"/Hits20");
	// TH1* hfr=rootHandler_->GetTH1(s.str()+"/Frequency");
	// TH1* hframetime=rootHandler_->GetTH1(s.str()+"/FrameTime");

	// if (han==NULL)
	//   {
	//     printf("booking %s \n",s.str().c_str());
	//     han =rootHandler_->BookTH1(s.str()+"/Hits",64,0.1,64.1);
	//     han20 =rootHandler_->BookTH1(s.str()+"/Hits20",64,0.1,64.1);
	//     hfr =rootHandler_->BookTH1(s.str()+"/Frequency",64,0.1,64.1);
	//     hframetime =rootHandler_->BookTH1(s.str()+"/FrameTime",2000,0.,2000.);
      
	//   }
	// hframetime->Fill(d->getFrameTimeToTrigger(i)*1.);
	//  for (uint32_t j=0;j<64;j++)
	//    {
	//      if (!(d->getFrameLevel(i,j,0) || d->getFrameLevel(i,j,1))) continue;
	//      han->Fill(j*1.);
	//      if (d->getFrameTimeToTrigger(i)<20)
	//        han20->Fill(j*1.);
	//    }
      }
    }
  // hacqtime->Fill(theEventTotalTime_);
  // theTotalTime_+=theEventTotalTime_;
  // printf("Processing %d - %d Total time %f Acquition time %f\n",evt_->getRunNumber(),evt_->getEventNumber(),(dbase->getAbsoluteBCID()-theStartBCID_)*2E-7,theTotalTime_);

  for (int ich=0;ich<2;ich++)
    {
      std::stringstream s;
      s<<"/Strips"<<ich;
      TH1* hncl=rootHandler_->GetTH1(s.str()+"/nbclusters");
      TH1* hsize=rootHandler_->GetTH1(s.str()+"/clustersize");
      TH1* hpos=rootHandler_->GetTH1(s.str()+"/pos");

      if (hncl==NULL)
	{
	  hncl =rootHandler_->BookTH1(s.str()+"/nbclusters" ,30,0.,30.);
	  hsize =rootHandler_->BookTH1(s.str()+"/clustersize" ,30,0.,30.);
	  hpos =rootHandler_->BookTH1(s.str()+"/pos" ,128,0.,128.);
	}
      /*for (int j=0;j<128;j++) printf("%1d",chambers[ich].strips[j]);
	printf("\n"); */
      chambers[ich].buildClusters();
      //printf("N cluster %d \n",chambers[ich].clusters.size());
      hncl->Fill(chambers[ich].clusters.size());
      for (std::vector<StripCluster>::iterator it=chambers[ich].clusters.begin();it!=chambers[ich].clusters.end();it++)
	{
	  //printf("%d %f %f \n",it->getSize(),it->getXLocal(),it->getXGeom());
	  hsize->Fill(it->getSize());
	  hpos->Fill(it->getXLocal());
	}
    }
  
  getchar();

  // Now loop on DIF
  // for (uint32_t i=0;i<255;i++)
  //   {
  //     if (!theCount_[i][0]) continue;
  //     for (uint32_t j=0;j<49;j++)
  // 	theTotalCount_[i][j]+=theCount_[i][j];
  //     uint32_t difid=i+1;
  //   }
  // for (uint32_t i=0;i<255;i++)
  //   {
  //     if (!theTotalCount_[i][0]) continue;
  //     for (uint32_t j=1;j<49;j++)
  // 	{
  // 	  hfr2->SetBinContent(i+1,j,theTotalCount_[i][j]/theTotalTime_);
  // 	  if (theTotalTime_<1.) continue;
  // 	  if ((theTotalCount_[i][j]/theTotalTime_)<700.) continue;
  // 	  std::stringstream s;
  // 	  s<<"/DIF"<<i+1<<"/Asic"<<j;
  // 	  TH1* han=rootHandler_->GetTH1(s.str()+"/Hits");
  // 	  TH1* hfr=rootHandler_->GetTH1(s.str()+"/Frequency");
  // 	  if (han==NULL) continue;
  // 	  for (uint32_t k=0;k<64;k++)
  // 	     hfr->SetBinContent(k+1,han->GetBinContent(k+1)/theTotalTime_);


	  
  // 	}
      
  //   }
  

  /*
  std::map<unsigned int,DifGeom>::iterator idg = reader_->getDifMap().find(d->getID());
  DifGeom& difgeom = idg->second;
  uint32_t chid = idg->second.getChamberId();
  */
  //  LCTOOLS::printParameters(rhcol->parameters());
  //DEBUG_PRINT("Time Stamp %d \n",evt_->getTimeStamp());

}
