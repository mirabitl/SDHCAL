#include "LCIOWritterInterface.h"

#include "DimJobControl.h"
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

#define _jobControl_NMAX 4096
#define _jobControl_MMAX 16
#define _jobControl_MAXENV 100

DimProcessData::DimProcessData(std::string json_string)
{
  Json::Reader reader;
  bool parsingSuccessful = reader.parse(json_string,_processInfo, parsedFromString);
  if (parsingSuccessful)
    {
      std::cout << styledWriter.write(parsedFromString) << std::endl;
    }
  _childPid=0;
  _status=notcreated;
}

void startProcess(DimProcessData* p)
{
  if (p->_status!=DimProcessStatus::notcreated) return;

  std::string sprog=p->_processInfo["Program"].asString();
  std::vector<std::string> sarg;
  for (uint32_t ia=0;ia<p->_processInfo["Arguments"].size();ia++)
    {
      sarg.append(p->_processInfo["Arguments"][ia].asString());
    }
 std::vector<std::string> senv=p->_processInfo["Environnement"].getMemberNames();
 std::map<std::string,std::string> menv;
  std::vector<std::string> venv;
 for (std::vector<std::string>::iterator it=senv.begin();it!=senv.end();it++)
   {
     std::stringstream ss;
     
     ss<<"export "<<(*it)<<"="<<p->_processInfo["Environnement"][(*it)];
     venv.push_back(ss.str());
     menv[(*it)]=p->_processInfo["Environnement"][(*it)];
   }
 // forking
 pid_t pid = fork();

 if (pid!=0) //Parent
   {
     p->_childPid=pid;
     p->_status=DimProcessStatus::running;
     return;
   }
 // client
    / 
  // we are in the child 
  //

   char executivePath[_jobControl_NMAX];
   char argv[_jobControl_MMAX][_jobControl_NMAX];  // build and initialize argv[][]
   char *pArgv[_jobControl_MMAX];
   char envp[_jobControl_MAXENV][_jobControl_NMAX];
   char* pEnvp[_jobControl_MAXENV];


   // Executive Path
   sprintf(executivePath,sprog.c_str());
 // fills arguments list

  for (int i = 0; i<_jobControl_MMAX ; i++) {                    
    for (int j = 0; j<_jobControl_NMAX ; j++) {
      argv[i][j] = (char)NULL;
    }
  }

  int i=1;
  for (std::vector<std::string>::iterator iter = sarg.begin(); iter!=sarg.end(); iter++) {
    sprintf( argv[i], "%s", (*iter).c_str());
    pArgv[i] = & argv[i][0];
    i++;
  }
  pArgv[0] = executivePath;
  pArgv[i] = NULL;


  
  // stop the watchdog
  //  watchdogEnabled_ = false;
  //  watchdogTimer_->stop();


  // brute force close 
  // xdaq only opens first 5.
  //
  close(0);
  for (int i = 3; i < 32; i++) {
    close(i);
  }
  
 // Fills environment list

  i=0;
  for (std::vector<std::string>::iterator iter = venv.begin(); iter!=venv.end(); iter++) {
    sprintf( envp[i], "%s", (*iter).c_str());
    // std::stringstream oss;
    // oss << "environment list : " << i << " : " << (*iter) << std::endl;
    // LOG4CPLUS_DEBUG(this->getApplicationLogger(), oss.str()); 
    pEnvp[i] = & envp[i][0];
    i++;
  }
  pEnvp[i] = NULL;

  // set new user id to root
  ret = setuid(0);
  if ( ret != 0 ) {
	//Let's try a second time
  	ret = setuid(0);
  	if ( ret != 0 ) {
	  DEBUG_PRINT("child: FATAL couldn't setuid() to %i.\n",execUid);
	}
  }
  
  
  // check for config file existence
  


  // open procID+log for stdout and stderr
  char logPath[100];
  
  pid_t mypid = getpid();                                          // get my pid to append to filename 	
  sprintf(logPath,"/tmp/dimjcPID%i.log",mypid);             // construct filename to /tmp/....
  try {
	  int tmpout = open( logPath , O_RDWR|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH );    // open file
	  dup2( tmpout, 1 );                                         // stdout to file
	  dup2( tmpout, 2 );                                         // stderr to file
	  close( tmpout );                                       // close unused descriptor
  }
  catch (std::exception &e) {
    DEBUG_PRINT("child: FATAL couldn't write log file to %s.\n", logPath);
    exit(-1);
  }
  catch (...)
    {
      DEBUG_PRINT("child: FATAL couldn't write log file to %s.\n", logPath);
      exit(-1);
    }


  int ret = execve( executivePath, pArgv, pEnvp); 	      

  DEBUG_PRINT("jobControl: FATAL OOps, we came back with ret = %i , dying",ret);
  exit(-1);
}
DimJobControl::DimJobControl() :theProxy_(NULL)  
{
  

  cout<<"Building DimJobControl"<<endl;
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0<<"/DSP/"<<hname<<"/STATUS";
  aliveService_ = new DimService(s0.str().c_str(),processStatus_);
  s0.str(std::string());
  processStatus_=DimJobControl::ALIVED;
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
  s0<<"DimJobControl-"<<hname;

  DimServer::start(s0.str().c_str()); 
  //  cout<<"Starting DimDaqCtrl"<<endl;
  memset(difState_,0,255*sizeof(DimInfo*));

}
DimJobControl::~DimJobControl()
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
void DimJobControl::clearInfo()
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
void  DimJobControl::registerDifs()
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
void DimJobControl::infoHandler()
{
   DimInfo *curr = getInfo(); // get current DimInfo address 
   if (curr->getSize()==1) return;
   if (curr==runInfo_)
     {
       theRun_=curr->getInt();
       std::cout<<"The current Run is "<<theRun_<<std::endl;
       return;
     }
   if (curr==dbstateInfo_)
     {

       dbState_.assign(curr->getString());
       std::cout<<"The current DbState is "<<dbState_<<std::endl;
       return;
     }
   for (int i=0;i<255;i++)
      {

	if (curr==difData_[i])
	  {
	    memcpy(&theBuffer_,curr->getData(),curr->getSize());
       // copy to Shm
	    uint8_t* cdata=(uint8_t*)  curr->getData();
	    JobControl::transferToFile(cdata,
				     curr->getSize(),
				     JobControl::getBufferABCID(cdata),
				     JobControl::getBufferDTC(cdata),
				     JobControl::getBufferGTC(cdata),
				     JobControl::getBufferDIF(cdata));

	    if (JobControl::getBufferDTC(cdata)%1000 == 0 &&JobControl::getBufferDTC(cdata)!=0 )
	      printf("%s DIF %d receieve %d  bytes, BCID %lld DTC %d GTC %d DIF %d \n",__PRETTY_FUNCTION__,i,
		     curr->getSize(),
		     JobControl::getBufferABCID(cdata),
		     JobControl::getBufferDTC(cdata),
		     JobControl::getBufferGTC(cdata),
		     JobControl::getBufferDIF(cdata));
	  }

      }
}





