
#include "LWienerServer.hh"
#include "fileTailer.hh"



void LWienerServer::configure(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  //uint32_t port=m->content()["port"].asInt();
  std::string ip=m->content()["ip"].asString();
  
 this->Open(device);
 // this->read(m);
 //std::cout<<"reponse=> "<<m->content()["answer"]<<std::endl;
}
void LWienerServer::on(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  this->Switch(1);
  this->read(m);
}
void LWienerServer::off(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  this->Switch(0);
  this->read(m);
}
void LWienerServer::read(levbdim::fsmmessage* m)
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
LWienerServer::LWienerServer(std::string name) : _hv(NULL)
{
  //_fsm=new levbdim::fsm(name);
  _fsm=new fsmweb(name);
// Register state
  _fsm->addState("CREATED");
  _fsm->addState("CONFIGURED");
  _fsm->addState("MONITORED");
  _fsm->addState("CONTROLED");

  _fsm->addTransition("CONFIGURE","CREATED","CONFIGURED",boost::bind(&LWienerServer::configure, this,_1));
  _fsm->addTransition("STARTMONITOR","CONFIGURED","MONITORED",boost::bind(&LWienerServer::startMonitoring, this,_1));
  _fsm->addTransition("STOPMONITOR","MONITORED","CONFIGURED",boost::bind(&LWienerServer::stopMonitoring, this,_1));
  _fsm->addTransition("STARTCONTROL","MONITORED","CONTROLED",boost::bind(&LWienerServer::startAutoControl, this,_1));
  _fsm->addTransition("STOPCONTROL","CONTROLED","MONITORED",boost::bind(&LWienerServer::startAutoControl, this,_1));
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
  _status[0]=_hv->getVMEPower()*12.0;
  _status[1]=_hv->getDIFPower()*6.0;
  _status[2]=0;
  this->publishStatus();
}

void LWienerServer::Switch(uint32_t mode)
{
  if (_hv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq," Zup not created ");
      return;
    }
  if (mode==0)
    {
      LOG4CXX_INFO(_logLdaq,"Switching OFF "<<mode);
      _hv->DIFOFF();
    }
  else
    {
      LOG4CXX_INFO(_logLdaq,"Switching ON "<<mode);
      _hv->DIFON();
    }
  this->Read();
}
