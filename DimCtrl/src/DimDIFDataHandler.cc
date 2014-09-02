#include "DimDIFDataHandler.h"
#include "ShmProxy.h"
DimDIFDataHandler::DimDIFDataHandler(uint32_t id,std::string prefix) : theId_(id),thePrefix_(prefix),theShmPrefix_("/dev/shm")
{
  std::stringstream s0;
  s0.str(std::string());
  s0<<prefix<<"DIF"<<id<<"/INFO";
  theDIFInfo_ = new DimInfo(s.str().c_str(),&difStatus_,sizeof(DIFStatus),this);
  s0.str(std::string());
  s0<<prefix<<"DIF"<<id<<"/DATA";
  theDIFData_ = new DimInfo(s.str().c_str(),&difData_,32*1024*sizeof(uint32_t),this);

}     
~DimDIFDataHandler::DimDIFDataHandler()
{
  delete theDIFInfo_;
  delete theDIFData_;
}

void DimDIFDataHandler::infoHandler()
{
   DimInfo *curr = (DimInfo*) getInfo(); // get current DimStampedInfo address
   std::cout<<curr->getName()<<std::endl;
   if (curr=theDIFInfo_)
     memcpy(&difStatus_,curr->getData(),sizeof(DIFStatus));
   if (curr=theDIFData_)
     {
       memcpy(&difData_,curr->getData(),curr->getSize());
       // copy to Shm
       ShmProxy::transferToFile(curr->getData(),
				curr->getSize(),
				ShmProxy::getBufferABCID(curr->getData()),
				ShmProxy::getBufferDTC(curr->getData()),
				ShmProxy::getBufferGTC(curr->getData()),
				ShmProxy::getBufferDIF(curr->getData()));

      if (ShmProxy::getBufferDTC(curr->getData())%10000 == 0)
	printf("%s receieve %d  bytes, BCID %lld DTC %d GTC %d DIF %d \n",__PRETTY_FUNCTION__,
	       curr->getSize(),
	       ShmProxy::getBufferABCID(curr->getData()),
	       ShmProxy::getBufferDTC(curr->getData()),
	       ShmProxy::getBufferGTC(curr->getData()),
	       ShmProxy::getBufferDIF(curr->getData()));
     }
}
