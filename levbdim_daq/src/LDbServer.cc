
#include "LDbServer.hh"

void LDbServer::download(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  std::string dbstate=m->content()["dbstate"].asString();
  doDownload(dbstate);
}
void LDbServer::destroy(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  doDelete();
}
void LDbServer::newrun(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  int32_t rc=0;
  rc=this->getRunFromDb();
  Json::Value r;
  r["run"]=rc;
  r["state"]=_dbState;
  m->setAnswer(r);
}





LDbServer::LDbServer(std::string name)
{
  _fsm=new levbdim::fsm(name);
// Register state
  _fsm->addState("CREATED");
  _fsm->addState("DOWNLOAD");
  _fsm->addState("DELETED");

  _fsm->addTransition("DOWNLOAD","CREATED","DOWNLOAD",boost::bind(&LDbServer::download, this,_1));
  _fsm->addTransition("DOWNLOAD","DELETED","DOWNLOAD",boost::bind(&LDbServer::download, this,_1));
  _fsm->addTransition("DELETE","DOWNLOAD","DELETED",boost::bind(&LDbServer::destroy, this,_1));
  _fsm->addTransition("NEWRUN","DOWNLOAD","DOWNLOAD",boost::bind(&LDbServer::newrun, this,_1));

  
  std::stringstream s0;
  s0.str(std::string());
  s0<<"/DB/RUNFROMDB";
  runFromDb_=0;
  _runService = new DimService(s0.str().c_str(),runFromDb_);
  _runService->updateService();
  
  _dbState="NONE";
  _dbstateService= new DimService("/DB/DBSTATE",(char*) _dbState.c_str());
    


  s0.str(std::string());
  s0<<"LDbServer-"<<name;
  DimServer::start(s0.str().c_str()); 
  memset(_difInfos,0,255*sizeof(DIFDbInfo));
  memset(_difServices,0,255*sizeof(DimService*));
	

}
void LDbServer::doDelete()
{
  for (int i=0;i<255;i++)
    if (_difInfos[i].id!=0) delete _difServices[i];
  memset(_difInfos,0,255*sizeof(DIFDbInfo));
  memset(_difServices,0,255*sizeof(DimService*));
}
void LDbServer::doDownload(std::string state)
{
  _dbState=state;
  if (theDBManager_!=NULL)
    {
      this->doDelete(); //delete exiting service
      delete theDBManager_;
    }
  try {
    theDBManager_= new OracleDIFDBManager("74",_dbState);
    theDBManager_->initialize();
  }
  catch(...)
    {
      LOG4CXX_ERROR(_logLdaq,"initialise failed");
      return;
    }
  LOG4CXX_WARN(_logLdaq," Donwloading "<<_dbState);
  theDBManager_->download();
  std::map<uint32_t,unsigned char*> dbm=theDBManager_->getAsicKeyMap();

  for (std::map<uint32_t,unsigned char*>::iterator idb=dbm.begin();idb!=dbm.end();idb++)
    {
      uint32_t id = (idb->first>>8)&0xFF;
      _difInfos[id].id=id;
    }
  for (uint32_t i=0;i<255;i++)
    {

      if (_difInfos[i].id==0) continue;
      uint32_t id=i;
      _difInfos[id].nbasic=0;
      for (uint32_t iasic=1;iasic<=48;iasic++)
	{
	  uint32_t key=(id<<8)|iasic;
	  std::map<uint32_t,unsigned char*>::iterator it=dbm.find(key);
	  if (it==dbm.end()) continue;
	  unsigned char* bframe=it->second;

	  uint32_t       framesize=bframe[0];
	  memcpy(&_difInfos[id].slow[_difInfos[id].nbasic],&bframe[1],framesize);
	  _difInfos[id].nbasic++;
	}
      std::stringstream s0;
      
      s0<<"/DB/"<<_dbState<<"/DIF"<<id;
      _difServices[id] = new DimService(s0.str().c_str(),"I:2;C",&_difInfos[id],2*sizeof(uint32_t)+_difInfos[id].nbasic*sizeof(SingleHardrocV2ConfigurationFrame));
      s0.str(std::string());
      _difServices[id]->updateService();
    }
  _dbstateService->updateService((char*) _dbState.c_str());
}

uint32_t LDbServer::getRunFromDb()
{

  if (theRunInfo_==NULL)
    {
      try {


	std::stringstream daqname("");    
	char dateStr [64];
            
	time_t tm= time(NULL);
	strftime(dateStr,50,"LaDaqAToto_%d%m%y_%H%M%S",localtime(&tm));
	daqname<<dateStr;
	Daq* me=new Daq(daqname.str());

	printf("la daq est creee %s\n",daqname.str().c_str());
	me->setStatus(0);
	printf("la daq a change de statut\n");
	me->setXML("/opt/dhcal/include/dummy.xml");
	me->uploadToDatabase();
	printf("Upload DOne");
  
	theRunInfo_=new RunInfo(0,"LaDaqAToto");
	printf("le run est creee\n");
	theRunInfo_->setStatus(1);
	uint32_t ntry=0;
      getrun:
	try {
	  ntry++;
	  runFromDb_=theRunInfo_->getRunNumber();
	}
	catch (ILCException::Exception e)
	  {
	    if (ntry<2) {
	      ::sleep(2);
	      goto getrun;
	    }
	    else
	      {
		LOG4CXX_ERROR(_logLdaq,"Cannot get run number "<<e.getMessage());
		theRunInfo_=NULL;
		std::cout<<e.getMessage()<<std::endl;
		return 0;

	      }
	  }
      } catch (ILCException::Exception e)
	{
	  LOG4CXX_ERROR(_logLdaq,"Cannot get run number "<<e.getMessage());
	  theRunInfo_=NULL;
	  std::cout<<e.getMessage()<<std::endl;
	  return 0;
	}
    }
  else
    {
      theRunInfo_->setStatus(4);
      delete theRunInfo_;
      theRunInfo_=new RunInfo(0,"LaDaqAToto");
      theRunInfo_->setStatus(1);
      uint32_t ntry=0;
    getrun1:
	try {
	  ntry++;
	  runFromDb_=theRunInfo_->getRunNumber();
	}
	catch (ILCException::Exception e)
	  {
	    if (ntry<2) {
	      ::sleep(2);
	      goto getrun1;
	    }
	    else
	      {
		LOG4CXX_ERROR(_logLdaq,"Cannot get run number "<<e.getMessage());
		theRunInfo_=NULL;
		std::cout<<e.getMessage()<<std::endl;
		return 0;

	      }
	  }

	//runFromDb_=theRunInfo_->getRunNumber();
      
    }
  _runService->updateService();
  return runFromDb_;
}
