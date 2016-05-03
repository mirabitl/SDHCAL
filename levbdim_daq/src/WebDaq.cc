#include "WebDaq.hh"
#include <unistd.h>
#include <stdint.h>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include "LdaqLogger.hh"

WebDaq::WebDaq(std::string name,uint32_t port) :_builderClient(NULL),_dbClient(NULL),_cccClient(NULL),_mdccClient(NULL),_zupClient(NULL)
  {
    _DIFClients.clear();
    
    _fsm=new fsmweb(name);
    
    _fsm->addState("CREATED");
    _fsm->addState("DISCOVERED");
    _fsm->addState("PREPARED");
    _fsm->addState("INITIALISED");
    _fsm->addState("CONFIGURED");
    _fsm->addState("RUNNING");

    _fsm->addTransition("DISCOVER","CREATED","DISCOVERED",boost::bind(&WebDaq::discover, this,_1));
    _fsm->addTransition("PREPARE","DISCOVERED","PREPARED",boost::bind(&WebDaq::prepare, this,_1));
    _fsm->addTransition("PREPARE","PREPARED","PREPARED",boost::bind(&WebDaq::prepare, this,_1));
    _fsm->addTransition("INITIALISE","PREPARED","INITIALISED",boost::bind(&WebDaq::initialise, this,_1));
    _fsm->addTransition("CONFIGURE","INITIALISED","CONFIGURED",boost::bind(&WebDaq::configure, this,_1));
    _fsm->addTransition("CONFIGURE","CONFIGURED","CONFIGURED",boost::bind(&WebDaq::configure, this,_1));
    _fsm->addTransition("START","CONFIGURED","RUNNING",boost::bind(&WebDaq::start, this,_1));
    _fsm->addTransition("STOP","RUNNING","CONFIGURED",boost::bind(&WebDaq::stop, this,_1));
    _fsm->addTransition("DESTROY","CONFIGURED","PREPARED",boost::bind(&WebDaq::destroy, this,_1));

    // Commands

    _fsm->addCommand("DOUBLESWITCHZUP",boost::bind(&WebDaq::doubleSwitchZup,this,_1,_2));
    _fsm->addCommand("LVSTATUS",boost::bind(&WebDaq::LVStatus,this,_1,_2));
    _fsm->addCommand("LVON",boost::bind(&WebDaq::LVON,this,_1,_2));
    _fsm->addCommand("LVOFF",boost::bind(&WebDaq::LVOFF,this,_1,_2));
    _fsm->addCommand("SETPAR",boost::bind(&WebDaq::setParameters,this,_1,_2));
    _fsm->addCommand("GETPAR",boost::bind(&WebDaq::getParameters,this,_1,_2));
    _fsm->addCommand("DOWNLOADDB",boost::bind(&WebDaq::downloadDB,this,_1,_2));
    _fsm->addCommand("CTRLREG",boost::bind(&WebDaq::setControlRegister,this,_1,_2));
    _fsm->addCommand("DBSTATUS",boost::bind(&WebDaq::dbStatus,this,_1,_2));
    _fsm->addCommand("EVBSTATUS",boost::bind(&WebDaq::builderStatus,this,_1,_2));
    _fsm->addCommand("DIFSTATUS",boost::bind(&WebDaq::status,this,_1,_2));
    _fsm->addCommand("PAUSE",boost::bind(&WebDaq::pauseTrigger,this,_1,_2));
    _fsm->addCommand("RESUME",boost::bind(&WebDaq::resumeTrigger,this,_1,_2));
    _fsm->addCommand("ECALPAUSE",boost::bind(&WebDaq::pauseEcal,this,_1,_2));
    _fsm->addCommand("ECALRESUME",boost::bind(&WebDaq::resumeEcal,this,_1,_2));
    _fsm->addCommand("RESETCOUNTERS",boost::bind(&WebDaq::resetTriggerCounters,this,_1,_2));
    _fsm->addCommand("TRIGGERSTATUS",boost::bind(&WebDaq::triggerStatus,this,_1,_2));
    _fsm->addCommand("SPILLON",boost::bind(&WebDaq::triggerSpillOn,this,_1,_2));
    _fsm->addCommand("SPILLOFF",boost::bind(&WebDaq::triggerSpillOff,this,_1,_2));
    _fsm->addCommand("BEAMON",boost::bind(&WebDaq::triggerBeam,this,_1,_2));
    _fsm->addCommand("SETTHRESHOLD",boost::bind(&WebDaq::setThreshold,this,_1,_2));
    _fsm->addCommand("SETGAIN",boost::bind(&WebDaq::setGain,this,_1,_2));
    
  cout<<"Building WebDaq"<<endl;
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0.str(std::string());
  s0<<"WebDaq-"<<hname;

  DimServer::start(s0.str().c_str()); 
  //  cout<<"Starting DimDaqCtrl"<<endl;
  _fsm->start(port);
  }



