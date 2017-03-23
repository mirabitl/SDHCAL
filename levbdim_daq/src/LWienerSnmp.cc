
#include "LWienerServer.hh"
#include "fileTailer.hh"



void LWienerServer::configure(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  //uint32_t port=m->content()["port"].asInt();

  std::string ip;
  if (m->content().isMember("ip"))
    { 
      ip=m->content()["ip"].asString();
      this->parameters()["ip"]=m->content()["ip"];
    }
  else
    ip=this->parameters()["ip"].asString();
  this->Open(ip);

  //std::string sdb =m->content()["account"].asString();
  std::string account;
  if (m->content().isMember("account"))
    { 
      account=m->content()["account"].asString();
      this->parameters()["account"]=m->content()["account"];
    }
  else
    account=this->parameters()["account"].asString();
  _my= new MyInterface(account);
  _my->connect();

  Json::Value jrep;
  _jchambers.clear();
  _my->executeSelect("select HVCHAN,VREF,IREF,P0,T0 FROM CHAMBERREF WHERE FIN>NOW() AND DEBUT<NOW()");
  MYSQL_ROW row=NULL;
  while ((row=_my->getNextRow())!=0)
    {
      //std::cout<<c.channel<<" "<<c.vref<<std::endl;
      Json::Value jch;
      jch["channel"]=atoi(row[0]);
      jch["vref"]=atof(row[1]);
      jch["iref"]=atof(row[2]);
      jch["pref"]=atof(row[3]);
      jch["tref"]=atof(row[4]);
      _jchambers.append(jch);
    }
  _my->disconnect();
  jrep["chambers"]=_jchambers;
  m->setAnswer(jrep);

  
}

void LWienerSnmp::doMonitoring()
{
  _storeRunning=true;
  LOG4CXX_INFO(_logLdaq,"Storage thread started");
  while (_storeRunning)
    {
      if (_my==NULL)
	{
	  sleep((unsigned int) 10);
	  continue;
	}
      _bsem.lock();
      _my->connect();

      const Json::Value& chambers = _jchambers;
      Json::Value array_keys;
      for (Json::ValueConstIterator it = chambers.begin(); it != chambers.end(); ++it)
	{
	  const Json::Value& ch = *it;

	  std::stringstream s0;
	  s0.str(std::string());
	  uint32_t i=ch["channel"].asUInt();
	  Json::Value rch=this->Read(i);
	  s0<<"insert into WIENERMON(HVCHAN,VSET,ISET,VOUT,IOUT) VALUES("<<ch["channel"].asUInt()<<","<<rch["vset"].asFloat()<<","<<rch["uset"].asFloat()<<","<<rch["vout"].asFloat()<<","<<rch["iout"].asFloat()<<")";
	  LOG4CXX_DEBUG(_logLdaq,"execute "<<s0.str());
	  _my->executeQuery(s0.str());

	}
      _my->disconnect();
      _bsem.unlock();
      for (int i=0;i<_storeTempo;i++)
	if (_storeRunning)
	  sleep((unsigned int) 1);
    }
  LOG4CXX_INFO(_logLdaq,"Storage thread stopped");
}

void LWienerServer::startMonitoring(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
 

  _storeTempo =m->content()["period"].asUInt();
  g_store.create_thread(boost::bind(&LWienerServer::doMonitoring, this));
  Json::Value jsta;
  jsta["action"]="MONITOR STARTED";
  m->setAnswer(jsta);
  
}
void LWienerServer::stopMonitoring(levbdim::fsmmessage* m)
{
    _storeRunning=false;
  g_store.join_all();
  LOG4CXX_INFO(_logLdaq,"Storage thread destroy");
  Json::Value jsta;
  jsta["action"]="MONITOR STOPPED";
  m->setAnswer(jsta);

}
void LWienerServer::startAutoControl(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());

  _checkTempo =m->content()["period"].asUInt();
  g_check.create_thread(boost::bind(&LWienerServer::doCheck, this));
  Json::Value jsta;
  jsta["action"]="HV CHECK STARTED";
  m->setAnswer(jsta);
  

}
void LWienerServer::stopAutoControl(levbdim::fsmmessage* m)
{

  _checkRunning=false;
  g_check.join_all();
  LOG4CXX_INFO(_logLdaq,"Check thread destroy");
  Json::Value jsta;
  jsta["action"]="HV CHECK STOPPED";
  m->setAnswer(jsta);

}

void LWienerServer::destroy(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  this->Read();
  Json::Value r;
  r["vset"]=_status[0];
  r["vout"]=_status[1];
  r["iout"]=_status[2];
  m->setAnswer(r);
}


void LWienerServer::c_joblog(Mongoose::Request &request, Mongoose::JsonResponse &response)
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

void LWienerServer::c_setOutputVoltage(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t first=atol(request.get("first","0").c_str());
  uint32_t last=atol(request.get("last","0").c_str());	
  float  vset=atof(request.get("value","0.0").c_str());
  for (uint32_t i=first;i<=last;i++)
  {
    uint32_t imodule=i/8;
    uint32_t ichannel=i%8;
    _hv->c_setOutputVoltage(imodule,ichannel,vset);
  }
  response["STATUS"]="DONE";
 
}

