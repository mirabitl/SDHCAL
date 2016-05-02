

#include "WebJobControl.hh"
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



WebJobControl::WebJobControl(std::string name,uint32_t port) : _djc(NULL)
{
  printf("parsing the config file \n");
  DOMConfigurator::configure("/etc/Log4cxxConfig.xml");
  //_logger->setLevel(log4cxx::Level::getInfo());
  LOG4CXX_INFO (_logLdaq, "this is a info message, after parsing configuration file")
    
  _fsm=new fsmweb(name);
 
  _fsm->addState("CREATED");
  _fsm->addState("INITIALISED");
  _fsm->addState("RUNNING");
  _fsm->addState("KILLED");

 
  _fsm->addTransition("INITIALISE","CREATED","INITIALISED",boost::bind(&WebJobControl::initialise, this,_1));
  _fsm->addTransition("START","INITIALISED","RUNNING",boost::bind(&WebJobControl::start, this,_1));
  _fsm->addTransition("KILL","RUNNING","INITIALISED",boost::bind(&WebJobControl::kill, this,_1));
  _fsm->addTransition("KILL","INITIALISED","INITIALISED",boost::bind(&WebJobControl::kill, this,_1));
  _fsm->addTransition("DESTROY","INITIALISED","CREATED",boost::bind(&WebJobControl::destroy, this,_1));

  // Commands

  _fsm->addCommand("STATUS",boost::bind(&WebJobControl::status,this,_1,_2));
  _fsm->addCommand("KILLJOB",boost::bind(&WebJobControl::killjob,this,_1,_2));
  _fsm->addCommand("RESTARTJOB",boost::bind(&WebJobControl::restartjob,this,_1,_2));
  
  cout<<"Building WebJobControl"<<endl;
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0.str(std::string());
  s0<<"WebJob-"<<hname;

  DimServer::start(s0.str().c_str()); 
  //  cout<<"Starting DimDaqCtrl"<<endl;
  _fsm->start(port);
}
void WebJobControl::initialise(levbdim::fsmmessage* m)
{
  _djc= new DimJobInterface();
  std::string config =m->content()["jsonfile"].asString();
  _djc->loadJSON(config);
  _djc->status();
  ::sleep(2);
  m->setAnswer(_djc->getProcessStatusValue());
}
void WebJobControl::start(levbdim::fsmmessage* m)
{

  _djc->startJobs("ALL");
  _djc->status();
  ::sleep(2);
  m->setAnswer(_djc->getProcessStatusValue());
}
void WebJobControl::kill(levbdim::fsmmessage* m)
{

  _djc->clearAllJobs();
  _djc->status();
  ::sleep(2);
  m->setAnswer(_djc->getProcessStatusValue());
}
void WebJobControl::destroy(levbdim::fsmmessage* m)
{
  delete _djc;
  _djc=NULL;
}

WebJobControl::~WebJobControl()
{
 
}
void WebJobControl::status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_djc==NULL)
    {
      response["STATUS"]="No jobcontrol initialised";
      return;
    }
  _djc->status();
  ::sleep(2);
  response["ANSWER"]=_djc->getProcessStatusValue();
  response["STATUS"]="DONE";
  
}

void WebJobControl::killjob(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  int32_t pid=atoi(request.get("pid","-1").c_str());
  std::string host=request.get("host","none");
  if (host.compare("none")==0 || pid<0)
    {
      response["STATUS"]="host or  pid not specified";
      return;

    }
  if (_djc==NULL)
    {
      response["STATUS"]="No jobcontrol initialised";
      return;
    }
  _djc->killJob(host,pid);
  _djc->status();
  ::sleep(2);
  response["ANSWER"]=_djc->getProcessStatusValue();
  response["STATUS"]="DONE";
}
void WebJobControl::restartjob(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  int32_t pid=atoi(request.get("pid","-1").c_str());
  std::string host=request.get("host","none");
  std::string job=request.get("job","none");
  if (host.compare("none")==0 || pid<0 || job.compare("none")==0 )
    {
      response["STATUS"]="host,job or  pid not specified";
      return;

    }
  if (_djc==NULL)
    {
      response["STATUS"]="No jobcontrol initialised";
      return;
    }
  _djc->restartJob(host,job,pid);
  _djc->status();
  ::sleep(2);
  response["ANSWER"]=_djc->getProcessStatusValue();
  response["STATUS"]="DONE";
}