std::string WebDaq::process(std::string command,std::string param)
{
  // Build the message
  Json::Value jmsg;
  jmsg["command"]=command;
  Json::Value jcont;
  // Parse the param string
  Json::Reader reader;
  bool parsingSuccessful = reader.parse(param, jcont);
  jmsg["content"]=jcont;
  Json::FastWriter fastWriter;
  levbdim::fsmmessage m;
  m.setValue(fastWriter.write(jmsg));
  _fsm->processCommand(&m);
  return m.value();
}

std::string WebDaq::process(std::string command)
{
  // Build the message
  Json::Value jmsg;
  jmsg["command"]=command;
  jmsg["content"]=_jparam;
  Json::FastWriter fastWriter;
  levbdim::fsmmessage m;
  m.setValue(fastWriter.write(jmsg));
  _fsm->processCommand(&m);
  return m.value();
}

LClient* WebDaq::findFSM(DimBrowser* dbr,std::string pattern)
{
  LClient* res=NULL;
  char *service, *format;
  int type;
  dbr->getServices(pattern.c_str());
  std::stringstream s0;
  while(type = dbr->getNextService(service, format)) 
    { 
      cout << service << " -  " << format << endl; 
      std::string ss;
      ss.assign(service);
      size_t n=ss.find("/STATE");
      s0.str(std::string());
      s0<<ss.substr(0,n)<<"/CMD";
      res = new LClient(s0.str());
      break;
    }
  return res;
}
void WebDaq::discover(levbdim::fsmmessage* m)
{
  //std::cout<<"doScandns"<<std::endl;
  // Look for DB server
  DimBrowser* dbr=new DimBrowser(); 
  char *service, *format; 
  int type;
  char *server,*node;
  dbr->getServers( ); 
  while(dbr->getNextServer(server, node)) 
    { 
      cout << server << " @ " << node << endl; 
    }
    

  _dbClient=findFSM(dbr,"/FSM/Db*/STATE" );
  _cccClient=findFSM(dbr,"/FSM/Ccc*/STATE" );
  _mdccClient=findFSM(dbr,"/FSM/Mdcc*/STATE" );
  _zupClient=findFSM(dbr,"/FSM/Zup*/STATE" );
  _builderClient=findFSM(dbr,"/FSM/Builder*/STATE" ); 
  _DIFClients.clear();
  dbr->getServices("/FSM/DIF*/STATE" );
  std::stringstream s0;

  while(type = dbr->getNextService(service, format)) 
    { 
      cout << service << " -  " << format << endl; 
      std::string ss;
      ss.assign(service);
      size_t n=ss.find("/STATE");
      s0.str(std::string());
      s0<<ss.substr(0,n)<<"/CMD";
      LClient* dc = new LClient(s0.str());
      dc->post("?");
      _DIFClients.push_back(dc);
      
    } 




}
void WebDaq::prepare(levbdim::fsmmessage* m)
{
  //std::cout<<"ON RENTREEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"<<std::endl;
  m->content();
  // DB
  if (_dbClient)
    {
      _dbClient->clear();
      _dbClient->set<std::string>("dbstate",m->content()["dbstate"].asString());
      _dbClient->post("DOWNLOAD");
    }
  // Zup
  if (_zupClient)
    {
      _zupClient->clear();
      _zupClient->set<std::string>("device",m->content()["zupdevice"].asString());
      _zupClient->set<int>("port",m->content()["zupport"].asInt());
      _zupClient->post("CONFIGURE");
      //std::cout<<"Current zup values "<<_zupClient->reply()<<std::endl;
    }
  // Ccc
  //std::cout<<" CCC client "<<_cccClient<<std::endl;
  if (_cccClient)
    {
      _cccClient->clear();
      _cccClient->set<std::string>("device",m->content()["dccname"].asString());
      _cccClient->post("OPEN");
      _cccClient->post("INITIALISE");
    }
  // Mdc
  if (_mdccClient)
    {
      _mdccClient->clear();
      _mdccClient->set<std::string>("device",m->content()["mdccname"].asString());
      _mdccClient->post("OPEN");
      //_mdccClient->clear();
      // Stop trigger ane reset counters
      //_mdccClient->post("PAUSE");
      //_mdccClient->post("RESET");
      _mdccClient->clear();
      _mdccClient->set<std::string>("name","STATUS");
      _mdccClient->post("CMD");
      //std::cout<<"Current MDCC values "<<_mdccClient->reply()<<std::endl;
      
    }
  // Builder
  if (_builderClient)
    {
      _builderClient->clear();
      //_builderClient->set<std::string>("proctype",m->content()["proctype"].asString());
      _builderClient->set<Json::Value>("proclist",m->content()["proclist"]);
      _builderClient->set<std::string>("filepath",m->content()["filepath"].asString());
      _builderClient->set<std::string>("memorypath",m->content()["memorypath"].asString());
      _builderClient->post("INITIALISE");

    }
}

