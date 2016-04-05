#include "DHCalEventReader.h"
#include "DIFUnpacker.h"
using namespace lcio ;
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <algorithm>

#include <string.h>
#include <string>
#include <iostream>
#include "buffer.hh"
#define CHECK_BIT(var,pos) ((var)& (1<<(pos)))
DHCalEventReader* DHCalEventReader::_me = 0 ;


DHCalEventReader* DHCalEventReader::instance() {
  
  if( _me == 0 ) 
    _me = new DHCalEventReader;
  
  return _me ;
}  

//static DCFrame theFrameBuffer[256*48*128];
DHCalEventReader::DHCalEventReader() :dropFirstRU_(true),theXdaqShift_(92),currentFileName_("NONE")
{
  evt_ = 0;
  runh_ = 0;
  lcReader_=0;
  lcWriter_=0;
  filenames_.clear();
  newRunHeader_=false;

  //  lcWriter_ = new LCSplitWriter(LCFactory::getInstance()->createLCWriter() , 2040109465 ) ;
  printf("%d %d %x \n",nGood_,nBad_,nBadTwo_);
  lcWriter_ = LCFactory::getInstance()->createLCWriter();
  lcWriter_->setCompressionLevel(2);
  compactify_=false;
  debugLevel_=0;
  nBad_=0;
  nBadTwo_=0;
  nGood_=0;
  nZero_=0;
  printf("%d %d %x \n",nGood_,nBad_,nBadTwo_);
  //getchar();
 
//  framePtr0_= &theFrameBuffer[0];
//static DCFrame theFrameBuffer[256*48*128];
  //framePtr0_= new  DCFrame[256*48*64];
  printf("%d %d %x \n",nGood_,nBad_,nBadTwo_);
  expectedDtc_=0;
  _me=this;
}

DHCalEventReader::~DHCalEventReader()
{
  clear();
  //if (evt_) delete evt_;
  //if(runh_) delete runh_;
  //if (lcReader_) delete lcReader_;
  //if (lcWriter_) delete lcWriter_;
}
void DHCalEventReader::open ( std::vector< std::string > &filenames)
{
  if (readers_.size()==0) 
    {
    for (std::vector< std::string >::iterator it=filenames.begin();it!=filenames.end();it++)
      
      readers_.push_back(LCFactory::getInstance()->createLCReader(IO::LCReader::directAccess)) ;
      //lcReader_->registerLCRunListener(this) ;
      // lcReader_->registerLCEventListener(this) ;
      
    }

  try{
    for (uint32_t i=0;i<readers_.size();i++)
      {
	readers_[i]->open(filenames_[i]);
	printf(" those files %s have %d events \n",filenames_[i].c_str(),readers_[i]->getNumberOfEvents());
      }
  }
  catch( IOException& e) {
    std::cout << e.what() << std::endl ;
    exit(1) ;
  }
}
void DHCalEventReader::open(std::string name)
{
  if (lcReader_==0) 
    {
      lcReader_ = LCFactory::getInstance()->createLCReader(0) ;
      lcReader_->registerLCRunListener(this) ;
      lcReader_->registerLCEventListener(this) ;
      
    }

  try{
    
    lcReader_->open( name.c_str()) ;
    // printf("%s has %d events \n",name.c_str(),lcReader_->getNumberOfEvents());
  }
  catch( IOException& e) {
    std::cout << e.what() << std::endl ;
    exit(1) ;
  }
}

void DHCalEventReader::close()
{
  //std::cout<<"On ferme "<<std::endl;
  if (lcReader_!=NULL)
    {
      try{
	lcReader_->close() ;
      }
      catch( IOException& e) {
	std::cout << e.what() << std::endl ;
	exit(2) ;
      }
    }
  //std::cout<<"Bye "<<std::endl;
  for (uint32_t i=0;i<readers_.size();i++)
    {
      readers_[i]->close();
    }
}
void DHCalEventReader::readRun()
{
  try{
    IMPL::LCRunHeaderImpl*  lrh=(IMPL::LCRunHeaderImpl*)	lcReader_->readNextRunHeader ();
    //std::cout<<" mon lrh il est t y bon ?"<<(int) lrh<<std::endl;
    if (lrh!=0) runh_=lrh;
  }
  catch( IOException& e) {
    std::cout << e.what() << std::endl ;
    //    exit(2) ;
  }
  if (runh_ == 0)
    {
      std::cout<<" No run header found \n Run is stopped "<<std::endl;
      //exit(3);
    }
  //   else
  //{
  //         LCTOOLS::dumpRunHeader(runh_);
  // }
}

int DHCalEventReader::readStream(int max_record)
{
  try{
    if (max_record)
      lcReader_->readStream(max_record) ; 
    else
      lcReader_->readStream() ; 

  }
  catch( IOException& e) {
    std::cout << e.what() << std::endl ;
    throw e;
    return -2;
    //  exit(2) ;
  }
  return -1;
}

//! Interface for readStream
void DHCalEventReader::processEvent( LCEvent * evt )  
{
  evt_ = (IMPL::LCEventImpl*) evt;
  // if (evt_!=0) LCTOOLS::dumpEvent( evt_ ) ;
  analyzeEvent();
    
}
//! Interface for readStream
void DHCalEventReader::processRunHeader( LCRunHeader* run) 
{
  runh_ =(IMPL::LCRunHeaderImpl*) run;
}
int DHCalEventReader::readEvent()
{
  try{
    evt_ = (IMPL::LCEventImpl*) lcReader_->readNextEvent() ; 
    //     if (evt_!=0) LCTOOLS::dumpEvent( evt_ ) ;
    analyzeEvent();
  }
  catch( IOException& e) {
    std::cout << e.what() << std::endl ;
    return -1;
    //  exit(2) ;
  }
  if (evt_!=0)
    return evt_->getEventNumber();
  else
    return 0;
}

void DHCalEventReader::findEvent(int run,int event)
{
  if (lcReader_==0) 
    {
      lcReader_ = LCFactory::getInstance()->createLCReader(IO::LCReader::directAccess ) ;
      lcReader_->registerLCRunListener(this) ;
      lcReader_->registerLCEventListener(this) ;
      
    }
  if (currentFileName_!="NONE") return;
  for (std::vector< std::string >::iterator it=filenames_.begin();it!=filenames_.end();it++)
    {
      lcReader_->open((*it));
      evt_ = (IMPL::LCEventImpl*) lcReader_->readEvent(run,event) ; 
      if (evt_!=0) {currentFileName_=(*it) ; break;}
      
      lcReader_->close();
    
    }
}
int DHCalEventReader::readOneEvent(int run,int event)
{
  try{
    for (uint32_t i=0;i<readers_.size();i++)
    {
      evt_ = (IMPL::LCEventImpl*) readers_[i]->readEvent(run,event);
      if (evt_!=0)
	{
	  this->analyzeEvent();
	  break;
	}
    }
    
  }
  catch( IOException& e) {
    std::cout << e.what() << std::endl ;
    return -1;
    //  exit(2) ;
  }
  if (evt_!=0)
    return evt_->getEventNumber();
  else
    return 0;
}


