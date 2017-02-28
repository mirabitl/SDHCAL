
#include "LBmp183Server.hh"
#include "fileTailer.hh"



void LBmp183Server::configure(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  
 
  if (_bmp!=NULL)
    delete _bmp;
  _bmp= new BMP183();
  
  Json::Value jrep;
 
  jrep["status"]="DONE";
  m->setAnswer(jrep);

  
}



void LBmp183Server::destroy(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());


  Json::Value r;
  r["status"]="DONE";
  m->setAnswer(r);
}


void LBmp183Server::c_joblog(Mongoose::Request &request, Mongoose::JsonResponse &response)
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

void LBmp183Server::c_getStatus(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_bmp==NULL)
    {
      LOG4CXX_ERROR(_logLdaq," BMP access not done not created ");
      response["STATUS"]="FAILED";
      return;
    }
  Json::Value rep;
  rep["P"]=_bmp->BMP183PressionRead();
  rep["T"]=_bmp->BMP183TemperatureRead();
  response["STATUS"]="DONE";
  response["BMPSTATUS"]=rep;
 
}
LBmp183Server::LBmp183Server(std::string name) : _bmp(NULL)
{
  //_fsm=new levbdim::fsm(name);
  _fsm=new fsmweb(name);
// Register state
  _fsm->addState("CREATED");
  _fsm->addState("CONFIGURED");
  
  _fsm->addTransition("CONFIGURE","CREATED","CONFIGURED",boost::bind(&LBmp183Server::configure, this,_1));
    _fsm->addTransition("DESTROY","CONFIGURED","CREATED",boost::bind(&LBmp183Server::destroy, this,_1));


  // Command
  _fsm->addCommand("JOBLOG",boost::bind(&LBmp183Server::c_joblog,this,_1,_2));
  _fsm->addCommand("GETSTATUS",boost::bind(&LBmp183Server::c_getStatus,this,_1,_2));
  std::stringstream s0;
  // s0.str(std::string());
  // s0<<"/DGPIO/"<<name<<"/STATUS";
  // memset(_status,0,3*sizeof(float));
  //_zsStatus = new DimService(s0.str().c_str(),"F:3",_status,3*sizeof(float));
  s0.str(std::string());
  s0<<"LBmp183Server-"<<name;
  DimServer::start(s0.str().c_str()); 

  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      std::cout<<"Service "<<name<<" started on port "<<atoi(wp)<<std::endl;
    _fsm->start(atoi(wp));
    }

	

}