std::string WebDaq::difstatus()
{
  Json::Value devlist;
  for (std::vector<LClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
      (*it)->clear();
      (*it)->post("STATUS");
      const Json::Value& jdevs=(*it)->reply();
      for (Json::ValueConstIterator jt = jdevs.begin(); jt != jdevs.end(); ++jt)
	devlist.append(*jt);
    }
  //std::cout<<devlist<<std::endl;
  Json::FastWriter fastWriter;
  return fastWriter.write(devlist);
}

void WebDaq::singlescan(LClient* d)
{
  d->clear();
  d->post("SCAN");
}
void WebDaq::singleinit(LClient* d)
{
  d->clear();
  d->set<int>("difid",0);
  d->post("INITIALISE");
}

void WebDaq::singleregisterdb(LClient* d)
{
  d->post("REGISTERDB");
  //  std::cout<<"Register DB"<<d->parameters()<<std::endl;
}
void WebDaq::singleconfigure(LClient* d)
{
  d->post("CONFIGURE");
}
void WebDaq::singlestart(LClient* d)
{
  d->clear();
  d->set<int>("difid",0);
  //std::cout<<"Posting START"<<std::endl;
  d->post("START");
  //std::cout<<"received "<<d->reply()<<std::endl;
}
void WebDaq::singlestop(LClient* d)
{
  d->clear();
  d->set<int>("difid",0);
  d->post("STOP");
}