void DHCalEventReader::clear()
{

}
class LMGeneric: public IMPL::LCGenericObjectImpl
{
 public:
  LMGeneric(){;}
  std::vector<int>& getIntVector(){return _intVec;}

};
int DHCalEventReader::parseRawEvent(int rushift)
{
  newRunHeader_=false;
  if (evt_ == 0) { std::string s =" no event found";throw s+__PRETTY_FUNCTION__;}
  this->clear();
 
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
  //std::cout<<"On rentre dans la boucle"<<std::endl;
  if (evt_->getEventNumber()%100 == 0)
    printf("Event %d Good Frame %d  6 bytes less %d  Bytes more %d Empty frames %d \n",evt_->getEventNumber(),nGood_,nBad_,nBadTwo_,nZero_);
  // Clear previous event
  for (uint32_t i=0;i<theDIFPtrList_.size();i++) 
    {	
      //theDIFPtrList_[i]->dumpDIFInfo();
      delete theDIFPtrList_[i];
    }
  theDIFPtrList_.clear();
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
	  if (j==0 && dropFirstRU_) continue;
	  LMGeneric* go= (LMGeneric*) col->getElementAt(j);
	  int* buf=&(go->getIntVector()[0]);
	  unsigned char* tcbuf = (unsigned char*) buf;
	  uint32_t rusize =go->getNInt()*sizeof(int32_t); 
	  uint32_t idstart=DIFUnpacker::getStartOfDIF(tcbuf,rusize,theXdaqShift_);
	  //bool slowcontrol; uint32_t version,hrtype,id0,iddif;
	  //DCBufferReader::checkType(tcbuf,rusize/4,slowcontrol,version,hrtype,id0,iddif,theXdaqShift_);
	  //printf(" Found start of Buffer at %d contains %x and %d bytes \n",idstart,tcbuf[idstart],rusize-idstart+1);

	  
	  

	  unsigned char* tcdif=&tcbuf[idstart];
	  DIFPtr* d= new DIFPtr(tcdif,rusize-idstart+1);
	  if (d->getID()==89)
	    {
	      //printf("%d %d \n",rusize,idstart);
	      // d->dumpDIFInfo();
	    }
	  theDIFPtrList_.push_back(d);
	  //getchar();
	}
    }
      return 0;
}

int DHCalEventReader::parseSDHCALEvent()
{
  newRunHeader_=false;
  if (evt_ == 0) { std::string s =" no event found";throw s+__PRETTY_FUNCTION__;}
  this->clear();
 
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
  //std::cout<<"On rentre dans la boucle"<<std::endl;
  if (evt_->getEventNumber()%100 == 0)
    printf("Event %d Good Frame %d  6 bytes less %d  Bytes more %d Empty frames %d \n",evt_->getEventNumber(),nGood_,nBad_,nBadTwo_,nZero_);
  // Clear previous event
  for (uint32_t i=0;i<theDIFPtrList_.size();i++) 
    {	
      //theDIFPtrList_[i]->dumpDIFInfo();
      delete theDIFPtrList_[i];
    }
  theDIFPtrList_.clear();
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
	  if (j==0 && dropFirstRU_) continue;
	  LMGeneric* go= (LMGeneric*) col->getElementAt(j);
	  int* buf=&(go->getIntVector()[0]);
	  unsigned char* tcbuf = (unsigned char*) buf;
	  uint32_t rusize =go->getNInt()*sizeof(int32_t);
	  levbdim::buffer m((char*) tcbuf,0);
	  m.setPayloadSize(rusize-3*sizeof(uint32_t)-sizeof(uint64_t));
	  //std::cout<<" Source found "<<m.detectorId()<<" "<<m.dataSourceId()<<std::endl;
	  if (m.detectorId()!=100) continue;
	  //uint32_t idstart=DIFUnpacker::getStartOfDIF(tcbuf,rusize,theXdaqShift_);
	  uint32_t idstart=DIFUnpacker::getStartOfDIF((unsigned char*) m.ptr(),m.size(),20);
	  //std::cout<<" Start at "<<idstart<<std::endl;
	  //bool slowcontrol; uint32_t version,hrtype,id0,iddif;
	  //DCBufferReader::checkType(tcbuf,rusize/4,slowcontrol,version,hrtype,id0,iddif,theXdaqShift_);
	  //printf(" Found start of Buffer at %d contains %x and %d bytes \n",idstart,tcbuf[idstart],rusize-idstart+1);

	  
	  

	  unsigned char* tcdif=&tcbuf[idstart];
	  DIFPtr* d= new DIFPtr(tcdif,rusize-idstart+1);
	  theDIFPtrList_.push_back(d);
	  //getchar();
	}
    }

  std::vector<uint32_t> seed;seed.clear();
  IMPL::LCCollectionVec* HitVec=this->createRawCalorimeterHits(seed);
  try{
    if (HitVec->getNumberOfElements()!=0 &&HitVec->getNumberOfElements()<200000 )
      evt_->addCollection(HitVec,"SDHCALRawHits");
    else
      delete HitVec;
  }
  catch( IOException& e) {
    std::cout << e.what() << std::endl ;
   //  exit(2) ;
  }

  return 0;
}



