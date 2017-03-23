#include "FullDaq.hh"
#include <unistd.h>
#include <stdint.h>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include "LdaqLogger.hh"

FullDaq::FullDaq(std::string name) : levbdim::baseApplication(name),_builderClient(NULL),_dbClient(NULL),_cccClient(NULL),_mdccClient(NULL),_zupClient(NULL),_gpioClient(NULL)
  {
    _DIFClients.clear();
    
    _fsm=this->fsm();
    
    
    _fsm->addState("DISCOVERED");
    _fsm->addState("PREPARED");
    _fsm->addState("INITIALISED");
    _fsm->addState("CONFIGURED");
    _fsm->addState("RUNNING");

    _fsm->addTransition("DISCOVER","CREATED","DISCOVERED",boost::bind(&FullDaq::discover, this,_1));
    _fsm->addTransition("DISCOVER","DISCOVERED","DISCOVERED",boost::bind(&FullDaq::discover, this,_1));
    _fsm->addTransition("PREPARE","DISCOVERED","PREPARED",boost::bind(&FullDaq::prepare, this,_1));
    _fsm->addTransition("PREPARE","PREPARED","PREPARED",boost::bind(&FullDaq::prepare, this,_1));
    _fsm->addTransition("INITIALISE","PREPARED","INITIALISED",boost::bind(&FullDaq::initialise, this,_1));
    _fsm->addTransition("CONFIGURE","INITIALISED","CONFIGURED",boost::bind(&FullDaq::configure, this,_1));
    _fsm->addTransition("CONFIGURE","CONFIGURED","CONFIGURED",boost::bind(&FullDaq::configure, this,_1));
    _fsm->addTransition("START","CONFIGURED","RUNNING",boost::bind(&FullDaq::start, this,_1));
    _fsm->addTransition("STOP","RUNNING","CONFIGURED",boost::bind(&FullDaq::stop, this,_1));
    _fsm->addTransition("DESTROY","CONFIGURED","PREPARED",boost::bind(&FullDaq::destroy, this,_1));
    _fsm->addTransition("DESTROY","INITIALISED","PREPARED",boost::bind(&FullDaq::destroy, this,_1));

    // Commands

    _fsm->addCommand("DOUBLESWITCHZUP",boost::bind(&FullDaq::doubleSwitchZup,this,_1,_2));
    _fsm->addCommand("LVSTATUS",boost::bind(&FullDaq::LVStatus,this,_1,_2));
    _fsm->addCommand("LVON",boost::bind(&FullDaq::LVON,this,_1,_2));
    _fsm->addCommand("FORCESTATE",boost::bind(&FullDaq::forceState,this,_1,_2));
    _fsm->addCommand("LVOFF",boost::bind(&FullDaq::LVOFF,this,_1,_2));
    _fsm->addCommand("DOWNLOADDB",boost::bind(&FullDaq::downloadDB,this,_1,_2));
    _fsm->addCommand("CTRLREG",boost::bind(&FullDaq::setControlRegister,this,_1,_2));
    _fsm->addCommand("DBSTATUS",boost::bind(&FullDaq::dbStatus,this,_1,_2));
    _fsm->addCommand("EVBSTATUS",boost::bind(&FullDaq::builderStatus,this,_1,_2));
    _fsm->addCommand("DIFSTATUS",boost::bind(&FullDaq::status,this,_1,_2));
    _fsm->addCommand("PAUSE",boost::bind(&FullDaq::pauseTrigger,this,_1,_2));
    _fsm->addCommand("RESUME",boost::bind(&FullDaq::resumeTrigger,this,_1,_2));
    _fsm->addCommand("ECALPAUSE",boost::bind(&FullDaq::pauseEcal,this,_1,_2));
    _fsm->addCommand("ECALRESUME",boost::bind(&FullDaq::resumeEcal,this,_1,_2));
    _fsm->addCommand("RESETCOUNTERS",boost::bind(&FullDaq::resetTriggerCounters,this,_1,_2));
    _fsm->addCommand("TRIGGERSTATUS",boost::bind(&FullDaq::triggerStatus,this,_1,_2));
    _fsm->addCommand("SPILLON",boost::bind(&FullDaq::triggerSpillOn,this,_1,_2));
    _fsm->addCommand("SPILLOFF",boost::bind(&FullDaq::triggerSpillOff,this,_1,_2));
    _fsm->addCommand("BEAMON",boost::bind(&FullDaq::triggerBeam,this,_1,_2));
    _fsm->addCommand("SETTHRESHOLD",boost::bind(&FullDaq::setThreshold,this,_1,_2));
    _fsm->addCommand("SETGAIN",boost::bind(&FullDaq::setGain,this,_1,_2));
    _fsm->addCommand("REGISTERDS",boost::bind(&FullDaq::registerDataSource,this,_1,_2));
    
  cout<<"Building FullDaq"<<endl;
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0.str(std::string());
  s0<<"FullDaq-"<<hname;

  DimServer::start(s0.str().c_str()); 
  //  cout<<"Starting DimDaqCtrl"<<endl;

  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      std::cout<<"Service "<<name<<" started on port "<<atoi(wp)<<std::endl;
    _fsm->start(atoi(wp));
    }

 
  }




