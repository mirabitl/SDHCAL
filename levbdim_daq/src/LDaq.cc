#include "LDaq.hh"
#include <unistd.h>
#include <stdint.h>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include "LdaqLogger.hh"

LDaq::LDaq() :_builderClient(NULL),_dbClient(NULL),_cccClient(NULL),_mdccClient(NULL),_zupClient(NULL)
  {
    _DIFClients.clear();
    
    _fsm=new levbdim::fsm("LDAQ");
    
    _fsm->addState("CREATED");
    _fsm->addState("DISCOVERED");
    _fsm->addState("PREPARED");
    _fsm->addState("INITIALISED");
    _fsm->addState("CONFIGURED");
    _fsm->addState("RUNNING");

    _fsm->addTransition("DISCOVER","CREATED","DISCOVERED",boost::bind(&LDaq::discover, this,_1));
    _fsm->addTransition("PREPARE","DISCOVERED","PREPARED",boost::bind(&LDaq::prepare, this,_1));
    _fsm->addTransition("PREPARE","PREPARED","PREPARED",boost::bind(&LDaq::prepare, this,_1));
    _fsm->addTransition("INITIALISE","PREPARED","INITIALISED",boost::bind(&LDaq::initialise, this,_1));
    _fsm->addTransition("CONFIGURE","INITIALISED","CONFIGURED",boost::bind(&LDaq::configure, this,_1));
    _fsm->addTransition("CONFIGURE","CONFIGURED","CONFIGURED",boost::bind(&LDaq::configure, this,_1));
    _fsm->addTransition("START","CONFIGURED","RUNNING",boost::bind(&LDaq::start, this,_1));
    _fsm->addTransition("STOP","RUNNING","CONFIGURED",boost::bind(&LDaq::stop, this,_1));
    _fsm->addTransition("DESTROY","CONFIGURED","PREPARED",boost::bind(&LDaq::destroy, this,_1));
    
  cout<<"Building LDaq"<<endl;
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0.str(std::string());
  s0<<"LDaq-"<<hname;

  DimServer::start(s0.str().c_str()); 
  //  cout<<"Starting DimDaqCtrl"<<endl;
  }



std::string LDaq::process(std::string command,std::string param)
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

std::string LDaq::process(std::string command)
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

LClient* LDaq::findFSM(DimBrowser* dbr,std::string pattern)
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
void LDaq::discover(levbdim::fsmmessage* m)
{
  std::cout<<"doScandns"<<std::endl;
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
      _DIFClients.push_back(dc);
      
    } 




}
void LDaq::prepare(levbdim::fsmmessage* m)
{
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
      std::cout<<"Current zup values "<<_zupClient->reply()<<std::endl;
    }
  // Ccc
  std::cout<<" CCC client "<<_cccClient<<std::endl;
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
      _mdccClient->clear();
      // Stop trigger ane reset counters
      _mdccClient->post("PAUSE");
      _mdccClient->post("RESET");
      _mdccClient->clear();
      _mdccClient->set<std::string>("name","STATUS");
      _mdccClient->post("CMD");
      std::cout<<"Current MDCC values "<<_mdccClient->reply()<<std::endl;
      
    }
  // Builder
  if (_builderClient)
    {
      _builderClient->clear();
      _builderClient->set<std::string>("proctype",m->content()["proctype"].asString());
      _builderClient->set<std::string>("filepath",m->content()["filepath"].asString());
      _builderClient->set<std::string>("memorypath",m->content()["memorypath"].asString());
      _builderClient->post("INITIALISE");

    }
}

std::string LDaq::difstatus()
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
  std::cout<<devlist<<std::endl;
  Json::FastWriter fastWriter;
  return fastWriter.write(devlist);
}

void LDaq::singlescan(LClient* d)
{
  d->clear();
  d->post("SCAN");
}
void LDaq::singleinit(LClient* d)
{
  d->clear();
  d->set<int>("difid",0);
  d->post("INITIALISE");
}

void LDaq::singleregisterdb(LClient* d)
{
  d->post("REGISTERDB");
}
void LDaq::singleconfigure(LClient* d)
{
  d->clear();
  d->set<int>("difid",0);
  
  d->post("CONFIGURE");
}
void LDaq::singlestart(LClient* d)
{
  d->clear();
  d->set<int>("difid",0);

  d->post("START");
}
void LDaq::singlestop(LClient* d)
{
  d->clear();
  d->set<int>("difid",0);
  d->post("STOP");
}

