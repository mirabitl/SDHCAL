#include "RpcDaq.h"
#include <unistd.h>
#include <stdint.h>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <boost/interprocess/sync/interprocess_mutex.hpp>

RpcDaq::RpcDaq()
  {
    
  cout<<"Building RpcDaq"<<endl;
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  _state="CREATED";
  s0<<"/SDHCALDAQ/STATE";
  _shmState = new DimService(s0.str().c_str(),(char*) _state.c_str());
  _shmState->updateService();

  s0.str(std::string());
  s0<<"RpcDaq-"<<hname;

  DimServer::start(s0.str().c_str()); 
  //  cout<<"Starting DimDaqCtrl"<<endl;
  }

RpcDaq::~RpcDaq()
  {
  delete _shmState;
  }


void RpcDaq::processStatus(const std::string &jsonString,Json::Value &m_processInfo)
  {
    Json::Reader reader;
    bool parsingSuccessful = reader.parse(jsonString, m_processInfo);
    
    if (parsingSuccessful)
      {
	Json::StyledWriter styledWriter;
	std::cout << styledWriter.write(m_processInfo) << std::endl;
      }
    
  }


void RpcDaq::scandns()
{
  if (_state.compare("CREATED")==0)
    {
      this->doScandns();
      this->allocateClients();
      this->publishState("DISCOVERED");
      _msg="=>OK";
    }
  else
    _msg="=>Wrong initial State";
  // else
  //  this->publishState("DISCOVER_FAILED");
}


void RpcDaq::doScandns()
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

void RpcDaq::allocateClients()
  {
    LOG4CXX_INFO (_logWriter, "this is a info message, after parsing configuration file");
    
    std::cout<<theDBPrefix_<<" "<<theCCCPrefix_<<" "<<theWriterPrefix_<<" "<<theProxyPrefix_<<" "<<theZupPrefix_<<std::endl;

    _shClient= new RpcShmClient::rpiClient(theWriterPrefix_);
    _zupClient=new RpcZupClient::rpiClient(theZupPrefix_);
    _dbClient=new RpcDbClient::rpiClient(theDBPrefix_);
    
    _cccClient=new RpcCCCClient::rpiClient(theCCCPrefix_);
    _DIFClients.clear();
    for (std::vector<std::string>::iterator it=theDIFPrefix_.begin();it!=theDIFPrefix_.end();it++)
      {
	RpcDIFClient::rpiClient* s=new RpcDIFClient::rpiClient((*it));
	_DIFClients.push_back(s);
      }

  }

void RpcDaq::initialiseWriter(std::string directory)
  {
    _shClient->initialise();
    _shClient->directory(directory);
  }

void RpcDaq::initialiseZup(int port,std::string device)
  {
    _zupClient->open(port,device);
  }
void RpcDaq::doubleSwicthZup(unsigned int pause)
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
void RpcDaq::LVON()
  {
     _zupClient->lvswitch(1);
  }
void RpcDaq::LVOFF()
  {
     _zupClient->lvswitch(0);
  }

void RpcDaq::downloadDB(std::string s)
  {
    _dbClient->deletedb();
    _dbClient->download(s);
  }

void RpcDaq::downloadDB()
{
  std::cout<<"Downloading "<<_dbstate<<std::endl;
  this->downloadDB(_dbstate);
}
void RpcDaq::openCCC(std::string device)
  {
    _cccClient->open(device);
    

    _cccClient->initialise();
  }

void RpcDaq::configureCCC()
  {
    _cccClient->configure();
    _cccClient->stop();
    sleep((unsigned int) 1);
    _cccClient->cccreset();
    _cccClient->difreset();
    sleep((unsigned int) 1);
  }

