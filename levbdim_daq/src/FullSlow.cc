

#include "FullSlow.hh"
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



FullSlow::FullSlow(std::string name) : levbdim::baseApplication(name)
{
  _caenClient=0;_zupClient=0;_genesysClient=0;_bmpClient=0;_gpioClient=0;
 
  _fsm=this->fsm();
 
  _fsm->addState("DISCOVERED");
  _fsm->addState("CONFIGURED");
 
 
  _fsm->addTransition("DISCOVER","CREATED","DISCOVERED",boost::bind(&FullSlow::discover, this,_1));
  
  _fsm->addTransition("DESTROY","DISCOVERED","CREATED",boost::bind(&FullSlow::destroy, this,_1));
  _fsm->addTransition("CONFIGURE","DISCOVERED","CONFIGURED",boost::bind(&FullSlow::configure, this,_1));
  _fsm->addTransition("DESTROY","CONFIGURED","CREATED",boost::bind(&FullSlow::destroy, this,_1));

  // Commands
  _fsm->addCommand("LVSTATUS",boost::bind(&FullSlow::LVStatus,this,_1,_2));
  _fsm->addCommand("LVON",boost::bind(&FullSlow::LVON,this,_1,_2));
  _fsm->addCommand("LVOFF",boost::bind(&FullSlow::LVOFF,this,_1,_2));

  _fsm->addCommand("HVSTATUS",boost::bind(&FullSlow::HVStatus,this,_1,_2));
  _fsm->addCommand("HVON",boost::bind(&FullSlow::HVON,this,_1,_2));
  _fsm->addCommand("HVOFF",boost::bind(&FullSlow::HVOFF,this,_1,_2));
  _fsm->addCommand("SETVOLTAGE",boost::bind(&FullSlow::setVoltage,this,_1,_2));
  _fsm->addCommand("SETCURRENTLIMIT",boost::bind(&FullSlow::setCurrentLimit,this,_1,_2));

  cout<<"Building Fullslow"<<endl;
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0.str(std::string());
  s0<<"FullSlow-"<<hname;

  DimServer::start(s0.str().c_str()); 
  //  cout<<"Starting DimDaqCtrl"<<endl;

  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      std::cout<<"Service "<<name<<" started on port "<<atoi(wp)<<std::endl;
      _fsm->start(atoi(wp));
    }

  _jConfigContent=Json::Value::null;
  


  
}

void  FullSlow::userCreate(levbdim::fsmmessage* m)
{
  // Stored the configuration file used
  if (m->content().isMember("url"))
    {
      _jConfigContent["url"]=m->content()["url"];
      if (m->content().isMember("login"))
	_jConfigContent["login"]=m->content()["login"];
    }
    
  else
    if (m->content().isMember("file"))
      _jConfigContent["file"]=m->content()["file"];
}