void FullDaq::discover(levbdim::fsmmessage* m)
{
  _DIFClients.clear();
  const Json::Value& cjsources=this->configuration()["HOSTS"];
  std::vector<std::string> lhosts=this->configuration()["HOSTS"].getMemberNames();
  // Loop on hosts
  for (auto host:lhosts)
    {
      // Loop on processes
      const Json::Value& cjsources=this->configuration()["HOSTS"][host];
      for (Json::ValueConstIterator it = cjsources.begin(); it != cjsources.end(); ++it)
	{
	  const Json::Value& process = *it;
	  std::string p_name=process["NAME"].asString();
	  // Loop on environenemntal variable
	  uint32_t port=0;
	  const Json::Value& cenv=process["ENV"];
	  for (Json::ValueConstIterator iev = cenv.begin(); iev != cenv.end(); ++iev)
	    {
	      std::string envp=(*iev).asString();
	      if (envp.substr(0,7).compare("WEBPORT")==0)
		{
		  port=atol(envp.substr(8,envp.length()-7).c_str());
		  break;
		}
	    }
	  if (port==0) continue;
	  // Now analyse process Name
	  if (p_name.compare("WRITER")==0)
	    {
	      _builderClient= new fsmwebClient(host,port);
	    }
	  if (p_name.compare("DBSERVER")==0)
	    {
	      _dbClient= new fsmwebClient(host,port);
	    }
	  if (p_name.compare("CCCSERVER")==0)
	    {
	      _cccClient= new fsmwebClient(host,port);
	    }
	  if (p_name.compare("MDCCSERVER")==0)
	    {
	      _mdccClient= new fsmwebClient(host,port);
	    }
	  if (p_name.compare("ZUPSERVER")==0)
	    {
	      _zupClient= new fsmwebClient(host,port);
	    }
	  if (p_name.compare("GPIOSERVER")==0)
	    {
	      _gpioClient= new fsmwebClient(host,port);
	      _gpioClient->sendTransition("CONFIGURE");
	      _gpioClient->sendCommand("VMEON");
	      _gpioClient->sendCommand("VMEOFF");
	      _gpioClient->sendCommand("VMEON");
	      
	    }
	   if (p_name.compare("DIFSERVER")==0)
	    {
	      fsmwebClient* dc= new fsmwebClient(host,port);
	      _DIFClients.push_back(dc);
	    }
	  
	}

    }
  
  

}
void FullDaq::prepare(levbdim::fsmmessage* m)
{
  //std::cout<<"ON RENTREEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"<<std::endl;
  //  for (int i=0;i<100;i++)
  printf("Clients: DB %x ZUP %x MDC %x SDCC %x \n",_dbClient,_zupClient,_mdccClient,_cccClient);
  m->content();
  // DB
  if (_dbClient)
    {
      
      if (this->parameters().isMember("dbstate"))
	{
	  _dbClient->sendTransition("DOWNLOAD",this->parameters());
	}
      else
	_dbClient->sendTransition("DOWNLOAD");
    }
  // Zup
  if (_zupClient)
    {
       if (this->parameters().isMember("zup"))
	{
	  _zupClient->sendTransition("CONFIGURE",this->parameters()["zup"]);
	}
       else
	 _zupClient->sendTransition("CONFIGURE");
      //std::cout<<"Current zup values "<<_zupClient->reply()<<std::endl;
    }
   // gpio

  // Ccc
  //std::cout<<" CCC client "<<_cccClient<<std::endl;
  if (_cccClient)
    {

      if (this->parameters().isMember("ccc"))
	{
	  _cccClient->sendTransition("OPEN",this->parameters()["ccc"]);
	}
       else
	 _cccClient->sendTransition("OPEN");

      
      _cccClient->sendTransition("INITIALISE");
    }
  // Mdc
  if (_mdccClient)
    {

      if (this->parameters().isMember("mdcc"))
	{
	  _mdccClient->sendTransition("OPEN",this->parameters()["mdcc"]);
	}
       else
	 _mdccClient->sendTransition("OPEN");

      _mdccClient->sendCommand("STATUS");
      //std::cout<<"Current MDCC values "<<_mdccClient->reply()<<std::endl;
      
    }
  // Builder
  if (_builderClient)
    {
      if (this->parameters().isMember("builder"))
	{
	  _builderClient->sendTransition("INITIALISE",this->parameters()["builder"]);
	}
       else
	 _builderClient->sendTransition("INITIALISE");

    }
}