void RpcDaq::scanFtdi1(RpcDIFClient::rpiClient* d) {d->scan();std::cout<<d->status()<<std::endl;}
void RpcDaq::scanFtdi()
  { boost::thread_group g;
  for (std::vector<RpcDIFClient::rpiClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
      {
	//	(*it)->scan();
	//	std::cout<<(*it)->status()<<std::endl;
	//     }
  	
	g.create_thread(boost::bind(&RpcDaq::scanFtdi1, this,(*it)));
      }
    g.join_all();
  }

void RpcDaq::setParameters(std::string jsonString)
  {
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
    _zupdevice=_jparam["zupdevice"].asString();
    _writerdir=_jparam["writerdir"].asString();;
    _zupport=_jparam["zupport"].asUInt();
    _ctrlreg=_jparam["ctrlreg"].asUInt();
    
  }

void RpcDaq::setDBState(std::string dbs) {_dbstate=dbs;
  std::cout<<" Change DB state to "<<_dbstate<<std::endl;}
void RpcDaq::setControlRegister(uint32_t reg) {_ctrlreg=reg;
  std::cout<<" Chamge Ctrlreg to "<<std::hex<<_ctrlreg<<std::dec<<std::endl;}


void RpcDaq::prepareServices()
  {
    if (_state.compare("DISCOVERED")==0 || _state.compare("DESTROYED")==0 || _state.compare("PREPARED") )
      {
	this->downloadDB(_dbstate);
	this->initialiseWriter(_writerdir);
	this->initialiseZup(_zupport,_zupdevice);
	this->openCCC(_dccname);
	this->publishState("PREPARED");
	_msg="=>OK";
      }
    else
      _msg="=>Wrong initial State";

    // else
    //  this->publishState("PREPARE_FAILED");
      
  }

void RpcDaq::initialise()
  {
    if (_state.compare("PREPARED")==0 || _state.compare("DESTROYED")==0 )
      {
	this->configureCCC();
	this->scanFtdi();
	this->initialiseDIF();
	this->publishState("INITIALISED");
      	_msg="=>OK";
      }
    else
      _msg="=>Wrong initial State";

    //else
    //  this->publishState("INITIALISE_FAILED");
  }

void RpcDaq::configure()
  {
    if (_state.compare("INITIALISED")==0 || _state.compare("STOPPED")==0 ||(_state.compare("CONFIGURED")==0 ))
      {

	_cccClient->cccreset();
	_cccClient->difreset();
	this->registerDB(_dbstate);
	this->configureDIF(_ctrlreg);
	this->publishState("CONFIGURED");
      	_msg="=>OK";
      }
    else
      _msg="=>Wrong initial State";

    //    else
    //  this->publishState("CONFIGURE_FAILED");

  }

void RpcDaq::initialise1DIF(RpcDIFClient::rpiClient* d) {d->initialise();std::cout<<d->status()<<std::endl;}
void RpcDaq::initialiseDIF()
  {
     boost::thread_group g;
    for (std::vector<RpcDIFClient::rpiClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
      {
	//(*it)->initialise();
	//std::cout<<(*it)->status()<<std::endl;
	
	g.create_thread(boost::bind(&RpcDaq::initialise1DIF, this,(*it)));
      }
    g.join_all();


      
  }
void RpcDaq::registerDB1(RpcDIFClient::rpiClient* d,std::string s) {d->registerDB(s);std::cout<<d->status()<<std::endl;}
void RpcDaq::registerDB(std::string s)
  { boost::thread_group g;
    for (std::vector<RpcDIFClient::rpiClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
      {
      // 	(*it)->registerDB(s);
      // 	std::cout<<(*it)->status()<<std::endl;
      // }
    	
	g.create_thread(boost::bind(&RpcDaq::registerDB1, this,(*it),s));
      }
    g.join_all();

    sleep((unsigned int) 3);

  }
void RpcDaq::configure1DIF(RpcDIFClient::rpiClient* d,uint32_t reg) {d->configure(reg);std::cout<<d->status()<<std::endl;}

void RpcDaq::configureDIF(uint32_t reg)
  {
     boost::thread_group g;
    for (std::vector<RpcDIFClient::rpiClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
      {
	//(*it)->configure(reg);
	//	std::cout<<(*it)->status()<<std::endl;
	// }
    	g.create_thread(boost::bind(&RpcDaq::configure1DIF, this,(*it),reg));
      }
    g.join_all();

  }
void RpcDaq::start1(RpcDIFClient::rpiClient* d) {d->start();}
void RpcDaq::start(uint32_t tempo)
  { boost::thread_group g;
    if (_state.compare("CONFIGURED")==0 || _state.compare("STOPPED")==0 )
      {


	uint32_t _run=_dbClient->newrun();
	std::cout<<" New run " <<_run<<std::endl;


	//s->configure(0x815A1B00);
	//std::cout<<s->status()<<std::endl;
	//getchar();
	//sleep((unsigned int) tempo);
	uint32_t ndif=0;
	for (std::vector<RpcDIFClient::rpiClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
	  {
	    	// }
	    g.create_thread(boost::bind(&RpcDaq::start1, this,(*it)));
	  }
	g.join_all();
	    //(*it)->start();

	for (std::vector<RpcDIFClient::rpiClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
	  {
	    //std::cout<<(*it)->status()<<std::endl;
	    Json::Value _jsroot;
	    processStatus((*it)->status(),_jsroot);
	    std::cout<<" Number of DIF "<<_jsroot["difs"].size()<<std::endl;
	    //sleep((unsigned int) tempo);
	    ndif+=_jsroot["difs"].size();
	  }

	_shClient->start(ndif);
	//std::cout<<"waiting "<<std::endl;
	sleep((unsigned int) tempo);
	
	_cccClient->start();
	this->publishState("STARTED");
      	_msg="=>OK";
      }
    else
      _msg="=>Wrong initial State";

    //    else
    //  this->publishState("START_FAILED");

  }

void RpcDaq::stop(uint32_t tempo)
  {
    if (_state.compare("STARTED")==0 )
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
	this->publishState("STOPPED");
      	_msg="=>OK";
      }
    else
      _msg="=>Wrong initial State";

    // else
    //  this->publishState("STOP_FAILED");

  }
void RpcDaq::destroy1(RpcDIFClient::rpiClient* d) {d->destroy();std::cout<<d->status()<<std::endl;}
void RpcDaq::destroy()
  {boost::thread_group g;
    if (_state.compare("STARTED")!=0 )
      {
	for (std::vector<RpcDIFClient::rpiClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
	  {
	  //   (*it)->destroy();
	  //   std::cout<<(*it)->status()<<std::endl;
	  // }void RpcDaq::initialise1DIF(RpcDIFClient::rpiClient* d) {d->initialise();std::cout<<d->status()<<std::endl;}
	    g.create_thread(boost::bind(&RpcDaq::destroy1,this,(*it)));
      }
    g.join_all();
 	this->publishState("DESTROYED");
      	_msg="=>OK";
      }
    else
      _msg="=>Wrong initial State";

    //  else
    //   this->publishState("DESTROY_FAILED");

  }

std::string RpcDaq::shmStatus()
{
  //  std::stringstream s0;
  _shClient->status();
  //s0<<" ShmWriter=>Run "<<_shClient->run()<<" Event "<<_shClient->event();
  //return s0.str();
  Json::FastWriter fastWriter;
  Json::Value fromScratch;
  fromScratch["run"]=_shClient->run();
  fromScratch["event"]=_shClient->event();
  return fastWriter.write(fromScratch);
}

std::string RpcDaq::status()
  {
    Json::FastWriter fastWriter;
    Json::Value fromScratch;
    Json::Value array;

    for (std::vector<RpcDIFClient::rpiClient*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
      {
	Json::Value _jsd;
	Json::Value _jsdifs;
	//_jsd["prefix"]=(*it)->prefix();

	processStatus((*it)->status(),_jsdifs);
	//_jsd["list"]=_jsdifs;
	array.append(_jsdifs);
      }
    //fromScratch["rpi"] = array;
    return fastWriter.write(array);
    
  }
void RpcDaq::setState(std::string s){_state.assign(s);}
std::string RpcDaq::state()  {return _state;}
  // Publish DIM services
void RpcDaq::publishState(std::string s){setState(s);_shmState->updateService((char*) _state.c_str());}