Json::Value WebDaq::toJson(std::string s)
{
  Json::Reader reader;
  Json::Value jsta;
  bool parsingSuccessful = reader.parse(s,jsta);
  return jsta;
}
void WebDaq::initialise(levbdim::fsmmessage* m)
{
  // Configure CCC
  if (_cccClient)
    {
      _cccClient->clear();
      _cccClient->post("CONFIGURE");
      _cccClient->post("STOP");
      ::sleep(1);
      _cccClient->clear();_cccClient->set<std::string>("name","CCCRESET");_cccClient->post("CMD");
      _cccClient->clear();_cccClient->set<std::string>("name","DIFRESET");_cccClient->post("CMD");
      ::sleep(1);
    }

  // Make a DIF SCAN
  boost::thread_group g;
  for (std::vector<LClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
      g.create_thread(boost::bind(&WebDaq::singlescan, this,(*it)));
  g.join_all();

  // Merge devlist
  Json::Value devlist;
  for (std::vector<LClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
       const Json::Value& jdevs=(*it)->reply();
      for (Json::ValueConstIterator jt = jdevs.begin(); jt != jdevs.end(); ++jt)
	devlist.append(*jt);
    }
  //std::cout<<devlist<<std::endl;

  // Initialise
  for (std::vector<LClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
      g.create_thread(boost::bind(&WebDaq::singleinit, this,(*it)));
  g.join_all();
  

  // Fill status
  m->setAnswer(toJson(this->difstatus()));
}

void WebDaq::configure(levbdim::fsmmessage* m)
{
  // Configure CCC
  //std::cout<<m->content();
  if (_cccClient)
    {

      _cccClient->clear();_cccClient->set<std::string>("name","CCCRESET");_cccClient->post("CMD");
      _cccClient->clear();_cccClient->set<std::string>("name","DIFRESET");_cccClient->post("CMD");
      //std::cout<<"RESET DONE"<<std::endl;
    }

  // register to the dbstate
  boost::thread_group g;
  for (std::vector<LClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
      (*it)->clear();
      //(*it)->set<std::string>("dbstate",m->content()["dbstate"].asString());
      (*it)->set<std::string>("dbstate",_jparam["dbstate"].asString());
      g.create_thread(boost::bind(&WebDaq::singleregisterdb, this,(*it)));
    }
  g.join_all();
  std::cout<<"REGISTER DB DONE"<<std::endl;
  ::sleep(2);
  //Configure them
  for (std::vector<LClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
      (*it)->clear();
      //(*it)->set<uint32_t>("ctrlreg",m->content()["ctrlreg"].asUInt());
      (*it)->set<uint32_t>("ctrlreg",_jparam["ctrlreg"].asUInt());
      (*it)->set<int32_t>("difid",0);
      g.create_thread(boost::bind(&WebDaq::singleconfigure, this,(*it)));
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
      _builderClient->clear();
      _builderClient->set<Json::Value>("sources",jsou);
      _builderClient->post("CONFIGURE");
    }
  m->setAnswer(jsta);
}

void WebDaq::start(levbdim::fsmmessage* m)
{
  // Get the new run number
   if (_dbClient)
    {
      _dbClient->clear();
      _dbClient->post("NEWRUN");
      _run=_dbClient->reply()["run"].asInt();
    }
   // Start the DIFs
  boost::thread_group g;
  for (std::vector<LClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
      //std::cout<<"Creating thread"<<std::endl;
      (*it)->clear();
      g.create_thread(boost::bind(&WebDaq::singlestart, this,(*it)));
    }
  g.join_all();
  //::sleep(5);
  // Start the builder
   if (_builderClient)
    {
      _builderClient->clear();
      _builderClient->set<int>("run",_run);
      _builderClient->post("START");
    }
  //Start the CCC
   if (_cccClient)
     {
       _cccClient->post("START");
     }
  // Resume the MDCC
   if (_mdccClient)
     {
       //_mdccClient->post("RESET");
       //_mdccClient->post("RESUME");
     }

   m->setAnswer(toJson(this->difstatus()));  
}
void WebDaq::stop(levbdim::fsmmessage* m)
{
    // Pause the MDCC
   if (_mdccClient)
     {
       _mdccClient->post("PAUSE");
     }

    //Stop the CCC
   if (_cccClient)
     {
       _cccClient->post("STOP");
     }

   // Stop the DIFs
  boost::thread_group g;
  for (std::vector<LClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
      (*it)->clear();
      g.create_thread(boost::bind(&WebDaq::singlestop, this,(*it)));
    }
  g.join_all();

  ::sleep(1);
  // Stop the builder
   if (_builderClient)
    {
      _builderClient->post("STOP");
    }

   m->setAnswer(toJson(this->difstatus()));  
}

void WebDaq::destroy(levbdim::fsmmessage* m)
{
  for (std::vector<LClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
      (*it)->clear();
      (*it)->set<int>("difid",0);

      (*it)->post("DESTROY");
    } 
  
}

WebDaq::~WebDaq() 
{
  if (_dbClient) delete _dbClient;
  if (_cccClient) delete _cccClient;
  if (_zupClient) delete _zupClient;
  if (_mdccClient) delete _mdccClient;
  if (_builderClient) delete _builderClient;
  for (std::vector<LClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    delete (*it);
  _DIFClients.clear();
}
void WebDaq::doubleSwitchZup(Mongoose::Request &request, Mongoose::JsonResponse &response)
  {
    if (_zupClient==NULL)
      {
	LOG4CXX_ERROR(_logLdaq, "No zup client");
	response["STATUS"]="NO Zup CLient";
	return;
      }
    uint32_t npause=atoi(request.get("pause","2").c_str());
    _zupClient->post("OFF");
    sleep((unsigned int) 2);
    _zupClient->post("ON");
    sleep((unsigned int)npause);
    _zupClient->post("OFF");
    sleep((unsigned int) 2);
    _zupClient->post("ON");
    ::sleep( npause);
    std::cout<<" LV is ON"<<std::endl;
    response["STATUS"]="DONE";

    response["DOUBLESWITCHZUP"]="ON";

  }
void  WebDaq::LVStatus(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_zupClient)
    {
      Json::FastWriter fastWriter;
      _zupClient->post("READ");
      response["STATUS"]="DONE";
      response["LVSTATUS"]=_zupClient->reply();
      return;
    }
  response["STATUS"]="NO Zup Client";


}
void WebDaq::LVON(Mongoose::Request &request, Mongoose::JsonResponse &response)
  {
    if (_zupClient==NULL){LOG4CXX_ERROR(_logLdaq, "No zup client");response["STATUS"]="NO Zup CLient";return;}

    _zupClient->post("ON");
     response["STATUS"]="DONE";
     response["LVON"]=_zupClient->reply();
  }
void WebDaq::LVOFF(Mongoose::Request &request, Mongoose::JsonResponse &response)
  {
    if (_zupClient==NULL){LOG4CXX_ERROR(_logLdaq, "No zup client");response["STATUS"]="NO Zup CLient";return;}
     _zupClient->post("OFF");
     response["STATUS"]="DONE";
     response["LVOFF"]=_zupClient->reply();

  }

void WebDaq::setParameters(Mongoose::Request &request, Mongoose::JsonResponse &response)//std::string jsonString)
  {
    _strParam=request.get("params","{}");

    Json::Reader reader;
    bool parsingSuccessful = reader.parse(_strParam, _jparam);
    
    if (parsingSuccessful)
      {
	Json::StyledWriter styledWriter;
	std::cout << styledWriter.write(_jparam) << std::endl;
      }
    else
      std::cout<<"Parsing failed"<<std::endl;
    _dbstate=_jparam["dbstate"].asString();
    _dccname=_jparam["dccname"].asString();
    _mdccname=_jparam["mdccname"].asString();
    _zupdevice=_jparam["zupdevice"].asString();
    _writerdir=_jparam["filepath"].asString();;
    _memorydir=_jparam["memorypath"].asString();;
    _proctype=_jparam["proctype"].asString();;
    _proclist=_jparam["proclist"];
    _zupport=_jparam["zupport"].asUInt();
    _ctrlreg=_jparam["ctrlreg"].asUInt();
    response["dbstate"]=_dbstate;
    response["dccname"]=_dccname;
    response["mdccname"]=_mdccname;
    response["zupdevice"]=_zupdevice;
    response["filepath"]=_writerdir;
    response["memorypath"]=_memorydir;
    response["proclist"]=_proclist;
    response["zupport"]=_zupport;
    response["ctrlreg"]=_ctrlreg;
    response["STATUS"]="DONE";
    std::cout<<"RETRUEN CODE "<<response<<std::endl;
  }
void WebDaq::getParameters(Mongoose::Request &request, Mongoose::JsonResponse &response)//std::string jsonString)
{
  response["dbstate"]=_dbstate;
  response["dccname"]=_dccname;
  response["mdccname"]=_mdccname;
  response["zupdevice"]=_zupdevice;
  response["filepath"]=_writerdir;
  response["memorypath"]=_memorydir;
  response["proclist"]=_proclist;
  response["zupport"]=_zupport;
  response["ctrlreg"]=_ctrlreg;
  response["STATUS"]="DONE";
}
void WebDaq::downloadDB(Mongoose::Request &request, Mongoose::JsonResponse &response)
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
  _jparam["dbstate"]=statereq;

  _dbClient->clear();
  _dbClient->set<std::string>("dbstate",_jparam["dbstate"].asString());
  _dbClient->post("DELETE");
  std::cout<<" Downloading"<<_jparam["dbstate"].asString()<<std::endl;
  _dbClient->clear();
  _dbClient->set<std::string>("dbstate",_jparam["dbstate"].asString());
  _dbClient->post("DOWNLOAD");
   response["STATUS"]="DONE";
   response["DOWNLOADBD"]=_dbstate;
}

void WebDaq::setControlRegister(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t reg=atoi(request.get("value","0").c_str());
  _jparam["ctrlreg"]=reg;
  _ctrlreg=reg;
  response["STATUS"]="DONE";
  response["CTRLREG"]=reg;
  std::cout<<" Chamge Ctrlreg to "<<std::hex<<_ctrlreg<<std::dec<<std::endl;
}

void WebDaq::dbStatus(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["run"]=_run;
  response["state"]=_dbstate;
  response["STATUS"]="DONE";
}

void  WebDaq::builderStatus(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  Json::FastWriter fastWriter;
  Json::Value fromScratch;
  response["run"]=-1;
  response["event"]=-1;
  
  if (_builderClient==NULL){LOG4CXX_ERROR(_logLdaq, "No SHM client");response["STATUS"]= "No SHM client";return;}
  _builderClient->post("STATUS");
  response["run"]=_builderClient->reply()["run"];
  response["event"]=_builderClient->reply()["event"];
  response["STATUS"]="DONE";
}

void WebDaq::status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  Json::Value devlist;
  for (std::vector<LClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
      (*it)->clear();
      (*it)->post("STATUS");
      const Json::Value& jdevs=(*it)->reply();
      for (Json::ValueConstIterator jt = jdevs.begin(); jt != jdevs.end(); ++jt)
	devlist.append(*jt);
    }
  response["diflist"]=devlist;
  response["STATUS"]="DONE";
}
void WebDaq::pauseTrigger(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq, "No MDC client");response["STATUS"]= "No MDCC client";return;}
  _mdccClient->post("PAUSE");
  response["TRIGGER"]="PAUSED";
  response["STATUS"]="DONE";
}
void WebDaq::resumeTrigger(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq, "No MDC client");response["STATUS"]= "No MDCC client";return;}
  _mdccClient->post("RESUME");
  response["TRIGGER"]="RESUMED";
  response["STATUS"]="DONE";
}

