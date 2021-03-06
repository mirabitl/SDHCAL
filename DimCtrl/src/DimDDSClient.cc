#include "DimDDSClient.h"
#include "ShmProxy.h"
using namespace std;
DimDDSClient::DimDDSClient(std::string name,std::string prefix) : theName_(name),thePrefix_(prefix)
{
  std::stringstream s0;
  s0.str(std::string());
  s0<<prefix<<"/STATUS";
  theDDSStatus_ = new DimInfo(s0.str().c_str(),&theStatus_,sizeof(uint32_t),this);
  s0.str(std::string());
  s0<<prefix<<"/DEVICES";
  theDDSDevices_ = new DimInfo(s0.str().c_str(),theDevices_,255*sizeof(uint32_t),this);
  theDDDHMap_.clear();
  bsem_.lock();

}     
DimDDSClient::~DimDDSClient()
{
  delete theDDSStatus_;
  delete theDDSDevices_;
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
  // cout<<"Oops\n";
  cout <<"Sending command "<<s0.str().c_str()<<" "<<DimClient::getDnsNode()<<endl;
  DimClient::sendCommand(s0.str().c_str(), 1);
  // cout <<"Send done \n";
  bsem_.lock();
}

bool DimDDSClient::checkState(std::string str)
{
  bool valid=true;
  for (std::map<uint32_t,DimDIFDataHandler*>::iterator it=theDDDHMap_.begin();it!=theDDDHMap_.end();it++)
    {
      std::string state=it->second->getState();
      // different member versions of find in the same order as above:
      std::size_t found = state.find(str);
      valid =valid && (found!=std::string::npos);
    }
  return valid;
}

void DimDDSClient::waitState(std::string str,uint32_t max_wait)
{
  uint32_t nw=0;
  while (!checkState(str) && nw<max_wait);
    {

      printf("%s uncompleted waiting %d s \n",str.c_str(),nw);
      sleep((unsigned int) 1);
      nw++;
    }
    this->print();
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
	  // cout <<"sending "<<s0.str().c_str()<<" "<<i<<endl;
	  DimClient::sendCommand(s0.str().c_str(),(int) i); 
	  bsem_.lock();
	  DimDIFDataHandler* d= new  DimDIFDataHandler(i,thePrefix_);
	  std::pair<uint32_t,DimDIFDataHandler*> p(i,d);
	  theDDDHMap_.insert(p);
	}
    }
  
  waitState("INIT",15);
 
}



void DimDDSClient::print()
{
  printf("========================");
  printf(" %s ======================== \n",thePrefix_.c_str());
  for (std::map<uint32_t,DimDIFDataHandler*>::iterator it=theDDDHMap_.begin();it!=theDDDHMap_.end();it++)
    {
      //printf("\t DIF %d %x %s \n",it->first,it->second,it->second->getState());
      DIFStatus& s=it->second->getStatus();
      printf(" %d %d %x %d %lld %lld %s \n",s.id,s.status,s.slc,s.gtc,s.bcid,s.bytes,it->second->getState()); 
    }
}
void DimDDSClient::setDBState(uint32_t ctrlreg,std::string state)
{
  theCtrlReg_=ctrlreg;
  theState_=state;
  std::stringstream s0;
  s0.str(std::string());
  s0<<thePrefix_<<"/REGISTERSTATE";
  DimClient::sendCommand(s0.str().c_str(), state.c_str()); 
  bsem_.lock();	
  waitState("DB_REG",15);

  
}
void DimDDSClient::setGain(uint32_t gain)
{
  theCalibrationGain_=gain;

  std::stringstream s0;
  s0.str(std::string());
  s0<<thePrefix_<<"/SETGAIN";
  DimClient::sendCommand(s0.str().c_str(),theCalibrationGain_); 
  bsem_.lock();	

}

void DimDDSClient::setThresholds(uint32_t b0,uint32_t b1,uint32_t b2)
{
  theCalibrationThresholds_[0]=b0;
  theCalibrationThresholds_[1]=b1;
  theCalibrationThresholds_[2]=b2;

  std::stringstream s0;
  s0.str(std::string());
  s0<<thePrefix_<<"/SETTHRESHOLD";
  DimClient::sendCommand(s0.str().c_str(),theCalibrationThresholds_,3*sizeof(int32_t)); 
  bsem_.lock();	

}

void DimDDSClient::configure()
{
  
  std::stringstream s0;
  s0.str(std::string());
  s0<<thePrefix_<<"/PRECONFIGURE";
  DimClient::sendCommand(s0.str().c_str(),(int) theCtrlReg_); 
  bsem_.lock();	

  waitState("CONFIG",15);
}


void DimDDSClient::start()
{
  std::stringstream s0;
  s0.str(std::string());
  s0<<thePrefix_<<"/START";
  DimClient::sendCommand(s0.str().c_str(), 1); 
  bsem_.lock();
}

void DimDDSClient::stop()
{
  std::stringstream s0;
  s0.str(std::string());
  s0<<thePrefix_<<"/STOP";
  DimClient::sendCommand(s0.str().c_str(), 1); 
  bsem_.lock();
  waitState("STOP",15);
}
void DimDDSClient::destroy()
{
  std::stringstream s0;
  s0.str(std::string());
  s0<<thePrefix_<<"/DESTROY";
  DimClient::sendCommand(s0.str().c_str(), 1); 
  bsem_.lock();
}


void DimDDSClient::infoHandler()
{
  DimInfo *curr = (DimInfo*) getInfo(); // get current DimStampedInfo address
  //std::cout<<"GOT info "<<curr->getName()<<std::endl;
  LOG4CXX_DEBUG(_logCtrl," DimInfo "<<curr->getName()<<" Size " <<curr->getSize());
  if (curr==theDDSStatus_)
    {
      theStatus_=curr->getInt();
      // cout<<"DimDDSClient status updated "<<theStatus_<<endl;
      bsem_.unlock();
    }
  if (curr==theDDSDevices_)
    {
      memcpy(&theDevices_[0],curr->getData(),curr->getSize());
      // copy to Shm
    }
     
     
}