void DimJobControl::allocateCommands()
{
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0.str(std::string());
  s0<<"/DJC/"<<hname<<"/CLEAR";
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


void DimJobControl::commandHandler()
{
  DimCommand *currCmd = getCommand();
  printf(" J'ai recu %s COMMAND  \n",currCmd->getName());
  if (currCmd==initialiseCommand_)
    {
      if (theProxy_ == NULL)
	{
	  int nd=currCmd->getInt();
	  LCIOWritterInterface* lc= new LCIOWritterInterface();
	  theProxy_=new JobControl(15,true,lc);
	  theProxy_->Initialise();
	  theProxy_->Configure();
	  this->registerDifs();
	  processStatus_=DimJobControl::INITIALISED;
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

	  processStatus_=DimJobControl::STARTED;
	  aliveService_->updateService();
	  run_=theRun_;
	  runService_->updateService();
	  theThread_ = boost::thread(&DimJobControl::svc, this);
	}
      return ;

    }

  if (currCmd==stopCommand_)
    {
      if (theProxy_ != NULL)
	{
	  theProxy_->Stop();
	  processStatus_=DimJobControl::STOPPED;
	  aliveService_->updateService();
	  //theThread_.join();
	}
      return ;

    }
  cout<<"Unknown command"<<currCmd->getName()<<" \n";
    
  return ;
}
void DimJobControl::svc()
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
