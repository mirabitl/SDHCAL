#include <Math/Point3Dfwd.h>

#include "combAnalysis.hh"
#include "DIFUnpacker.h"
#include <TCanvas.h>
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
#include "CaloTrans.h"
#include "jsonGeo.hh"
#include "buffer.hh"

static TCanvas* TCPlot=NULL;
static TCanvas* TCHits=NULL;
static TCanvas* TCShower=NULL;
static TCanvas* TCEdge=NULL;
static TCanvas* TCHT=NULL;
static TCanvas* TCCluster=NULL;

combAnalysis::combAnalysis() :nAnalyzed_(0),theMonitoringPeriod_(0),theMonitoringPath_("/dev/shm/Monitoring"),theSkip_(0),draw_(false),_geo(NULL),_monitor(NULL)
{
  reader_=DHCalEventReader::instance();
  rootHandler_ =DCHistogramHandler::instance();
  this->initialise();
}



void combAnalysis::initialise()
{

  theTotalTime_=0.;
  memset(theTotalCount_,0,255*49*sizeof(uint32_t));
  theStartBCID_=0;  
}
void combAnalysis::initJob(){
  presetParameters();
  if (_geo!=NULL)
    _monitor=new hitMonitor(_geo);
}
void combAnalysis::endJob(){
  if (theMonitoringPeriod_!=0)
    {
      rootHandler_->writeXML(theMonitoringPath_);
      //char c;c=getchar();putchar(c); if (c=='.') exit(0);;
		
    }
	
}
void combAnalysis::presetParameters()
{
  std::map<std::string,MarlinParameter> m=reader_->getMarlinParameterMap();
  std::map<std::string,MarlinParameter>::iterator it;
  try
    {
      if ((it=m.find("Interactif"))!=m.end()) draw_=it->second.getBoolValue();
      if ((it=m.find("SkipEvents"))!=m.end()) theSkip_=it->second.getIntValue();
      if ((it=m.find("MonitoringPath"))!=m.end()) theMonitoringPath_=it->second.getStringValue();
      if ((it=m.find("MonitoringPeriod"))!=m.end()) theMonitoringPeriod_=it->second.getIntValue();

      INFO_PRINT("Interactif %d \n",draw_);
      //std::string geofile;
      if ((it=m.find("geometry"))!=m.end())
	_geo=new jsonGeo(it->second.getStringValue());
      

    }
  catch (std::string s)
    {
      std::cout<<__PRETTY_FUNCTION__<<" error "<<s<<std::endl;
    }
	
}