void WebDaq::pauseEcal(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq, "No MDC client");response["STATUS"]= "No MDCC client";return;}
  _mdccClient->post("ECALPAUSE");
  response["ECAL"]="PAUSED";
  response["STATUS"]="DONE";

}
void WebDaq::resumeEcal(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq, "No MDC client");response["STATUS"]= "No MDCC client";return;}
  _mdccClient->post("ECALRESUME");
  response["ECAL"]="RESUMED";
  response["STATUS"]="DONE";
}
void WebDaq::resetTriggerCounters(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq, "No MDC client");response["STATUS"]= "No MDCC client";return;}
  _mdccClient->post("RESET");
  response["COUNTERS"]="RESETTED";
  response["STATUS"]="DONE";
}
  
void WebDaq::triggerStatus(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq, "No MDC client");response["STATUS"]= "No MDCC client";return;}
  _mdccClient->clear();
  _mdccClient->set<std::string>("name","STATUS");
  _mdccClient->post("CMD");
  response["COUNTERS"]=_mdccClient->reply();
  response["STATUS"]="DONE";
}
void WebDaq::triggerSpillOn(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{
  uint32_t nc=atoi(request.get("clock","50").c_str());
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq, "No MDC client");response["STATUS"]= "No MDCC client";return;}
  _mdccClient->clear();
  _mdccClient->set<std::string>("name","SPILLON");
  _mdccClient->set<uint32_t>("nclock",nc);
  _mdccClient->post("CMD");
  response["SPILLON"]=nc;
  response["STATUS"]="DONE";
  return;
}
void WebDaq::triggerSpillOff(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{
  uint32_t nc=atoi(request.get("clock","50000").c_str());
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq, "No MDC client");response["STATUS"]= "No MDCC client";return;}
  _mdccClient->clear();
  _mdccClient->set<std::string>("name","SPILLOFF");
  _mdccClient->set<uint32_t>("nclock",nc);
  _mdccClient->post("CMD");
  response["SPILLOFF"]=nc;
  response["STATUS"]="DONE";
  
  return;
}
void WebDaq::triggerBeam(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{
  uint32_t nc=atoi(request.get("clock","250000000").c_str());
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq, "No MDC client");response["STATUS"]= "No MDCC client";return;}
  _mdccClient->clear();
  _mdccClient->set<std::string>("name","BEAM");
  _mdccClient->set<uint32_t>("nclock",nc);
  _mdccClient->post("CMD");
  response["BEAM"]=nc;
  response["STATUS"]="DONE";
  
  return;
}

    


