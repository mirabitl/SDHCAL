#include "LRootPublisher.hh"
LRootPublisher::LRootPublisher(std::string name) : levbdim::baseApplication(name),_directory("/dev/shm/root"),_detid(101),_running(false)
{
  _fsm= this->fsm();
  // Register state
  _fsm->addState("CONFIGURED");
  _fsm->addState("RUNNING");
  _fsm->addTransition("CONFIGURE","CREATED","CONFIGURED",boost::bind(&LRootPublisher::configure, this,_1));
  _fsm->addTransition("START","CONFIGURED","RUNNING",boost::bind(&LRootPublisher::start, this,_1));
  _fsm->addTransition("STOP","RUNNING","CONFIGURED",boost::bind(&LRootPublisher::stop, this,_1));
  _fsm->addTransition("DESTROY","CONFIGURED","CREATED",boost::bind(&LRootPublisher::destroy, this,_1));
  _fsm->addTransition("DESTROY","RUNNING","CREATED",boost::bind(&LRootPublisher::destroy, this,_1));
  _fsm->addCommand("STATUS",boost::bind(&LRootPublisher::cmdStatus,this,_1,_2));

  _sources.clear();
  
  //Start server
  std::stringstream s0;
  s0.str(std::string());
  s0<<"LRootPublisher-"<<name;
  DimServer::start(s0.str().c_str()); 
  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      std::cout<<"Service "<<name<<" started on port "<<atoi(wp)<<std::endl;
    _fsm->start(atoi(wp));
    }

}
void LRootPublisher::configure(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  Json::Value jc=m->content();
  _directory=jc["directory"].asString();
  _detid=jc["detid"].asUInt();

  for (std::vector<levbdim::datasource*>::iterator it=_sources.begin();it!=_sources.end();it++)
      delete (*it);
  _sources.clear();

  for (uint32_t i=1;i<=255;i++)
    {
      levbdim::datasource* ds= new levbdim::datasource(_detid,i,0x80000);
      _sources.push_back(ds);
    }
      
  

}
void LRootPublisher::destroy(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
}
void LRootPublisher::start(levbdim::fsmmessage* m)
{
    LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
    Json::Value jc=m->content();
    _run=jc["run"].asUInt();
    //_writer->start(run);
    _running=true;
    _gThread.create_thread(boost::bind(&LRootPublisher::scanMemory, this));
  //_gThread.create_thread(boost::bind(&levbdim::shmdriver::processEvents, this));

}
void LRootPublisher::scanMemory()
{
  //levbdim::buffer* b=new levbdim::buffer(0x80000);
  levbdim::buffer* b=new levbdim::buffer(0x80000);
  std::vector<std::string> vnames;
  while (_running)
    {
      
      levbdim::shmdriver::ls(_directory,vnames);
      if (vnames.size()==0) {::sleep(1);continue;}
      //continue;
      for ( std::vector<std::string>::iterator it=vnames.begin();it!=vnames.end();it++)
	{

	  levbdim::shmdriver::pull((*it),b,_directory);
	  //std::cout<<" Reading "<<_detid<<"-> "<<(*it)<<" "<<b->detectorId()<<" "<<b->dataSourceId()<<" "<<b->eventId()<<" "<<b->payloadSize()<<std::endl;
	  if (b->detectorId()!=_detid+100) continue;
	  memcpy(_sources[b->dataSourceId()]->buffer()->ptr(),b->ptr(),b->size());
	  _sources[b->dataSourceId()]->buffer()->setDetectorId(_detid);
	  _sources[b->dataSourceId()]->publish(b->eventId(),b->bxId(),b->payloadSize());
	  _lastEvent=b->eventId();
	}
      usleep(50000);	

    }
} 
void LRootPublisher::stop(levbdim::fsmmessage* m)
{
  
    LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
    _running=false;
    _gThread.join_all();


}
void LRootPublisher::cmdStatus(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
      response["STATUS"]="DONE";

      Json::Value rc;
      rc["directory"]=_directory;
      rc["detid"]=_detid;
      rc["run"]=_run;
      rc["event"]=_lastEvent;
      response["COUNTERS"]=rc;
      return;
}

