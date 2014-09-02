#include "DimDDSClient.h"
#include "ShmProxy.h"
DimDDSClient::DimDDSClient(uint32_t id,std::string prefix) : theName_(name),thePrefix_(prefix),theShmPrefix_("/dev/shm")
{
  std::stringstream s0;
  s0.str(std::string());
  s0<<prefix<<"/STATUS";
  theDDSStatus_ = new DimInfo(s.str().c_str(),&theStatus_,sizeof(uint32_t),this);
  s0.str(std::string());
  s0<<prefix<<"/DEVICES";
  theDDSDevices_ = new DimInfo(s.str().c_str(),theDevices_,255*sizeof(uint32_t),this);
  theDDDHMap_.clear();
  bsem_.lock();

}     
~DimDDSClient::DimDDSClient()
{
  delete theDIFInfo_;
  delete theDIFData_;
  for (std::map<uint32_t,DimDIFDataHandler*>::iterator it=theDDDHMap_.begin();it!=theDDDHMap_.end();it++)
    {
      delete it->second; 
    }
  theDDDHMap_.clear();

}

void DimDDSClient::scanDevices()
{
  std::stringstream s0;
  s0.str(std::string());
  s0<<thePrefix_<<"/SCANDEVICES";
  DimClient::sendCommand(s.0.str().c_str(), 1); 
  bsem_.lock();
}

void DimDDSClient::initialise()
{
  for (uint32_t i=1;i<255;i++)
    {
      if (theDevices_[i]==i)
	{
	  std::stringstream s0;
	  s0.str(std::string());
	  s0<<thePrefix_<<"/INITIALISE";
	  DimClient::sendCommand(s.0.str().c_str(), i); 
	  bsem_.lock();
	  
	  DimDIFDataHandler* d= new  DimDIFDataHandler(i,thePrefix_);
	  std::pair<uint32_t,DimDIFDataHandler*> p(i,d);
	  theDDDHMap_.insert(p);
	}
    }
}
void DimDDSClient::setDBState(uint32_t ctrlreg,std::string state)
{
  theCtrlReg_=ctrlreg;
  theState_=state;
  std::stringstream s0;
  s0.str(std::string());
  s0<<thePrefix_<<"/REGISTERSTATE";
  DimClient::sendCommand(s.0.str().c_str(), state.c_str()); 
  bsem_.lock();	
}

void DimDDSClient::configure()
{
  
  std::stringstream s0;
  s0.str(std::string());
  s0<<thePrefix_<<"/PRECONFIGURE";
  DimClient::sendCommand(s.0.str().c_str(), theCtrlreg); 
  bsem_.lock();	
}

void DimDDSClient::start()
{
  std::stringstream s0;
  s0.str(std::string());
  s0<<thePrefix_<<"/START";
  DimClient::sendCommand(s.0.str().c_str(), 1); 
  bsem_.lock();
}

void DimDDSClient::stop()
{
  std::stringstream s0;
  s0.str(std::string());
  s0<<thePrefix_<<"/STOP";
  DimClient::sendCommand(s.0.str().c_str(), 1); 
  bsem_.lock();
}
void DimDDSClient::destroy()
{
  std::stringstream s0;
  s0.str(std::string());
  s0<<thePrefix_<<"/DESTROY";
  DimClient::sendCommand(s.0.str().c_str(), 1); 
  bsem_.lock();
}


void DimDDSClient::infoHandler()
{
  DimInfo *curr = (DimInfo*) getInfo(); // get current DimStampedInfo address
  std::cout<<curr->getName()<<std::endl;
  if (curr=theDDSStatus_)
    {
      theStatus_=curr->getInt();
      bsem_.unlock();
    }
  if (curr=theDDSDevices_)
    {
      memcpy(&theDevices_,curr->getData(),curr->getSize());
      // copy to Shm
    }
     
     
}