void DHCalEventReader::findTimeSeeds(int32_t nasic_min)
{
  theTimeSeeds_.clear();
  map<uint32_t,uint32_t> tcount;
  map<uint32_t,int32_t> tedge;
  IMPL::LCCollectionVec* rhcol=NULL;
  try {
    rhcol=(IMPL::LCCollectionVec*) evt_->getCollection("DHCALRawHits");
  }
  catch(...)
    {
      // printf("No RHCOL yet \n");
    }
  // Tcount is the time histo
  if (rhcol!=NULL)
    {
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
    }
  else
    {
      for (std::vector<DIFPtr*>::iterator it = theDIFPtrList_.begin();it!=theDIFPtrList_.end();it++)
	{
	  DIFPtr* d = (*it);
     // Loop on frames
	  for (uint32_t i=0;i<d->getNumberOfFrames();i++)
	    {
	      uint32_t bc = d->getFrameTimeToTrigger(i);
	      map<uint32_t,uint32_t>::iterator it=tcount.find(bc);
	      if (it!=tcount.end()) 
		it->second=it->second+1;
	      else
		{
		  std::pair<uint32_t,uint32_t> p(bc,1);
		  tcount.insert(p);
		}
	    }
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
      if (it->second<-1*(nasic_min-2))
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
  theTimeSeeds_.clear();
  for (uint32_t i=0;i<seed.size();)
    {
      if ((i+1)<=(seed.size()-1))
	{
	  if (seed[i+1]-seed[i]<=5)
	    {
	      //theTimeSeeds_.push_back(int((seed[i+1]+seed[i])/2));
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
		theTimeSeeds_.push_back(max_it);
	      else
		theTimeSeeds_.push_back(seed[i]);
	      i+=2;
	    }
	  else
	    {
	      theTimeSeeds_.push_back(seed[i]);
	      i++;
	    }
	}
      else
	{
	  theTimeSeeds_.push_back(seed[i]);
	  i++;
	}

		
    }
  //td::cout<<theTimeSeeds_.size()<<" good showers "<< tedge.size()<<std::endl;
  std::sort(theTimeSeeds_.begin(),theTimeSeeds_.end(),std::greater<uint32_t>());

  /*  
      for (std::vector<uint32_t>::iterator is=theTimeSeeds_.begin();is!=theTimeSeeds_.end();is++)
    std::cout<<(*is)<<" ---> "<<tcount[(*is)]<<std::endl;
  */
  if (rhcol!=NULL)
    {
      // Fill std::map<uint32_t,std::vector<IMPL::RawCalorimeterHitImpl*> > thePhysicsEventMap_;
      thePhysicsEventMap_.clear();
      for (std::vector<uint32_t>::iterator is=theTimeSeeds_.begin();is!=theTimeSeeds_.end();is++)
	{
	  std::vector<IMPL::RawCalorimeterHitImpl*> v;
	  
	  std::pair<uint32_t,std::vector<IMPL::RawCalorimeterHitImpl*> > p((*is),v);
	  thePhysicsEventMap_.insert(p);
	}
      std::map<uint32_t,std::vector<IMPL::RawCalorimeterHitImpl*> >::iterator im=thePhysicsEventMap_.end();

      for (int i=0;i<rhcol->getNumberOfElements();i++)
	{
	  IMPL::RawCalorimeterHitImpl* hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(i);
	  if (hit==0) continue;
	  im=thePhysicsEventMap_.find(hit->getTimeStamp());
	  if (im!=thePhysicsEventMap_.end()) {im->second.push_back(hit);continue;}
	  im=thePhysicsEventMap_.find(hit->getTimeStamp()-1);
	  if (im!=thePhysicsEventMap_.end()) {im->second.push_back(hit);continue;}
	  im=thePhysicsEventMap_.find(hit->getTimeStamp()+1);
	  if (im!=thePhysicsEventMap_.end()) {im->second.push_back(hit);continue;}
	  im=thePhysicsEventMap_.find(hit->getTimeStamp()-2);
	  if (im!=thePhysicsEventMap_.end()) {im->second.push_back(hit);continue;}
	  im=thePhysicsEventMap_.find(hit->getTimeStamp()+2);
	  if (im!=thePhysicsEventMap_.end()) {im->second.push_back(hit);continue;}
	  im=thePhysicsEventMap_.find(hit->getTimeStamp()-3);
	  if (im!=thePhysicsEventMap_.end()) {im->second.push_back(hit);continue;}
	  im=thePhysicsEventMap_.find(hit->getTimeStamp()+3);
	  if (im!=thePhysicsEventMap_.end()) {im->second.push_back(hit);continue;}
	  im=thePhysicsEventMap_.find(hit->getTimeStamp()-4);
	  if (im!=thePhysicsEventMap_.end()) {im->second.push_back(hit);continue;}
	  im=thePhysicsEventMap_.find(hit->getTimeStamp()+4);
	  if (im!=thePhysicsEventMap_.end()) {im->second.push_back(hit);continue;}
	}

      theTimeSeeds_.clear();
      // Check that at least nasic_min DIfs are hit per seed
      for( std::map<uint32_t,std::vector<IMPL::RawCalorimeterHitImpl*> >::iterator im=thePhysicsEventMap_.begin(),im_next=im;im!=thePhysicsEventMap_.end();im=im_next)
	{
	  ++im_next;
	  std::bitset<255> difs;
	  difs.reset();
	  for (std::vector<IMPL::RawCalorimeterHitImpl*>::iterator ih=im->second.begin();ih!=im->second.end();ih++)
	    difs.set((*ih)->getCellID0()&0xFF);
	  // if (difs.count()>=nasic_min)
	  //   printf("seed %d ndif  %d \n",im->first,difs.count());
	  if (difs.count()<nasic_min)
	    thePhysicsEventMap_.erase(im);
	  else
	    theTimeSeeds_.push_back(im->first);
	}
      //printf("%s EventMap size %d  \n",__PRETTY_FUNCTION__,thePhysicsEventMap_.size());

    }
  return ;
}

IMPL::LCCollectionVec* DHCalEventReader::createRawCalorimeterHits(std::vector<uint32_t> seeds)
{
  // Save current Object count
  bool useSynch=seeds.size()!=0;
  IMPL::LCCollectionVec *RawVec=new IMPL::LCCollectionVec(LCIO::RAWCALORIMETERHIT) ;
 
  //Prepare a flag to tag data type in RawVec (dit les types de data qu'on va enregistrer?)
  IMPL::LCFlagImpl chFlag(0) ;
  EVENT::LCIO bitinfo;
  chFlag.setBit(bitinfo.RCHBIT_LONG ) ;                    // raw calorimeter data -> format long //(sert a qq chose?)
  chFlag.setBit(bitinfo.RCHBIT_BARREL ) ;                  // barrel
  chFlag.setBit(bitinfo.RCHBIT_ID1 ) ;                     // cell ID 
  chFlag.setBit(bitinfo.RCHBIT_TIME ) ;                    // timestamp
  RawVec->setFlag( chFlag.getFlag()  ) ;   
  

  for (std::vector<DIFPtr*>::iterator it = theDIFPtrList_.begin();it!=theDIFPtrList_.end();it++)
    {
      DIFPtr* d = (*it);
      // d->dumpDIFInfo();
     // for (uint32_t i=1;i<d->getNumberOfFrames();i++)
     // 	    {
     // 	      std::bitset<64> bs0(0);
     // 	      std::bitset<64> bs1(0);
     // 	      for (uint32_t ip=0;ip<64;ip++) {bs0.set(ip,d->getFrameLevel(i,ip,0));bs1.set(ip,d->getFrameLevel(i,ip,1));}
     // 	      printf("\t %d %d %d \n",d->getID(),d->getFrameTimeToTrigger(i),d->getFrameAsicHeader(i));
     // 	      std::cout<<"\t \t"<<bs0<<std::endl;
     // 	      std::cout<<"\t \t"<<bs1<<std::endl;
     //   }
     // //continue;
     // Loop on frames
      for (uint32_t i=0;i<d->getNumberOfFrames();i++)
      {
	
	bool isSynchronised = true;
	if (std::find(seeds.begin(), seeds.end(), d->getFrameTimeToTrigger(i))==seeds.end() 
	    && std::find(seeds.begin(), seeds.end(), d->getFrameTimeToTrigger(i)-1)==seeds.end() 
	    && std::find(seeds.begin(), seeds.end(), d->getFrameTimeToTrigger(i)+1)==seeds.end() 
	    && std::find(seeds.begin(), seeds.end(), d->getFrameTimeToTrigger(i)-2)==seeds.end() 
	    && std::find(seeds.begin(), seeds.end(), d->getFrameTimeToTrigger(i)+2)==seeds.end()) isSynchronised=false;
	if (useSynch && !isSynchronised) continue;
	//printf("\t %d %d %d \n",d->getID(),d->getFrameTimeToTrigger(i),d->getFrameAsicHeader(i));
      // Loop on Hits
      //      std::cout<<f->getDifId()<<" new frame "<<f->getAsicId()<<std::endl;
      for (uint32_t j=0;j<64;j++)
	{
	  if (!(d->getFrameLevel(i,j,0) || d->getFrameLevel(i,j,1))) continue; // skip empty pads
	  //  std::cout <<" New hit "<<std::endl;
	  unsigned long int ID0;
	  ID0=(unsigned long int)(((unsigned short)d->getID())&0xFF);			//8 firsts bits: DIF Id
	  ID0+=(unsigned long int)(((unsigned short)d->getFrameAsicHeader(i)<<8)&0xFF00);	//8 next bits:   Asic Id
	  bitset<6> Channel(j);														
	  ID0+=(unsigned long int)((Channel.to_ulong()<<16)&0x3F0000);				//6 next bits:   Asic's Channel
	  unsigned long BarrelEndcapModule=0;  //(40 barrel + 24 endcap) modules to be coded here  0 for testbeam (over 6 bits)
	  ID0+=(unsigned long int)((BarrelEndcapModule<<22)&0xFC00000);	
	  unsigned long int ID1 = (unsigned long int)(d->getFrameBCID(i));
	  bitset<3> ThStatus;
	  ThStatus[0]=d->getFrameLevel(i,j,0);
	  ThStatus[1]=d->getFrameLevel(i,j,1);
	  ThStatus[2]=isSynchronised;
	  
	  IMPL::RawCalorimeterHitImpl *hit=new IMPL::RawCalorimeterHitImpl() ;
	  hit->setCellID0((unsigned long int)ID0);               
	  hit->setCellID1(ID1);
	  hit->setAmplitude(ThStatus.to_ulong());
	  //unsigned long int TTT = (unsigned long int)rint(f->getRecoBunchCrossingTime()/DCBufferReader::getDAQ_BC_Period());
	  /// LM test unsigned long int TTT = (unsigned long int)rint(f->getBunchCrossingTime()/DCBufferReader::getDAQ_BC_Period());
	  unsigned long int TTT = (unsigned long int)(d->getFrameTimeToTrigger(i));
	  //std::cout<<f->getDifId()<<" "<<f->getAsicId()<<" "<<f->getBunchCrossingTime()<<" "<<f->getRecoBunchCrossingTime()<<" "<<f->getTimeToTrigger()<<std::endl;
	  //if (fabs((f->getRecoBunchCrossingTime()/DCBufferReader::getDAQ_BC_Period())-f->getTimeToTrigger())>0.1)
	  //  {
	  //    std::cout<<f->getDifId()<<" "<<f->getAsicId()<<" "<<f->getBunchCrossingTime()<<" "<<(f->getRecoBunchCrossingTime()/DCBufferReader::getDAQ_BC_Period())<<" "<<f->getTimeToTrigger()<<std::endl;
	      //	      getchar();
	  //  }
	  hit->setTimeStamp(TTT);											//Time stamp of this event from Run Begining

	  //	  std::cout<<"rebd "<<TTT<<" "<<hit->getCellID1()<<std::endl;
	   RawVec->addElement(hit);
	   if (RawVec->getNumberOfElements()>100000) break; //too may noise
	}
      }
    }
  
for (std::vector<DIFPtr*>::iterator it = theDIFPtrList_.begin();it!=theDIFPtrList_.end();it++)
    {
      DIFPtr* d = (*it);
      lcio::IntVec trig(8);
      trig[0] = d->getDTC();
      trig[1] = d->getGTC();
      trig[2] = d->getBCID();
      trig[3] = d->getAbsoluteBCID()&0xFFFFFF;
      trig[4] = (d->getAbsoluteBCID()/(0xFFFFFF+1))&0xFFFFFF;
      trig[5] = d->getTASU1();
      trig[6] = d->getTASU2();
      trig[7] = d->getTDIF();

      std::stringstream ss("");
      ss<<"DIF"<<d->getID()<<"_Triggers";
      RawVec->parameters().setValues(ss.str(),trig);
    }
  return RawVec;
}

void DHCalEventReader::buildEvent(bool rebuild)
{
  nslow_=0;
  // exit if no event is found
  if (evt_==0) return;

  // Do nothing if the DHCAL collection already exits unless it's required
  IMPL::LCCollectionVec *RawHitsVec=0;
  try 
    {
      RawHitsVec= (IMPL::LCCollectionVec*) evt_->getCollection("DHCALRawHits");
    }
  catch (IOException& e) {
    std::cout << e.what() << std::endl ;
    //  exit(2) ;
  }
  catch (...)
    {
      // std::cout <<" No DHCALRawHits" << std::endl ;
    }

  if (!rebuild && RawHitsVec>0) return;

  std::string srhcol="DHCALRawHits";
  if (rebuild && RawHitsVec>0)
    {
      srhcol="DHCALRawHitsNew";
    }

  // Parse the RAW buffer
  IMPL::LCCollectionVec *RawVec=0;
  try 
    {
      RawVec= (IMPL::LCCollectionVec*) evt_->getCollection("RU_XDAQ");
    }
  catch (IOException& e) {
    std::string s=e.what();
    s+=__PRETTY_FUNCTION__;
    s+=" cannot build the event";
    throw s;
  }
  catch (...)
    {
      std::string s=" unknown exception ";
      s+=__PRETTY_FUNCTION__;
      s+=" cannot build the event";
      throw s;

    }

  


  std::vector<uint32_t> seed;
 if (RawVec>0)
    {

      try 
	{
	  nslow_=parseRawEvent();
	  //printf("Event parsed \n");
	  
	  
	  if (compactify_)
	    {
	      //printf("Calling FastFlag2\n");
	      
	      fastFlag2(seed,2,5);
	      //	 printf("End of FastFlag2 \n");
	      
	    }
	  else
	    {
	      
	      seed.clear();
	    }
	}
      catch (std::string se)
	{
	  throw se;
	}
    }
 else
   {
     std::string s= "Cannot build the event";s+=__PRETTY_FUNCTION__;
     throw s;
   }
 // Add raw calorimeter hit collection
      
 try{
   evt_->addCollection(this->createRawCalorimeterHits(seed),srhcol);
 }
 catch( IOException& e) {
   std::cout << e.what() << std::endl ;
   //  exit(2) ;
 }


}


void DHCalEventReader::openOutput(std::string filename)
{

  std::cout<<"Opening new file "<<filename<<std::endl;
  lcWriter_->open(filename) ;

}

void DHCalEventReader::fillRunHeader()
{
  // Fill it with vslow info
  //  IMPL::LCRunHeaderImpl*  lrh= new IMPL::LCRunHeaderImpl();
#ifdef USE_SLOWCONTROL_HEADER
  for (std::vector<DIFSlowControl*>::iterator it=vslow_.begin();it!=vslow_.end();it++)
    {
      DIFSlowControl* sc = (*it);
      int difid=sc->getDIFId();
      std::map < int,std::map <std::string,int> > msc= sc->getChipsMap();
      for(std::map < int,std::map <std::string,int> >::iterator jt=msc.begin(); jt!=msc.end();jt++)
	{
	  int asicid = jt->first;
	  std::map <std::string,int> mas = jt->second;
	  for (std::map <std::string,int>::iterator kt=mas.begin();kt!=mas.end();kt++)
	    {
	      std::stringstream ss("");
	      ss<<"DIF_"<<difid<<"_ASIC_"<<asicid<<"_"<<kt->first;
	      runh_->parameters().setValue(ss.str(),kt->second);
	    }
	}
    }
#endif
  runh_->parameters().setValue("Version",(int) version_);
  runh_->parameters().setValue("HardRoc",(int) hrType_);

  newRunHeader_ = true;
	
}

void DHCalEventReader::writeRunHeader()
{
 
  //fillRunHeader();
  std::cout<<"writing run header "<<std::endl;


  lcWriter_->writeRunHeader(runh_);


}

void DHCalEventReader::write(  IMPL::LCEventImpl* evtOutput)
{

 
  lcWriter_->writeEvent(evtOutput);
  delete evtOutput;
  //     std::cout<<"Event written" <<std::endl;
  return;
  

}

void DHCalEventReader::writeEvent(bool dropRaw)
{

  if (!dropRaw) {
    lcWriter_->writeEvent(evt_);
    //     std::cout<<"Event written" <<std::endl;
    return;
  }
  IMPL::LCEventImpl* evtOutput_ =new IMPL::LCEventImpl();
  evtOutput_->setRunNumber(evt_->getRunNumber());
  evtOutput_->setEventNumber(evt_->getEventNumber());
  evtOutput_->setTimeStamp(evt_->getTimeStamp());
  evtOutput_->setDetectorName(evt_->getDetectorName());
  evtOutput_->setWeight(evt_->getWeight());
  
  std::vector<std::string >* 	vnames= (std::vector<std::string >*)evt_->getCollectionNames();
  for (  std::vector<std::string >::iterator it=vnames->begin();it!=vnames->end();it++)
    {
      if (dropRaw && ((*it).compare("RU_XDAQ")==0)) continue;
      try {
	IMPL::LCCollectionVec* v=(IMPL::LCCollectionVec*)evt_->takeCollection((*it));
	v->setTransient(false);
	evtOutput_->addCollection(v,(*it));
      }
      catch( IOException& e) {
	std::cout << e.what() << std::endl ;
	//  exit(2) ;
      }
    }
  
  // reste a copier les parametres
  //  LCTOOLS::dumpEvent(evtOutput_);
  //  getchar();
  
  lcWriter_->writeEvent(evtOutput_);
  delete evtOutput_;
  //std::cout<<"Event written" <<std::endl;
}




void DHCalEventReader::closeOutput()
{
  lcWriter_->close();

  //  delete lcWriter_;

}


void DHCalEventReader::createEvent(int nrun,std::string detname,bool deleteEvent)
{
  // Clear DIF and Frames
  this->clear();
  // Create the event
  int lastevt=0;
  if (evt_!=0 && deleteEvent) {
    lastevt=evt_->getEventNumber();
    delete evt_;
  }
  evt_ =  new IMPL::LCEventImpl();
  IMPL::LCCollectionVec *RawVec=new IMPL::LCCollectionVec(LCIO::LCGENERICOBJECT ) ;
  evt_->addCollection(RawVec,"RU_XDAQ");
  evt_->setRunNumber(nrun);
  evt_->setEventNumber(lastevt+1);
  evt_->setTimeStamp(time(0));
  evt_->setDetectorName(detname);


}
void DHCalEventReader::createRunHeader(int nrun,std::string detname)
{
  if (runh_!=0) delete runh_;
  runh_ =  new IMPL::LCRunHeaderImpl();
  runh_->setRunNumber(nrun);
  //evt_->setTimeStamp(time(0));
  runh_->setDetectorName(detname);


}

void DHCalEventReader::addRawOnlineRU(int *buf,int rusize )
{
  IMPL::LCCollectionVec *RawVec= (IMPL::LCCollectionVec*) evt_->getCollection("RU_XDAQ");
  // printf("Readout Unit  size %d \n",rusize);
  //if (i==0 && evt!=0) evt->setEventNumber(buf[30]);
  
  
  IMPL::LCGenericObjectImpl *hit=new IMPL::LCGenericObjectImpl() ;
  for (int j=0;j<rusize;j++) hit->setIntVal(j,buf[j]);
  //for (int j=0;j<rusize;j++) printf("%x",buf[j]);


  //printf("RU SIZE %d \n",rusize);
  //for (int i=0;i<rusize;i++) {printf("%08x\n",buf[i]);}

  //printf("\n");
  RawVec->addElement(hit);
  
  //std::cout<<RawVec->getNumberOfElements()<<std::endl;
  return;
}

void  DHCalEventReader::registerAnalysis(DHCALAnalyzer* a)
{
  vProcess_.push_back(a);
}

void  DHCalEventReader::analyzeEvent()
{
  for (std::vector<DHCALAnalyzer*>::iterator it=vProcess_.begin();it!=vProcess_.end();it++)
    {
      //std::cout<<"Calling analysis"<<__PRETTY_FUNCTION__ <<std::endl;
      (*it)->processEvent();
    }
}
void  DHCalEventReader::initJob()
{
  for (std::vector<DHCALAnalyzer*>::iterator it=vProcess_.begin();it!=vProcess_.end();it++)
    {
      //std::cout<<"Calling analysis"<<__PRETTY_FUNCTION__ <<std::endl;
      (*it)->initJob();
    }
}
void  DHCalEventReader::endJob()
{
  for (std::vector<DHCALAnalyzer*>::iterator it=vProcess_.begin();it!=vProcess_.end();it++)
    {
      //std::cout<<"Calling analysis"<<__PRETTY_FUNCTION__ <<std::endl;
      (*it)->endJob();
    }
}
void  DHCalEventReader::initRun()
{
  for (std::vector<DHCALAnalyzer*>::iterator it=vProcess_.begin();it!=vProcess_.end();it++)
    {
      //std::cout<<"Calling analysis"<<__PRETTY_FUNCTION__ <<std::endl;
      (*it)->initRun();
    }
}
void  DHCalEventReader::endRun()
{
  for (std::vector<DHCALAnalyzer*>::iterator it=vProcess_.begin();it!=vProcess_.end();it++)
    {
      //std::cout<<"Calling analysis "<<__PRETTY_FUNCTION__ <<std::endl;
      (*it)->endRun();
    }
}





void DHCalEventReader::difInChamber(uint32_t dif,uint32_t ch)
{
  std::pair<uint32_t,uint32_t> p(dif,ch);
  chambers_.insert(p);
  
}
//! Get Chamber of a dif
uint32_t DHCalEventReader::getChamber(uint32_t dif)
{
  if (chambers_.find(dif) != chambers_.end())
    return chambers_[dif];
  else
    return 0;
  
}
void DHCalEventReader::compactify(uint32_t synchcut,uint32_t nfcut)
{
  compactify_=true;
  bd_synchcut_ = synchcut;
  bd_nfcut_ = nfcut;
}

void DHCalEventReader::fastFlag2(std::vector<uint32_t> &seed_,uint32_t seedCut,uint32_t minChamberInTime)
{
  //getchar();
	seed_.clear();
  flagger_.clear();
  // flag
  uint32_t max_time=0;
  for (std::vector<DIFPtr*>::iterator it = theDIFPtrList_.begin();it!=theDIFPtrList_.end();it++) // Loop on DIF
    {
      DIFPtr* d = (*it);
      uint32_t chid= getChamber(d->getID());
      // LMTest      uint32_t bc = rint(f->getBunchCrossingTime()/DCBufferReader::getDAQ_BC_Period());
      // Loop on Frames
      for (uint32_t ifra=0;ifra<d->getNumberOfFrames();ifra++)
      {
	uint32_t bc=d->getFrameTimeToTrigger(ifra);
	max_time=(bc>max_time && bc<MAX_TIME_WORD*32)?bc:max_time;
	if (!flagger_.isDifSet(d->getID(),bc)) //Flag only once
	  {
	    flagger_.setDifTime(d->getID(),bc);
	    flagger_.setChamberTime(chid,bc);
	  }
      }	
      //std::cout<<f->getDifId()<<" "<<chid<<" "<<bc<<std::endl;

    }
  flagger_.setMaxTime(max_time);
  //std::cout<<max_time<<std::endl;
  //flagger_.summarize();
  // seeding
  for (uint32_t i=0;i<max_time+1;i++)
    {
      uint32_t c=flagger_.countChamber(i);
      if (c!=0)
	{
	  //std::cout<<i<<" "<<c<<std::endl;
	  //getchar();
	}
      if (c>=seedCut) 
	{
	  uint32_t cm = (i>0)?flagger_.countChamber(i-1):0;
	  uint32_t cp =(i<MAX_TIME_WORD*32-2)?flagger_.countChamber(i+1):0;
	  if (c+cm+cp>=minChamberInTime) 
	    {
	      // std::cout<<i<<" "<<cm<<" "<<c<<" "<<cp<<std::endl;
	      // Now really count chambers
	      uint32_t chall=0;
	      for (uint32_t j=0;j<MAX_CHAMBER_NUMBER;j++)
		{
		  if (flagger_.isChamberSet(j,i)) chall++;
		  if (flagger_.isChamberSet(j,i+1)) chall++;
		  if (flagger_.isChamberSet(j,i-1)) chall++;
		  if (flagger_.isChamberSet(j,i+2)) chall++;
		  if (flagger_.isChamberSet(j,i-2)) chall++;
		}
	      if (chall>=minChamberInTime)
		seed_.push_back(i);
	      if (cp>=seedCut) i++;
	    }
	  
	}
    }
    return ;
 

}

void DHCalEventReader::findTimeSeeds(  int32_t nhit_min,std::vector<uint32_t>& candidate)
{
  std::map<uint32_t,uint32_t> tcount;
  std::map<uint32_t,int32_t> tedge;

  for (std::vector<DIFPtr*>::iterator it = theDIFPtrList_.begin();it!=theDIFPtrList_.end();it++) // Loop on DIF
    {
      DIFPtr* d = (*it);
      uint32_t chid= getChamber(d->getID());
      // LMTest      uint32_t bc = rint(f->getBunchCrossingTime()/DCBufferReader::getDAQ_BC_Period());
      // Loop on Frames
      for (uint32_t ifra=0;ifra<d->getNumberOfFrames();ifra++)
	{
	  uint32_t bc=d->getFrameTimeToTrigger(ifra);
	  
	  
	  
	  std::map<uint32_t,uint32_t>::iterator it=tcount.find(bc);
	  if (it!=tcount.end()) 
	    it->second=it->second+1;
	  else
	    {
	      std::pair<uint32_t,uint32_t> p(bc,1);
	      tcount.insert(p);
	    }
	}
    }
  std::vector<uint32_t> seed;
  seed.clear();
	
  //d::cout<<"Size =>"<<tcount.size()<<std::endl;
  // Tedge is convolute with +1 -1 +1 apply to tcount[i-1],tcount[i],tcount[i+1]
  for ( std::map<uint32_t,uint32_t>::iterator it=tcount.begin();it!=tcount.end();it++)
    {
      //std::cout<<it->first<<" "<<it->second<<std::endl;
		
      std::map<uint32_t,uint32_t>::iterator ita=tcount.find(it->first+1);
      std:: map<uint32_t,uint32_t>::iterator itb=tcount.find(it->first-1);
      int32_t c=-1*it->second;
      if (ita!=tcount.end()) c+=ita->second;
      if (itb!=tcount.end()) c+=itb->second;
      std::pair<uint32_t,int32_t> p(it->first,c);
      tedge.insert(p);
		
    }
  //d::cout<<"Size Edge =>"<<tedge.size()<<std::endl;
  // Now ask for a minimal number of hits
  uint32_t nshti=0;
  for ( std::map<uint32_t,int32_t>::iterator it=tedge.begin();it!=tedge.end();)
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

  /*  for (std::vector<uint32_t>::iterator is=candidate.begin();is!=candidate.end();is++)
      std::cout<<__PRETTY_FUNCTION__<<" Time "<< (*is)<<" gives ---> "<<tcount[(*is)]<<std::endl; */
  return ;
}







void DHCalEventReader::ParseSteering(std::string filename)
{
  theMarlinParameterMap_.clear();
  xmlDoc *doc = NULL;
  xmlNode *root_element = NULL;

  /*
   * this initialize the library and check potential ABI mismatches
   * between the version it was compiled for and the actual shared
   * library used.
   */
  LIBXML_TEST_VERSION

    /*parse the file and get the DOM */
    doc = xmlReadFile(filename.c_str(), NULL, 0);

  if (doc == NULL) {
    printf("error: could not parse file %s\n", filename.c_str());
  }

  /*Get the root element node */
  root_element = xmlDocGetRootElement(doc);

  //print_element_names(root_element);


  ParseElement(root_element);
  



  /*free the document */
  xmlFreeDoc(doc);

  /*
   *Free the global variables that may
   *have been allocated by the parser.
   */
  xmlCleanupParser();

  // Correct geometry map
  this->correctGeometry();

  return ;

}

void DHCalEventReader::correctGeometry()
{
  for (std::map<uint32_t,ChamberPos>::iterator ic=poschambermap_.begin();ic!=poschambermap_.end();ic++)
    {
      std::map<uint32_t,PlanShift>::iterator ip=planshiftmap_.find(ic->second.getPlan());
      if (ip!=planshiftmap_.end())
	{
 	  ic->second.setX0(ic->second.getX0()+ip->second.getX0());
	  ic->second.setY0(ic->second.getY0()+ip->second.getY0());
	  ic->second.setZ0(ic->second.getZ0()+ip->second.getZ0());
	  ic->second.setX1(ic->second.getX1()+ip->second.getX0());
	  ic->second.setY1(ic->second.getY1()+ip->second.getY0());
	  ic->second.setZ1(ic->second.getZ1()+ip->second.getZ0());
	}
    }
}

void DHCalEventReader::ParseElement(xmlNode * a_node)
{
  xmlChar* cgeom=(unsigned char*) "ChamberGeom";
  xmlNode *cur_node = NULL;

  for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
    if (cur_node->type == XML_ELEMENT_NODE) {
      //printf("node type: Element, name: %s\n", cur_node->name);
      xmlChar* name=xmlGetNoNsProp(cur_node,(const xmlChar*) "name");
      if (name!=NULL && (strcmp((const char*) cur_node->name,(const char*)"processor")==0)) 
	{
	  xmlChar* pname=xmlGetNoNsProp(cur_node,(const xmlChar*) "name");
	  xmlChar* lname=xmlGetNoNsProp(cur_node,(const xmlChar*) "library");
	  printf("Processor %s loads from library %s \n",pname,lname);

	}
      if (name!=NULL && (strcmp((const char*) cur_node->name,(const char*)"parameter")==0)) 
	{

	  std::string stype("");
	  bool bval=false;
	  int ival=0;
	  double dval=0;
	  std::string sval("");
	  if (strcmp((const char*) name,(const char*)"DifGeom")==0)
	    {
	      printf("%s \n",name);
	      //getchar();
	      xmlChar* xc=cur_node->content;
	      if (cur_node->children)
		{
		  //printf("%s \n",cur_node->children->content);
		  std::string s((const char*) cur_node->children->content);
		  std::cout<<"DEBUT "<<s<<"FIN"<<std::endl;
		  char *str1, *str2, *token, *subtoken;
		  char *saveptr1, *saveptr2;
		  str1 =( char*) cur_node->children->content;
		  for (int j = 1 ; ; j++) 
		    {
			  
		      token = strtok_r(str1,"\n", &saveptr1);
		      if (token == NULL)
			break;
		      //printf("%d: %s\n", j, token);

			  
		      std::string sdifgeo(token);
		      std::vector<std::string> tokens ;
		      LCTokenizer t( tokens ,',') ;
		      std::for_each( sdifgeo.begin(), sdifgeo.end(), t ) ;
		      std::cout<<tokens.size()<<" "<<tokens[0]<<std::endl;
		      if (tokens.size()==6)
			{
			  int id = atoi(tokens[0].c_str());
			  int ch = atoi(tokens[1].c_str());
			  float xs = atof(tokens[2].c_str());
			  float ys = atof(tokens[3].c_str());
			  float polx = atof(tokens[4].c_str());
			  float poly = atof(tokens[5].c_str());
			  DifGeom dg(id,ch,xs,ys,polx,poly);
			  std::pair<uint32_t,DifGeom> p(id,dg);
			  geodifmap_.insert(p);
			  this->difInChamber(id,ch);
			  printf("%d is in %d \n",id,getChamber(id));
			}
		      str1=NULL;
		    }
		}
	    }
	  if (strcmp((const char*) name,(const char*)"ChamberGeom")==0)
	    {
	      printf("%s \n",name);
	      //getchar();
	      xmlChar* xc=cur_node->content;
	      if (cur_node->children)
		{
		  printf("%s \n",cur_node->children->content);
		  char *str1, *str2, *token, *subtoken;
		  char *saveptr1, *saveptr2;
		  str1 =( char*) cur_node->children->content;
		  for (int j = 1 ; ; j++) 
		    {
			  
		      token = strtok_r(str1,"\n", &saveptr1);
		      if (token == NULL)
			break;
		      printf("%d: %s\n", j, token);

			  
		      std::string schgeo(token);
		      std::vector<std::string> tokens ;
		      LCTokenizer t( tokens ,',') ;
		      std::for_each( schgeo.begin(), schgeo.end(), t ) ;
		      std::cout<<tokens.size()<<" "<<tokens[0]<<std::endl;
		      if (tokens.size()==5)
			{
			  int id = atoi(tokens[0].c_str());
			  float xs = atof(tokens[1].c_str());
			  float ys = atof(tokens[2].c_str());
			  float zs = atof(tokens[3].c_str());
			  float ang = atof(tokens[4].c_str());

			  std::cout<<xs<<" "<<ys<<" "<<zs<<std::endl;
			  ChamberGeom chg(id,xs,ys,zs,ang,id);
			  std::pair<uint32_t,ChamberGeom> p(id,chg);
			  geochambermap_.insert(p);
			}
		      if (tokens.size()==6)
			{
			  int id = atoi(tokens[0].c_str());
			  float xs = atof(tokens[1].c_str());
			  float ys = atof(tokens[2].c_str());
			  float zs = atof(tokens[3].c_str());
			  float ang = atof(tokens[4].c_str());
			  int plan = atoi(tokens[5].c_str());
			  std::cout<<xs<<" "<<ys<<" "<<zs<<"----> plan "<<plan<<std::endl;
			  ChamberGeom chg(id,xs,ys,zs,ang,plan);
			  std::pair<uint32_t,ChamberGeom> p(id,chg);
			  geochambermap_.insert(p);
			}
		      str1=NULL;
		    }
		}

	    }

	  if (strcmp((const char*) name,(const char*)"ChamberPos")==0)
	    {
	      printf("%s \n",name);
	      //getchar();
	      xmlChar* xc=cur_node->content;
	      if (cur_node->children)
		{
		  printf("%s \n",cur_node->children->content);
		  char *str1, *str2, *token, *subtoken;
		  char *saveptr1, *saveptr2;
		  str1 =( char*) cur_node->children->content;
		  for (int j = 1 ; ; j++) 
		    {
			  
		      token = strtok_r(str1,"\n", &saveptr1);
		      if (token == NULL)
			break;
		      printf("%d: %s\n", j, token);

			  
		      std::string schgeo(token);
		      std::vector<std::string> tokens ;
		      LCTokenizer t( tokens ,',') ;
		      std::for_each( schgeo.begin(), schgeo.end(), t ) ;
		      std::cout<<tokens.size()<<" "<<tokens[0]<<std::endl;
		      if (tokens.size()==9)
			{
			  int id = atoi(tokens[0].c_str());
			  float x0 = atof(tokens[1].c_str());
			  float y0 = atof(tokens[2].c_str());
			  float z0 = atof(tokens[3].c_str());
			  float x1 = atof(tokens[4].c_str());
			  float y1 = atof(tokens[5].c_str());
			  float z1 = atof(tokens[6].c_str());
			  int plan = atoi(tokens[7].c_str());
			  int type = atoi(tokens[8].c_str());

			  std::cout<<x0<<" "<<y0<<" "<<z0<<std::endl;
			  std::cout<<x1<<" "<<y1<<" "<<z1<<std::endl;
			  ChamberPos chg(id,x0,y0,z0,x1,y1,z1,plan,type);
			  std::pair<uint32_t,ChamberPos> p(id,chg);
			  poschambermap_.insert(p);
			}
		      str1=NULL;
		    }
		}

	    }
	  if (strcmp((const char*) name,(const char*)"PlanShift")==0)
	    {
	      printf("%s \n",name);
	      //getchar();
	      xmlChar* xc=cur_node->content;
	      if (cur_node->children)
		{
		  printf("%s \n",cur_node->children->content);
		  char *str1, *str2, *token, *subtoken;
		  char *saveptr1, *saveptr2;
		  str1 =( char*) cur_node->children->content;
		  for (int j = 1 ; ; j++) 
		    {
			  
		      token = strtok_r(str1,"\n", &saveptr1);
		      if (token == NULL)
			break;
		      printf("%d: %s\n", j, token);

			  
		      std::string schgeo(token);
		      std::vector<std::string> tokens ;
		      LCTokenizer t( tokens ,',') ;
		      std::for_each( schgeo.begin(), schgeo.end(), t ) ;
		      std::cout<<tokens.size()<<" "<<tokens[0]<<std::endl;
		      if (tokens.size()==4)
			{
			  int id = atoi(tokens[0].c_str());
			  float dx0 = atof(tokens[1].c_str());
			  float dy0 = atof(tokens[2].c_str());
			  float dz0 = atof(tokens[3].c_str());
			  std::cout<<dx0<<" "<<dy0<<" "<<dz0<<std::endl;
			  PlanShift chg(id,dx0,dy0,dz0);
			  std::pair<uint32_t,PlanShift> p(id,chg);
			  planshiftmap_.insert(p);
			}
		      str1=NULL;
		    }
		}

	    }


	  if (strcmp((const char*) name,(const char*)"ChamberGeom")!=0 &&
	      strcmp((const char*) name,(const char*)"ChamberPos")!=0 &&
	      strcmp((const char*) name,(const char*)"PlanShift")!=0 &&
	      strcmp((const char*) name,(const char*)"DifGeom")!=0)
	    {
	      //std::cout<<name<<std::endl;
	      xmlChar* ctype=xmlGetNoNsProp(cur_node,(const xmlChar*) "type");
	      if (ctype!=NULL)
		{
		  //std::cout<<"Type "<<ctype<<std::endl;
		  if (strcmp((const char*) ctype,(const char*)"string")==0)
		    {
		      xmlChar* xc=cur_node->children->content;
		      sval.assign((const char*) xc);
		      stype="string";
		    }
		       

		}
	      else
		//Check the value
		{
		  xmlChar* cvalue=xmlGetNoNsProp(cur_node,(const xmlChar*) "value");
		  if (cvalue==NULL) continue;
		       
		  if (strcmp((const char*) cvalue,(const char*)"true")==0)
		    {
		      bval=true;
		      stype="bool";
		    }
		       
		  if (strcmp((const char*) cvalue,(const char*)"false")==0)
		    {
		      bval=false;
		      stype="bool";
		    }
		  std::string svalue((const char*) cvalue);
		  if (svalue.find(".")!=std::string::npos)
		    {
		      dval=atof((const char*)cvalue);
		      stype="double";
		    }
		  else
		    if (stype.compare("bool")!=0)
		      {
			ival = atoi((const char*)cvalue);
			stype="int";
		      }
		   
		}
	      std::cout<<stype<<"===>"<<name<<" "<<bval<<" "<<dval<<" "<<ival<<" "<<sval<<std::endl;
	      std::string sname((const char*) name);
	      MarlinParameter p(sname,stype,bval,dval,ival,sval);
	      std::pair<std::string,MarlinParameter> pa(sname,p);
	      theMarlinParameterMap_.insert(pa);
	    }
	}
    }
    /* else */
    /*   printf("%s %s \n",cur_node->name,cur_node->content); */

    ParseElement(cur_node->children);
  }
}
