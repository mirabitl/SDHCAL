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

#define CHECK_BIT(var,pos) ((var)& (1<<(pos)))

//static DCFrame theFrameBuffer[256*48*128];
DHCalEventReader::DHCalEventReader() :dropFirstRU_(true),theXdaqShift_(92),currentFileName_("NONE")
{
  evt_ = 0;
  runh_ = 0;
  lcReader_=0;
  lcWriter_=0;
  vdif_.clear();
  vframe_.clear();
  vslow_.clear();
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
  difPtr0_ = new DCDIF[256];
//  framePtr0_= &theFrameBuffer[0];
//static DCFrame theFrameBuffer[256*48*128];
  //framePtr0_= new  DCFrame[256*48*64];
  printf("%d %d %x \n",nGood_,nBad_,nBadTwo_);
  expectedDtc_=0;
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
  if (lcReader_==0) 
    {
      lcReader_ = LCFactory::getInstance()->createLCReader(IO::LCReader::directAccess) ;
      //lcReader_->registerLCRunListener(this) ;
      // lcReader_->registerLCEventListener(this) ;
      
    }

  try{
    for (std::vector< std::string >::iterator it=filenames.begin();it!=filenames.end();it++)
      std::cout<<"File "<<(*it)<<std::endl;
    lcReader_->open( filenames) ;
    printf("All those files have %d events \n",lcReader_->getNumberOfEvents());
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
      lcReader_ = LCFactory::getInstance()->createLCReader() ;
      lcReader_->registerLCRunListener(this) ;
      lcReader_->registerLCEventListener(this) ;
      
    }

  try{
    
    lcReader_->open( name.c_str() ) ;
    printf("%s has %d events \n",name.c_str(),lcReader_->getNumberOfEvents());
  }
  catch( IOException& e) {
    std::cout << e.what() << std::endl ;
    exit(1) ;
  }
}

