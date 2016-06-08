#define NPLANS_USED 48

#include "rootProcessor.hh"
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
#include "datasource.hh"
#include "CaloTrans.h"


void rootProcessor::initHistograms()
{
  //  rootHandler_->BookTH1("/Clusters/EST1",100,0.,300.);
}

rootProcessor::rootProcessor() : nAnalyzed_(0),collectionName_("DHCALRawHits"),theBCIDSpill_(0),theLastBCID_(0),theSpillLength_(8.),_geo(NULL)
{
  
  reader_=DHCalEventReader::instance();
  
  rootHandler_ =DCHistogramHandler::instance();
  
  this->initialise();
  _nvhits=0;
}


void rootProcessor::initialise()
{
  presetParameters();
 
}
void rootProcessor::initJob()
{
}
void rootProcessor::endJob(){
	
}
void rootProcessor::presetParameters()
{
  std::map<std::string,MarlinParameter> m=reader_->getMarlinParameterMap();
  std::map<std::string,MarlinParameter>::iterator it;
  try
    {
        if ((it=m.find("geometry"))!=m.end())
	 _geo=new jsonGeo(it->second.getStringValue());
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
void rootProcessor::prepareDataSources()
{
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
  _sources.clear();
  //std::cout<<"On rentre dans la boucle"<<std::endl;
  for ( std::vector<std::string >::iterator it=(vnames)->begin();it!=vnames->end();it++)
    {
      if ((*it).compare("RU_XDAQ")!=0) continue;
      //      std::cout<<"Collection on  ulle"<<std::endl;
      EVENT::LCCollection* col= evt_->getCollection(*it); 
      //std::vector<unsigned char*> vF;
      //std::vector<unsigned char*> vL;
      

      for (int j=0;j<col->getNumberOfElements(); j++)
	{
	  LMGeneric* go= (LMGeneric*) col->getElementAt(j);
	  int* buf=&(go->getIntVector()[0]);
	  unsigned char* tcbuf = (unsigned char*) buf;
	  uint32_t rusize =go->getNInt()*sizeof(int32_t);
	  levbdim::buffer m((char*) tcbuf,0);
	  m.setPayloadSize(rusize-3*sizeof(uint32_t)-sizeof(uint64_t));
	  //std::cout<<" Source found "<<m.detectorId()<<" "<<m.dataSourceId()<<std::endl;
	  if (m.detectorId()!=100) continue;
	  levbdim::datasource* ds= new levbdim::datasource(101,m.dataSourceId(),0x80000);
	  _sources.insert(std::pair<uint32_t,levbdim::datasource*>(m.dataSourceId(),ds));
	}
    }

}

bool rootProcessor::decodeTrigger(LCCollection* rhcol)
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

  if (difid==0) {
    std::cout<<"No dif id "<<std::endl;
    return false;
  }

  //Find the parameters
  DEBUG_PRINT("DIF ID %d found \n",difid);
  std::stringstream pname("");
  pname <<"DIF"<<difid<<"_Triggers";

  rhcol->getParameters().getIntVals(pname.str(),vTrigger);
  INFO_PRINT("Vtrigger size %d found \n",vTrigger.size());
  if (vTrigger.size()==0) return false; 

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


  return true;
}

void rootProcessor::processSeed(IMPL::LCCollectionVec* rhcol,uint32_t seed)
{

  // ptime("Enter");

  currentTime_=seed;
  theAbsoluteTime_=theBCID_-currentTime_;
  if (theTimeZero_==0) theTimeZero_=theAbsoluteTime_;
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
   

   theNplans_=this->fillVolume(seed);
   //std::cout<<ptime("fillVolume")<<" Hits "<<iseed->second.size()<<std::endl;

   if (nPlansAll_.count()<_geo->cuts()["minPlans"].asUInt()) return;
   // Et on met _hits dans ROOT
   //  for (std::vector<RecoHit>::iterator ih=_hits.begin();ih!=_hits.end();ih++)
   for (int i=0;i<_nvhits;i++)
     {
       RecoHit* ih=&_vhits[i];
       std::map<uint32_t,levbdim::datasource*>::iterator ids=_sources.find(ih->dif());
       if (ids==_sources.end()) continue;
       levbdim::datasource* ds=ids->second;
       uint8_t* ptr = (uint8_t*) (ds->payload());
       struct CaloTransHit* ct=(struct CaloTransHit*) &ptr[_dsidx[ih->dif()]];
       ct->dif_id=ih->dif();
       ct->asic_id=ih->asic();
       ct->chan_id=ih->channel();
       ct->map_x=ih->I();
       ct->map_y=ih->J();
       ct->map_z=ih->plan();
       ct->x=ih->X();
       ct->y=ih->Y();
       ct->z=ih->Z();
       ct->asic_bcid=theBCID_;
       ct->spill=evt_->getEventNumber();
       ct->time=theAbsoluteTime_*2E-7;
       ct->adc_energy=ih->amplitude();
       _dsidx[ih->dif()]+=sizeof(struct CaloTransHit);
       
     }
   
  return;

}

//void rootProcessor::pseed(
void rootProcessor::processEvent()
{

  //printf("Reader %x \n",reader_);
  //printf("Event %x \n",reader_->getEvent());

  if (reader_->getEvent()==0) return;
    
  evt_=reader_->getEvent();
  //theSkip_=380;
  
  
  printf("Processing %d - %d \n",evt_->getRunNumber(),evt_->getEventNumber());
  

    

  
  IMPL::LCCollectionVec* rhcol=NULL;
  bool rhcoltransient=false;
  try {
    rhcol=(IMPL::LCCollectionVec*) evt_->getCollection(collectionName_);

  }
  catch (...)
    {
      std::cout<<"No SDHCALRawHist"<<std::endl;
    }
    if (rhcol==NULL) return;


  
  DEBUG_PRINT("End of CreaetRaw %d \n",rhcol->getNumberOfElements());
  std::cout<<ptime("Full Event ")<<" hits "<<rhcol->getNumberOfElements()<<std::endl;
  if (rhcol->getNumberOfElements()>4E6) return;
  
  //_monitor->FillTimeAsic(rhcol);

  //  LCTOOLS::printParameters(rhcol->parameters());
  //DEBUG_PRINT("Time Stamp %d \n",evt_->getTimeStamp());

  if (rhcol->getNumberOfElements()==0) return;
  if (nAnalyzed_==0)
    this->prepareDataSources();

  nAnalyzed_++;
  DEBUG_PRINT("Calling decodeTrigger\n");
  // TESTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
  
  if (!decodeTrigger(rhcol) ) { return;}
  
  //if (isNewSpill_) return;
  DEBUG_PRINT("Calling find time seed %d \n",_geo->cuts()["minPlans"].asUInt());
  reader_->findTimeSeeds(_geo->cuts()["minPlans"].asUInt());

  DEBUG_PRINT("Calling cleanMap\n");  
  std::vector<uint32_t> vseeds =this->cleanMap(_geo->cuts()["minPlans"].asUInt());
  //std::vector<uint32_t> vseeds=reader_->getTimeSeeds();

  
  INFO_PRINT("================>  %d  Number of seeds %d > %d plans \n",evt_->getEventNumber(),(int) vseeds.size(), _geo->cuts()["minPlans"].asUInt());

  if (vseeds.size()==0)  { return;}
  // Clear indexes for sources

  memset(_dsidx,0,255*sizeof(uint32_t));
  uint32_t ns=vseeds.size();
  for (uint32_t is=0;is<ns;is++)
    {
      //printf("%d %d %x \n",is,vseeds[is],rhcol);
      //printf("Element %d traité par le thread %d \n",is,omp_get_thread_num());

      this->processSeed(rhcol,vseeds[is]);      
    }

  for (int32_t i=0;i<255;i++)
    {
      if (_dsidx[i]==0) continue;
      _sources[i]->publish(evt_->getEventNumber(),theBCID_,_dsidx[i]);
      
      unsigned char* cdata=(unsigned char*)  _sources[i]->buffer()->ptr();
      int32_t* idata=(int32_t*) cdata;
      //printf("\t writing %d bytes \n",_dsidx[i]);
      int difsize= _sources[i]->buffer()->size();
      reader_->addRawOnlineRU(idata,difsize/4+1);

    }


  return;
}  


#define DBG printf("%d %s\n",__LINE__,__PRETTY_FUNCTION__);


std::string rootProcessor::ptime( std::string s)
{
  //#ifdef DEBUG
  std::stringstream ss;
  gettimeofday(&endT, NULL); 
  
  timersub(&endT, &startT, &diffT);
  
  ss<<s<<" time taken = "<<diffT.tv_sec<<":"<<diffT.tv_usec;
  gettimeofday(&startT, NULL);
  return ss.str();
  //#endif
}

uint32_t rootProcessor::fillVolume(uint32_t seed)
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
  _nvhits=0;

  //std::bitset<61> planes(0);
  nPlansAll_.reset();

  uint32_t ncount=0;
  
  if (iseed->second.size()>4096) return 0;

  std::bitset<64> difc[255];
  for (int i=0;i<255;i++) difc[i].set(0);
  for (std::vector<IMPL::RawCalorimeterHitImpl*>::iterator ihit=iseed->second.begin();ihit!=iseed->second.end();ihit++)
    {
      IMPL::RawCalorimeterHitImpl* hit =(*ihit);
      uint32_t dif =hit->getCellID0()&0xFF;
      uint32_t asic = (0xFF & (hit->getCellID0()&0xFF00)>>8);
      difc[dif].set(asic,1);
      
    }
  /*
  for (int i=0;i<255;i++) if (difc[i].count()>24) {
      INFO_PRINT(" DIF %d Count %d  at seed %d \n",i,difc[i].count(),seed);
      return 0;
    }
  */
  
  for (std::vector<IMPL::RawCalorimeterHitImpl*>::iterator ihit=iseed->second.begin();ihit!=iseed->second.end();ihit++)
    {
      IMPL::RawCalorimeterHitImpl* hit =(*ihit);
      //RecoHit h(_geo,hit);
      //
      //_hits.push_back(h);
      //_keys.push_back(makekey(I,J,chid));
      _vhits[_nvhits].initialise(_geo,hit);
       nPlansAll_.set(_vhits[_nvhits].plan());
      _nvhits++;
      ncount++;
    }
  DEBUG_PRINT("Total number of Hit in buildVolume %d  %d => planes %d \n",ncount,seed,nPlansAll_.count());
  return nPlansAll_.count();
}

std::vector<uint32_t> rootProcessor::cleanMap(uint32_t nchmin)
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
	  Json::Value dif=_geo->difGeo(difid);
	  plans.set(dif["chamber"].asUInt(),1);
	  
	}

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
      DHCALAnalyzer* d= new rootProcessor();
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
