#include "DimDIFDataHandler.h"
#include "ShmProxy.h"
DimDIFDataHandler::DimDIFDataHandler(uint32_t id,std::string prefix) : theId_(id),theShmPrefix_("/dev/shm")
{
  thePrefix_="/DDS";
  std::stringstream s0;
  s0.str(std::string());
  s0<<thePrefix_<<"/DIF"<<id<<"/INFO";
  theDIFInfo_ = new DimInfo(s0.str().c_str(),&difStatus_,sizeof(DIFStatus),this);
  s0.str(std::string());
  s0<<thePrefix_<<"/DIF"<<id<<"/STATE";
  theDIFState_ = new DimInfo(s0.str().c_str(),difState_,this);


#ifdef DATA_HANDLER_ENABLE
  s0.str(std::string());
  s0<<thePrefix_<<"/DIF"<<id<<"/DATA";
  theDIFData_ = new DimInfo(s0.str().c_str(),&difData_,32*1024*sizeof(uint32_t),this);
  cout<<" DIMDIF DATAHANDLER Infos registerd\n";
#endif
}     
DimDIFDataHandler::~DimDIFDataHandler()
{
  delete theDIFInfo_;
  delete theDIFData_;
}

void DimDIFDataHandler::infoHandler()
{
   DimInfo *curr = (DimInfo*) getInfo(); // get current DimStampedInfo address
   // std::cout<<curr->getName()<<" DimDIFDataHandler received "<< curr->getSize()<<std::endl;
   if (curr==theDIFInfo_)
     {
       // cout<<"copying to difStatus\n";
       memcpy(&difStatus_,curr->getData(),sizeof(DIFStatus));
       // cout<<"copy done\n";
       return;
     }
   if (curr==theDIFState_)
     {
       //cout<<"copying to difState"<<curr->getString()<<endl;
       memcpy(&difState_,curr->getString(),curr->getSize());
       // cout<<"copy done\n";
       return;
     }
#ifdef DATA_HANDLER_ENABLE

   if (curr==theDIFData_)
     {
       memcpy(&difData_,curr->getData(),curr->getSize());
       // copy to Shm
       uint8_t* cdata=(uint8_t*)  curr->getData();
       ShmProxy::transferToFile(cdata,
				curr->getSize(),
				ShmProxy::getBufferABCID(cdata),
				ShmProxy::getBufferDTC(cdata),
				ShmProxy::getBufferGTC(cdata),
				ShmProxy::getBufferDIF(cdata));

      if (ShmProxy::getBufferDTC(cdata)%10 == 0)
	printf("%s receieve %d  bytes, BCID %lld DTC %d GTC %d DIF %d \n",__PRETTY_FUNCTION__,
	       curr->getSize(),
	       ShmProxy::getBufferABCID(cdata),
	       ShmProxy::getBufferDTC(cdata),
	       ShmProxy::getBufferGTC(cdata),
	       ShmProxy::getBufferDIF(cdata));
     }
#endif
}
