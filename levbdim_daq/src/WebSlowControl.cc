

#include "WebSlowControl.hh"
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>

#include <string.h>
#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;



WebSlowControl::WebSlowControl(std::string name,uint32_t port) : _storeRunning(false),_checkRunning(false),_my(NULL)
{
  printf("parsing the config file \n");
  DOMConfigurator::configure("/etc/Log4cxxConfig.xml");
  //_logger->setLevel(log4cxx::Level::getInfo());
  LOG4CXX_INFO (_logLdaq, "this is a info message, after parsing configuration file")
    _chambers.clear();
    
  _fsm=new fsmweb(name);
 
  _fsm->addState("CREATED");
  _fsm->addState("DISCOVERED");
  _fsm->addState("INITIALISED");
  _fsm->addState("MONITORING");
  _fsm->addState("CHECKING");

 
  _fsm->addTransition("DISCOVER","CREATED","DISCOVERED",boost::bind(&WebSlowControl::discover, this,_1));
  _fsm->addTransition("INITIALISE","DISCOVERED","INITIALISED",boost::bind(&WebSlowControl::initialise, this,_1));
  _fsm->addTransition("STARTMONITOR","INITIALISED","MONITORING",boost::bind(&WebSlowControl::startMonitor, this,_1));
  _fsm->addTransition("STOPMONITOR","MONITORING","INITIALISED",boost::bind(&WebSlowControl::stopMonitor, this,_1));
  _fsm->addTransition("STARTCHECK","MONITORING","CHECKING",boost::bind(&WebSlowControl::startCheck, this,_1));
  _fsm->addTransition("STOPCHECK","CHECKING","MONITORING",boost::bind(&WebSlowControl::stopCheck, this,_1));
  _fsm->addTransition("DESTROY","INITIALISED","DISCOVERED",boost::bind(&WebSlowControl::destroy, this,_1));

  // Commands

  _fsm->addCommand("SETPERIOD",boost::bind(&WebSlowControl::setReadoutPeriod,this,_1,_2));
  _fsm->addCommand("HVREADCHANNEL",boost::bind(&WebSlowControl::readChannel,this,_1,_2));
  _fsm->addCommand("HVON",boost::bind(&WebSlowControl::HVON,this,_1,_2));
  _fsm->addCommand("HVOFF",boost::bind(&WebSlowControl::HVOFF,this,_1,_2));
  _fsm->addCommand("SETVOLTAGE",boost::bind(&WebSlowControl::setVoltage,this,_1,_2));
  _fsm->addCommand("SETCURRENTLIMIT",boost::bind(&WebSlowControl::setCurrentLimit,this,_1,_2));
  _fsm->addCommand("LOADREFERENCES",boost::bind(&WebSlowControl::loadReferences,this,_1,_2));
  _fsm->addCommand("PTREAD",boost::bind(&WebSlowControl::ptRead,this,_1,_2));
  
  cout<<"Building WebSlowControl"<<endl;
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0.str(std::string());
  s0<<"WebSlow-"<<hname;

  DimServer::start(s0.str().c_str()); 
  //  cout<<"Starting DimDaqCtrl"<<endl;
  _fsm->start(port);
}
void WebSlowControl::discover(levbdim::fsmmessage* m)
{
  Json::Value jrep;
  jrep.clear();
  DimBrowser* dbr=new DimBrowser(); 
  char *service, *format; 
  int type;
  // Get DB service
  cout<<"On rentre dans scandns "<<endl;

  char *server,*node;
  _BMPPrefix="";
  dbr->getServices("/BMP183/*/PRESSIONREADVALUES" ); 
  while(type = dbr->getNextService(service, format)) 
    { 
      cout << service << " -  " << format << endl; 
      std::string ss;
      ss.assign(service);
      size_t n=ss.find("/PRESSIONREADVALUES");
      _BMPPrefix=ss.substr(0,n);
    }
  // Register Info
  _pressionInfo=NULL;
  _temperatureInfo=NULL;
  if (_BMPPrefix.length()>6)
    {
      jrep["BMPREFIX"]=_BMPPrefix;
      std::stringstream s0;
      s0.str(std::string());
      s0<<_BMPPrefix<<"/PRESSIONREADVALUES";
      _pressionInfo= new DimInfo(s0.str().c_str(),_PRead,this);
      s0.str(std::string());
      s0<<_BMPPrefix<<"/TEMPERATUREREADVALUES";
      _temperatureInfo= new DimInfo(s0.str().c_str(),_TRead,this);
    }
  else
    {
      jrep["BMPREFIX"]="NONE";
      LOG4CXX_FATAL(_logLdaq,"NO P,T readout ");
    }
  for (int i=0;i<7;i++)
    for (int j=0;j<8;j++)
      {
	std::stringstream s0;
	s0.str(std::string());
	s0<<"/WIENER/MODULE"<<i<<"-CHANNEL"<<j<<"/"<<i*8+j;
	_wienerInfo[i*8+j]=new DimInfo(s0.str().c_str(),&_hvchannels[i*8+j],sizeof(wienerChannel),this);
      }
  m->setAnswer(jrep);
}
WebSlowControl::~WebSlowControl()
{
 
}

