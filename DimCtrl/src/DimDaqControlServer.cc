
#include "DimDaqControlServer.h"
#include "DimDaqControl.h"

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
#include "ShmProxy.h"
#include <string.h>

DimDaqControlServer::DimDaqControlServer(DimDaqControl* c) :theControl_(c)  
{
  

  cout<<"Building DimDaqCtrl"<<endl;
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0<<"/DDC/"<<hname<<"/STATUS";
  aliveService_ = new DimService(s0.str().c_str(),processStatus_);
  s0.str(std::string());
  processStatus_=DimDaqControlServer::ALIVED;
  aliveService_->updateService();
  allocateCommands();
  s0.str(std::string());
  s0<<"DimDaqControl-"<<hname;

   DimServer::start(s0.str().c_str()); 
  //  cout<<"Starting DimDaqCtrl"<<endl;
  

}
DimDaqControlServer::~DimDaqControlServer()
{
  delete aliveService_;
  delete browseCommand_;
  delete scanCommand_;
  delete initialiseCommand_;
  delete registerstateCommand_;
  delete configureCommand_;
  delete startCommand_;
  delete stopCommand_;
  delete destroyCommand_;
}
void DimDaqControlServer::allocateCommands()
{
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0<<"/DDC/"<<hname<<"/BROWSEDNS";
  browseCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DDC/"<<hname<<"/SCANDEVICES";
  scanCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DDC/"<<hname<<"/INITIALISE";
  initialiseCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DDC/"<<hname<<"/CONFIGURE";
  configureCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DDC/"<<hname<<"/REGISTERSTATE";
  registerstateCommand_=new DimCommand(s0.str().c_str(),"I:1,C",this);
  s0.str(std::string());
  s0<<"/DDC/"<<hname<<"/START";
  startCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DDC/"<<hname<<"/STOP";
  stopCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DDC/"<<hname<<"/DESTROY";
  destroyCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  
}


void DimDaqControlServer::commandHandler()
{
  DimCommand *currCmd = getCommand();
  printf(" J'ai recu %s COMMAND  \n",currCmd->getName());
  if (currCmd==browseCommand_)
    {
      theControl_->scandns();
      processStatus_=DimDaqControlServer::BROWSED;
      aliveService_->updateService();
      return ;

    }

  if (currCmd==scanCommand_)
    {
      theControl_->scan();
      processStatus_=DimDaqControlServer::SCANNED;
      aliveService_->updateService();
      return ;

    }
  if (currCmd==initialiseCommand_)
    {
      theControl_->initialise();
      processStatus_=DimDaqControlServer::INITIALISED;
      aliveService_->updateService();
      return ;

    }

  if (currCmd==registerstateCommand_)
    {
      uint32_t ctr;
      std::string sta;
      char* data= (char*) currCmd->getData();
      memcpy(&ctr,data,sizeof(uint32_t));
      char* cdata= &data[4];
      sta.assign(cdata);
      theControl_->registerstate(ctr,sta);
      processStatus_=DimDaqControlServer::DBREGISTERED;
      aliveService_->updateService();
      return ;

    }
  if (currCmd==configureCommand_)
    {
      theControl_->configure();
      processStatus_=DimDaqControlServer::CONFIGURED;
      aliveService_->updateService();
      return ;

    }
  if (currCmd==startCommand_)
    {
      theControl_->start();
      processStatus_=DimDaqControlServer::STARTED;
      aliveService_->updateService();
      return ;

    }
  if (currCmd==stopCommand_)
    {
      theControl_->stop();
      processStatus_=DimDaqControlServer::STOPPED;
      aliveService_->updateService();
      return ;

    }
  if (currCmd==destroyCommand_)
    {
      theControl_->destroy();
      processStatus_=DimDaqControlServer::DESTROYED;
      aliveService_->updateService();
      return ;

    }
  cout<<"Unknown command \n";
    
  return ;
}
   
