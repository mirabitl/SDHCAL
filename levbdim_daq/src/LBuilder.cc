#include "LBuilder.hh"
#include "basicwriter.hh"
#include "monitorProcessor.hh"
#ifdef USE_LCIO
#include "LcioShmProcessor.hh"
#include "RootShmProcessor.hh"
#endif
LBuilder::LBuilder(std::string name) : levbdim::baseApplication(name),_evb(NULL),_writer(NULL)
{
  //_fsm= new fsmweb(name); 
  _fsm=this->fsm();
  // Register state
  //_fsm->addState("CREATED");
  _fsm->addState("INITIALISED");
  _fsm->addState("CONFIGURED");
  _fsm->addState("RUNNING");
  _fsm->addTransition("INITIALISE","CREATED","INITIALISED",boost::bind(&LBuilder::initialise, this,_1));
  _fsm->addTransition("CONFIGURE","INITIALISED","CONFIGURED",boost::bind(&LBuilder::configure, this,_1));
  _fsm->addTransition("START","CONFIGURED","RUNNING",boost::bind(&LBuilder::start, this,_1));
  _fsm->addTransition("STOP","RUNNING","CONFIGURED",boost::bind(&LBuilder::stop, this,_1));
  _fsm->addTransition("HALT","RUNNING","INITIALISED",boost::bind(&LBuilder::halt, this,_1));
  _fsm->addTransition("DESTROY","INITIALISED","CREATED",boost::bind(&LBuilder::destroy, this,_1));
  _fsm->addTransition("STATUS","CONFIGURED","CONFIGURED",boost::bind(&LBuilder::status, this,_1));
  _fsm->addTransition("STATUS","RUNNING","RUNNING",boost::bind(&LBuilder::status, this,_1));

  _fsm->addTransition("REGISTERDS","INITIALISED","INITIALISED",boost::bind(&LBuilder::registerDataSource,this,_1));


  _fsm->addCommand("SETHEADER",boost::bind(&LBuilder::c_setheader,this,_1,_2));
  //Start server
  std::stringstream s0;
  s0.str(std::string());
  s0<<"LBuilder-"<<name;
  DimServer::start(s0.str().c_str()); 
  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      std::cout<<"Service "<<name<<" started on port "<<atoi(wp)<<std::endl;
    _fsm->start(atoi(wp));
    }

}
void LBuilder::c_setheader(Mongoose::Request &request, Mongoose::JsonResponse &response)
{

  if (_evb==NULL)    {response["STATUS"]="NO EVB created"; return;}
  std::string shead=request.get("header","None");
  if (shead.compare("None")==0)
    {response["STATUS"]="NO header provided "; return;}
  Json::Reader reader;
  Json::Value jsta;
  bool parsingSuccessful = reader.parse(shead,jsta);
  if (!parsingSuccessful)
    {response["STATUS"]="Cannot parse header tag "; return;}
  const Json::Value& jdevs=jsta;
  std::vector<uint32_t>& v=_evb->runHeader();
  v.clear();
  for (Json::ValueConstIterator jt = jdevs.begin(); jt != jdevs.end(); ++jt)
    v.push_back((*jt).asInt());
  _evb->processRunHeader();
  response["STATUS"]="DONE";
  response["VALUE"]=jsta;

}
void LBuilder::registerDataSource(levbdim::fsmmessage* m)
{

  
  uint32_t detid=m->content()["detid"].asUInt();
  uint32_t sid=m->content()["sourceid"].asUInt();

  levbdim::datasocket* ds= new levbdim::datasocket(detid,sid,0x80000);
  ds->save2disk(_memorypath);
  _sources.push_back(ds);
  _evb->registerDataSource(detid,sid);
  Json::Value response;
  response["STATUS"]="DONE";
  response["SOURCE"]=sid;
  response["DETECTOR"]=detid;
  m->setAnswer(response);
} 
void LBuilder::configure(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  _sources.clear();
  for (std::vector<levbdim::datasocket*>::iterator it=_sources.begin();it!=_sources.end();it++)
      delete (*it);

  Json::Value jc=m->content();
  const Json::Value& jsources = jc["sources"];
  for (Json::ValueConstIterator it = jsources.begin(); it != jsources.end(); ++it)
    {
      const Json::Value& source = *it;
      int32_t detid=source["detid"].asInt();
      int32_t sid=source["sourceid"].asInt();
    // rest as before
      LOG4CXX_INFO(_logLdaq,"Creating datasocket "<<detid<<" "<<sid);
      levbdim::datasocket* ds= new levbdim::datasocket(detid,sid,0x80000);
      ds->save2disk(_memorypath);
      _sources.push_back(ds);
      _evb->registerDataSource(detid,sid);
    }

}
void LBuilder::destroy(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  if (_evb!=NULL)
    delete _evb;
  _evb=NULL;
  if (_writer!=NULL)
    delete _writer;
}
void LBuilder::initialise(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  if (_evb!=NULL)
    delete _evb;

   Json::Value jc=m->content();
   if (jc.isMember("memorypath"))
     {
       _memorypath = m->content()["memorypath"].asString();
       this->parameters()["memorypath"]=jc["memorypath"];
     }
   else
     _memorypath=this->parameters()["memorypath"].asString();
   
  _filepath = m->content()["filepath"].asString();
  if (jc.isMember("filepath"))
     {
       _filepath = m->content()["filepath"].asString();
       this->parameters()["filepath"]=jc["filepath"];
     }
   else
     _filepath=this->parameters()["filepath"].asString();
   
  _proctype= m->content()["proctype"].asString();
  // Now create the builder

  _evb= new levbdim::shmdriver(_memorypath);
  _evb->createDirectories();
  _evb->cleanShm();


  
  Json::Value jsources;
  if (jc.isMember("proclist"))
    {
      jsources= jc["proclist"];
      this->parameters()["proclist"]=jsources;
    }
  else
    jsources=this->parameters()["proclist"];
  const Json::Value& cjsources=jsources;
  for (Json::ValueConstIterator it = cjsources.begin(); it != cjsources.end(); ++it)
    {
      const Json::Value& source = *it;
      std::string name=source.asString();
        // Add a writer
      if (name.compare("basicwriter")==0)
    {
      _writer= new levbdim::basicwriter(_filepath);
      _evb->registerProcessor(_writer);
    }
      if (name.compare("monitor")==0)
    {
      levbdim::shmprocessor* m= new levbdim::monitorProcessor("/dev/shm/monitor");
      _evb->registerProcessor(m);
    }

#ifdef USE_LCIO
  if (name.compare("lcio")==0)
    {
      _writer= new LcioShmProcessor(_filepath,"UNSETUP");
      _evb->registerProcessor(_writer);
    }
  if (name.compare("root")==0)
    {
      _writer= new RootShmProcessor(_filepath,"/opt/dhcal/etc/geometry.xml");
      _evb->registerProcessor(_writer);
    }
#endif

    }


  /*  
  // Add a writer
  if (_proctype.compare("basicwriter")==0)
    {
      _writer= new levbdim::basicwriter(_filepath);
      _evb->registerProcessor(_writer);
    }
#ifdef USE_LCIO
  if (_proctype.compare("lcio")==0)
    {
      _writer= new LcioShmProcessor(_filepath,"UNSETUP");
      _evb->registerProcessor(_writer);
    }
#endif
  */
}
void LBuilder::start(levbdim::fsmmessage* m)
{
    LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
    Json::Value jc=m->content();
    int32_t run=jc["run"].asInt();
    //_writer->start(run);
    _evb->start(run);
}
void LBuilder::stop(levbdim::fsmmessage* m)
{
  
    LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());

    _evb->stop();
    //_writer->stop();
}
void LBuilder::status(levbdim::fsmmessage* m)
{
  
    LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
    Json::Value rp;
    rp["run"]=_evb->run();
    rp["event"]=_evb->event();
    m->setAnswer(rp);
}
void LBuilder::halt(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  this->stop(m);
  //delete data sockets
  for (std::vector<levbdim::datasocket*>::iterator it=_sources.begin();it!=_sources.end();it++)
    delete (*it);
  _sources.clear();
}