Json::Value LDaq::toJson(std::string s)
{
  Json::Reader reader;
  Json::Value jsta;
  bool parsingSuccessful = reader.parse(s,jsta);
  return jsta;
}
void LDaq::initialise(levbdim::fsmmessage* m)
{
  // Configure CCC
  if (_cccClient)
    {
      _cccClient->clear();
      _cccClient->post("CONFIGURE");
      _cccClient->post("STOP");
      _cccClient->clear();_cccClient->set<std::string>("name","CCCRESET");_cccClient->post("CMD");
      _cccClient->clear();_cccClient->set<std::string>("name","DIFRESET");_cccClient->post("CMD");

    }

  // Make a DIF SCAN
  boost::thread_group g;
  for (std::vector<LClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
      g.create_thread(boost::bind(&LDaq::singlescan, this,(*it)));
  g.join_all();

  // Merge devlist
  Json::Value devlist;
  for (std::vector<LClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
       const Json::Value& jdevs=(*it)->reply();
      for (Json::ValueConstIterator jt = jdevs.begin(); jt != jdevs.end(); ++jt)
	devlist.append(*jt);
    }
  std::cout<<devlist<<std::endl;

  // Initialise
  for (std::vector<LClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
      g.create_thread(boost::bind(&LDaq::singleinit, this,(*it)));
  g.join_all();
  

  // Fill status
  m->setAnswer(toJson(this->difstatus()));
}

void LDaq::configure(levbdim::fsmmessage* m)
{
  // Configure CCC
  std::cout<<m->content();
  if (_cccClient)
    {

      _cccClient->clear();_cccClient->set<std::string>("name","CCCRESET");_cccClient->post("CMD");
      _cccClient->clear();_cccClient->set<std::string>("name","DIFRESET");_cccClient->post("CMD");
      std::cout<<"RESET DONE"<<std::endl;
    }

  // register to the dbstate
  boost::thread_group g;
  for (std::vector<LClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
      (*it)->clear();
      (*it)->set<std::string>("dbstate",m->content()["dbstate"].asString());
      g.create_thread(boost::bind(&LDaq::singleregisterdb, this,(*it)));
    }
  g.join_all();
  std::cout<<this->status()<<std::endl;
  //Configure them
  for (std::vector<LClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
      (*it)->clear();
      (*it)->set<uint32_t>("ctrlreg",m->content()["ctrlreg"].asUInt());
      g.create_thread(boost::bind(&LDaq::singleconfigure, this,(*it)));
    }
  g.join_all();
  std::cout<<this->status()<<std::endl;
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
      std::cout<<"SENDING "<<jsou<<std::endl;
      _builderClient->clear();
      _builderClient->set<Json::Value>("sources",jsou);
      _builderClient->post("CONFIGURE");
    }
  m->setAnswer(jsta);
}

void LDaq::start(levbdim::fsmmessage* m)
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
      (*it)->clear();
      g.create_thread(boost::bind(&LDaq::singlestart, this,(*it)));
    }
  g.join_all();

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
       _mdccClient->post("RESET");
       _mdccClient->post("RESUME");
     }

   m->setAnswer(toJson(this->difstatus()));  
}
void LDaq::stop(levbdim::fsmmessage* m)
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
      g.create_thread(boost::bind(&LDaq::singlestop, this,(*it)));
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

void LDaq::destroy(levbdim::fsmmessage* m)
{
  for (std::vector<LClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
      (*it)->clear();
      (*it)->set<int>("difid",0);

      (*it)->post("DESTROY");
    } 
  
}

LDaq::~LDaq() 
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
void LDaq::doubleSwitchZup(unsigned int pause)
  {
    if (_zupClient==NULL)
      {
	LOG4CXX_ERROR(_logLdaq, "No zup client");
	return;
      }
   
    _zupClient->post("OFF");
    sleep((unsigned int) 2);
    _zupClient->post("ON");
    sleep((unsigned int)pause);
    _zupClient->post("OFF");
    sleep((unsigned int) 2);
    _zupClient->post("ON");
    sleep((unsigned int) pause);
    std::cout<<" LV is ON"<<std::endl;

  }
std::string LDaq::LVStatus()
{
  if (_zupClient)
    {
      Json::FastWriter fastWriter;
      _zupClient->post("READ");
      return fastWriter.write(_zupClient->reply());
    }

  return "NONE";

}
void LDaq::LVON()
  {
    if (_zupClient==NULL){LOG4CXX_ERROR(_logLdaq, "No zup client");return;}

    _zupClient->post("ON");
  }
void LDaq::LVOFF()
  {
    if (_zupClient==NULL){LOG4CXX_ERROR(_logLdaq, "No zup client");return;}
     _zupClient->post("OFF");
  }

void LDaq::setParameters(std::string jsonString)
  {
    _strParam=jsonString;
    std::cout<<jsonString<<std::endl;
    Json::Reader reader;
    bool parsingSuccessful = reader.parse(jsonString, _jparam);
    
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
    _zupport=_jparam["zupport"].asUInt();
    _ctrlreg=_jparam["ctrlreg"].asUInt();
    
  }

void LDaq::setDBState(std::string dbs)
{_dbstate=dbs;
  _jparam["dbstate"]=dbs;
  std::cout<<" Change DB state to "<<_dbstate<<std::endl;
}
void LDaq::setControlRegister(uint32_t reg)
{
  _jparam["ctrlreg"]=reg;
  _ctrlreg=reg;
  std::cout<<" Chamge Ctrlreg to "<<std::hex<<_ctrlreg<<std::dec<<std::endl;
}


std::string LDaq::builderStatus()
{
  Json::FastWriter fastWriter;
  Json::Value fromScratch;
  fromScratch["run"]=-1;
  fromScratch["event"]=-1;
  
  if (_builderClient==NULL){LOG4CXX_ERROR(_logLdaq, "No SHM client");return  fastWriter.write(fromScratch);}
  _builderClient->post("STATUS");
  return fastWriter.write(_builderClient->reply());
}

std::string LDaq::status()
  {
    return difstatus();
  }
void LDaq::pauseTrigger()
{
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq, "No MDC client");return;}
  _mdccClient->post("PAUSE");
}
void LDaq::resumeTrigger()
{
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq, "No MDC client");return;}
  _mdccClient->post("RESUME");
}
void LDaq::resetTriggerCounters()
{
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq, "No MDC client");return;}
  _mdccClient->post("RESET");
}
  
std::string LDaq::triggerStatus()
{
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq, "No MDC client");return "NONE";}
  _mdccClient->clear();
  _mdccClient->set<std::string>("name","STATUS");
  _mdccClient->post("CMD");
  Json::FastWriter fastWriter;
  return fastWriter.write(_mdccClient->reply());
}