void WebDaq::setThreshold(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{
  if (state().compare("CONFIGURED")!=0)
    {
      response["STATUS"]="SetThreshold cannot be called if not in CONFIGURED state";
      return;
    }
  std::stringstream saction;
  saction<<"CMD?name=SETTHRESHOLD&B0="<<request.get("B0","250");
  saction<<"&B1="<<request.get("B1","250");
  saction<<"&B2="<<request.get("B2","250");
  saction<<"&CTRLREG="<<std::hex<<_jparam["ctrlreg"].asUInt();

  Json::Value jlist;
 for (std::vector<LClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
      
      std::string rep=(*it)->postweb(saction.str());
      Json::Value jrep;
      jrep["diflist"]=rep;
      jlist.append(jrep);
    }
 response["HOSTS"]=jlist;
 response["STATUS"]="DONE";
}

void WebDaq::setGain(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{
  if (state().compare("CONFIGURED")!=0)
    {
      response["STATUS"]="SetGAIN cannot be called if not in CONFIGURED state";
      return;
    }

  std::stringstream saction;
  saction<<"CMD?name=SETGAIN&GAIN="<<request.get("GAIN","128");
  saction<<"&CTRLREG="<<std::hex<<_jparam["ctrlreg"].asUInt();
  Json::Value jlist;
 for (std::vector<LClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
      
      std::string rep=(*it)->postweb(saction.str());
      Json::Value jrep;
      jrep["diflist"]=rep;
      jlist.append(jrep);
    }
 response["HOSTS"]=jlist;
 response["STATUS"]="DONE";
  
  
}