std::string FullDaq::difstatus()
{
  Json::Value devlist;
  for (auto dc:_DIFClients)
    {
      dc->sendTransition("STATUS");
      const Json::Value& jdevs=dc->answer();
      for (Json::ValueConstIterator jt = jdevs.begin(); jt != jdevs.end(); ++jt)
	devlist.append(*jt);
    }
  
 
  //std::cout<<devlist<<std::endl;
  Json::FastWriter fastWriter;
  return fastWriter.write(devlist);
}

void FullDaq::singlescan(fsmwebClient* d)
{
 
  d->sendTransition("SCAN");
}
void FullDaq::singleinit(fsmwebClient* d)
{
  Json::Value c;
  c["difid"]=0;
  d->sendTransition("INITIALISE",c);
}

void FullDaq::singleregisterdb(fsmwebClient* d)
{
  
  d->sendTransition("REGISTERDB",this->parameters());
  //  std::cout<<"Register DB"<<d->parameters()<<std::endl;
}
void FullDaq::singleconfigure(fsmwebClient* d)
{
  Json::Value jc=this->parameters();
  jc["difid"]=0;
  d->sendTransition("CONFIGURE",jc);
}
void FullDaq::singlestart(fsmwebClient* d)
{
 
  Json::Value c;
  c["difid"]=0;
  d->sendTransition("START",c);
  //std::cout<<"received "<<d->reply()<<std::endl;
}
void FullDaq::singlestop(fsmwebClient* d)
{

  Json::Value c;
  c["difid"]=0;
  d->sendTransition("STOP",c);
}