void FullSlow::destroy(levbdim::fsmmessage* m)
{
   _caenClient=0;_zupClient=0;_genesysClient=0;_bmpClient=0;_gpioClient=0;
 
}
void FullSlow::configure(levbdim::fsmmessage* m)
{
  if (_caenClient!=0) _caenClient->sendTransition("CONFIGURE");
  if (_genesysClient!=0) _genesysClient->sendTransition("CONFIGURE");
 
}
void FullSlow::discover(levbdim::fsmmessage* m)
{

  Json::Value cjs=this->configuration()["HOSTS"];
  //  std::cout<<cjs<<std::endl;
  std::vector<std::string> lhosts=this->configuration()["HOSTS"].getMemberNames();
  // Loop on hosts
  for (auto host:lhosts)
    {
      //std::cout<<" Host "<<host<<" found"<<std::endl;
      // Loop on processes
      const Json::Value cjsources=this->configuration()["HOSTS"][host];
      //std::cout<<cjsources<<std::endl;
      for (Json::ValueConstIterator it = cjsources.begin(); it != cjsources.end(); ++it)
	{
	  const Json::Value& process = *it;
	  std::string p_name=process["NAME"].asString();
	  Json::Value p_param=Json::Value::null;
	  if (process.isMember("PARAMETER")) p_param=process["PARAMETER"];
	  // Loop on environenemntal variable
	  uint32_t port=0;
	  const Json::Value& cenv=process["ENV"];
	  for (Json::ValueConstIterator iev = cenv.begin(); iev != cenv.end(); ++iev)
	    {
	      std::string envp=(*iev).asString();
	      //      std::cout<<"Env found "<<envp.substr(0,7)<<std::endl;
	      //std::cout<<"Env found "<<envp.substr(8,envp.length()-7)<<std::endl;
	      if (envp.substr(0,7).compare("WEBPORT")==0)
		{
		  port=atol(envp.substr(8,envp.length()-7).c_str());
		  break;
		}
	    }
	  if (port==0) continue;
	  // Now analyse process Name
	  if (p_name.compare("CAEN")==0)
	    {
	      _caenClient= new fsmwebCaller(host,port);
	      std::string state=_caenClient->queryState();
	      printf("CAEN client %x  %s \n",_caenClient,state.c_str());
	      if (state.compare("VOID")==0 && !_jConfigContent.empty())
		{
		  _caenClient->sendTransition("CREATE",_jConfigContent);
		}
	      if (!p_param.empty()) this->parameters()["caen"]=p_param;
	    }
	  if (p_name.compare("GENESYS")==0)
	    {
	      _genesysClient= new fsmwebCaller(host,port);
	      std::string state=_genesysClient->queryState();
	      printf("GENESYS client %x  %s \n",_genesysClient,state.c_str());
	      if (state.compare("VOID")==0 && !_jConfigContent.empty())
		{
		  _genesysClient->sendTransition("CREATE",_jConfigContent);
		}
	      if (!p_param.empty()) this->parameters()["genesys"]=p_param;
	      //printf("DB client %x \n",_dbClient);
	    }
	  if (p_name.compare("BMP183")==0)
	    {
	      _bmpClient= new fsmwebCaller(host,port);
	      std::string state=_bmpClient->queryState();
	      printf("BMP183 client %x  %s \n",_bmpClient,state.c_str());
	      if (state.compare("VOID")==0 && !_jConfigContent.empty())
		{
		  _bmpClient->sendTransition("CREATE",_jConfigContent);
		}
	      if (!p_param.empty()) this->parameters()["bmp"]=p_param;
	      //printf("CCC client %x \n",_cccClient);
	    }
	  if (p_name.compare("ZUPSERVER")==0)
	    {
	      _zupClient= new fsmwebCaller(host,port);
	      std::string state=_zupClient->queryState();
	      printf("ZUP client %x  %s \n",_zupClient,state.c_str());
	      if (state.compare("VOID")==0 && !_jConfigContent.empty())
		{
		  _zupClient->sendTransition("CREATE",_jConfigContent);
		}
	      if (!p_param.empty()) this->parameters()["zup"]=p_param;
	      //printf("ZUP client %x \n",_zupClient);
	    }
	  if (p_name.compare("GPIOSERVER")==0)
	    {
	      _gpioClient= new fsmwebCaller(host,port);
	      std::string state=_gpioClient->queryState();
	      printf("GPIO client %x  %s \n",_gpioClient,state.c_str());
	      if (state.compare("VOID")==0 && !_jConfigContent.empty())
		{
		  _gpioClient->sendTransition("CREATE",_jConfigContent);
		}
	      if (!p_param.empty()) this->parameters()["gpio"]=p_param;

	      //printf("Gpio client %x \n",_gpioClient);
	      _gpioClient->sendTransition("CONFIGURE");
	      _gpioClient->sendCommand("VMEON");
	      _gpioClient->sendCommand("VMEOFF");
	      _gpioClient->sendCommand("VMEON");
	      
	    }
	}

    }
  
  printf("Clients: CAEN %x GENESYS %x ZUP %x BMP %x GPIO %x \n",_caenClient,_genesysClient,_zupClient,_bmpClient,_gpioClient);
}

FullSlow::~FullSlow()
{
 
}


