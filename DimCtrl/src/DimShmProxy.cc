#include "LCIOWritterInterface.h"

#include "DimShmProxy.h"
#include <stdint.h>

#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>

#include <string.h>

DimShmProxy::DimShmProxy() :theProxy_(NULL)  
{
  

  cout<<"Building DimShmProxy"<<endl;
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0<<"/DSP/"<<hname<<"/STATUS";
  aliveService_ = new DimService(s0.str().c_str(),processStatus_);
  s0.str(std::string());
  processStatus_=DimShmProxy::ALIVED;
  aliveService_->updateService();
  s0.str(std::string());
  run_=0;
  s0<<"/DSP/"<<hname<<"/RUN";
  runService_ = new DimService(s0.str().c_str(),run_);
  runService_->updateService();
  event_=0;

  s0.str(std::string());
  s0<<"/DSP/"<<hname<<"/EVENT";
  eventService_ = new DimService(s0.str().c_str(),event_);
  eventService_->updateService();
  allocateCommands();
  s0.str(std::string());
  s0<<"DimShmProxy-"<<hname;

  DimServer::start(s0.str().c_str()); 
  //  cout<<"Starting DimDaqCtrl"<<endl;
  memset(difState_,0,255*sizeof(DimInfo*));

}
DimShmProxy::~DimShmProxy()
{
  delete aliveService_;
  delete initialiseCommand_;
  delete startCommand_;
  delete stopCommand_;
  delete directoryCommand_;
  delete setupCommand_;
  delete destroyCommand_;
  for (uint32_t i=1;i<255;i++)
    if (difState_[i]!=NULL)			
      {
	delete difState_[i];
	delete difInfo_[i];
	delete difData_[i];
      }

}
void DimShmProxy::clearInfo()
{
   for (int i=0;i<255;i++)
    {
      if (difState_[i]!=NULL)			
	{
	  delete difState_[i];
	  delete difInfo_[i];
	  delete difData_[i];
	}
    }
   delete runInfo_;
}
void  DimShmProxy::registerDifs()
{
  for (int i=0;i<255;i++)
    {
      if (difState_[i]!=NULL)			
	{
	  delete difState_[i];
	  delete difInfo_[i];
	  delete difData_[i];
	}

      std::stringstream s0;
      s0.str(std::string());
      s0<<"/DDS/DIF"<<i<<"/STATE";
      memset(theState_,0,255);
      difState_[i]=new DimInfo(s0.str().c_str(),theState_,this);
      s0.str(std::string());
      s0<<"/DDS/DIF"<<i<<"/INFO";
      memset(&theInfo_,0,sizeof(DIFStatus));
      difInfo_[i]=new DimInfo(s0.str().c_str(),(void*) &theInfo_,sizeof(DIFStatus),this);
      s0.str(std::string());
      s0<<"/DDS/DIF"<<i<<"/DATA";
      memset(theBuffer_,0,32*1024*sizeof(uint32_t));
      difData_[i]=new DimInfo(s0.str().c_str(),theBuffer_,32*1024*sizeof(uint32_t),this);

      
    }
  runInfo_=new DimInfo("/DB/RUNFROMDB",theRun_,this);
  dbstateInfo_=new DimInfo("/DB/DBSTATE",theState_,this);
   
}
void DimShmProxy::infoHandler()
{
   DimInfo *curr = getInfo(); // get current DimInfo address 
   LOG4CXX_DEBUG(_logWriter," DimInfo "<<curr->getName()<<" Size " <<curr->getSize());
   if (curr->getSize()==1) return;
   if (curr==runInfo_)
     {
       theRun_=curr->getInt();
       std::cout<<"The current Run is "<<theRun_<<std::endl;
       LOG4CXX_INFO(_logWriter,"The current Run is "<<theRun_);
       return;
     }
   if (curr==dbstateInfo_)
     {

       dbState_.assign(curr->getString());
       std::cout<<"The current DbState is "<<dbState_<<std::endl;
       LOG4CXX_INFO(_logWriter,"The current DbState is "<<dbState_);
       return;
     }
   for (int i=0;i<255;i++)
      {

	if (curr==difData_[i])
	  {
	    memcpy(&theBuffer_,curr->getData(),curr->getSize());
       // copy to Shm
	    uint8_t* cdata=(uint8_t*)  curr->getData();
	    ShmProxy::transferToFile(cdata,
				     curr->getSize(),
				     ShmProxy::getBufferABCID(cdata),
				     ShmProxy::getBufferDTC(cdata),
				     ShmProxy::getBufferGTC(cdata),
				     ShmProxy::getBufferDIF(cdata));

	    if (ShmProxy::getBufferDTC(cdata)%1000 == 0 &&ShmProxy::getBufferDTC(cdata)!=0 )
	      printf("%s DIF %d receieve %d  bytes, BCID %lld DTC %d GTC %d DIF %d \n",__PRETTY_FUNCTION__,i,
		     curr->getSize(),
		     ShmProxy::getBufferABCID(cdata),
		     ShmProxy::getBufferDTC(cdata),
		     ShmProxy::getBufferGTC(cdata),
		     ShmProxy::getBufferDIF(cdata));
	  }

      }
}