Json::Value FullDaq::toJson(std::string s)
{
  Json::Reader reader;
  Json::Value jsta;
  bool parsingSuccessful = reader.parse(s,jsta);
  return jsta;
}
void FullDaq::initialise(levbdim::fsmmessage* m)
{
  // Configure CCC
  if (_cccClient)
    {
   
      _cccClient->sendTransition("CONFIGURE");
      _cccClient->sendTransition("STOP");
      ::sleep(1);
      _cccClient->sendCommand("CCCRESET");
      _cccClient->sendCommand("DIFRESET");
      ::sleep(1);
    }

  // Make a DIF SCAN
  boost::thread_group g;
  for (std::vector<fsmwebClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
      g.create_thread(boost::bind(&FullDaq::singlescan, this,(*it)));
  g.join_all();

  // Merge devlist
  Json::Value devlist;
  for (std::vector<fsmwebClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
       const Json::Value& jdevs=(*it)->answer();
      for (Json::ValueConstIterator jt = jdevs.begin(); jt != jdevs.end(); ++jt)
	devlist.append(*jt);
    }
  //std::cout<<devlist<<std::endl;

  // Initialise
  for (std::vector<fsmwebClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
      g.create_thread(boost::bind(&FullDaq::singleinit, this,(*it)));
  g.join_all();
  

  // Fill status
  m->setAnswer(toJson(this->difstatus()));
}

void FullDaq::configure(levbdim::fsmmessage* m)
{
  // Configure CCC
  //std::cout<<m->content();
  if (_cccClient)
    {
      _cccClient->sendCommand("CCCRESET");
      _cccClient->sendCommand("DIFRESET");
      //std::cout<<"RESET DONE"<<std::endl;
    }

  // register to the dbstate
  boost::thread_group g;
  for (std::vector<fsmwebClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
      g.create_thread(boost::bind(&FullDaq::singleregisterdb, this,(*it)));
    }
  g.join_all();
  std::cout<<"REGISTER DB DONE"<<std::endl;
  ::sleep(2);
  //Configure them
  for (std::vector<fsmwebClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
     
      g.create_thread(boost::bind(&FullDaq::singleconfigure, this,(*it)));
    }
  g.join_all();
  std::cout<<"DIF CONFIGURE  DONE"<<std::endl;
  // Status
  Json::Value jsta= toJson(this->difstatus());
  // Configure the builder
  if (_builderClient)
    {
      // Build complete list of data sources
      Json::Value jsou;
      jsou.clear();
      const Json::Value& jdevs=jsta;
      for (Json::ValueConstIterator it = jdevs.begin(); it != jdevs.end(); ++it)
	{
	  Json::Value jd;
	  jd["detid"]=(*it)["detid"];
	  jd["sourceid"]=(*it)["id"];
	  jsou.append(jd);
	}
      //std::cout<<"SENDING "<<jsou<<std::endl;
      Json::Value jl;
      jl["sources"]=jsou;
      _builderClient->sendTransition("CONFIGURE",jl);
    }
  m->setAnswer(jsta);
}

void FullDaq::start(levbdim::fsmmessage* m)
{
  // Get the new run number
   if (_dbClient)
    {
      _dbClient->sendTransition("NEWRUN");
      _run=_dbClient->answer()["run"].asInt();
    }
   // Start the DIFs
  boost::thread_group g;
  for (std::vector<fsmwebClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
      //std::cout<<"Creating thread"<<std::endl;
      g.create_thread(boost::bind(&FullDaq::singlestart, this,(*it)));
    }
  g.join_all();
  //::sleep(5);
  // Start the builder
   if (_builderClient)
    {
      Json::Value jl;
      jl["run"]=_run;
      _builderClient->sendTransition("START");
    }
  //Start the CCC
   if (_cccClient)
     {
       _cccClient->sendTransition("START");
     }
  // Resume the MDCC
   if (_mdccClient)
     {
       //_mdccClient->sendTransition("RESET");
       //_mdccClient->sendTransition("RESUME");
     }

   m->setAnswer(toJson(this->difstatus()));  
}
void FullDaq::stop(levbdim::fsmmessage* m)
{
    // Pause the MDCC
   if (_mdccClient)
     {
       _mdccClient->sendTransition("PAUSE");
     }

    //Stop the CCC
   if (_cccClient)
     {
       _cccClient->sendTransition("STOP");
     }

   // Stop the DIFs
  boost::thread_group g;
  for (std::vector<fsmwebClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
      g.create_thread(boost::bind(&FullDaq::singlestop, this,(*it)));
    }
  g.join_all();

  ::sleep(1);
  // Stop the builder
   if (_builderClient)
    {
      _builderClient->sendTransition("STOP");
    }

   m->setAnswer(toJson(this->difstatus()));  
}

void FullDaq::destroy(levbdim::fsmmessage* m)
{
  for (std::vector<fsmwebClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
      Json::Value jl;
      jl["difid"]=0;
      (*it)->sendTransition("DESTROY",jl);
    } 
  
}

FullDaq::~FullDaq() 
{
  if (_dbClient) delete _dbClient;
  if (_cccClient) delete _cccClient;
  if (_zupClient) delete _zupClient;
  if (_gpioClient) delete _gpioClient;
  if (_mdccClient) delete _mdccClient;
  if (_builderClient) delete _builderClient;
  for (std::vector<fsmwebClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    delete (*it);
  _DIFClients.clear();
}
void FullDaq::doubleSwitchZup(Mongoose::Request &request, Mongoose::JsonResponse &response)
  {
    if (_zupClient==NULL && _gpioClient==NULL)
      {
	LOG4CXX_ERROR(_logLdaq, "No zup or GPIO client");
	response["STATUS"]="NO Zup/GPIO CLient";
	return;
      }
    uint32_t npause=atoi(request.get("pause","2").c_str());
    if (_zupClient!=NULL){
      
    _zupClient->sendTransition("OFF");
    sleep((unsigned int) 2);
    _zupClient->sendTransition("ON");
    sleep((unsigned int)npause);
    _zupClient->sendTransition("OFF");
    sleep((unsigned int) 2);
    _zupClient->sendTransition("ON");
    ::sleep( npause);
    std::cout<<" LV is ON"<<std::endl;
    response["STATUS"]="DONE";

    response["DOUBLESWITCHZUP"]="ON";
    }
    else
      {
	      
    _gpioClient->sendTransition("OFF");
    sleep((unsigned int) 2);
    _gpioClient->sendTransition("ON");
    sleep((unsigned int)npause);
    _gpioClient->sendTransition("OFF");
    sleep((unsigned int) 2);
    _gpioClient->sendTransition("ON");
    ::sleep( npause);
    std::cout<<" LV is ON"<<std::endl;
    response["STATUS"]="DONE";

    response["DOUBLESWITCHZUP"]="ON";
      }
  }
void  FullDaq::LVStatus(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_zupClient)
    {
      Json::FastWriter fastWriter;
      _zupClient->sendTransition("READ");
      response["STATUS"]="DONE";
      response["LVSTATUS"]=_zupClient->answer();
      return;
    }
   if (_gpioClient)
    {
      Json::FastWriter fastWriter;
      _gpioClient->sendTransition("READ");
      response["STATUS"]="DONE";
      response["LVSTATUS"]=_gpioClient->answer();
      return;
    }
  response["STATUS"]="NO Zup Client";


}
void FullDaq::LVON(Mongoose::Request &request, Mongoose::JsonResponse &response)
  {
    if (_zupClient!=NULL){

    _zupClient->sendTransition("ON");
     response["STATUS"]="DONE";
     response["LVON"]=_zupClient->answer();
     return;
    }
    if (_gpioClient!=NULL){

    _gpioClient->sendTransition("ON");
     response["STATUS"]="DONE";
     response["LVON"]=_gpioClient->answer();
     return;
    }
     LOG4CXX_ERROR(_logLdaq, "No zup client");response["STATUS"]="NO Zup CLient";return;
  }
void FullDaq::forceState(Mongoose::Request &request, Mongoose::JsonResponse &response)
  {
    
    std::string states=request.get("state",_fsm->state());
    _fsm->setState(states);
     response["STATUS"]="DONE";
     response["NEWSTATE"]=states;
  }
void FullDaq::LVOFF(Mongoose::Request &request, Mongoose::JsonResponse &response)
  {
        if (_zupClient!=NULL){

    _zupClient->sendTransition("OFF");
     response["STATUS"]="DONE";
     response["LVON"]=_zupClient->answer();
     return;
    }
    if (_gpioClient!=NULL){

    _gpioClient->sendTransition("OFF");
     response["STATUS"]="DONE";
     response["LVON"]=_gpioClient->answer();
     return;
    }
     LOG4CXX_ERROR(_logLdaq, "No zup client");response["STATUS"]="NO Zup CLient";return;
     
   

  }



void FullDaq::downloadDB(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  std::string statereq=request.get("state","NONE");
  if (statereq.compare("NONE")==0)
    {
      response["STATUS"]="NO STATE GIVEN";
      return;
    }
  Json::FastWriter fastWriter;
  Json::Value fromScratch;
  fromScratch.clear();
  if (_dbClient==NULL){LOG4CXX_ERROR(_logLdaq, "No DB client"); response["STATUS"]="NO DB Client";return;}
  _dbstate=statereq;
  this->parameters()["dbstate"]=statereq;

  _dbClient->sendTransition("DELETE",this->parameters());
  std::cout<<" Downloading"<<this->parameters()["dbstate"].asString()<<std::endl;
  
  _dbClient->sendTransition("DOWNLOAD",this->parameters());
   response["STATUS"]="DONE";
   response["DOWNLOADBD"]=_dbstate;
}

void FullDaq::setControlRegister(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t reg=atoi(request.get("value","0").c_str());
  this->parameters()["ctrlreg"]=reg;
 
  response["STATUS"]="DONE";
  response["CTRLREG"]=reg;
  std::cout<<" Chamge Ctrlreg to "<<std::hex<<_ctrlreg<<std::dec<<std::endl;
}

void FullDaq::dbStatus(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["run"]=_run;
  response["state"]=_dbstate;
  response["STATUS"]="DONE";
}
void FullDaq::registerDataSource(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t detid=atoi(request.get("detid","0").c_str());
  uint32_t sid=atoi(request.get("sourceid","0").c_str());
  if (detid==0 || sid==0)
    {
      response["STATUS"]="Missing detid or sourceid";
      return;
    }
  if (_dbClient==NULL){LOG4CXX_ERROR(_logLdaq, "No DB client"); response["STATUS"]="NO DB Client";return;}
  Json::Value jl;
  jl["detid"]=detid;
  jl["sourceid"]=sid;
  
  _builderClient->sendTransition("REGISTERDS",jl);
   response["STATUS"]="DONE";
   response["IDS"]=sid;
}

void  FullDaq::builderStatus(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  Json::FastWriter fastWriter;
  Json::Value fromScratch;
  response["run"]=-1;
  response["event"]=-1;
  
  if (_builderClient==NULL){LOG4CXX_ERROR(_logLdaq, "No SHM client");response["STATUS"]= "No SHM client";return;}
  _builderClient->sendTransition("STATUS");
  response["run"]=_builderClient->answer()["run"];
  response["event"]=_builderClient->answer()["event"];
  response["STATUS"]="DONE";
}

void FullDaq::status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  Json::Value devlist;
  for (std::vector<fsmwebClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {

      (*it)->sendTransition("STATUS");
      const Json::Value& jdevs=(*it)->answer();
      for (Json::ValueConstIterator jt = jdevs.begin(); jt != jdevs.end(); ++jt)
	devlist.append(*jt);
    }
  response["diflist"]=devlist;
  response["STATUS"]="DONE";
}
void FullDaq::pauseTrigger(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq, "No MDC client");response["STATUS"]= "No MDCC client";return;}
  _mdccClient->sendTransition("PAUSE");
  response["TRIGGER"]="PAUSED";
  response["STATUS"]="DONE";
}
void FullDaq::resumeTrigger(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq, "No MDC client");response["STATUS"]= "No MDCC client";return;}
  _mdccClient->sendTransition("RESUME");
  response["TRIGGER"]="RESUMED";
  response["STATUS"]="DONE";
}

