
#include "LMdccServer.hh"
void LMdccServer::open(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  std::string device=m->content()["device"].asString();
  doOpen(device);
  //_mdcc->maskTrigger();
  //_mdcc->resetCounter();
}
void LMdccServer::close(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  if (_mdcc==NULL)
    {
       LOG4CXX_ERROR(_logLdaq,"Please open MDC01 first");
       return;
    }
  _mdcc->close();
  _mdcc=NULL;
}
void LMdccServer::pause(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  if (_mdcc==NULL)
    {
       LOG4CXX_ERROR(_logLdaq,"Please open MDC01 first");
       return;
    }
  _mdcc->maskTrigger();
}
void LMdccServer::resume(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  if (_mdcc==NULL)
    {
       LOG4CXX_ERROR(_logLdaq,"Please open MDC01 first");
       return;
    }
  _mdcc->unmaskTrigger();
}
void LMdccServer::ecalpause(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  if (_mdcc==NULL)
    {
       LOG4CXX_ERROR(_logLdaq,"Please open MDC01 first");
       return;
    }
  _mdcc->maskEcal();
}
void LMdccServer::ecalresume(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  if (_mdcc==NULL)
    {
       LOG4CXX_ERROR(_logLdaq,"Please open MDC01 first");
       return;
    }
  _mdcc->unmaskEcal();
}

void LMdccServer::reset(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  if (_mdcc==NULL)
    {
       LOG4CXX_ERROR(_logLdaq,"Please open MDC01 first");
       return;
    }
  _mdcc->resetCounter();
}


void LMdccServer::cmd(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  std::string cmd_name=m->content()["name"].asString();

  
  
  if (_mdcc==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"Please open MDC01 first");
      return;
    }
  if (cmd_name.compare("STATUS")==0)
    {
      LOG4CXX_INFO(_logLdaq," execute: "<<cmd_name);
      Json::Value rc;
      rc["version"]=_mdcc->version();
      rc["id"]=_mdcc->id();
      rc["mask"]=_mdcc->mask();
      rc["spill"]=_mdcc->spillCount();
      rc["busy1"]=_mdcc->busy1Count();
      rc["busy2"]=_mdcc->busy2Count();
      rc["busy3"]=_mdcc->busy3Count();
      rc["spillon"]=_mdcc->spillOn();
      rc["spilloff"]=_mdcc->spillOff();
      rc["ecalmask"]=_mdcc->ecalmask();
      m->setAnswer(rc);

      return;
    }
  if (cmd_name.compare("WRITEREG")==0)
    {
      LOG4CXX_INFO(_logLdaq," execute: "<<cmd_name);
      uint32_t adr=m->content()["address"].asInt();
      uint32_t val=m->content()["value"].asInt();
      _mdcc->writeRegister(adr,val);
      return;
    }
  if (cmd_name.compare("READREG")==0)
    {
      LOG4CXX_INFO(_logLdaq," execute: "<<cmd_name);
      uint32_t adr=m->content()["address"].asInt();

      uint32_t val=_mdcc->readRegister(adr);
      Json::Value r;
      r["address"]=adr;
      r["value"]=val;
      m->setAnswer(r);
      return;
    }
  if (cmd_name.compare("SPILLON")==0)
    {
      LOG4CXX_INFO(_logLdaq," execute: "<<cmd_name);
      uint32_t nc=m->content()["nclock"].asUInt();

      _mdcc->setSpillOn(nc);
      return;
    }
  if (cmd_name.compare("SPILLOFF")==0)
    {
      LOG4CXX_INFO(_logLdaq," execute: "<<cmd_name);
      uint32_t nc=m->content()["nclock"].asUInt();

      _mdcc->setSpillOff(nc);
      return;
    }

}




LMdccServer::LMdccServer(std::string name) : _mdcc(NULL)
{

  
 

  _fsm=new levbdim::fsm(name);

  
// Register state
  _fsm->addState("CREATED");
  _fsm->addState("PAUSED");
  _fsm->addState("RUNNING");

  _fsm->addTransition("OPEN","CREATED","PAUSED",boost::bind(&LMdccServer::open, this,_1));
  _fsm->addTransition("RESUME","PAUSED","RUNNING",boost::bind(&LMdccServer::resume, this,_1));
  _fsm->addTransition("RESET","PAUSED","PAUSED",boost::bind(&LMdccServer::reset, this,_1));
  _fsm->addTransition("PAUSE","RUNNING","PAUSED",boost::bind(&LMdccServer::pause, this,_1));
  _fsm->addTransition("PAUSE","PAUSED","PAUSED",boost::bind(&LMdccServer::pause, this,_1));
  _fsm->addTransition("DESTROY","PAUSED","CREATED",boost::bind(&LMdccServer::close, this,_1));
  
  _fsm->addTransition("CMD","PAUSED","PAUSED",boost::bind(&LMdccServer::cmd, this,_1));
  _fsm->addTransition("CMD","RUNNING","RUNNING",boost::bind(&LMdccServer::cmd, this,_1));
  _fsm->addTransition("ECALPAUSE","PAUSED","PAUSED",boost::bind(&LMdccServer::ecalpause, this,_1));
  _fsm->addTransition("ECALPAUSE","RUNNING","RUNNING",boost::bind(&LMdccServer::ecalpause, this,_1));
  _fsm->addTransition("ECALRESUME","PAUSED","PAUSED",boost::bind(&LMdccServer::ecalresume, this,_1));
  _fsm->addTransition("ECALRESUME","RUNNING","RUNNING",boost::bind(&LMdccServer::ecalresume, this,_1));

  std::stringstream s0;
  s0.str(std::string());
  s0<<"LMdccServer-"<<name;
  DimServer::start(s0.str().c_str()); 

}


void LMdccServer::doOpen(std::string s)
{
  std::cout<<"calling open "<<std::endl;
  if (_mdcc!=NULL)
    delete _mdcc;
  _mdcc= new MDCCReadout(s);
  _mdcc->open();
  //std::cout<<" Open Ptr "<<_mdcc<<std::endl;
}