void LWienerServer::c_setOutputVoltageRiseRate(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t first=atol(request.get("first","0").c_str());
  uint32_t last=atol(request.get("last","0").c_str());	
  float  vset=atof(request.get("value","0.0").c_str());

  for (uint32_t i=first;i<=last;i++)
  {
    uint32_t imodule=i/8;
    uint32_t ichannel=i%8;
    _hv->c_setOutputVoltageRiseRate(imodule,ichannel,vset);

  }
  response["STATUS"]="DONE";
 
}
void LWienerServer::c_setCurrentLimit(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t first=atol(request.get("first","0").c_str());
  uint32_t last=atol(request.get("last","0").c_str());	
  float  vset=atof(request.get("value","0.0").c_str());
  for (uint32_t i=first;i<=last;i++)
  {
    uint32_t imodule=i/8;
    uint32_t ichannel=i%8;
    _hv->c_setOutputCurrentLimit(imodule,ichannel,vset);
  }
  response["STATUS"]="DONE";
 
}
void LWienerServer::c_setOutputSwitch(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t first=atol(request.get("first","0").c_str());
  uint32_t last=atol(request.get("last","0").c_str());	
  uint32_t  vset=atof(request.get("value","0").c_str());
  for (uint32_t i=first;i<=last;i++)
  {
    uint32_t imodule=i/8;
    uint32_t ichannel=i%8;
    _hv->c_setOutputSwitch(imodule,ichannel,vset);
  }
  response["STATUS"]="DONE";
 
}
void LWienerServer::c_getStatus(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t first=atol(request.get("first","0").c_str());
  uint32_t last=atol(request.get("last","0").c_str());	
 
  Json::Value rep;
  for (uint32_t i=first;i<=last;i++)
  {
    rep.append(this->Read(i));
  }
  response["STATUS"]="DONE";
  response["HVSTATUS"]=rep;
 
}
LWienerServer::LWienerServer(std::string name) : levbdim::baseApplication(name),_hv(NULL)
{
  //_fsm=new levbdim::fsm(name);
  _fsm=this->fsm();
// Register state

  _fsm->addState("CONFIGURED");
  _fsm->addState("MONITORED");
  _fsm->addState("CONTROLED");

  _fsm->addTransition("CONFIGURE","CREATED","CONFIGURED",boost::bind(&LWienerServer::configure, this,_1));
  _fsm->addTransition("STARTMONITOR","CONFIGURED","MONITORED",boost::bind(&LWienerServer::startMonitoring, this,_1));
  _fsm->addTransition("STOPMONITOR","MONITORED","CONFIGURED",boost::bind(&LWienerServer::stopMonitoring, this,_1));
  _fsm->addTransition("STARTCONTROL","MONITORED","CONTROLED",boost::bind(&LWienerServer::startAutoControl, this,_1));
  _fsm->addTransition("STOPCONTROL","CONTROLED","MONITORED",boost::bind(&LWienerServer::stopAutoControl, this,_1));
  _fsm->addTransition("DESTROY","CONFIGURED","CREATED",boost::bind(&LWienerServer::destroy, this,_1));


  // Command
  _fsm->addCommand("JOBLOG",boost::bind(&LWienerServer::c_joblog,this,_1,_2));
  _fsm->addCommand("SETOUTPUTSWITCH",boost::bind(&LWienerServer::c_setOutputSwitch,this,_1,_2));
  _fsm->addCommand("SETOUTPUTVOLTAGE",boost::bind(&LWienerServer::c_setOutputVoltage,this,_1,_2));
  _fsm->addCommand("SETOUTPUTVOLTAGERISERATE",boost::bind(&LWienerServer::c_setOutputVoltageRiseRate,this,_1,_2));
  _fsm->addCommand("SETCURRENTLIMIT",boost::bind(&LWienerServer::c_setCurrentLimit,this,_1,_2));
  _fsm->addCommand("GETSTATUS",boost::bind(&LWienerServer::c_getStatus,this,_1,_2));
  std::stringstream s0;
  // s0.str(std::string());
  // s0<<"/DGPIO/"<<name<<"/STATUS";
  // memset(_status,0,3*sizeof(float));
  //_zsStatus = new DimService(s0.str().c_str(),"F:3",_status,3*sizeof(float));
  s0.str(std::string());
  s0<<"LWienerServer-"<<name;
  DimServer::start(s0.str().c_str()); 

  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      std::cout<<"Service "<<name<<" started on port "<<atoi(wp)<<std::endl;
    _fsm->start(atoi(wp));
    }

	

}

void LWienerServer::Open(std::string s)
{
  
  if (_hv!=NULL)
    delete _hv;
  _hv= new WienerSnmp(s);

}
Json::Value LWienerServer::Read(uint32_t i)
{
  if (_hv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq," GPIO not created ");
      return;
    }
  uint32_t imodule=i/8;
  uint32_t ichannel=i%8;
  Json::Value r;	
  r["channel"]=i;
  r["vset"]=_hv->getOutputVoltage(imodule,ichannel);
  r["iset"]=_hv->getOutputCurrentLimit(imodule,ichannel);
  r["vout"]=_hv->getOutputMeasurementSenseVoltage(imodule,ichannel);
  r["iout"]=_hv->getOutputMeasurementCurrent(imodule,ichannel);
  r["ramp"]=_hv->getOutputVoltageRiseRate(imodule,ichannel);
}

