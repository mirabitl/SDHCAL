#include "LCIOWritterInterface.h"

#include "RpcShmServer.h"
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
RpcShmInitialise::RpcShmInitialise(RpcShmServer* r,std::string name) : DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}

void RpcShmInitialise::rpcHandler()
{
  LOG4CXX_INFO(_logWriter," CMD: INITIALISE called");


  _server->initialise();
  _server->publishState("DOWNLOAD");
  int32_t rc=0;
  setData(rc);
}
RpcShmStart::RpcShmStart(RpcShmServer* r,std::string name) : DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}

void RpcShmStart::rpcHandler()
{
  LOG4CXX_INFO(_logWriter," CMD: START called "<<getInt());
  int32_t rc=0;
  int nb=getInt();
  ShmProxy* sp=_server->getProxy();
  if (sp != NULL)
    {

      sp->setSetupName(_server->getDbState());
      sp->setNumberOfDIF(nb);
      cout<<" Number of DIF "<<sp->getNumberOfDIF()<<endl;
      sp->purgeShm(); // remove old data not written
      sp->Start(_server->getDbRun(),"/tmp");


      _server->publishState("RUNNING");
      _server->publishRun(_server->getDbRun());
    }
  else
    {
      LOG4CXX_ERROR(_logWriter," NO Proxy found ");
      _server->publishState("START_FAILED");
      rc=-1;setData(rc);
      return;
    }

  setData(rc);
}

RpcShmStop::RpcShmStop(RpcShmServer* r,std::string name) : DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}

void RpcShmStop::rpcHandler()
{
  LOG4CXX_INFO(_logWriter," CMD: STOP called "<<getInt());
  int32_t rc=0;
  ShmProxy* sp=_server->getProxy();
  if (sp != NULL)
    {
      sp->Stop();
      _server->publishState("STOPPED");
    }
  else
    {
      LOG4CXX_ERROR(_logWriter," NO Proxy found ");
      _server->publishState("STOP_FAILED");
      rc=-1;setData(rc);
      return;
    }

  setData(rc);
}

RpcShmDestroy::RpcShmDestroy(RpcShmServer* r,std::string name) : DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}

void RpcShmDestroy::rpcHandler()
{
  LOG4CXX_INFO(_logWriter," CMD: DESTROY called "<<getInt());
  int32_t rc=0;
  _server->destroy();
  LOG4CXX_FATAL(_logWriter," CMD: DESTROY is not working please recreate the process if needed (2bdbg) ");
  setData(rc);
}

RpcShmSetup::RpcShmSetup(RpcShmServer* r,std::string name) : DimRpc(name.c_str(),"C","I:1"),_server(r) {}

void RpcShmSetup::rpcHandler()
{
  LOG4CXX_INFO(_logWriter," CMD: SETUP called "<<getString());
  int32_t rc=0;
  _server->destroy();
  std::string s;
  s.assign(getString());
  ShmProxy* sp=_server->getProxy();
  if (sp != NULL)
    {
      sp->setSetupName(s);
    }
  else
    {
      LOG4CXX_ERROR(_logWriter," NO Proxy found ");
      rc=-1;setData(rc);
      return;
    }

  setData(rc);
}

RpcShmDirectory::RpcShmDirectory(RpcShmServer* r,std::string name) : DimRpc(name.c_str(),"C","I:1"),_server(r) {}

void RpcShmDirectory::rpcHandler()
{
  LOG4CXX_INFO(_logWriter," CMD: DIRECTORY called "<<getString());
  int32_t rc=0;
  _server->destroy();
  std::string s;
  s.assign(getString());
  ShmProxy* sp=_server->getProxy();
  if (sp != NULL)
    {
      sp->setDirectoryName(s);
    }
  else
    {
      LOG4CXX_ERROR(_logWriter," NO Proxy found ");
      rc=-1;setData(rc);
      return;
    }

  setData(rc);
}