void combAnalysis::processEvent()
{
  
  if (reader_->getEvent()==0) return;
  evt_=reader_->getEvent();
  if (evt_->getEventNumber()<=theSkip_) return;
  TH1* hacqtime=rootHandler_->GetTH1("AcquisitionTime");
  TH2* hfr2=rootHandler_->GetTH2("HitFrequency");
  if (hacqtime==NULL)
    {
      hacqtime =rootHandler_->BookTH1( "AcquisitionTime",2000,0.,2.);

      hfr2=rootHandler_->BookTH2("HitFrequency",255,0.1,255.1,48,0.1,48.1);
    }

  // Loop on SDHCAL DIFS and find seeds
   std::vector<std::string >* vnames;
  try {
    vnames= (std::vector<std::string >*)evt_->getCollectionNames();
  }
  catch (IOException& e) {
    std::cout << e.what() << std::endl ;
    std::string s=e.what();s+=__PRETTY_FUNCTION__;
    throw s;
  }
  catch (...)
    {
      std::cout<<" No se perque on se plante "<<std::endl;
      exit(2);
    }
  uint32_t dbcid[255];
  memset(dbcid,0,255*4);
     for ( std::vector<std::string >::iterator it=(vnames)->begin();it!=vnames->end();it++)
    {
      if ((*it).compare("RU_XDAQ")!=0) continue;
      //      std::cout<<"Collection on  ulle"<<std::endl;
      EVENT::LCCollection* col= evt_->getCollection(*it); 
      //std::vector<unsigned char*> vF;
      //std::vector<unsigned char*> vL;
      

      for (int j=0;j<col->getNumberOfElements(); j++)
	{
	  //IMPL::LCGenericObjectImpl* go= (IMPL::LCGenericObjectImpl*) col->getElementAt(j);
	  //if (j==0 && dropFirstRU_) continue;
	  LMGeneric* go= (LMGeneric*) col->getElementAt(j);
	  int* buf=&(go->getIntVector()[0]);




	  unsigned char* tcbuf = (unsigned char*) buf;
	  uint32_t rusize =go->getNInt()*sizeof(int32_t);
	  levbdim::buffer m((char*) tcbuf,0);
	  m.setPayloadSize(rusize-3*sizeof(uint32_t)-sizeof(uint64_t));
	  //std::cout<<" Source found "<<m.detectorId()<<" "<<m.dataSourceId()<<std::endl;
	  if (m.detectorId()!=100) continue;
	   uint32_t idstart=DIFUnpacker::getStartOfDIF((unsigned char*) m.ptr(),m.size(),20);
	  //std::cout<<" Start at "<<idstart<<std::endl;
	  //bool slowcontrol; uint32_t version,hrtype,id0,iddif;
	  //DCBufferReader::checkType(tcbuf,rusize/4,slowcontrol,version,hrtype,id0,iddif,theXdaqShift_);
	  //printf(" Found start of Buffer at %d contains %x and %d bytes \n",idstart,tcbuf[idstart],rusize-idstart+1);

	  
	  

	  unsigned char* tcdif=&tcbuf[idstart];
	  DIFPtr* d= new DIFPtr(tcdif,rusize-idstart+1);
	  dbcid[d->getID()]=d->getBCID();
	}
    }



  
  std::map<uint32_t,uint32_t> mecal;
  std::map<uint32_t,uint32_t> mecalp;
  mecal.clear();
  mecalp.clear();
   for ( std::vector<std::string >::iterator it=(vnames)->begin();it!=vnames->end();it++)
    {
      if ((*it).compare("RU_XDAQ")!=0) continue;
      //      std::cout<<"Collection on  ulle"<<std::endl;
      EVENT::LCCollection* col= evt_->getCollection(*it); 
      //std::vector<unsigned char*> vF;
      //std::vector<unsigned char*> vL;
      

      for (int j=0;j<col->getNumberOfElements(); j++)
	{
	  //IMPL::LCGenericObjectImpl* go= (IMPL::LCGenericObjectImpl*) col->getElementAt(j);
	  //if (j==0 && dropFirstRU_) continue;
	  LMGeneric* go= (LMGeneric*) col->getElementAt(j);
	  int* buf=&(go->getIntVector()[0]);




	  unsigned char* tcbuf = (unsigned char*) buf;
	  uint32_t rusize =go->getNInt()*sizeof(int32_t);
	  levbdim::buffer m((char*) tcbuf,0);
	  m.setPayloadSize(rusize-3*sizeof(uint32_t)-sizeof(uint64_t));
	  //std::cout<<" Source found "<<m.detectorId()<<" "<<m.dataSourceId()<<std::endl;
	  if (m.detectorId()!=1100) continue;
	  struct CaloTransHit *ptr=(struct CaloTransHit*) m.payload();
	  
	  uint32_t nhit=m.payloadSize()/sizeof(struct CaloTransHit);
	  //printf("DIF %d %d has %d hits  %d %d \n",m.detectorId(),m.dataSourceId(),nhit,m.payloadSize(),sizeof(struct CaloTransHit));
	  for (int i=0;i<nhit;i++)
	    {
	      //printf("%d %f %f %f %f %f \n",ptr->asic_bcid,ptr->time,ptr->energy,ptr->x,ptr->y,ptr->z);
	      uint32_t ti=ptr->asic_bcid,tf=0;
	      std::map<uint32_t,uint32_t>::iterator imf=mecal.find(ti);
	      if (imf==mecal.end())
		imf=mecal.find(ptr->asic_bcid-1);
	      else
		tf=ti;
	      if (imf==mecal.end())
		imf=mecal.find(ptr->asic_bcid+1);
	      else
		tf=ti;
	      if (imf==mecal.end())
		imf=mecal.find(ptr->asic_bcid-2);
	      else
		tf=ti;
	      if (imf==mecal.end())
		imf=mecal.find(ptr->asic_bcid+2);
	      else
		tf=ti;
	      if (imf==mecal.end())
		{
		  mecal.insert(std::pair<uint32_t,uint32_t>(ptr->asic_bcid,1));
		  mecalp.insert(std::pair<uint32_t,uint32_t>(ptr->asic_bcid,1<<m.dataSourceId()));
		}
	      else
		{
		imf->second++;
		std::map<uint32_t,uint32_t>::iterator imfp=mecalp.find(tf);
		imfp->second |=(1<<m.dataSourceId());
		}    
	      ptr++;
	    }
	}
    }
   printf("%d %d =>\n",evt_->getRunNumber(),evt_->getEventNumber());
   std::vector<uint32_t> vcand;
   vcand.clear();
   bool good=false;
   for (std::map<uint32_t,uint32_t>::iterator imf=mecal.begin();imf!=mecal.end();imf++)
     {
       std::bitset<32> mask(mecalp[imf->first]);
       if (mask.count()<4) continue;
       std::cout<<mask<<std::endl;
       printf("%d -> %d mask count %d \n",imf->first*2,imf->second,mask.count());
       vcand.push_back(imf->first*2);
       good=true;
     }
   if (!good) return;
   //getchar();
    std::map<uint32_t,uint32_t> mhcal;
  mhcal.clear();
   for ( std::vector<std::string >::iterator it=(vnames)->begin();it!=vnames->end();it++)
    {
      if ((*it).compare("RU_XDAQ")!=0) continue;
      //      std::cout<<"Collection on  ulle"<<std::endl;
      EVENT::LCCollection* col= evt_->getCollection(*it); 
      //std::vector<unsigned char*> vF;
      //std::vector<unsigned char*> vL;
      

      for (int j=0;j<col->getNumberOfElements(); j++)
	{
	  //IMPL::LCGenericObjectImpl* go= (IMPL::LCGenericObjectImpl*) col->getElementAt(j);
	  //if (j==0 && dropFirstRU_) continue;
	  LMGeneric* go= (LMGeneric*) col->getElementAt(j);
	  int* buf=&(go->getIntVector()[0]);




	  unsigned char* tcbuf = (unsigned char*) buf;
	  uint32_t rusize =go->getNInt()*sizeof(int32_t);
	  levbdim::buffer m((char*) tcbuf,0);
	  m.setPayloadSize(rusize-3*sizeof(uint32_t)-sizeof(uint64_t));
	  //std::cout<<" Source found "<<m.detectorId()<<" "<<m.dataSourceId()<<std::endl;
	  if (m.detectorId()!=201) continue;
	  struct CaloTransHit *ptr=(struct CaloTransHit*) m.payload();
	  
	  uint32_t nhit=m.payloadSize()/sizeof(struct CaloTransHit);
	  //printf("DIF %d %d has %d hits  %d %d \n",m.detectorId(),m.dataSourceId(),nhit,m.payloadSize(),sizeof(struct CaloTransHit));
	  
	  for (int i=0;i<nhit;i++)
	    {
	      //printf("%d %f %f %f %f %f %d\n",ptr->asic_bcid,ptr->time,ptr->energy,ptr->x,ptr->y,ptr->z,dbcid[m.dataSourceId()]);
	      uint32_t ti =dbcid[m.dataSourceId()]-ptr->asic_bcid;
	      std::map<uint32_t,uint32_t>::iterator imf=mhcal.find(ti);
	      if (imf==mhcal.end())
		imf=mhcal.find(ti-1);
	      if (imf==mhcal.end())
		imf=mhcal.find(ti+1);
	      if (imf==mhcal.end())
		imf=mhcal.find(ti-2);
	      if (imf==mhcal.end())
		imf=mhcal.find(ti+2);
	      if (imf==mhcal.end())
		mhcal.insert(std::pair<uint32_t,uint32_t>(ti,1));
	      else
		imf->second++;
			     
	      ptr++;
	    }
	}
    }
   printf("%d %d =>\n",evt_->getRunNumber(),evt_->getEventNumber());
   std::vector<uint32_t> vgood;
   vgood.clear();
   for (std::map<uint32_t,uint32_t>::iterator imf=mhcal.begin();imf!=mhcal.end();imf++)
     {
       bool found=false;
       for (int ic=0;ic<vcand.size();ic++)
	 {
	   double diff=vcand[ic]*1.-2490.-imf->first*1.;
	   if (abs(diff)<3 ) {
	   printf("%d %d \n",vcand[ic],(imf->first+2490));
	   found=true;break;}
	 }
       //if (imf->second>50)
       if (found)
	 {
	 printf("%d -> %d (+%d) \n",imf->first,imf->second,imf->first+2490);
	 vgood.push_back(imf->first+2490);
	 }
     }
   if (vgood.size()==0)
     return;

   for (uint32_t is=0;is<vgood.size();is++)
     {
       std::vector<ROOT::Math::XYZPoint> _epoints;
       std::vector<ROOT::Math::XYZPoint> _hpoints;
       printf("Seed %d \n",vgood[is]);
       for ( std::vector<std::string >::iterator it=(vnames)->begin();it!=vnames->end();it++)
	 {
	   if ((*it).compare("RU_XDAQ")!=0) continue;
	   EVENT::LCCollection* col= evt_->getCollection(*it); 


	   for (int j=0;j<col->getNumberOfElements(); j++)
	     {

	       LMGeneric* go= (LMGeneric*) col->getElementAt(j);
	       int* buf=&(go->getIntVector()[0]);




	       unsigned char* tcbuf = (unsigned char*) buf;
	       uint32_t rusize =go->getNInt()*sizeof(int32_t);
	       levbdim::buffer m((char*) tcbuf,0);
	       m.setPayloadSize(rusize-3*sizeof(uint32_t)-sizeof(uint64_t));
	  //std::cout<<" Source found "<<m.detectorId()<<" "<<m.dataSourceId()<<std::endl;
	       if (m.detectorId()==201)
		 {
		   struct CaloTransHit *ptr=(struct CaloTransHit*) m.payload();
	  
		   uint32_t nhit=m.payloadSize()/sizeof(struct CaloTransHit);
		   //printf("DIF %d %d has %d hits  %d %d \n",m.detectorId(),m.dataSourceId(),nhit,m.payloadSize(),sizeof(struct CaloTransHit));
	  
		   for (int i=0;i<nhit;i++)
		     {

		       uint32_t ti =dbcid[m.dataSourceId()]-ptr->asic_bcid+2490;
		       double tdist=ti*1-vgood[is]*1.;
		       //printf("%d %d %f \n",ti,vgood[is],tdist);
		       if (abs(tdist)>2 ) {ptr++;continue;}
		       //printf("%d %f %f %f %f %f %d\n",ptr->asic_bcid,ptr->time,ptr->energy,ptr->x,ptr->y,ptr->z,ti);
		       ROOT::Math::XYZPoint p(ptr->x,ptr->y,ptr->z);
		       _hpoints.push_back(p);
		       ptr++;
		     }
		 }
	       if (m.detectorId()==1100)
		 {
		   struct CaloTransHit *ptr=(struct CaloTransHit*) m.payload();
	  
		   uint32_t nhit=m.payloadSize()/sizeof(struct CaloTransHit);
		   //printf("DIF %d %d has %d hits  %d %d \n",m.detectorId(),m.dataSourceId(),nhit,m.payloadSize(),sizeof(struct CaloTransHit));
	  
		   for (int i=0;i<nhit;i++)
		     {

		       uint32_t ti =ptr->asic_bcid;
		       double tdist=ti*1-vgood[is]*0.5;
		       //printf("ecal %d %f \n",ti,tdist);
		       if (abs(tdist)>2 ) {ptr++;continue;}
		       //printf("%d %f %f %f %f %f %d\n",ptr->asic_bcid,ptr->time,ptr->energy,ptr->x,ptr->y,ptr->z,ti);
		       ROOT::Math::XYZPoint p(-ptr->y/10.+81.,-ptr->x/10.+75,+ptr->z/10.-117.);
		       _epoints.push_back(p);
		       ptr++;
		     }
		 }
	     }

	   // Plots
	   TH3* hcgposi = rootHandler_->GetTH3("InstantHitMap");
	   TH3* hcgpose = rootHandler_->GetTH3("InstantHitMapE");
	   if (hcgposi==NULL)
	     {
	       hcgposi =rootHandler_->BookTH3("InstantHitMap",330,-200.,130.,120,-10.,110.,120,-10.,110.);
	       hcgpose =rootHandler_->BookTH3("InstantHitMapE",330,-200.,130.,120,-10.,110.,120,-10.,110.);
	     }
	   else
	     {
	       hcgposi->Reset();
	       hcgpose->Reset();
	     }
	   if (hcgposi!=0 )
	     {
	       //hcgposi->Reset();
      
	       for (std::vector<ROOT::Math::XYZPoint>::iterator ih=_epoints.begin();ih!=_epoints.end();ih++)
		 {
		   hcgpose->Fill(ih->Z(),ih->X(),ih->Y());//(*ih)->Z(),(*ih)->X(),(*ih)->Y());

		   //std::cout<<"ECAL "<<ih->Z()<<" "<<ih->X()<<" "<<ih->Y()<<std::endl;
		 }
	       for (std::vector<ROOT::Math::XYZPoint>::iterator ih=_hpoints.begin();ih!=_hpoints.end();ih++)
		 {
		   hcgposi->Fill(ih->Z(),ih->X(),ih->Y());//(*ih)->Z(),(*ih)->X(),(*ih)->Y());
		   //std::cout<<"HCAL "<<ih->Z()<<" "<<ih->X()<<" "<<ih->Y()<<std::endl;
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
	       hcgpose->SetMarkerStyle(23);
	       hcgpose->SetMarkerColor(kBlue);
	       hcgpose->Draw("PSAME");

	        TCHits->cd(3);
		TProfile2D* hpx=hcgposi->Project3DProfile("yx");
		hpx->SetLineColor(kRed);
		
		hpx->Draw("BOX");
		TProfile2D* epx=hcgpose->Project3DProfile("yx");
		epx->SetLineColor(kBlue);
		

		epx->Draw("BOXSAME");
	        TCHits->cd(4);
		TProfile2D* hpy=hcgposi->Project3DProfile("zx");
		hpy->SetLineColor(kRed);
		
		hpy->Draw("BOX");
		TProfile2D* epy=hcgpose->Project3DProfile("zx");
		epy->SetLineColor(kBlue);
		

		epy->Draw("BOXSAME");
		
	       TCHits->Modified();
	       TCHits->Draw();
	       TCHits->Update();
	       std::stringstream ss("");
	       ss<<"/tmp/Display_"<<evt_->getRunNumber()<<"_"<<evt_->getEventNumber()<<"_"<<vgood[is]<<".png";
	       TCHits->SaveAs(ss.str().c_str());
	     }
	 
	 }
  

	   
       
     }

   getchar();

}
extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
    DHCALAnalyzer* loadAnalyzer(void)
    {
      return (new combAnalysis);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
    void deleteAnalyzer(DHCALAnalyzer* obj)
    {
      delete obj;
    }
}