void WebSlowControl::infoHandler()
{
  DimInfo *curr = getInfo(); // get current DimInfo address 
  //if (curr->getSize()==1) return;
  if (curr==_pressionInfo)
    {
      _PRead=curr->getFloat();
      return;
    }
  if (curr==_temperatureInfo)
    {
      _TRead=curr->getFloat();
      return;
    }
  for (int i=0;i<56;i++)
     
    if (curr==_wienerInfo[i])
      {
	memcpy(&_hvchannels[i],curr->getData(),curr->getSize());
	return;
      }

}

void WebSlowControl::setReadoutPeriod(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t p=atoi(request.get("period","10").c_str());
  std::stringstream s0;
  if (_BMPPrefix.length()>3)
    {
      s0.str(std::string());
      s0<<_BMPPrefix<<"/SETPERIOD";
      DimClient::sendCommand(s0.str().c_str(),(int) p);
    }
  
  DimClient::sendCommand("/WIENER/SetPeriod",(int) p);
  response["STATUS"]="DONE";
  
}

void WebSlowControl::readChannel(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t ch=atoi(request.get("channel","99").c_str());
  DimClient::sendCommand("/WIENER/ReadChannel",(int) ch);
  ::sleep(2); // Wait 2s for reply
  Json::Value jlist;jlist.clear();
  if (ch<56)
    {
      Json::Value jval;
      jval["channel"]=ch;
      jval["vset"]=vset(ch);
      jval["iset"]=iset(ch);
      jval["vout"]=vout(ch);
      jval["iout"]=iout(ch);
      jlist.append(jval);
    }
  else
    for (int i=0;i<56;i++)
      {
	Json::Value jval;
      jval["channel"]=i;
      jval["vset"]=vset(i);
      jval["iset"]=iset(i);
      jval["vout"]=vout(i);
      jval["iout"]=iout(i);
      jlist.append(jval);

      }
  response["STATUS"]="DONE";
  response["ANSWER"]=jlist;
}
void WebSlowControl::dimsetVoltage(int ch,float v)
{
  int32_t ibuf[2];
  float* fbuf=(float*) ibuf;
  ibuf[0]=ch;fbuf[1]=v;
  DimClient::sendCommand("/WIENER/SetVoltage",ibuf,sizeof(int32_t)+sizeof(float));
}
void WebSlowControl::dimreadChannel(int ch)
{
  DimClient::sendCommand("/WIENER/ReadChannel",(int) ch);
}