void DimShmProxy::allocateCommands()
{
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0.str(std::string());
  s0<<"/DSP/"<<hname<<"/INITIALISE";
  initialiseCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DSP/"<<hname<<"/START";
  startCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DSP/"<<hname<<"/STOP";
  stopCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DSP/"<<hname<<"/DESTROY";
  destroyCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DSP/"<<hname<<"/SETUP";
  setupCommand_=new DimCommand(s0.str().c_str(),"C",this);
  s0.str(std::string());
  s0<<"/DSP/"<<hname<<"/DIRECTORY";
  directoryCommand_=new DimCommand(s0.str().c_str(),"C",this);
  
}


void DimShmProxy::commandHandler()
{
  DimCommand *currCmd = getCommand();
  printf(" J'ai recu %s COMMAND  \n",currCmd->getName());
  LOG4CXX_INFO(_logWriter,"CMD : "<<currCmd->getName());
  if (currCmd==initialiseCommand_)
    {
      if (theProxy_ == NULL)
	{
	  int nd=currCmd->getInt();
	  LCIOWritterInterface* lc= new LCIOWritterInterface();
	  theProxy_=new ShmProxy(15,true,lc);
	  theProxy_->Initialise();
	  theProxy_->Configure();
	  this->registerDifs();
	  processStatus_=DimShmProxy::INITIALISED;
	  aliveService_->updateService();

	}
      return ;

    }

  if (currCmd==destroyCommand_)
    {
      if (theProxy_ != NULL)
	{
	  //delete theProxy_;
	  //this->clearInfo();

	}
    }

  if (currCmd==setupCommand_)
    {
       if (theProxy_ != NULL)
	{
	  std::string s;
	  s.assign(currCmd->getString());
	  theProxy_->setSetupName(s);
	}
       return;
    }

  if (currCmd==directoryCommand_)
    {
       if (theProxy_ != NULL)
	{
	  std::stringstream s;
	  s<<currCmd->getString();
	  std::cout<<" Directory is set to "<<currCmd->getString()<<std::endl;
	  theProxy_->setDirectoryName(s.str());
	}
       return;
    }
  if (currCmd==startCommand_)
    {
      int nb=currCmd->getInt();
      if (theProxy_ != NULL)
	{
	  cout<<" Changing Proxy setup to  "<<dbState_<<endl;
	  theProxy_->setSetupName(dbState_);
	  cout<<" Number of DIF "<<nb<<endl;
	  theProxy_->setNumberOfDIF(nb);
	  cout<<" Number of DIF "<<theProxy_->getNumberOfDIF()<<endl;
	  theProxy_->purgeShm(); // remove old data not written
	  theProxy_->Start(theRun_,"/tmp");

	  processStatus_=DimShmProxy::STARTED;
	  aliveService_->updateService();
	  run_=theRun_;
	  runService_->updateService();
	  theThread_ = boost::thread(&DimShmProxy::svc, this);
	}
      return ;

    }

  if (currCmd==stopCommand_)
    {
      if (theProxy_ != NULL)
	{
	  theProxy_->Stop();
	  processStatus_=DimShmProxy::STOPPED;
	  aliveService_->updateService();
	  //theThread_.join();
	}
      return ;

    }
  cout<<"Unknown command"<<currCmd->getName()<<" \n";
    
  return ;
}
void DimShmProxy::svc()
{
  while (1)
    {
      sleep((unsigned int) 1);
      if (theProxy_!=NULL)
	{
	  event_=theProxy_->getEventNumber();
	  //printf("Event is %x %d \n",theProxy_,event_);
	  eventService_->updateService();
	}
    }
}   
