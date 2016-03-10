#include "RpcDaqServer.h"
#include <unistd.h>
#include <stdint.h>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <boost/interprocess/sync/interprocess_mutex.hpp>


RpcDaqPrepare::RpcDaqPrepare(RpcDaqServer* r,std::string name) : DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}

void RpcDaqPrepare::rpcHandler()
  {
    LOG4CXX_INFO(_logCtrl," CMD: PREPARE called");


    _server->prepareServices();
    _server->publishState("READY");
    int32_t rc=0;
    setData(rc);
  }
RpcDaqInitialise::RpcDaqInitialise(RpcDaqServer* r,std::string name): DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}

void RpcDaqInitialise::rpcHandler()
  {
    LOG4CXX_INFO(_logCtrl," CMD: INITIALISE called");


    _server->initialise();
    _server->publishState("INITIALISED");
    int32_t rc=0;
    setData(rc);
  }

RpcDaqConfigure::RpcDaqConfigure(RpcDaqServer* r,std::string name): DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}

void RpcDaqConfigure::rpcHandler()
  {
    LOG4CXX_INFO(_logCtrl," CMD: CONFIGURE called");


    _server->configure();
    _server->publishState("CONFIGURED");
    int32_t rc=0;
    setData(rc);
  }

RpcDaqStart::RpcDaqStart(RpcDaqServer* r,std::string name): DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}

void RpcDaqStart::rpcHandler()
  {
    LOG4CXX_INFO(_logCtrl," CMD: START called");


    _server->start();
    _server->publishState("RUNNING");
    int32_t rc=0;
    setData(rc);
  }
RpcDaqStop::RpcDaqStop(RpcDaqServer* r,std::string name): DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}

void RpcDaqStop::rpcHandler()
  {
    LOG4CXX_INFO(_logCtrl," CMD: STOP called");


    _server->stop();
    _server->publishState("STOPPED");
    int32_t rc=0;
    setData(rc);
  }
RpcDaqDestroy::RpcDaqDestroy(RpcDaqServer* r,std::string name): DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}

void RpcDaqDestroy::rpcHandler()
  {
    LOG4CXX_INFO(_logCtrl," CMD: DESTROY called");


    _server->destroy();
    _server->publishState("READY");
    int32_t rc=0;
    setData(rc);
  }
RpcDaqDiscover::RpcDaqDiscover(RpcDaqServer* r,std::string name)
    : DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}

void RpcDaqDiscover::rpcHandler()
  {
    LOG4CXX_INFO(_logCtrl," CMD: DISCOVER called");


    _server->scandns();
    _server->publishState("DISCOVERED");
    int32_t rc=0;
    setData(rc);
  }
RpcDaqStatus::RpcDaqStatus(RpcDaqServer* r,std::string name)
    : DimRpc(name.c_str(),"I:1","C"),_server(r) {}

void RpcDaqStatus::rpcHandler()
  {
    LOG4CXX_INFO(_logCtrl," CMD: STATUS called");


    std::string s=_server->status();
    setData((char*) s.c_str(),s.length());
  }
RpcDaqParameters::RpcDaqParameters(RpcDaqServer* r,std::string name): DimRpc(name.c_str(),"C","I:1"),_server(r) {}

void RpcDaqParameters::rpcHandler()
  {
    LOG4CXX_INFO(_logCtrl," CMD: PARAMETERS called");

    std::string s;
    s.assign(getString());
    _server->setParameters(s);
    
    int32_t rc=0;
    setData(rc);
  }
RpcDaqServer::RpcDaqServer()
  {
    
  cout<<"Building RpcDaqServer"<<endl;
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  _state="CREATED";
  s0<<"/SDHCALDAQ/STATE";
  _shmState = new DimService(s0.str().c_str(),(char*) _state.c_str());
  _shmState->updateService();
  allocateCommands();
  s0.str(std::string());
  s0<<"RpcDaqServer-"<<hname;

  DimServer::start(s0.str().c_str()); 
  //  cout<<"Starting DimDaqCtrl"<<endl;
  }

RpcDaqServer::~RpcDaqServer()
  {
  delete _shmState;
  delete _prepareCommand;
  delete _initialiseCommand;
  delete _configureCommand;
  delete _startCommand;
  delete _stopCommand;
  delete _parametersCommand;
  delete _statusCommand;
  delete _destroyCommand;

  }