void WebSlowControl::setVoltage(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t chf=atoi(request.get("first","99").c_str());
  uint32_t chl=atoi(request.get("last","99").c_str());
  float v=atof(request.get("voltage","-1.0").c_str());
  if (chf==99 || chl==99 || v==-1.0)
    {
      response["STATUS"]="Missing parameters";
      return;
    }
  Json::Value jlist;
  for (int ch=chf;ch<=chl;ch++)
    {
      std::stringstream s0;
      s0.str(std::string());
      int32_t ibuf[2];
      float* fbuf=(float*) ibuf;
      ibuf[0]=ch;fbuf[1]=v;
      DimClient::sendCommand("/WIENER/SetVoltage",ibuf,sizeof(int32_t)+sizeof(float));
      Json::Value jval;
      jval["channel"]=ch;
      jval["vset"]=v;
      jlist.append(jval);
    }
  response["STATUS"]="DONE";
  response["ANSWER"]=jlist;
 
}
void WebSlowControl::setCurrentLimit(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t chf=atoi(request.get("first","99").c_str());
  uint32_t chl=atoi(request.get("last","99").c_str());
  float v=atof(request.get("current","-1.0").c_str());
  if (chf==99 || chl==99 || v==-1.0)
    {
      response["STATUS"]="Missing parameters";
      return;
    }
  Json::Value jlist;
  for (int ch=chf;ch<=chl;ch++)
    {
      std::stringstream s0;
      s0.str(std::string());
      int32_t ibuf[2];
      float* fbuf=(float*) ibuf;
      ibuf[0]=ch;fbuf[1]=v*1E-6;
      DimClient::sendCommand("/WIENER/SetCurrent",ibuf,sizeof(int32_t)+sizeof(float));
      Json::Value jval;
      jval["channel"]=ch;
      jval["iset"]=v;
      jlist.append(jval);
    }
  response["STATUS"]="DONE";
  response["ANSWER"]=jlist;
 
}

void WebSlowControl::HVON(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t chf=atoi(request.get("first","99").c_str());
  uint32_t chl=atoi(request.get("last","99").c_str());

  if (chf==99 || chl==99 )
    {
      response["STATUS"]="Missing parameters";
      return;
    }
  Json::Value jlist;
  for (int ch=chf;ch<=chl;ch++)
    {
      std::stringstream s0;
      s0.str(std::string());
      int32_t ibuf[2];
      ibuf[0]=ch;ibuf[1]=1;
      DimClient::sendCommand("/WIENER/Switch",ibuf,2*sizeof(int32_t));
      Json::Value jval;
      jval["channel"]=ch;
      jval["on"]=1;
      jlist.append(jval);
    }
  response["STATUS"]="DONE";
  response["ANSWER"]=jlist;
 
}
void WebSlowControl::HVOFF(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t chf=atoi(request.get("first","99").c_str());
  uint32_t chl=atoi(request.get("last","99").c_str());

  if (chf==99 || chl==99 )
    {
      response["STATUS"]="Missing parameters";
      return;
    }
  Json::Value jlist;
  for (int ch=chf;ch<=chl;ch++)
    {
      std::stringstream s0;
      s0.str(std::string());
      int32_t ibuf[2];
      ibuf[0]=ch;ibuf[1]=0;
      DimClient::sendCommand("/WIENER/Switch",ibuf,2*sizeof(int32_t));
      Json::Value jval;
      jval["channel"]=ch;
      jval["on"]=0;
      jlist.append(jval);
    }
  response["STATUS"]="DONE";
  response["ANSWER"]=jlist;
 
}

void WebSlowControl::loadReferences(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  Json::Value jlist;
  for (std::vector<chamberRef>::iterator it=_chambers.begin();it!=_chambers.end();it++)
    {
      LOG4CXX_INFO(_logLdaq,"Setting "<<it->channel<<" to V="<<it->vref<<" Imax="<<it->iref);
      
      int32_t ibuf[2];
      float* fbuf=(float*) ibuf;
      ibuf[0]=it->channel;fbuf[1]=it->vref;
      DimClient::sendCommand("/WIENER/SetVoltage",ibuf,sizeof(int32_t)+sizeof(float));
      ibuf[0]=it->channel;fbuf[1]=it->iref*1E-6;
      DimClient::sendCommand("/WIENER/SetCurrent",ibuf,sizeof(int32_t)+sizeof(float));
      Json::Value jch;
      jch["channel"]=it->channel;
      jch["vset"]=it->vref;
      jch["iset"]=it->iref;
      jlist.append(jch);
    }
  response["STATUS"]="DONE";
  response["ANSWER"]=jlist;

}

void WebSlowControl::ptRead(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  Json::Value jrep;
  jrep["P"]=_PRead;
  jrep["T"]=_TRead;
  response["STATUS"]="DONE";
  response["ANSWER"]=jrep;
  
}

