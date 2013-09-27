#include "DIFUnpacker.h"

#include "FilterAnalyzer.h"
std::map<uint32_t,IMPL::LCEventImpl*> evtmap;
void FilterAnalyzer::initHistograms()
{

}
FilterAnalyzer::FilterAnalyzer(DHCalEventReader* r,DCHistogramHandler* h)  :useSynchronized_(false),minChambersInTime_(3),lastEvent_(0)
{
  reader_=r;
  handler_ =h;
  headerWritten_=false;
}
void FilterAnalyzer::endJob()
{
  for (std::map<uint32_t,IMPL::LCEventImpl*>::iterator it=evtmap.begin();it!=evtmap.end();it++)
    {
      std::cout<<" Eritting"<<it->first;
      reader_->write(it->second);
    }
}

void FilterAnalyzer::processEvent()
{
  
  if (reader_->getEvent()==0) return;
  if (reader_->getRunHeader()!=0 && !headerWritten_) 
    {
      reader_->writeRunHeader();
      headerWritten_=true;
    }

  try {

    //    reader_->buildEvent(rebuild);
    //LCTOOLS::dumpEvent( reader_->getEvent() ) ;
    //    getchar();
    // reader_->analyzeEvent();
      // getchar();
    std::cout<<"event "<<reader_->getEvent()->getEventNumber()<<std::endl;
      reader_->parseRawEvent();
      //reader_->flagSynchronizedFrame(9);
#if DU_DATA_FORMAT_VERSION <= 12
      if (useSynchronized_)
	reader_->fastFlag(minChambersInTime_/3,minChambersInTime_);

      //
      // getchar();
      IMPL::LCCollectionVec* HitVec=reader_->createRawCalorimeterHits(useSynchronized_);
#else
      std::vector<uint32_t> seed;
     if (useSynchronized_)
       {
	 //printf("Calling FastFlag2\n");
      
	 reader_->findTimeSeeds(minChambersInTime_,seed);
	 // printf("End of FastFlag2 \n");
      
       }
     else
       {
	 
	 seed.clear();
       }
  //
  // getchar();
     //printf("Calling CreaetRaw\n");

      IMPL::LCCollectionVec* HitVec=reader_->createRawCalorimeterHits(seed);
      
      //printf("End of CreaetRaw %d \n",rhcol->getNumberOfElements());
#endif
      IMPL::LCEventImpl* evt_=reader_->getEvent();
      if (evt_->getEventNumber()%1==0)
	std::cout<<evt_->getEventNumber()<<" Number of Hit "<<HitVec->getNumberOfElements()<<std::endl;
      
      IMPL::LCEventImpl* evtOutput_ =new IMPL::LCEventImpl();
      evtOutput_->setRunNumber(evt_->getRunNumber());
      evtOutput_->setEventNumber(evt_->getEventNumber());
      evtOutput_->setTimeStamp(evt_->getTimeStamp());
      evtOutput_->setDetectorName(evt_->getDetectorName());
      evtOutput_->setWeight(evt_->getWeight());
      evtOutput_->addCollection(HitVec,"DHCALRawHits");
      //LCTOOLS::printRawCalorimeterHits(HitVec);

      if (writing_)
	//reader_->write(evtOutput_);
	{
	  std::pair<uint32_t,IMPL::LCEventImpl*> p(evt_->getEventNumber(),evtOutput_);
	  evtmap.insert(p);
	  std::map<uint32_t,IMPL::LCEventImpl*>::iterator nextevt=evtmap.find(lastEvent_+1);
	  if (nextevt!=evtmap.end())
	    {
	      printf(" Writting %d \n",nextevt->first);
	      reader_->write(nextevt->second);
	      evtmap.erase(nextevt);
	      lastEvent_=lastEvent_+1;
	    }
	  
	}
      else
	delete evtOutput_;
  }
  catch (std::string e )
    {
      std::cout<<e<<std::endl;
    }


}
