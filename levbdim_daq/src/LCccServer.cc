
#include "LCccServer.hh"

#include "fileTailer.hh"

void LCccServer::open(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  
  std::string device;
  if (m->content().isMember("device"))
    { 
      device=m->content()["device"].asString();
      this->parameters()["device"]=m->content()["device"];
    }
  else
    device=this->parameters()["device"].asString();
  this->Open(device);
}

void LCccServer::initialise(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());

  CCCManager* ccc= this->getManager();
  if (ccc==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"Please open CCC DCCCCC01 first");
      return;
    }
  this->getManager()->initialise();
}
void LCccServer::configure(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());

  CCCManager* ccc= this->getManager();
  if (ccc==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"Please open CCC DCCCCC01 first");
      return;
    }
  this->getManager()->configure();
}
void LCccServer::start(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());

  CCCManager* ccc= this->getManager();
  if (ccc==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"Please open CCC DCCCCC01 first");
      return;
    }
  this->getManager()->start();
}
void LCccServer::stop(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());

  CCCManager* ccc= this->getManager();
  if (ccc==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"Please open CCC DCCCCC01 first");
      return;
    }
  this->getManager()->stop();
}

void LCccServer::cmd(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  std::string cmd_name=m->content()["name"].asString();

  
  CCCManager* ccc= this->getManager();
  if (ccc==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"Please open CCC DCCCCC01 first");
      return;
    }
  if (cmd_name.compare("PAUSE")==0)
    {
      LOG4CXX_INFO(_logLdaq," execute: "<<cmd_name);
      ccc->getCCCReadout()->DoSendPauseTrigger();
      return;
    }
  if (cmd_name.compare("RESUME")==0)
    {
      LOG4CXX_INFO(_logLdaq," execute: "<<cmd_name);
      ccc->getCCCReadout()->DoSendResumeTrigger();
      return;
    }
  if (cmd_name.compare("DIFRESET")==0)
    {
      LOG4CXX_INFO(_logLdaq," execute: "<<cmd_name);
      ccc->getCCCReadout()->DoSendDIFReset();
      return;
    }
  if (cmd_name.compare("CCCRESET")==0)
    {
      LOG4CXX_INFO(_logLdaq," execute: "<<cmd_name);
      ccc->getCCCReadout()->DoSendCCCReset();
      return;
    }
  if (cmd_name.compare("WRITEREG")==0)
    {
      LOG4CXX_INFO(_logLdaq," execute: "<<cmd_name);
      uint32_t adr=m->content()["address"].asInt();
      uint32_t val=m->content()["value"].asInt();
      ccc->getCCCReadout()->DoWriteRegister(adr,val);
      return;
    }
  if (cmd_name.compare("READREG")==0)
    {
      LOG4CXX_INFO(_logLdaq," execute: "<<cmd_name);
      uint32_t adr=m->content()["address"].asInt();

      uint32_t val=ccc->getCCCReadout()->DoReadRegister(adr);
      Json::Value r;
      r["address"]=adr;
      r["value"]=val;
      m->setAnswer(r);
      return;
    }

}

void LCccServer::pause(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  CCCManager* ccc= this->getManager();
  if (ccc==NULL)    {response["STATUS"]="NO Ccc created"; return;}
  ccc->getCCCReadout()->DoSendPauseTrigger();
  response["STATUS"]="DONE";
}
void LCccServer::resume(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  CCCManager* ccc= this->getManager();
  if (ccc==NULL)    {response["STATUS"]="NO Ccc created"; return;}
  ccc->getCCCReadout()->DoSendResumeTrigger();
  response["STATUS"]="DONE";
}
void LCccServer::difreset(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  CCCManager* ccc= this->getManager();
  if (ccc==NULL)    {response["STATUS"]="NO Ccc created"; return;}
  ccc->getCCCReadout()->DoSendDIFReset();
  response["STATUS"]="DONE";
}
void LCccServer::cccreset(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  CCCManager* ccc= this->getManager();
  if (ccc==NULL)    {response["STATUS"]="NO Ccc created"; return;}
  ccc->getCCCReadout()->DoSendCCCReset();
  response["STATUS"]="DONE";
}
 