void WebSlowControl::destroy(levbdim::fsmmessage* m)
{
  _chambers.clear();
  if (_my!=NULL)
    delete _my;
  _my=NULL;
}
void WebSlowControl::initialise(levbdim::fsmmessage* m)
{
  std::string sdb =m->content()["dbstate"].asString();
  
  _my= new MyInterface(sdb);
  _my->connect();

  Json::Value jrep;
  Json::Value jchambers;
  _chambers.clear();
  _my->executeSelect("select HVCHAN,VREF,IREF,P0,T0 FROM CHAMBERREF WHERE FIN>NOW() AND DEBUT<NOW()");
  MYSQL_ROW row=NULL;
  while ((row=_my->getNextRow())!=0)
    {
      chamberRef c;
      c.channel=atoi(row[0]);
      c.vref=atof(row[1]);
      c.iref=atof(row[2]);
      c.p0=atof(row[3]);
      c.t0=atof(row[4]);
      _chambers.push_back(c);
      //std::cout<<c.channel<<" "<<c.vref<<std::endl;
      Json::Value jch;
      jch["channel"]=c.channel;
      jch["vref"]=c.vref;
      jch["iref"]=c.iref;
      jch["pref"]=c.p0;
      jch["tref"]=c.t0;
      jchambers.append(jch);
    }
  _my->disconnect();
  jrep["chambers"]=jchambers;
  m->setAnswer(jrep);
}

bool  WebSlowControl::getPTMean()
{
  if (_my==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No mysql interface available");
      return false;
    }
  _my->connect();
  _my->executeSelect("select P,TK FROM BMPMON WHERE TIS BETWEEN NOW() - INTERVAL 10 MINUTE AND NOW()");
  _PMean=0;_TMean=0;
  int32_t nmeas=0;
  MYSQL_ROW row=NULL;
  while ((row=_my->getNextRow())!=0)
    {
      _PMean+=atof(row[0]);
      _TMean+=atof(row[1]);
      nmeas++;
    }
  _my->disconnect();
  if (nmeas<5)
    {
      LOG4CXX_WARN(_logLdaq,"Not enough PT measure "<<nmeas);
      return false;
      
    }
  _PMean/=nmeas;
  _TMean/=nmeas;
  return true;
  
}

float WebSlowControl::pression(){ return _PRead;}
float WebSlowControl::temperature(){ return _TRead;}
float WebSlowControl::vset(uint32_t i){ return _hvchannels[i].vset;}
float WebSlowControl::iset(uint32_t i){ return _hvchannels[i].iset;}
float WebSlowControl::vout(uint32_t i){ return _hvchannels[i].vout;}
float WebSlowControl::iout(uint32_t i){ return _hvchannels[i].iout;}


std::string WebSlowControl::hvinfoChannel(uint32_t ch)
{
  Json::FastWriter fastWriter;
  Json::Value fromScratch;

  fromScratch["channel"] = _hvchannels[ch].channel;
  fromScratch["vset"] = _hvchannels[ch].vset;
  fromScratch["iset"] = _hvchannels[ch].iset;
  fromScratch["vout"] = _hvchannels[ch].vout;
  fromScratch["iout"] = _hvchannels[ch].iout;
  return fastWriter.write(fromScratch);
}
std::string WebSlowControl::hvinfoCrate()
{
  Json::FastWriter fastWriter;

  Json::Value array;

  for (int ch=0;ch<56;ch++)
      {
	Json::Value fromScratch;
	fromScratch["channel"] = _hvchannels[ch].channel;
	fromScratch["vset"] = _hvchannels[ch].vset;
	fromScratch["iset"] = _hvchannels[ch].iset;
	fromScratch["vout"] = _hvchannels[ch].vout;
	fromScratch["iout"] = _hvchannels[ch].iout;

	array.append(fromScratch);
      }
    return fastWriter.write(array);
}


void WebSlowControl::startMonitor(levbdim::fsmmessage* m)
{
  _storeTempo =m->content()["period"].asUInt();
  g_store.create_thread(boost::bind(&WebSlowControl::doStore, this));
  Json::Value jsta;
  jsta["action"]="MONITOR STARTED";
  m->setAnswer(jsta);
}
void WebSlowControl::doStore()
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
      for (int i=0;i<56;i++)
	{
	  std::stringstream s0;
	  s0.str(std::string());
	  s0<<"insert into WIENERMON(HVCHAN,VSET,ISET,VOUT,IOUT) VALUES("<<_hvchannels[i].channel<<","<<_hvchannels[i].vset<<","<<_hvchannels[i].iset<<","<<_hvchannels[i].vout<<","<<_hvchannels[i].iout<<")";
	  LOG4CXX_DEBUG(_logLdaq,"execute "<<s0.str());
	  _my->executeQuery(s0.str());

	}
      std::stringstream s0;
      s0.str(std::string());
      s0<<"insert into BMPMON(P,TK) VALUES("<<_PRead<<","<<_TRead+273.15<<")";
      LOG4CXX_DEBUG(_logLdaq,"execute "<<s0.str());
      _my->executeQuery(s0.str());

      _my->disconnect();
      _bsem.unlock();
      for (int i=0;i<_storeTempo;i++)
	if (_storeRunning)
	  sleep((unsigned int) 1);
    }
  LOG4CXX_INFO(_logLdaq,"Storage thread stopped");
}