void FullDaq::pauseEcal(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq, "No MDC client");response["STATUS"]= "No MDCC client";return;}
  _mdccClient->sendTransition("ECALPAUSE");
  response["ECAL"]="PAUSED";
  response["STATUS"]="DONE";

}
void FullDaq::resumeEcal(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq, "No MDC client");response["STATUS"]= "No MDCC client";return;}
  _mdccClient->sendTransition("ECALRESUME");
  response["ECAL"]="RESUMED";
  response["STATUS"]="DONE";
}
void FullDaq::resetTriggerCounters(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq, "No MDC client");response["STATUS"]= "No MDCC client";return;}
  _mdccClient->sendTransition("RESET");
  response["COUNTERS"]="RESETTED";
  response["STATUS"]="DONE";
}
// a continuer
void FullDaq::triggerStatus(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq, "No MDC client");response["STATUS"]= "No MDCC client";return;}
  _mdccClient->sendCommand("STATUS");
  response["COUNTERS"]=_mdccClient->answer()["COUNTERS"];
  response["STATUS"]="DONE";
}
void FullDaq::triggerSpillOn(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{
  uint32_t nc=atoi(request.get("clock","50").c_str());
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq, "No MDC client");response["STATUS"]= "No MDCC client";return;}
 
  std::stringstream sp;sp<<"&nclock="<<nc;
  _mdccClient->sendCommand("SPILLON",sp.str());
  
  response["SPILLON"]=nc;
  response["STATUS"]="DONE";
  return;
}
void FullDaq::triggerSpillOff(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{
  uint32_t nc=atoi(request.get("clock","50000").c_str());
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq, "No MDC client");response["STATUS"]= "No MDCC client";return;}
   std::stringstream sp;sp<<"&nclock="<<nc;
  _mdccClient->sendCommand("SPILLOFF",sp.str());
  
  response["SPILLOFF"]=nc;
  response["STATUS"]="DONE";
  
  return;
}
void FullDaq::triggerBeam(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{
  uint32_t nc=atoi(request.get("clock","250000000").c_str());
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq, "No MDC client");response["STATUS"]= "No MDCC client";return;}
   std::stringstream sp;sp<<"&nclock="<<nc;
  _mdccClient->sendCommand("BEAMON",sp.str());
  response["BEAM"]=nc;
  response["STATUS"]="DONE";
  
  return;
}

    