void RpcDaqServer::allocateCommands()
{
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0.str(std::string());
  s0<<"/SDHCALDAQ/DISCOVER";  
  _discoverCommand=new RpcDaqDiscover(this,s0.str());
  s0.str(std::string());
  s0<<"/SDHCALDAQ/PREPARE";  
  _prepareCommand=new RpcDaqPrepare(this,s0.str());
  s0.str(std::string());
  s0<<"/SDHCALDAQ/INITIALISE";  
  _initialiseCommand=new RpcDaqInitialise(this,s0.str());
  s0.str(std::string());
  s0<<"/SDHCALDAQ/CONFIGURE";  
  _configureCommand=new RpcDaqConfigure(this,s0.str());
  s0.str(std::string());
  s0<<"/SDHCALDAQ/START";
  _startCommand=new RpcDaqStart(this,s0.str());
  s0.str(std::string());
  s0<<"/SDHCALDAQ/STOP";
  _stopCommand=new RpcDaqStop(this,s0.str());
  s0.str(std::string());
  s0<<"/SDHCALDAQ/DESTROY";
  _destroyCommand=new RpcDaqDestroy(this,s0.str());
  s0.str(std::string());
  s0<<"/SDHCALDAQ/PARAMETERS";
  _parametersCommand=new RpcDaqParameters(this,s0.str());
  s0.str(std::string());
  s0<<"/SDHCALDAQ/STATUS";
  _statusCommand=new RpcDaqStatus(this,s0.str());
  
}


void RpcDaqServer::processStatus(const std::string &jsonString,Json::Value &m_processInfo)
  {
    Json::Reader reader;
    bool parsingSuccessful = reader.parse(jsonString, m_processInfo);
    
    if (parsingSuccessful)
      {
	Json::StyledWriter styledWriter;
	std::cout << styledWriter.write(m_processInfo) << std::endl;
      }
    
  }


void RpcDaqServer::scandns()
{
    this->doScandns();
    this->allocateClients();
}


void RpcDaqServer::doScandns()
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
    
    cout<<"0"<<endl;
    dbr->getServices("/DB/*/DOWNLOAD" ); 
    cout<<"1\n";
    while(type = dbr->getNextService(service, format)) 
      { 
	cout << service << " -  " << format << endl; 
	std::string ss;
	ss.assign(service);
	size_t n=ss.find("/DOWNLOAD");
	cout<<ss.substr(0,n)<<endl;
	theDBPrefix_=ss.substr(0,n);
      } 
    // Get the CCC prefix
    cout<<"2\n";
    dbr->getServices("/DCS/*/STATE" ); 
    while(type = dbr->getNextService(service, format)) 
      { 
	cout << service << " -  " << format << endl; 
	std::string ss;
	ss.assign(service);
	size_t n=ss.find("/STATE");
	theCCCPrefix_=ss.substr(0,n);
      } 

    theWriterPrefix_="";
    dbr->getServices("/DSP/*/STATE" ); 
    while(type = dbr->getNextService(service, format)) 
      { 
	cout << service << " -  " << format << endl; 
	std::string ss;
	ss.assign(service);
	size_t n=ss.find("/STATE");
	theWriterPrefix_=ss.substr(0,n);

 
      } 

    theZupPrefix_="";
    dbr->getServices("/DZUP/*/STATE" ); 
    while(type = dbr->getNextService(service, format)) 
      { 
	cout << service << " -  " << format << endl; 
	std::string ss;
	ss.assign(service);
	size_t n=ss.find("/STATE");
	theZupPrefix_=ss.substr(0,n);
      } 
    std::string theProxyPrefix_="";
    dbr->getServices("/DSP/*/STATE" ); 
    while(type = dbr->getNextService(service, format)) 
      { 
	cout << service << " -  " << format << endl; 
	std::string ss;
	ss.assign(service);
	size_t n=ss.find("/STATE");
	theProxyPrefix_=ss.substr(0,n);;
      }
    theDIFPrefix_.clear();
    dbr->getServices("/DDS/*/STATE" ); 
    while(type = dbr->getNextService(service, format)) 
      { 
	cout << service << " -  " << format << endl; 
	std::string ss;
	ss.assign(service);
	size_t n=ss.find("/STATE");
	std::cout<<"DIF on "<<ss.substr(0,n)<<std::endl;
	theDIFPrefix_.push_back(ss.substr(0,n));
      } 


  }

void RpcDaqServer::allocateClients()
  {
    LOG4CXX_INFO (_logWriter, "this is a info message, after parsing configuration file");
    
    std::cout<<theDBPrefix_<<" "<<theCCCPrefix_<<" "<<theWriterPrefix_<<" "<<theProxyPrefix_<<" "<<theZupPrefix_<<std::endl;

    _shClient= new RpcShmClient::rpiClient(theWriterPrefix_);
    _zupClient=new RpcZupClient::rpiClient(theZupPrefix_);
    _dbClient=new RpcDbClient::rpiClient(theDBPrefix_);
    
    _cccClient=new RpcCCCClient::rpiClient(theCCCPrefix_);

    for (std::vector<std::string>::iterator it=theDIFPrefix_.begin();it!=theDIFPrefix_.end();it++)
      {
	RpcDIFClient::rpiClient* s=new RpcDIFClient::rpiClient((*it));
	_DIFClients.push_back(s);
      }

  }

void RpcDaqServer::initialiseWriter(std::string directory)
  {
    _shClient->initialise();
    _shClient->directory(directory);
  }

void RpcDaqServer::initialiseZup(int port,std::string device)
  {
    _zupClient->open(port,device);
  }