void WebSlowControl::stopMonitor(levbdim::fsmmessage* m)
{
  _storeRunning=false;
  g_store.join_all();
  LOG4CXX_INFO(_logLdaq,"Storage thread destroy");
  Json::Value jsta;
  jsta["action"]="MONITOR STOPPED";
  m->setAnswer(jsta);
  
}
void WebSlowControl::startCheck(levbdim::fsmmessage* m)
{
  _checkTempo =m->content()["period"].asUInt();
  g_check.create_thread(boost::bind(&WebSlowControl::doCheck, this));
  Json::Value jsta;
  jsta["action"]="HV CHECK STARTED";
  m->setAnswer(jsta);
	
}
void WebSlowControl::doCheck()
{
  _checkRunning=true;
  LOG4CXX_INFO(_logLdaq,"Check thread started");
  while (_checkRunning)
    {
      if (!this->getPTMean())
	{
	  sleep((unsigned int) 10);
	  continue;
	}
      _bsem.lock();
      _my->connect();
      for (std::vector<chamberRef>::iterator it=_chambers.begin();it!=_chambers.end();it++)
	{
	  std::stringstream s0;
	  s0.str(std::string());
	  s0<<"select VSET,VOUT FROM WIENERMON WHERE  HVCHAN="<<it->channel<<" ORDER BY IDX DESC LIMIT 1";
	  _my->executeSelect(s0.str());
	  MYSQL_ROW row=NULL;
	  float vset=0,vout=0;
	  while ((row=_my->getNextRow())!=0)
	    {
	      vset=atof(row[0]);
	      vout=abs(atof(row[1]));
	    }
 
	  float vexpected=it->vref*it->t0/it->p0*_PMean/_TMean;
	  float veffective=vout*it->p0/it->t0*_TMean/_PMean;
	  float deltav=abs(veffective-it->vref);
	  if (deltav>10 and deltav<200)
	    {
	      LOG4CXX_INFO(_logLdaq,"checkChannel "<<it->channel<<" : Vout"<<vout<<" Veffective "<<veffective<<" Vref "<<it->vref<<" , the expected value "<<vexpected<<" will be set automatically ");
	      this->dimsetVoltage(it->channel,vexpected);
	      this->dimreadChannel(it->channel);
	      std::cout<<hvinfoChannel(it->channel);
	    }
	  if (deltav>=200 && vout>=500)
	    LOG4CXX_WARN(_logLdaq,"checkChannel "<<it->channel<<" : Vout"<<vout<<" Veffective "<<veffective<<" Vref "<<it->vref<<" , the expected value "<<vexpected<<" cannot be set automatically  Delta= "<<deltav);
	  
	  if (deltav>=200 && vout<500)
	    LOG4CXX_WARN(_logLdaq,"checkChannel " <<it->channel<<" : Vout "<<vout<<" the channel is OFF");


	}
      //std::cout<<"disconnecting"<<std::endl;
      _my->disconnect();
      _bsem.unlock();
      //std::cout<<"sleeping"<<std::endl;
      for (int i=0;i<_checkTempo;i++)
	if (_checkRunning)
	  sleep((unsigned int) 1);

      //sleep((unsigned int) _checkTempo);
    }
  LOG4CXX_INFO(_logLdaq,"Check thread stopped");
}


void WebSlowControl::stopCheck(levbdim::fsmmessage* m)
{
  _checkRunning=false;
  g_check.join_all();
  LOG4CXX_INFO(_logLdaq,"Check thread destroy");
  Json::Value jsta;
  jsta["action"]="HV CHECK STOPPED";
  m->setAnswer(jsta);

}