void LCccServer::readreg(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  CCCManager* ccc= this->getManager();
  if (ccc==NULL)    {response["STATUS"]="NO Ccc created"; return;}
  uint32_t adr=atol(request.get("address","2").c_str());
  uint32_t val=ccc->getCCCReadout()->DoReadRegister(adr);

  response["STATUS"]="DONE";
  response["ADDRESS"]=adr;
  response["VALUE"]=val;
} 
void LCccServer::writereg(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  CCCManager* ccc= this->getManager();
  if (ccc==NULL)    {response["STATUS"]="NO Ccc created"; return;}
  uint32_t adr=atol(request.get("address","2").c_str());
  uint32_t value=atol(request.get("value","1234").c_str());
  ccc->getCCCReadout()->DoWriteRegister(adr,value);

  response["STATUS"]="DONE";
  response["ADDRESS"]=adr;
  response["VALUE"]=value;
} 



void LCccServer::joblog(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t nlines=atol(request.get("lines","100").c_str());
  uint32_t pid=getpid();
  std::stringstream s;
  s<<"/tmp/dimjcPID"<<pid<<".log";
  std::stringstream so;
  fileTailer t(1024*512);
  char buf[1024*512];
  t.tail(s.str(),nlines,buf);
  so<<buf;
  response["STATUS"]="DONE";
  response["FILE"]=s.str();
  response["LINES"]=so.str();
}


 LCccServer::LCccServer(std::string name) : levbdim::baseApplication(name),_manager(NULL)
{
  //  _fsm=new levbdim::fsm(name);

  _fsm=this->fsm();

  
// Register state
  _fsm->addState("CONFIGURED");
  _fsm->addState("ON");
  _fsm->addState("OFF");

  _fsm->addTransition("OPEN","CREATED","OPENED",boost::bind(&LCccServer::open, this,_1));
  _fsm->addTransition("INITIALISE","OPENED","INITIALISED",boost::bind(&LCccServer::initialise, this,_1));
  _fsm->addTransition("CONFIGURE","INITIALISED","CONFIGURED",boost::bind(&LCccServer::configure, this,_1));
  _fsm->addTransition("START","CONFIGURED","RUNNING",boost::bind(&LCccServer::start, this,_1));
  _fsm->addTransition("STOP","RUNNING","CONFIGURED",boost::bind(&LCccServer::stop, this,_1));
  _fsm->addTransition("STOP","CONFIGURED","CONFIGURED",boost::bind(&LCccServer::stop, this,_1));
  _fsm->addTransition("DESTROY","CONFIGURED","OPENED",boost::bind(&LCccServer::open, this,_1));
  
  // _fsm->addTransition("CMD","OPENED","OPENED",boost::bind(&LCccServer::cmd, this,_1));
  _fsm->addTransition("CMD","INITIALISED","INITIALISED",boost::bind(&LCccServer::cmd, this,_1));
  _fsm->addTransition("CMD","CONFIGURED","CONFIGURED",boost::bind(&LCccServer::cmd, this,_1));


 _fsm->addCommand("JOBLOG",boost::bind(&LCccServer::joblog,this,_1,_2));
 _fsm->addCommand("PAUSE",boost::bind(&LCccServer::pause,this,_1,_2));
 _fsm->addCommand("RESUME",boost::bind(&LCccServer::resume,this,_1,_2));
 _fsm->addCommand("DIFRESET",boost::bind(&LCccServer::difreset,this,_1,_2));
 _fsm->addCommand("CCCRESET",boost::bind(&LCccServer::cccreset,this,_1,_2));
 _fsm->addCommand("WRITEREG",boost::bind(&LCccServer::writereg,this,_1,_2));
 _fsm->addCommand("READREG",boost::bind(&LCccServer::readreg,this,_1,_2));

  
  std::stringstream s0;
  s0.str(std::string());
  s0<<"LCccServer-"<<name;
  DimServer::start(s0.str().c_str()); 

  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      std::cout<<"Service "<<name<<" started on port "<<atoi(wp)<<std::endl;
    _fsm->start(atoi(wp));
    }

	

}


void LCccServer::Open(std::string s)
{
  if (_manager!=NULL)
    delete _manager;
  _manager= new CCCManager(s);
}