RpcShmServer::RpcShmServer() :theProxy_(NULL)  
{
  

  cout<<"Building RpcShmServer"<<endl;
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  _state="CREATED";
  s0<<"/DSP/"<<hname<<"/STATE";
  _shmState = new DimService(s0.str().c_str(),(char*) _state.c_str());
  _shmState->updateService();
  s0.str(std::string());
  _run=0;
  s0<<"/DSP/"<<hname<<"/RUN";
  _runService = new DimService(s0.str().c_str(),_run);
  _runService->updateService();
  _event=0;

  s0.str(std::string());
  s0<<"/DSP/"<<hname<<"/EVENT";
  _eventService = new DimService(s0.str().c_str(),_event);
  _eventService->updateService();
  allocateCommands();
  s0.str(std::string());
  s0<<"RpcShmServer-"<<hname;

  DimServer::start(s0.str().c_str()); 
  //  cout<<"Starting DimDaqCtrl"<<endl;
  memset(_difData,0,255*sizeof(DimInfo*));
  theThread_ = boost::thread(&RpcShmServer::svc, this);

}
RpcShmServer::~RpcShmServer()
{
  delete _shmState;
  delete _initialiseCommand;
  delete _startCommand;
  delete _stopCommand;
  delete _directoryCommand;
  delete _setupCommand;
  delete _destroyCommand;
  for (uint32_t i=1;i<255;i++)
    if (_difData[i]!=NULL)			
      {
	delete _difData[i];
      }

}
void RpcShmServer::clearInfo()
{
   for (int i=0;i<255;i++)
    {
      if (_difData[i]!=NULL)			
	{
	  delete _difData[i];
	}
    }
   delete _runInfo;
   delete _dbstateInfo;
   
}
void  RpcShmServer::registerDifs()
{
  for (int i=0;i<255;i++)
    {
      if (_difData[i]!=NULL)			
	{
	  delete _difData[i];
	}

      std::stringstream s0;
      s0.str(std::string());
      s0<<"/DDS/DIF"<<i<<"/DATA";
      memset(_buffer,0,32*1024*sizeof(uint32_t));
      _difData[i]=new DimInfo(s0.str().c_str(),_buffer,32*1024*sizeof(uint32_t),this);

      
    }
  _dbState="NONE";
  _runInfo=new DimInfo("/DB/RUNFROMDB",_dbRun,this);
  _dbstateInfo=new DimInfo("/DB/DBSTATE",(char*) _dbState.c_str(),this);
   
}
void RpcShmServer::infoHandler()
{
   DimInfo *curr = getInfo(); // get current DimInfo address 
   LOG4CXX_DEBUG(_logWriter," DimInfo "<<curr->getName()<<" Size " <<curr->getSize());
   if (curr->getSize()==1) return;
   if (curr==_runInfo)
     {
       _dbRun=curr->getInt();
       std::cout<<"The current Run is "<<_dbRun<<std::endl;
       LOG4CXX_INFO(_logWriter,"The current Run is "<<_dbRun);
       return;
     }
   if (curr==_dbstateInfo)
     {

       _dbState.assign(curr->getString());
       std::cout<<"The current DbState is "<<_dbState<<std::endl;
       LOG4CXX_INFO(_logWriter,"The current DbState is "<<_dbState);
       return;
     }
   for (int i=0;i<255;i++)
      {

	if (curr==_difData[i])
	  {
	    memcpy(_buffer,curr->getData(),curr->getSize());
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





void RpcShmServer::allocateCommands()
{
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0.str(std::string());
  s0<<"/DSP/"<<hname<<"/INITIALISE";  
  _initialiseCommand=new RpcShmInitialise(this,s0.str());
  s0.str(std::string());
  s0<<"/DSP/"<<hname<<"/START";
  _startCommand=new RpcShmStart(this,s0.str());
  s0.str(std::string());
  s0<<"/DSP/"<<hname<<"/STOP";
  _stopCommand=new RpcShmStop(this,s0.str());
  s0.str(std::string());
  s0<<"/DSP/"<<hname<<"/DESTROY";
  _destroyCommand=new RpcShmDestroy(this,s0.str());
  s0.str(std::string());
  s0<<"/DSP/"<<hname<<"/SETUP";
  _setupCommand=new RpcShmSetup(this,s0.str());
  s0.str(std::string());
  s0<<"/DSP/"<<hname<<"/DIRECTORY";
  _directoryCommand=new RpcShmDirectory(this,s0.str());
  
}

void RpcShmServer::initialise()
{
  if (theProxy_ == NULL)
	{
	  LCIOWritterInterface* lc= new LCIOWritterInterface();
	  theProxy_=new ShmProxy(15,true,lc);
	  theProxy_->Initialise();
	  theProxy_->Configure();
	  this->registerDifs();
	  this->publishState("INITIALISED");
	}
}

void RpcShmServer::destroy()
{
  return; //A DEBUGGER
  if (theProxy_ != NULL)
    {
      delete theProxy_;
      theProxy_=NULL;
      clearInfo();
    }
}



void RpcShmServer::svc()
{
  while (1)
    {
      sleep((unsigned int) 1);
      if (theProxy_!=NULL && _state.compare("RUNNING")==0 && _eventService!=NULL)
	{
	  _event=theProxy_->getEventNumber();
	  //printf("Event is %x %d \n",theProxy_,_event);
	  _eventService->updateService();
	}
    }
}   