void DHCalEventReader::close()
{
  //std::cout<<"On ferme "<<std::endl;
  try{
    lcReader_->close() ;
  }
  catch( IOException& e) {
    std::cout << e.what() << std::endl ;
    exit(2) ;
  }
  //std::cout<<"Bye "<<std::endl;
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
    this->findEvent(run,event);
    evt_ = (IMPL::LCEventImpl*) lcReader_->readEvent(run,event) ; 
    if (evt_==0) {currentFileName_="NONE"; this->findEvent(run,event);}
    if (evt_!=0)
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


void DHCalEventReader::clear()
{
#ifdef DELETE_ALL
  //  std::cout<<"DIF size "<<vdif_.size()<<std::endl;
  for (std::vector<DCDIF*>::iterator it=vdif_.begin();it!=vdif_.end();it++) {delete (*it);}
  //  std::cout<<"FRAME size "<<vframe_.size()<<std::endl;
  for (std::vector<DCFrame*>::iterator it=vframe_.begin();it!=vframe_.end();it++){delete (*it);}
#endif
  vdif_.clear();
  for (std::vector<DCFrame*>::iterator it=vframe_.begin();it!=vframe_.end();it++){delete (*it);}
  vframe_.clear();

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
  difPtr_=difPtr0_;
  //framePtr_=framePtr0_;
  std::vector<DIFSlowControl*> tmpslow;
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
      
#if DU_DATA_FORMAT_VERSION >= 13
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
	  //printf(" Found start of Buffer at %d %d contains %x and %d bytes \n",idstart,id0,tcbuf[idstart],rusize-idstart+1);

	  
	  

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

      return 0;
#endif
      //getchar();
      for (int j=0;j<col->getNumberOfElements(); j++)
	{
	  //IMPL::LCGenericObjectImpl* go= (IMPL::LCGenericObjectImpl*) col->getElementAt(j);
	  LMGeneric* go= (LMGeneric*) col->getElementAt(j);
	  int rusize =go->getNInt(); 
	  //	  std::cout<<" New RU found with "<<rusize<<" int stored" <<std::endl;


	  // if (rusize==56) continue; // RU_0
	  if (j==0 && dropFirstRU_) continue;
	  int difshift=0;

	  DCDIF* dif=NULL;
	  //std::vector<DCFrame*> frames;
	  // int buf[rusize];
	  int* buf=&(go->getIntVector()[0]);
	  int shift=-1;
	  // for (int k=0;k<go->getNInt();k++)
	  //   {
	    
	  //     buf[k] = go->getIntVal(k);
	  //   }
	  // for (int kk=0;kk<100;kk++) printf(" %x ",buf[kk]);
	  // printf(" New Buffer %d \n",rusize);

	  // printf("From vector it gives %d \n",go->getVector().size());
	  // int *ibuf=&(go->getVector()[0]);
	  // for (int kk=0;kk<100;kk++) printf(" %x ",ibuf[kk]);
	  // getchar();
	  if (debugLevel_>1)
	    {
	      unsigned char* ucdata= (unsigned char*)buf;	  for (int i=0;i<rusize;i++) {printf("%02x",ucdata[i]); if ((i+1)%100000==0) 	printf("\n");};	  printf("\n");
	    }
	  // Check event structure
	  unsigned char* tcbuf = (unsigned char*) buf;

	  // uint32_t idstart=DIFUnpacker::getStartOfDIF(tcbuf,rusize*sizeof(int32_t),94);
	  // printf(" Found start of Buffer at %d contains %x \n",idstart,tcbuf[idstart]);
	  // unsigned char* tcdif=&tcbuf[idstart];
	  // DIFPtr d(tcdif,rusize*sizeof(int32_t)-idstart+1);
	  // d.dumpDIFInfo();
	  // printf("DIF %d DTC %d GTC %d ABCID %lld BCID %d Lines %d Temeprature %d \n",
	  // 	 DIFUnpacker::getID(tcdif),
	  // 	 DIFUnpacker::getDTC(tcdif),
	  // 	 DIFUnpacker::getGTC(tcdif),
	  // 	 DIFUnpacker::getAbsoluteBCID(tcdif),
	  // 	 DIFUnpacker::getBCID(tcdif),
	  // 	 DIFUnpacker::getLines(tcdif),
	  // 	 DIFUnpacker::hasTemperature(tcdif));

	  // if (DIFUnpacker::hasTemperature(tcdif))
	  //   printf("T: ASU1 %d ASU2 %d DIF %d \n",DIFUnpacker::getTASU1(tcdif),DIFUnpacker::getTASU2(tcdif),DIFUnpacker::getTDIF(tcdif));
	  
	  // vF.push_back(tcdif);
	  // vL.push_back(tcdif);
	  // try
	  //   {
	  //     DIFUnpacker::getFramePtr(vF,vL,tcdif);
	  //   }
	  // catch (std::string e)
	  //   {
	  //     std::cout<<e<<std::endl;
	  //   }
	  // printf("Found %d Lines and %d Frames \n",vL.size(),vF.size());
	  // for (uint32_t i=1;i<d.getNumberOfFrames();i++)
	  //   {
	  //     std::bitset<64> bs0(0);
	  //     std::bitset<64> bs1(0);
	  //     for (uint32_t ip=0;ip<64;ip++) {bs0.set(ip,d.getFrameLevel(i,ip,0));bs1.set(ip,d.getFrameLevel(i,ip,1));}
	  //     printf("\t %d %d %d \n",d.getID(),d.getFrameTimeToTrigger(i),d.getFrameAsicHeader(i));
	  //     std::cout<<"\t \t"<<bs0<<std::endl;
	  //     std::cout<<"\t \t"<<bs1<<std::endl;
	  //     std::cout<<"Old way "<<std::endl;
	  //     DIFUnpacker::dumpFrameOld(d.getFramePtr(i));

	  //   }
	  //getchar();
	  bool slowcontrol; uint32_t version,hrtype,id0,iddif;
	  DCBufferReader::checkType(tcbuf,rusize,slowcontrol,version,hrtype,id0,iddif,theXdaqShift_);
	  // std::cout<<"RU "<<j<<" Slow control "<<(int) slowcontrol<<" version "<<version<<"  Hardroc type "<<hrtype<<" shift "<<id0<<std::endl;
#ifndef USE6NEWSTUFF
	  if (debugLevel_>1) getchar();
	  if (DebugBuffer((unsigned char*) &tcbuf[id0],go->getNInt()*sizeof(int)-id0)<0) continue;

	  continue;
#endif
	  if (slowcontrol)
	    {
	      std::cout<<" Slow control "<<(int) slowcontrol<<" version "<<version<<"  Hardroc type "<<hrtype<<" shift "<<id0<<std::endl;
	      version_=version;
	      hrType_=hrtype;
	      difshift=iddif;
	    }
	  if (hrtype==0) hrtype=2;
	  shift=id0;
	  if (shift>400 || shift==-1) continue;
	  unsigned char* cbuf = &tcbuf[shift];
	  //std::cout<<std::dec<<" First Buffer is "<< shift << " "<<std::hex<<(int) cbuf[0] <<std::dec<<std::endl;


	  if (shift == 88 && cbuf[shift] >= 10 && cbuf[shift]<=15) // Small chamber no Slow control read
	    {
	      std::cout<<"SMAL DIF FORMAT "<<std::endl;
	      unsigned char* tbuf = (unsigned char*) &cbuf[88];
	      DCBufferReader::ReadDIFBlockSmall(tbuf,dif,&vframe_);


	      //	      DEBUG(small DIF read)
	      vdif_.push_back(dif);

	    }
	  
	  else // Normal plane HR1 or HR2
	    if (cbuf[difshift]==0xb0)
	      {
		try
		  {
		    DCBufferReader::ReadDIFBlock((unsigned char*) &cbuf[difshift],dif,&vframe_,getHardRocType());
		  }
		catch (std::string e)
		  {
		    
		    std::string s=" Cannot Read DIF "+e;
		    throw s+__PRETTY_FUNCTION__;
		  }
		if (dif == 0) continue;
		vdif_.push_back(dif);
		//std::cout<<" DIF  "<<  dif->getId()<<" time " <<dif->getTBC()<<"  # of frames"<<vframe_.size()<<"  Slow control"<<difshift<<std::endl;
		if (difshift!=0) 
		  {
		    DIFSlowControl* dsc = new DIFSlowControl(getVersion(),dif->getId(),cbuf);
		    tmpslow.push_back(dsc);
		  }
		
		
		int nsynch=0,nframe=0;
		for (uint32_t j=0;j<vframe_.size();j++)
		  {
		    DCFrame*	f=vframe_[j];
		    //std::cout<<" Time to trigger "<<f->getTimeToTrigger()<<std::endl;
		    if (f->getTimeToTrigger()>=1 && f->getTimeToTrigger()<=5)
		      {
			f->setSynchronised(true);
			nsynch++;
		      }
		    else
		      f->setSynchronised(false);
		    nframe++;
		  }
		
		//	std::cout<<" in Time " <<nsynch<<std::endl;	     
	      } 
	}
    }


  if (tmpslow.size()!=0)
    {
      // Clear vslow
      std::cout<<"SLOW size "<<vslow_.size()<<std::endl;
      for (std::vector<DIFSlowControl*>::iterator it=vslow_.begin();it!=vslow_.end();it++) {delete (*it);}
      vslow_.clear();
      for (uint32_t i =0;i<tmpslow.size();i++)
	vslow_.push_back(tmpslow[i]);
      std::cout<<"SLOW size "<<vslow_.size()<<std::endl;
      fillRunHeader();
    }
  //std::cout<< "End of Event "<<std::endl;
  return (tmpslow.size());
}


