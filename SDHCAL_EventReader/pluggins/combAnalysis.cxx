
#include "combAnalysis.hh"
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
#include "CaloTrans.h"
#include "jsonGeo.hh"
#include "buffer.hh"


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
   bool good=false;
   for (std::map<uint32_t,uint32_t>::iterator imf=mecal.begin();imf!=mecal.end();imf++)
     {
       std::bitset<32> mask(mecalp[imf->first]);
       if (mask.count()<4) continue;
       std::cout<<mask<<std::endl;
       printf("%d -> %d mask count %d \n",imf->first*2,imf->second,mask.count());
       good=true;
     }
   if (!good) return;
   getchar();
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
   for (std::map<uint32_t,uint32_t>::iterator imf=mhcal.begin();imf!=mhcal.end();imf++)
     if (imf->second>50)
       printf("%d -> %d (+%d) \n",imf->first,imf->second,imf->first+2490);
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
