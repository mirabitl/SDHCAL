
#include "LGpioServer.hh"
#include "fileTailer.hh"



void LGpioServer::configure(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  uint32_t port=m->content()["port"].asInt();
  std::string device=m->content()["device"].asString();
  
 this->Open(device,port);
 this->read(m);
 std::cout<<"reponse=> "<<m->content()["answer"]<<std::endl;
}
void LGpioServer::on(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  this->Switch(1);
  this->read(m);
}
void LGpioServer::off(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  this->Switch(0);
  this->read(m);
}
void LGpioServer::read(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  this->Read();
  Json::Value r;
  r["vset"]=_status[0];
  r["vout"]=_status[1];
  r["iout"]=_status[2];
  m->setAnswer(r);
}

void LGpioServer::c_joblog(Mongoose::Request &request, Mongoose::JsonResponse &response)
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

void LGpioServer::c_vmeon(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  _gpio->VMEON();
  response["STATUS"]="DONE";
 
}
void LGpioServer::c_vmeoff(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  _gpio->VMEOFF();
  response["STATUS"]="DONE";
}

LGpioServer::LGpioServer(std::string name) : _gpio(NULL)
{
  //_fsm=new levbdim::fsm(name);
  _fsm=new fsmweb(name);
// Register state
  _fsm->addState("CREATED");
  _fsm->addState("CONFIGURED");
  _fsm->addState("ON");
  _fsm->addState("OFF");

  _fsm->addTransition("CONFIGURE","CREATED","CONFIGURED",boost::bind(&LGpioServer::configure, this,_1));
  _fsm->addTransition("ON","CONFIGURED","ON",boost::bind(&LGpioServer::on, this,_1));
  _fsm->addTransition("ON","OFF","ON",boost::bind(&LGpioServer::on, this,_1));
  _fsm->addTransition("OFF","CONFIGURED","OFF",boost::bind(&LGpioServer::off, this,_1));
  _fsm->addTransition("OFF","ON","OFF",boost::bind(&LGpioServer::off, this,_1));
  _fsm->addTransition("READ","ON","ON",boost::bind(&LGpioServer::read, this,_1));
  _fsm->addTransition("READ","OFF","OFF",boost::bind(&LGpioServer::read, this,_1));
  _fsm->addTransition("READ","CONFIGURED","CONFIGURED",boost::bind(&LGpioServer::read, this,_1));

  _fsm->addCommand("JOBLOG",boost::bind(&LGpioServer::c_joblog,this,_1,_2));
  _fsm->addCommand("VMEON",boost::bind(&LGpioServer::c_vmeon,this,_1,_2));
  _fsm->addCommand("VMEON",boost::bind(&LGpioServer::c_vmeoff,this,_1,_2));
  std::stringstream s0;
  s0.str(std::string());
  s0<<"/DGPIO/"<<name<<"/STATUS";
  memset(_status,0,3*sizeof(float));
  _zsStatus = new DimService(s0.str().c_str(),"F:3",_status,3*sizeof(float));
  s0.str(std::string());
  s0<<"LGpioServer-"<<name;
  DimServer::start(s0.str().c_str()); 

  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      std::cout<<"Service "<<name<<" started on port "<<atoi(wp)<<std::endl;
    _fsm->start(atoi(wp));
    }

	

}

void LGpioServer::Open(std::string s, uint32_t port)
{
  
  if (_gpio!=NULL)
    delete _gpio;
  _gpio= new GPIO();

}
void LGpioServer::Read()
{
  if (_gpio==NULL)
    {
      LOG4CXX_ERROR(_logLdaq," GPIO not created ");
      return;
    }
  sleep((unsigned int) 1);
  _status[0]=_gpio->getVMEPower()*12.0;
  _status[1]=_gpio->getDIFPower()*6.0;
  _status[2]=0;
  this->publishStatus();
}

void LGpioServer::Switch(uint32_t mode)
{
  if (_gpio==NULL)
    {
      LOG4CXX_ERROR(_logLdaq," Zup not created ");
      return;
    }
  if (mode==0)
    {
      LOG4CXX_INFO(_logLdaq,"Switching OFF "<<mode);
      _gpio->DIFOFF();
    }
  else
    {
      LOG4CXX_INFO(_logLdaq,"Switching ON "<<mode);
      _gpio->DIFON();
    }
  this->Read();
}