void FullDaq::setThreshold(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{
  if (state().compare("CONFIGURED")!=0)
    {
      response["STATUS"]="SetThreshold cannot be called if not in CONFIGURED state";
      return;
    }
  std::stringstream saction;
  saction<<"&B0="<<request.get("B0","250");
  saction<<"&B1="<<request.get("B1","250");
  saction<<"&B2="<<request.get("B2","250");
  saction<<"&CTRLREG="<<std::hex<<this->parameters()["ctrlreg"].asUInt();

  Json::Value jlist;
 for (std::vector<fsmwebClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
      
      std::string rep=(*it)->sendCommand("SETTHRESHOLD",saction.str());
      Json::Value jrep;
      jrep["diflist"]=rep;
      jlist.append(jrep);
    }
 response["HOSTS"]=jlist;
 response["STATUS"]="DONE";
}

void FullDaq::setGain(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{
  if (state().compare("CONFIGURED")!=0)
    {
      response["STATUS"]="SetGAIN cannot be called if not in CONFIGURED state";
      return;
    }

  std::stringstream saction;
  saction<<"&GAIN="<<request.get("GAIN","128");
  saction<<"&CTRLREG="<<std::hex<<this->parameters()["ctrlreg"].asUInt();
  Json::Value jlist;
 for (std::vector<fsmwebClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
      
      std::string rep=(*it)->sendCommand("SETGAIN",saction.str());
      Json::Value jrep;
      jrep["diflist"]=rep;
      jlist.append(jrep);
    }
 response["HOSTS"]=jlist;
 response["STATUS"]="DONE";
  
  
}