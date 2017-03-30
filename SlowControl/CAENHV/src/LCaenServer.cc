
#include "LCaenServer.hh"
#include "fileTailer.hh"



void LCaenServer::configure(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  //uint32_t port=m->content()["port"].asInt();
  std::string ip=m->content()["ip"].asString();
  
  this->Open(ip);

  _hv->Connect();
  Json::Value jrep;
 
  jrep["status"]="DONE";
  m->setAnswer(jrep);

  
}



void LCaenServer::destroy(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  _hv->Disconnect();

  Json::Value r;
  r["status"]="DONE";
  m->setAnswer(r);
}


void LCaenServer::c_joblog(Mongoose::Request &request, Mongoose::JsonResponse &response)
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

void LCaenServer::c_setOutputVoltage(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t first=atol(request.get("first","0").c_str());
  uint32_t last=atol(request.get("last","0").c_str());	
  float  vset=atof(request.get("value","0.0").c_str());
   if (_hv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq," CAEN access not done not created ");
      response["STATUS"]="FAILED";
      return;
    }
  if (!_hv->isConnected()) _hv->Connect();
  for (uint32_t i=first;i<=last;i++)
  {
   
    _hv->SetVoltage(i,vset);
  }
  response["STATUS"]="DONE";
  _hv->Disconnect();
 
}

void LCaenServer::c_setOutputVoltageRiseRate(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t first=atol(request.get("first","0").c_str());
  uint32_t last=atol(request.get("last","0").c_str());	
  float  vset=atof(request.get("value","0.0").c_str());
  if (_hv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq," CAEN access not done not created ");
      response["STATUS"]="FAILED";
      return;
    }
 

  if (!_hv->isConnected()) _hv->Connect();
  for (uint32_t i=first;i<=last;i++)
    {
   
    _hv->SetVoltageRampUp(i,vset);
  }
  response["STATUS"]="DONE";

  _hv->Disconnect();
}
void LCaenServer::c_setCurrentLimit(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t first=atol(request.get("first","0").c_str());
  uint32_t last=atol(request.get("last","0").c_str());	
  float  vset=atof(request.get("value","0.0").c_str());
   if (_hv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq," CAEN access not done not created ");
      response["STATUS"]="FAILED";
      return;
    }
  if (!_hv->isConnected()) _hv->Connect();
  for (uint32_t i=first;i<=last;i++)
  {
    _hv->SetCurrent(i,vset);
  }
  response["STATUS"]="DONE";
  _hv->Disconnect();
}
void LCaenServer::c_setOutputSwitch(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t first=atol(request.get("first","0").c_str());
  uint32_t last=atol(request.get("last","0").c_str());	
  uint32_t  vset=atof(request.get("value","0").c_str());
  
   if (_hv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq," CAEN access not done not created ");
      response["STATUS"]="FAILED";
      return;
    }
  if (!_hv->isConnected()) _hv->Connect();
  for (uint32_t i=first;i<=last;i++)
  {
    if (vset)
      _hv->SetOn(i);
    else
      _hv->SetOff(i);
  }
  _hv->Disconnect();
  response["STATUS"]="DONE";
 
}
void LCaenServer::c_getStatus(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t first=atol(request.get("first","0").c_str());
  uint32_t last=atol(request.get("last","0").c_str());	
 if (_hv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq," CAEN access not done not created ");
      response["STATUS"]="FAILED";
      return;
    }
  Json::Value rep;
  for (uint32_t i=first;i<=last;i++)
  {
    rep.append(this->Read(i));
  }
  response["STATUS"]="DONE";
  response["HVSTATUS"]=rep;
 
}
LCaenServer::LCaenServer(std::string name) : _hv(NULL)
{
  //_fsm=new levbdim::fsm(name);
  _fsm=new fsmweb(name);
// Register state
  _fsm->addState("CREATED");
  _fsm->addState("CONFIGURED");
  
  _fsm->addTransition("CONFIGURE","CREATED","CONFIGURED",boost::bind(&LCaenServer::configure, this,_1));
    _fsm->addTransition("DESTROY","CONFIGURED","CREATED",boost::bind(&LCaenServer::destroy, this,_1));


  // Command
  _fsm->addCommand("JOBLOG",boost::bind(&LCaenServer::c_joblog,this,_1,_2));
  _fsm->addCommand("SETOUTPUTSWITCH",boost::bind(&LCaenServer::c_setOutputSwitch,this,_1,_2));
  _fsm->addCommand("SETOUTPUTVOLTAGE",boost::bind(&LCaenServer::c_setOutputVoltage,this,_1,_2));
  _fsm->addCommand("SETOUTPUTVOLTAGERISERATE",boost::bind(&LCaenServer::c_setOutputVoltageRiseRate,this,_1,_2));
  _fsm->addCommand("SETCURRENTLIMIT",boost::bind(&LCaenServer::c_setCurrentLimit,this,_1,_2));
  _fsm->addCommand("GETSTATUS",boost::bind(&LCaenServer::c_getStatus,this,_1,_2));
  std::stringstream s0;
  // s0.str(std::string());
  // s0<<"/DGPIO/"<<name<<"/STATUS";
  // memset(_status,0,3*sizeof(float));
  //_zsStatus = new DimService(s0.str().c_str(),"F:3",_status,3*sizeof(float));
  s0.str(std::string());
  s0<<"LCaenServer-"<<name;
  DimServer::start(s0.str().c_str()); 

  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      std::cout<<"Service "<<name<<" started on port "<<atoi(wp)<<std::endl;
    _fsm->start(atoi(wp));
    }

	

}

void LCaenServer::Open(std::string account)
{
  
  if (_hv!=NULL)
    delete _hv;

  int ipass = account.find("/");
  int ipath = account.find("@");
  std::string Name,Pwd,Host;
  Name.clear();
  Name=account.substr(0,ipass); 
  Pwd.clear();
  Pwd=account.substr(ipass+1,ipath-ipass-1); 
  Host.clear();
  Host=account.substr(ipath+1,account.size()-ipath); 
  std::cout<<Name<<std::endl;
  std::cout<<Pwd<<std::endl;
  std::cout<<Host<<std::endl;
  
  _hv= new HVCaenInterface(Host,Name,Pwd);

}
Json::Value LCaenServer::Read(uint32_t i)
{
  
  if (!_hv->isConnected()) _hv->Connect();
  Json::Value r;	
  r["channel"]=i;
  r["name"]=_hv->GetName(i);
  r["vset"]=_hv->GetVoltageSet(i);
  r["iset"]=_hv->GetCurrentSet(i);
  r["vout"]=_hv->GetVoltageRead(i);
  r["iout"]=_hv->GetCurrentRead(i);
  r["status"]=_hv->GetStatus(i);
  r["ramp"]=_hv->GetVoltageRampUp(i);
  _hv->Disconnect();
  return r;
}