IMPL::LCCollectionVec* DHCalEventReader::createRawCalorimeterHits(bool useSynch)
{
  // Save current Object count
  IMPL::LCCollectionVec *RawVec=new IMPL::LCCollectionVec(LCIO::RAWCALORIMETERHIT) ;
 
  //Prepare a flag to tag data type in RawVec (dit les types de data qu'on va enregistrer?)
  IMPL::LCFlagImpl chFlag(0) ;
  EVENT::LCIO bitinfo;
  chFlag.setBit(bitinfo.RCHBIT_LONG ) ;                    // raw calorimeter data -> format long //(sert a qq chose?)
  chFlag.setBit(bitinfo.RCHBIT_BARREL ) ;                  // barrel
  chFlag.setBit(bitinfo.RCHBIT_ID1 ) ;                     // cell ID 
  chFlag.setBit(bitinfo.RCHBIT_TIME ) ;                    // timestamp
  RawVec->setFlag( chFlag.getFlag()  ) ;   
  

  for (std::vector<DCFrame*>::iterator it = vframe_.begin();it!=vframe_.end();it++)
    {
      DCFrame* f = (*it);
      if (useSynch && !f->isSynchronised()) continue;
      // Loop on Hits
      //      std::cout<<f->getDifId()<<" new frame "<<f->getAsicId()<<std::endl;
      for (uint32_t j=0;j<64;j++)
	{
	  if (!(f->getLevel0(j) || f->getLevel1(j))) continue; // skip empty pads
	  //  std::cout <<" New hit "<<std::endl;
	  unsigned long int ID0;
	  ID0=(unsigned long int)(((unsigned short)f->getDifId())&0xFF);			//8 firsts bits: DIF Id
	  ID0+=(unsigned long int)(((unsigned short)f->getAsicId()<<8)&0xFF00);	//8 next bits:   Asic Id
	  bitset<6> Channel(j);														
	  ID0+=(unsigned long int)((Channel.to_ulong()<<16)&0x3F0000);				//6 next bits:   Asic's Channel
	  unsigned long BarrelEndcapModule=0;  //(40 barrel + 24 endcap) modules to be coded here  0 for testbeam (over 6 bits)
	  ID0+=(unsigned long int)((BarrelEndcapModule<<22)&0xFC00000);	
	  unsigned long int ID1 = (unsigned long int)(f->getBunchCrossingTime()/DCBufferReader::getDAQ_BC_Period());
	  bitset<3> ThStatus;
	  ThStatus[0]=f->getLevel0(j);
	  ThStatus[1]=f->getLevel1(j);
	  ThStatus[2]=f->isSynchronised();
	  
	  IMPL::RawCalorimeterHitImpl *hit=new IMPL::RawCalorimeterHitImpl() ;
	  hit->setCellID0((unsigned long int)ID0);               
	  hit->setCellID1(ID1);
	  hit->setAmplitude(ThStatus.to_ulong());
	  //unsigned long int TTT = (unsigned long int)rint(f->getRecoBunchCrossingTime()/DCBufferReader::getDAQ_BC_Period());
	  /// LM test unsigned long int TTT = (unsigned long int)rint(f->getBunchCrossingTime()/DCBufferReader::getDAQ_BC_Period());
	  unsigned long int TTT = (unsigned long int)(f->getTimeToTrigger());
	  //std::cout<<f->getDifId()<<" "<<f->getAsicId()<<" "<<f->getBunchCrossingTime()<<" "<<f->getRecoBunchCrossingTime()<<" "<<f->getTimeToTrigger()<<std::endl;
	  //if (fabs((f->getRecoBunchCrossingTime()/DCBufferReader::getDAQ_BC_Period())-f->getTimeToTrigger())>0.1)
	  //  {
	  //    std::cout<<f->getDifId()<<" "<<f->getAsicId()<<" "<<f->getBunchCrossingTime()<<" "<<(f->getRecoBunchCrossingTime()/DCBufferReader::getDAQ_BC_Period())<<" "<<f->getTimeToTrigger()<<std::endl;
	      //	      getchar();
	  //  }
	  hit->setTimeStamp(TTT);											//Time stamp of this event from Run Begining

	  //	  std::cout<<"rebd "<<TTT<<" "<<hit->getCellID1()<<std::endl;
	  RawVec->addElement(hit);
	}
    }
  for (std::vector<DCDIF*>::iterator it = vdif_.begin();it!=vdif_.end();it++)
    {
      DCDIF* d = (*it);
      lcio::IntVec trig(6);
      trig[0] = d->getDTC();
      trig[1] = d->getGTC();
      trig[2] = d->getATC();
      trig[3] = (unsigned long int) (d->getTBC()/DCBufferReader::getDAQ_BC_Period());
      unsigned long long  lbc = (unsigned long long) (d->getLBC()/DCBufferReader::getDAQ_BC_Period());
      
      trig[4] = d->getLBA();
      trig[5] = d->getLBB();
      std::stringstream ss("");
      ss<<"DIF"<<d->getId()<<"_Triggers";
      RawVec->parameters().setValues(ss.str(),trig);
    }
  return RawVec;
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
	   if (RawVec->getNumberOfElements()>10000) break; //too may noise
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

  

#if DU_DATA_FORMAT_VERSION <= 12
  if (RawVec>0)
    {
      try {
	nslow_=parseRawEvent();
	//printf("Event parsed \n");
	if (compactify_)
	  {
	    flagSynchronizedFrame(bd_synchcut_,bd_nfcut_);
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
    evt_->addCollection(this->createRawCalorimeterHits(compactify_),srhcol);
  }
  catch( IOException& e) {
    std::cout << e.what() << std::endl ;
    //  exit(2) ;
  }

#else
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
 
#endif

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





void DHCalEventReader::fastFlag(uint32_t seedCut,uint32_t minChamberInTime)
{
  //getchar();
  flagger_.clear();
  // flag
  uint32_t max_time=0;
  for (std::vector<DCFrame*>::iterator it = vframe_.begin();it!=vframe_.end();it++)
    {
      DCFrame* f = (*it);
      uint32_t chid= getChamber(f->getDifId());
      // LMTest      uint32_t bc = rint(f->getBunchCrossingTime()/DCBufferReader::getDAQ_BC_Period());
      uint32_t bc=f->getTimeToTrigger();
      max_time=(bc>max_time && bc<MAX_TIME_WORD*32)?bc:max_time;
      if (!flagger_.isDifSet(f->getDifId(),bc)) //Flag only once
	{
	  flagger_.setDifTime(f->getDifId(),bc);
	  flagger_.setChamberTime(chid,bc);
	}
      //std::cout<<f->getDifId()<<" "<<chid<<" "<<bc<<std::endl;

    }
  flagger_.setMaxTime(max_time);
  //std::cout<<max_time<<std::endl;
  //flagger_.summarize();
  // seeding
  std::vector<uint32_t> seed_;
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
  // Tag
  //std::cout<<"# of seed "<<seed_.size()<<std::endl;
  // getchar();
  uint32_t nsynch=0;
   for (std::vector<DCFrame*>::iterator it = vframe_.begin();it!=vframe_.end();it++)
    {
      DCFrame* f = (*it);
      f->setSynchronised(false);
      // LM test uint32_t bc = rint(f->getBunchCrossingTime()/DCBufferReader::getDAQ_BC_Period());
      uint32_t bc=f->getTimeToTrigger();
      if (find(seed_.begin(),seed_.end(),bc)!=seed_.end()) f->setSynchronised(true);
      if (find(seed_.begin(),seed_.end(),bc-1)!=seed_.end()) f->setSynchronised(true);
      if (find(seed_.begin(),seed_.end(),bc+1)!=seed_.end()) f->setSynchronised(true);
      if (find(seed_.begin(),seed_.end(),bc-2)!=seed_.end()) f->setSynchronised(true);
      if (find(seed_.begin(),seed_.end(),bc+2)!=seed_.end()) f->setSynchronised(true);
      if (f->isSynchronised()) nsynch++;
    }
   // if (nsynch)
   //   std::cout<<"# of synch "<<nsynch<<std::endl;
   // getchar();

}
void DHCalEventReader::flagSynchronizedFrame(uint32_t synchcut,uint32_t nfcut)
{

  uint32_t window=2;
  if (DCBufferReader::getDAQ_BC_Period()==0.4) window=2;
  std::map<uint32_t,std::bitset<50> > timeChamber;
  std::vector<double_t> seedtime;seedtime.clear();
  for (std::vector<DCFrame*>::iterator it = vframe_.begin();it!=vframe_.end();it++)
    {
      DCFrame* f = (*it);
      uint32_t chid= getChamber(f->getDifId());
      //unsigned long int bc = (unsigned long int)rint(f->getRecoBunchCrossingTime()/DCBufferReader::getDAQ_BC_Period());
      unsigned long int bc=f->getTimeToTrigger();
#ifndef OLDTIMEWINDOW
      
      bool found=false;
      for (uint32_t iw=0;iw<window;iw++)
	{
	  if (timeChamber.find(bc+iw) != timeChamber.end())
	    {
	      std::map<uint32_t,std::bitset<50> >::iterator tc=timeChamber.find(bc+iw);
	      tc->second.set(chid,true);
	      found=true;
	      break;
	    }
	  if (timeChamber.find(bc-iw) != timeChamber.end())
	    {
	      std::map<uint32_t,std::bitset<50> >::iterator tc=timeChamber.find(bc-iw);
	      tc->second.set(chid,true);
	      found=true;
	      break;
	    }

	}
      if (!found)
	{
	  std::bitset<50> sc(0);
	  sc.set(chid,true);
	  pair <uint32_t,std::bitset<50> > pc(bc,sc);
	  timeChamber.insert(pc);
	}
	
#else
      if (timeChamber.find(bc) != timeChamber.end())
	{
	  std::map<uint32_t,std::bitset<50> >::iterator tc=timeChamber.find(bc);
	  tc->second.set(chid,true);
	} else
	if (timeChamber.find(bc-1)!= timeChamber.end())
	  {
	    std::map<uint32_t,std::bitset<50> >::iterator tc=timeChamber.find(bc-1);
	    tc->second.set(chid,true);
	  }
	else
	  if (timeChamber.find(bc+1)!= timeChamber.end())
	    {
	      std::map<uint32_t,std::bitset<50> >::iterator tc=timeChamber.find(bc+1);
	      tc->second.set(chid,true);

	    } 
	  else
	    {
	      std::bitset<50> sc(0);
	      sc.set(chid,true);
	      pair <uint32_t,std::bitset<50> > pc(bc,sc);
	      timeChamber.insert(pc);
	    }
#endif

    }
#undef DEBUGPATTERN
#ifdef DEBUGPATTERN
  for (std::map<uint32_t,std::bitset<50> >::iterator it=timeChamber.begin();it!=timeChamber.end();it++)
    {
      uint32_t nchambers=0;
      for (uint32_t ib=0;ib<50;ib++)
	if (it->second[ib]!=0) nchambers++;
      

      //if (nchambers<nfcut) continue;
      std::cout<<it->first<<" "<<nchambers<<" "<<it->second<<std::endl;
      seedtime.push_back(it->first*DCBufferReader::getDAQ_BC_Period());
    }
#endif
  //  std::cout<<"Number of Seed"<<seedtime.size()<<" and frames "<<vframe_.size()<<std::endl;
  uint32_t nsynch=0;

  for (std::vector<DCFrame*>::iterator ft = vframe_.begin();ft!=vframe_.end();ft++)
    {
      DCFrame* f = (*ft);
      //LM Test unsigned long int bc = (unsigned long int)rint(f->getRecoBunchCrossingTime()/DCBufferReader::getDAQ_BC_Period());
      unsigned long int bc=f->getTimeToTrigger();
      for (uint32_t iw=0;iw<window;iw++)
	{
      
	  std::map<uint32_t,std::bitset<50> >::iterator tc=timeChamber.find(bc+iw);
	  if (tc!=timeChamber.end())
	    {
	      uint32_t nchambers=0;
	      for (uint32_t ib=0;ib<50;ib++)
		if (tc->second[ib]!=0) nchambers++;
	      
	      if (nchambers>=nfcut) f->setSynchronised(true);
	      break;
	    }
	  else
	    {
	      tc=timeChamber.find(bc-iw);
	      if (tc!=timeChamber.end())
		{
		  uint32_t nchambers=0;
		  for (uint32_t ib=0;ib<50;ib++)
		    if (tc->second[ib]!=0) nchambers++;
		  
		  if (nchambers>=nfcut) f->setSynchronised(true);
		  break;
		}
	    }
	}
#ifdef OLDTIMEWINDOW
      else
	{
	  tc=timeChamber.find(bc-1);
	  if (tc!=timeChamber.end())
	    {
	      uint32_t nchambers=0;
	      for (uint32_t ib=0;ib<50;ib++)
		if (tc->second[ib]!=0) nchambers++;
	      
	      if (nchambers>=nfcut) f->setSynchronised(true);
	    }
	  else
	    {
	      tc=timeChamber.find(bc+1);
	      if (tc!=timeChamber.end())
		{
		  uint32_t nchambers=0;
		  for (uint32_t ib=0;ib<50;ib++)
		    if (tc->second[ib]!=0) nchambers++;
		  
		  if (nchambers>=nfcut) f->setSynchronised(true);
		}
	      
	    }
	}
#endif
      if (f->isSynchronised()) nsynch++;
    }
  //std::cout<<"Synch frames"<<nsynch<<std::endl;
}
int DHCalEventReader::DebugBuffer(unsigned char* buf,uint32_t max_size)
{

  bool PAD [128];
  unsigned short un = 1;
  uint32_t idx1;
  uint32_t idx=0;

  if (buf[idx]!=0xb0)
    {
      printf(" %s This is not the start of buffer %x \n",__PRETTY_FUNCTION__,buf[0]);
      return -1;
    }
  // Dump DIF info
  idx++;
  uint32_t dif_id=buf[idx];

  //if (dif_id==89) for (int i=0;i<max_size;i++) {printf("%02x",buf[i]); if ((i+1)%100==0) 	printf("\n");}

  //if (dif_id==89)
  //printf("DIF ID %d \n",dif_id);

  idx++;
  uint32_t dtc,atc,gtc;
  dtc= (buf[idx]<<24);
  dtc= dtc | (buf[idx+1]<<16);
  dtc= dtc | (buf[idx+2]<<8);
  dtc= dtc | (buf[idx+3]);
  idx=idx+4;
  atc= (buf[idx]<<24);
  atc= atc | (buf[idx+1]<<16);
  atc= atc | (buf[idx+2]<<8);
  atc= atc | (buf[idx+3]);
  idx=idx+4;
  gtc= (buf[idx]<<24);
  gtc= gtc | (buf[idx+1]<<16);
  gtc= gtc | (buf[idx+2]<<8);
  gtc= gtc | (buf[idx+3]);
  idx=idx+4;
  unsigned long long Shift=16777216ULL;//to shift the value from the 24 first bits
  uint32_t LBCA= (buf[idx]<<16);
  LBCA=LBCA| (buf[idx+1]<<8);
  LBCA=LBCA| (buf[idx+2]);
  idx=idx+3;
  uint32_t LBCB= (buf[idx]<<16);
  LBCB=LBCB| (buf[idx+1]<<8);
  LBCB=LBCB| (buf[idx+2]);
  idx=idx+3;
  unsigned long long LBC = LBCA*Shift + LBCB;

  uint32_t DTS= (buf[idx]<<16);
  DTS=DTS| (buf[idx+1]<<8);
  DTS=DTS| (buf[idx+2]);
  //printf("RU_XDAQ %d %d %d %llu \n",dif_id,LBCA,LBCB,LBC);

  //printf(" %s DIF %d DTC %d ATC %d GTC %d LCB %llu Time Stamp %d  Buffer Size  %d\n",__PRETTY_FUNCTION__,dif_id,dtc,atc,gtc,LBC,DTS,max_size);
  //getchar();
  idx=idx+3;
  if (debugLevel_>0 )
    printf(" %s DIF %d DTC %d ATC %d GTC %d LCB %llu Time Stamp %d  Buffer Size  %d\n",__PRETTY_FUNCTION__,dif_id,dtc,atc,gtc,LBC,DTS,max_size);
#ifdef ONEDIF
  if (dtc!= (uint32_t) (expectedDtc_+1))
    {
      printf(" %s corrupted Trigger %d DIF %d Trigger %d %d %d \n",__PRETTY_FUNCTION__,expectedDtc_+1,dif_id,dtc,atc,gtc);

      
      //return -3;
    }

  if (dtc>expectedDtc_) expectedDtc_=dtc;
#endif
  // New DIF
  DCDIF* dif = difPtr_;difPtr_++;
  dif->setAll(dif_id, dtc, atc, gtc, DTS*DCBufferReader::getDAQ_BC_Period(), LBC);
  dif->setLBA(LBCA);
  dif->setLBB(LBCB);
  
  vdif_.push_back(dif);
  // Now loop on Frames
  for (uint32_t ist=0;ist<10;ist++) // Additional words addedd by Guillaume
    {
      if (buf[idx]==0xb4) break;
      
      if (buf[idx]==0xa0) return 0;
      idx++;
    }
  if (buf[idx]!=0xb4)
    {
      printf(" %s There is no start of frame after 10 increments %x \n",__PRETTY_FUNCTION__,buf[idx]);
      return -2;
    }
  idx++;
  // Loop on frames
  bool newheader=false;
  uint32_t lastgood=0;
  std::vector<DCFrame*> vasic;
  double lasttbc=0;
  bool l0[64],l1[64];
  uint32_t nFrames=0;
  while (buf[idx]!=0xa0)
    {
      uint32_t TS;
      unsigned long long timeStamp;
      std::bitset<64> hits0;
      std::bitset<64> hits1;
      std::bitset<64> hits2;
      uint32_t nonzero;
    StartOfFrame:
      if (buf[idx]==0xb4)
	{
	  idx++;
	  newheader=true;
	  // Correct Reco Time opf previous asic
	  bool dump=DCBufferReader::correctRecoTime(lasttbc,vasic);
	  if (dump) return -10;
	  vasic.clear();
	  lasttbc=dif->getTBC();
	}
      else
	newheader=false;

      uint32_t header=buf[idx];
      if (buf[idx]==0xa3) goto EndOfFrame;

      // Skip corrupted data
      if (header<1 || header>48)
	{
	  //#define DEBUFPRNT
#ifdef DEBUFPRNT
	  printf(" %s corrupted data wrong header %d DIF %d Trigger %d Last Good Frame %x Bytes %d mod %d\n",__PRETTY_FUNCTION__,header,dif_id,dtc,lastgood,idx,idx%256);
#endif
	  if (lastgood==0xFFFFFFFF)
	    {
#ifdef DEBUFPRNT
	      printf(" %s Second corrupted data wrong header %d DIF %d Trigger %d Last Good Frame %x Bytes %d mod %d\n",__PRETTY_FUNCTION__,header,dif_id,dtc,lastgood,idx,idx%256);
#endif
	      nBadTwo_=nBadTwo_+1;
	      //	    getchar();

	      return -4;
	    }

	  // Mots probably the buffer missed 6 bytes
#ifdef DEBUFPRNT
	  for (int32_t jb=-10;jb<20;jb++)
	    printf("%d %.2x ",jb,buf[idx+jb]);
	  printf("\n");
#endif
	  nBad_=nBad_+1;
#ifdef DEBUFPRNT
	  //printf("Good Frame %d  6 bytes less %d  Bytes more %d Empty frames %d \n",nGood_,nBad_,nBadTwo_,nZero_);
	  //getchar();
#endif
	  idx=idx-6;
	  lastgood=0xFFFFFFFF;
	  goto EndOfFrame;
	 
	}
      else
	nGood_=nGood_+1;
      lastgood=(header<<24);

      // Get time stamp
      idx++;
      TS= (buf[idx]<<16);
      TS=TS| (buf[idx+1]<<8);
      TS=TS| (buf[idx+2]);
      lastgood= lastgood| TS;
      timeStamp=DCBufferReader::GrayToBin(TS);
      idx=idx+3;
      nonzero=0;
      unsigned char thr[64];
      memset(thr,0,64);
      // find hit pattern
      for (uint32_t i=0;i<16;i++)
	{
	  if (buf[idx+i]!=0) nonzero++;
	  
	  for (uint32_t j=0;j<4;j++)
	    {
	      if (CHECK_BIT(buf[idx+15-i],(2*j))) 
		{thr[i*4+j] = thr[i*4+j] | 0x1;}
		
	      if (CHECK_BIT(buf[idx+15-i],(2*j+1))) {thr[i*4+j] = thr[i*4+j] | 0x2;}
	      l0[i*4+j]= (thr[i*4+j] & 0x1)!=0;
	      l1[i*4+j]= (thr[i*4+j] & 0x2)!=0;

	    }
	}

      // if (1>0)
// 	{

      for(int ip= 0; ip<128; ip++){PAD[ip]=0;} //init PADs
      idx1=idx;
      for(int ik=0;ik<4;ik++)
	{
	
	  unsigned long PadEtat= DCBufferReader::swap_bytes(4,&buf[idx1]);
	  idx1+=4;
	  
	  for(int e=0;e<32;e++)
	    {	
	      PAD[((3-ik)*32)+(31-e)]=PadEtat & un; //binary operation
	      PadEtat=PadEtat>>1;	//décalage des bit de 1
	    }
	}
      // fill bool arrays
      for(int p=0; p<64;p++)
	{
	  l0[p]=(bool)PAD[(2*p)]; //_Lev0 (PAD paire)
	  l1[p]=(bool)PAD[(2*p)+1]; //_Lev1 (PAD impaires)
	  
	}
      

	  //	}







      idx=idx+16;

      // skip empty frames
      if (nonzero)
	{
	  //DCFrame* f = framePtr_;framePtr_++;
	  DCFrame* f = new DCFrame();
	  f->setAll(dif_id,header,timeStamp*DCBufferReader::getDAQ_BC_Period(),l0,l1,(dif->getTBC())/DCBufferReader::getDAQ_BC_Period()-timeStamp,4);
	  vasic.push_back(f);
	  vframe_.push_back(f);
	  if (lasttbc==0) lasttbc=dif->getTBC();
	  //	  getchar();
	  nFrames=nFrames+1;
	}
      else
	{
	  if (debugLevel_>1)
	    {
	      char pattern[64];
	      memset(pattern,0,64);
	      for (uint32_t i=0;i<64;i++)
		{  
		  sprintf(&pattern[i],"%d",(int) thr[i]);
		}
	      printf(" %s Empy Frame Event %d DIF %d ASIC %d timeStamp %llu %x %d \n Hits  %s \n",__PRETTY_FUNCTION__,dtc,dif_id,header,timeStamp,TS,nonzero,pattern);
	    }
	  nZero_=nZero_+1;
	}
      if (nonzero>0 && newheader && debugLevel_>0)
	{
	  char pattern[64];
	  memset(pattern,0,64);
	  for (uint32_t i=0;i<64;i++)
	    {  
	      if (thr[i])
		std::cout<<i<<"->"<<(int) thr[i]<<std::endl;
	      sprintf(&pattern[i],"%d",(int) thr[i]);
	    }


	  if (newheader)
	    printf(" %s ASIC %d timeStamp %llu %x %d \n Hits  %s \n",__PRETTY_FUNCTION__,header,timeStamp,TS,nonzero,pattern);
	  // End of frame
	}
    EndOfFrame:
      if (idx>max_size) 
	{
	  printf(" %s corrupted data exceeding size %d %d \n",__PRETTY_FUNCTION__,idx,max_size);
	  return -3;
	}
      if (buf[idx]==0xa3) 
	{idx++;
	  if (buf[idx]!=0xb4 && buf[idx]!=0xa0)
	    {
	      printf(" %s corrupted frame %x in dif %d Index %d \n",__PRETTY_FUNCTION__,buf[idx],dif_id,idx);
	      printf(" %s DIF %d DTC %d ATC %d GTC %d LCB %llu Time Stamp %d  Buffer Size  %d\n",__PRETTY_FUNCTION__,dif_id,dtc,atc,gtc,LBC,DTS,max_size);

	      // Correct Reco time of last ASIC
	      bool dump=DCBufferReader::correctRecoTime(lasttbc,vasic);
	      return -4;
	    }
	}
      if (buf[idx]==0xc3) 
	{
	  printf(" %s DIF corrupted frame %x \n",__PRETTY_FUNCTION__,buf[idx]);
	  // Correct Reco time of last ASIC
	  bool dump=DCBufferReader::correctRecoTime(lasttbc,vasic);
	  
	  return -5;

	}
      if (buf[idx]==0xa0) {
	// Correct Reco time of last ASIC
	//if (dtc%100==0)
	//  std::cout<<"Frames "<<nFrames<<std::endl;
	bool dump=DCBufferReader::correctRecoTime(lasttbc,vasic);
	if (dump) return -10;
	return 0;
      }
      
    }
  // Correct Reco time of last ASIC
  bool dump=DCBufferReader::correctRecoTime(lasttbc,vasic);
	  
  return 0;
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

  return ;

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
			  ChamberGeom chg(id,xs,ys,zs,ang);
			  std::pair<uint32_t,ChamberGeom> p(id,chg);
			  geochambermap_.insert(p);
			}
		      str1=NULL;
		    }
		}

	    }

	  if (strcmp((const char*) name,(const char*)"ChamberGeom")!=0 &&
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