void FullSlow::HVStatus(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  
  uint32_t first=atoi(request.get("first","0").c_str());
  uint32_t last=atoi(request.get("last","0").c_str());

  if (_caenClient==NULL)
    {
      LOG4CXX_ERROR(_logLdaq, "No CAEN client");response["STATUS"]="NO CAEN CLient";return;
    }
  std::stringstream sp;sp<<"&first="<<first<<"&last="<<last;
  _caenClient->sendCommand("GETSTATUS",sp.str());
  
  response["STATUS"]="DONE";
  response["ANSWER"]=_caenClient->answer();
}

void FullSlow::setVoltage(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t first=atoi(request.get("first","0").c_str());
  uint32_t last=atoi(request.get("last","0").c_str());
  float v=atof(request.get("value","0.0").c_str());
  if (_caenClient==NULL)
    {
      LOG4CXX_ERROR(_logLdaq, "No CAEN client");response["STATUS"]="NO CAEN CLient";return;
    }
  std::stringstream sp;sp<<"&first="<<first<<"&last="<<last<<"&value="<<v;
  _caenClient->sendCommand("SETOUTPUTVOLTAGE",sp.str());
  response["STATUS"]="DONE";
  response["ANSWER"]==_caenClient->answer();
 
}
void FullSlow::setCurrentLimit(Mongoose::Request &request, Mongoose::JsonResponse &response)
{

  uint32_t first=atoi(request.get("first","0").c_str());
  uint32_t last=atoi(request.get("last","0").c_str());
  float v=atof(request.get("value","1.0").c_str());
  if (_caenClient==NULL)
    {
      LOG4CXX_ERROR(_logLdaq, "No CAEN client");response["STATUS"]="NO CAEN CLient";return;
    }
  std::stringstream sp;sp<<"&first="<<first<<"&last="<<last<<"&value="<<v;
  _caenClient->sendCommand("SETCURRENTLIMIT",sp.str());
  response["STATUS"]="DONE";
  response["ANSWER"]==_caenClient->answer();
 

 
}

void FullSlow::HVON(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t first=atoi(request.get("first","0").c_str());
  uint32_t last=atoi(request.get("last","0").c_str());
  if (_caenClient==NULL)
    {
      LOG4CXX_ERROR(_logLdaq, "No CAEN client");response["STATUS"]="NO CAEN CLient";return;
    }
  std::stringstream sp;sp<<"&first="<<first<<"&last="<<last<<"&value=1";
  _caenClient->sendCommand("SETOUTPUTSWITCH",sp.str());
  response["STATUS"]="DONE";
  response["ANSWER"]==_caenClient->answer();
 


}
void FullSlow::HVOFF(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t first=atoi(request.get("first","0").c_str());
  uint32_t last=atoi(request.get("last","0").c_str());
  if (_caenClient==NULL)
    {
      LOG4CXX_ERROR(_logLdaq, "No CAEN client");response["STATUS"]="NO CAEN CLient";return;
    }
  std::stringstream sp;sp<<"&first="<<first<<"&last="<<last<<"&value=0";
  _caenClient->sendCommand("SETOUTPUTSWITCH",sp.str());
  response["STATUS"]="DONE";
  response["ANSWER"]==_caenClient->answer();
 
}

void FullSlow::LVON(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_genesysClient!=NULL){

    _genesysClient->sendCommand("ON");
    response["STATUS"]="DONE";
    response["LVON"]=_genesysClient->answer();
    return;
  }

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
void FullSlow::LVOFF(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_genesysClient!=NULL){

    _genesysClient->sendCommand("OFF");
    response["STATUS"]="DONE";
    response["LVOFF"]=_genesysClient->answer();
    return;
  }
  if (_zupClient!=NULL){

    _zupClient->sendTransition("OFF");
    response["STATUS"]="DONE";
    response["LVOFF"]=_zupClient->answer();
    return;
  }
  if (_gpioClient!=NULL){

    _gpioClient->sendTransition("OFF");
    response["STATUS"]="DONE";
    response["LVOFF"]=_gpioClient->answer();
    return;
  }
  LOG4CXX_ERROR(_logLdaq, "No zup client");response["STATUS"]="NO Zup CLient";return;
     
   

}
void  FullSlow::LVStatus(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_genesysClient!=NULL){

    _genesysClient->sendCommand("STATUS");
    response["STATUS"]="DONE";
    response["LVSTATUS"]=_genesysClient->answer();
    return;
  }
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
