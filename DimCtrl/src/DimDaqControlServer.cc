
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
  registerstateCommand_=new DimCommand(s0.str().c_str(),"I:1;C",this);
  s0.str(std::string());
  s0<<"/DDC/"<<hname<<"/START";
  startCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DDC/"<<hname<<"/STOP";
  stopCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DDC/"<<hname<<"/DESTROY";
  destroyCommand_=new DimCommand(s0.str().c_str(),"I:1",this);

  s0.str(std::string());
  s0<<"/DDC/"<<hname<<"/ON";
  onCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DDC/"<<hname<<"/OFF";
  offCommand_=new DimCommand(s0.str().c_str(),"I:1",this);


  s0.str(std::string());
  s0<<"/DDC/"<<hname<<"/PRINT";
  printCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  m_Thread_s = boost::thread(&DimDaqControlServer::services, this); 
  
}

void DimDaqControlServer::services()
{
  while (true)
    {
      usleep(10000);

      while (!todo_.empty())
	{
	  std::string s = todo_.back();
	  //std::cout<<" String to process "<<s<<std::endl;

	  dim_wait();
	  // std::cout<<" Dim Ok to process "<<s<<std::endl;
	  
	  if (s.compare("browse")==0)
	    {
	      //std::cout<<"On appelle BROWSE \n";
	      theControl_->scandns();
	      processStatus_=DimDaqControlServer::BROWSED;
	      aliveService_->updateService();
	      todo_.pop_back();continue; ;

	    }

	  if (s.compare("scan")==0)
	    {
	      //std::cout<<"On appelle SCAN \n";
	      theControl_->scan();
	      processStatus_=DimDaqControlServer::SCANNED;
	      aliveService_->updateService();
	      todo_.pop_back();continue; ;

	    }
	  if (s.compare("initialise")==0)
	    {
	      theControl_->initialise();
	      processStatus_=DimDaqControlServer::INITIALISED;
	      aliveService_->updateService();
	      todo_.pop_back();continue; ;

	    }

	  if (s.compare("registerstate")==0)
	    {
	      
	      theControl_->registerstate(ctrlreg_,state_);
	      processStatus_=DimDaqControlServer::DBREGISTERED;
	      aliveService_->updateService();
	      todo_.pop_back();continue; ;
	      
	    }
	  if (s.compare("configure")==0)
	    {
	      theControl_->configure();
	      processStatus_=DimDaqControlServer::CONFIGURED;
	      aliveService_->updateService();
	      todo_.pop_back();continue; ;

	    }
	  if (s.compare("start")==0)
	    {
	      theControl_->start();
	      processStatus_=DimDaqControlServer::STARTED;
	      aliveService_->updateService();
	      todo_.pop_back();continue; ;

	    }
	  if (s.compare("stop")==0)
	    {
	      theControl_->stop();
	      processStatus_=DimDaqControlServer::STOPPED;
	      aliveService_->updateService();
	      todo_.pop_back();continue; ;

	    }
	  if (s.compare("destroy")==0)
	    {
	      theControl_->destroy();
	      processStatus_=DimDaqControlServer::DESTROYED;
	      aliveService_->updateService();
	      todo_.pop_back();continue; ;
	      
	    }
	  if (s.compare("print")==0)
	    {
	      theControl_->print();
	      todo_.pop_back();continue; ;
	      
	    }
	  if (s.compare("on")==0)
	    {
	      theControl_->on();
	      todo_.pop_back();continue; ;
	      
	    }
	  if (s.compare("off")==0)
	    {
	      theControl_->off();
	      todo_.pop_back();continue; ;
	      
	    }

	  todo_.pop_back();
	}

      
    }
}
void DimDaqControlServer::commandHandler()
{
  DimCommand *currCmd = getCommand();
  printf(" J'ai recu %s COMMAND  \n",currCmd->getName());
  if (currCmd==browseCommand_)
    {
      todo_.push_back("browse");
      return ;

    }

  if (currCmd==scanCommand_)
    {
      todo_.push_back("scan");
      return ;

    }
  if (currCmd==initialiseCommand_)
    {
      todo_.push_back("initialise");
      return ;

    }

  if (currCmd==registerstateCommand_)
    {
      char* data= (char*) currCmd->getData();
      memcpy(&ctrlreg_,data,sizeof(uint32_t));
      char* cdata= &data[4];
      state_.assign(cdata);
      //std::cout<<"SERVING SETTING DB "<<state_<<" "<<ctrlreg_<<std::endl;
  
      todo_.push_back("registerstate");
      return ;

    }
  if (currCmd==configureCommand_)
    {
      todo_.push_back("configure");
      return ;

    }
  if (currCmd==startCommand_)
    {
      todo_.push_back("start");
      return ;

    }
  if (currCmd==stopCommand_)
    {
      todo_.push_back("stop");
      return ;

    }
  if (currCmd==destroyCommand_)
    {
      todo_.push_back("destroy");
      return ;

    }
  if (currCmd==printCommand_)
    {
      todo_.push_back("print");
      return ;

    }
  if (currCmd==onCommand_)
    {
      todo_.push_back("on");
      return ;

    }
  if (currCmd==offCommand_)
    {
      todo_.push_back("off");
      return ;

    }
  cout<<"Unknown command \n";
    
  return ;
}
   
