
#include "LCccServer.hh"



void LCccServer::open(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  std::string device=m->content()["device"].asString();
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


 
 




 LCccServer::LCccServer(std::string name) : _manager(NULL)
{
  _fsm=new levbdim::fsm(name);

  
// Register state
  _fsm->addState("CREATED");
  _fsm->addState("CONFIGURED");
  _fsm->addState("ON");
  _fsm->addState("OFF");

  _fsm->addTransition("OPEN","CREATED","OPENED",boost::bind(&LCccServer::open, this,_1));
  _fsm->addTransition("INITIALISE","OPENED","INITIALISED",boost::bind(&LCccServer::initialise, this,_1));
  _fsm->addTransition("CONFIGURE","INITIALISED","CONFIGURED",boost::bind(&LCccServer::configure, this,_1));
  _fsm->addTransition("START","CONFIGURED","RUNNING",boost::bind(&LCccServer::start, this,_1));
  _fsm->addTransition("STOP","RUNNING","CONFIGURED",boost::bind(&LCccServer::stop, this,_1));
  _fsm->addTransition("DESTROY","CONFIGURED","OPENED",boost::bind(&LCccServer::open, this,_1));
  
  // _fsm->addTransition("CMD","OPENED","OPENED",boost::bind(&LCccServer::cmd, this,_1));
  _fsm->addTransition("CMD","INITIALISED","INITIALISED",boost::bind(&LCccServer::cmd, this,_1));
  _fsm->addTransition("CMD","CONFIGURED","CONFIGURED",boost::bind(&LCccServer::cmd, this,_1));

  
  std::stringstream s0;
  s0.str(std::string());
  s0<<"LCccServer-"<<name;
  DimServer::start(s0.str().c_str()); 


	

}


void LCccServer::Open(std::string s)
{
  if (_manager!=NULL)
    delete _manager;
  _manager= new CCCManager(s);
}