void RpcDaqServer::doubleSwicthZup(unsigned int pause)
  {
    _zupClient->lvswitch(0);
    sleep((unsigned int) 2);
    _zupClient->lvswitch(1);
    sleep((unsigned int)pause);
    _zupClient->lvswitch(0);
    sleep((unsigned int) 2);
    _zupClient->lvswitch(1);
    sleep((unsigned int) pause);
    std::cout<<" LV is ON"<<std::endl;

  }
void RpcDaqServer::LVON()
  {
     _zupClient->lvswitch(1);
  }
void RpcDaqServer::LVOFF()
  {
     _zupClient->lvswitch(0);
  }

void RpcDaqServer::downloadDB(std::string s)
  {
    _dbClient->download(s);
  }

void RpcDaqServer::openCCC(std::string device)
  {
    _cccClient->open(device);
    

    _cccClient->initialise();
  }

void RpcDaqServer::configureCCC()
  {
    _cccClient->configure();

  }
void RpcDaqServer::scanFtdi()
  {
  for (std::vector<RpcDIFClient::rpiClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
      {
	(*it)->scan();
	std::cout<<(*it)->status()<<std::endl;
      }
  }

void RpcDaqServer::setParameters(const std::string &jsonString)
  {
    Json::Reader reader;
    bool parsingSuccessful = reader.parse(jsonString, _jparam);
    
    if (parsingSuccessful)
      {
	Json::StyledWriter styledWriter;
	std::cout << styledWriter.write(_jparam) << std::endl;
      }
    _dbstate=_jparam["dbstate"].asString();
    _dccname=_jparam["dccname"].asString();
    _zupdevice=_jparam["zupdevice"].asString();
    _writerdir=_jparam["writerdir"].asString();;
    _zupport=_jparam["zupport"].asUInt();
    _ctrlreg=_jparam["ctrlreg"].asUInt();
    
  }


void RpcDaqServer::prepareServices()
  {

    this->downloadDB(_dbstate);
    this->initialiseWriter(_writerdir);
    this->initialiseZup(_zupport,_zupdevice);
    this->openCCC(_dccname);
  }
void RpcDaqServer::initialise()
  {
    this->scanFtdi();
    this->initialiseDIF();
  }

void RpcDaqServer::configure()
  {
    this->configureCCC();
    this->registerDB(_dbstate);
    this->configureDIF(_ctrlreg);
  }
void RpcDaqServer::initialiseDIF()
  {
    for (std::vector<RpcDIFClient::rpiClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
      {
	(*it)->initialise();
	std::cout<<(*it)->status()<<std::endl;
      }
  }

void RpcDaqServer::registerDB(std::string s)
  {
    for (std::vector<RpcDIFClient::rpiClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
      {
	(*it)->registerDB(s);
	std::cout<<(*it)->status()<<std::endl;
      }
  }
void RpcDaqServer::configureDIF(uint32_t reg)
  {
    for (std::vector<RpcDIFClient::rpiClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
      {
	(*it)->configure(reg);
	std::cout<<(*it)->status()<<std::endl;
      }
  }
void RpcDaqServer::start(uint32_t tempo)
  {
    uint32_t _run=_dbClient->newrun();
    std::cout<<" New run " <<_run<<std::endl;


	//s->configure(0x815A1B00);
	//std::cout<<s->status()<<std::endl;
	//getchar();
    sleep((unsigned int) tempo);
    uint32_t ndif=0;
    for (std::vector<RpcDIFClient::rpiClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
      {
	    (*it)->start();
	    std::cout<<(*it)->status()<<std::endl;
	    Json::Value _jsroot;
	    processStatus((*it)->status(),_jsroot);
	    std::cout<<" Number of DIF "<<_jsroot["difs"].size()<<std::endl;
	    ndif+=_jsroot["difs"].size();
	  }

    _shClient->start(ndif);
    sleep((unsigned int) tempo);
    _cccClient->start();

  }

void RpcDaqServer::stop(uint32_t tempo)
  {
    _cccClient->stop();
    //getchar();
    sleep((unsigned int) tempo);
    for (std::vector<RpcDIFClient::rpiClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
      {
	(*it)->stop();
	std::cout<<(*it)->status()<<std::endl;
      }


    _shClient->stop();

  }
void RpcDaqServer::destroy()
  {
    for (std::vector<RpcDIFClient::rpiClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
      {
	(*it)->destroy();
	std::cout<<(*it)->status()<<std::endl;
      }

  }
  
std::string RpcDaqServer::status()
  {
    Json::FastWriter fastWriter;
    Json::Value fromScratch;
    Json::Value array;

    for (std::vector<RpcDIFClient::rpiClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
      {
	Json::Value _jsd;
	Json::Value _jsdifs;
	_jsd["prefix"]=(*it)->prefix();

	processStatus((*it)->status(),_jsdifs);
	_jsd["list"]=
	  array.append(_jsd);
      }
    fromScratch["rpi"] = array;
    return fastWriter.write(fromScratch);
    
  }
void RpcDaqServer::setState(std::string s){_state.assign(s);}
std::string RpcDaqServer::state()  {return _state;}
  // Publish DIM services
void RpcDaqServer::publishState(std::string s){setState(s);_shmState->updateService((char*) _state.c_str());}
