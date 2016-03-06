
#include "RpcCccServer.h"

RpcCccOpen::RpcCccOpen(RpcCccServer* r,std::string name) : DimRpc(name.c_str(),"C","I:1"),_server(r) {}

void RpcCccOpen::rpcHandler()
{
  LOG4CXX_INFO(_logDDIF," CMD: OPEN called called"<<getData()<<" "<<getSize());

  unsigned char* cbuf= (unsigned char*) getData();
  char name[80];
  memset(name,0,80);
  memcpy(name,cbuf,getSize());
  std::string s;s.assign(name);
  _server->Open(s);
  _server->publishState("INITIALISED");
  int32_t rc=0;
   setData(rc);
}

RpcCccInitialise::RpcCccInitialise(RpcCccServer* r,std::string name) : DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}

void RpcCccInitialise::rpcHandler()
{
  LOG4CXX_INFO(_logDDIF," CMD: INITIALISE called");
  int32_t rc=0;
  CCCManager* ccc= _server->getManager();
  if (ccc==NULL)
    {
      LOG4CXX_ERROR(_logDDIF,"Please open CCC DCCCCC01 first");
      _server->publishState("INITIALISE_FAILED");
      rc=-1;
      setData(rc);
      return;
    }
  _server->getManager()->initialise();
  _server->publishState("INITIALISED");

   setData(rc);
}
RpcCccConfigure::RpcCccConfigure(RpcCccServer* r,std::string name) : DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}

void RpcCccConfigure::rpcHandler()
{
  LOG4CXX_INFO(_logDDIF," CMD: CONFIGURE called");
  int32_t rc=0;
  CCCManager* ccc= _server->getManager();
  if (ccc==NULL)
    {
      LOG4CXX_ERROR(_logDDIF,"Please open CCC DCCCCC01 first");
      _server->publishState("CONFIGURE_FAILED");
      rc=-1;
      setData(rc);
      return;
    }
  _server->getManager()->configure();
  _server->publishState("CONFIGURED");

   setData(rc);

}
RpcCccStart::RpcCccStart(RpcCccServer* r,std::string name) : DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}

void RpcCccStart::rpcHandler()
{
  LOG4CXX_INFO(_logDDIF," CMD: START called");
  int32_t rc=0;
  CCCManager* ccc= _server->getManager();
  if (ccc==NULL)
    {
      LOG4CXX_ERROR(_logDDIF,"Please open CCC DCCCCC01 first");
      _server->publishState("START_FAILED");
      rc=-1;
      setData(rc);
      return;
    }
  _server->getManager()->start();
  _server->publishState("RUNNING");

   setData(rc);

}

RpcCccStop::RpcCccStop(RpcCccServer* r,std::string name) : DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}

void RpcCccStop::rpcHandler()
{
  LOG4CXX_INFO(_logDDIF," CMD: STOP called");
  int32_t rc=0;
  CCCManager* ccc= _server->getManager();
  if (ccc==NULL)
    {
      LOG4CXX_ERROR(_logDDIF,"Please open CCC DCCCCC01 first");
      _server->publishState("STOP_FAILED");
      rc=-1;
      setData(rc);
      return;
    }
  _server->getManager()->configure();
  _server->publishState("STOPPED");

   setData(rc);

}
RpcCccPause::RpcCccPause(RpcCccServer* r,std::string name) : DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}

void RpcCccPause::rpcHandler()
{
  LOG4CXX_INFO(_logDDIF," CMD: PAUSE called");
  int32_t rc=0;
  CCCManager* ccc= _server->getManager();
  if (ccc==NULL)
    {
      LOG4CXX_ERROR(_logDDIF,"Please open CCC DCCCCC01 first");
      _server->publishState("PAUSE_FAILED");
      rc=-1;
      setData(rc);
      return;
    }
  ccc->getCCCReadout()->DoSendPauseTrigger();
  _server->publishState("PAUSED");

   setData(rc);

}

RpcCccResume::RpcCccResume(RpcCccServer* r,std::string name) : DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}
void RpcCccResume::rpcHandler()
{
  LOG4CXX_INFO(_logDDIF," CMD: RESUME called");
  int32_t rc=0;
  CCCManager* ccc= _server->getManager();
  if (ccc==NULL)
    {
      LOG4CXX_ERROR(_logDDIF,"Please open CCC DCCCCC01 first");
      _server->publishState("RESUME_FAILED");
      rc=-1;
      setData(rc);
      return;
    }
  ccc->getCCCReadout()->DoSendResumeTrigger();
  _server->publishState("RUNNING");

   setData(rc);

}

RpcCccDIFReset::RpcCccDIFReset(RpcCccServer* r,std::string name) : DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}
void RpcCccDIFReset::rpcHandler()
{
  LOG4CXX_INFO(_logDDIF," CMD: DIFRESET called");
  int32_t rc=0;
  CCCManager* ccc= _server->getManager();
  if (ccc==NULL)
    {
      LOG4CXX_ERROR(_logDDIF,"Please open CCC DCCCCC01 first");
      _server->publishState("DIFRESET_FAILED");
      rc=-1;
      setData(rc);
      return;
    }
  ccc->getCCCReadout()->DoSendDIFReset();


   setData(rc);

}

RpcCccCCCReset::RpcCccCCCReset(RpcCccServer* r,std::string name) : DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}
void RpcCccCCCReset::rpcHandler()
{
  LOG4CXX_INFO(_logDDIF," CMD: CCCRESET called");
  int32_t rc=0;
  CCCManager* ccc= _server->getManager();
  if (ccc==NULL)
    {
      LOG4CXX_ERROR(_logDDIF,"Please open CCC DCCCCC01 first");
      _server->publishState("CCCRESET_FAILED");
      rc=-1;
      setData(rc);
      return;
    }
  ccc->getCCCReadout()->DoSendCCCReset();


   setData(rc);

}

RpcCccWriteReg::RpcCccWriteReg(RpcCccServer* r,std::string name) : DimRpc(name.c_str(),"I:2","I:1"),_server(r) {}

void RpcCccWriteReg::rpcHandler()
{
  uint32_t* ibuf=(uint32_t*) getData();
  LOG4CXX_INFO(_logDDIF," CMD: WRITEREG called reg["<<ibuf[0]<<"]<-"<<ibuf[1]);
  int32_t rc=0;
  CCCManager* ccc= _server->getManager();
  if (ccc==NULL)
    {
      LOG4CXX_ERROR(_logDDIF,"Please open CCC DCCCCC01 first");
      _server->publishState("WRITEREG_FAILED");
      rc=-1;
      setData(rc);
      return;
    }
  ccc->getCCCReadout()->DoWriteRegister(ibuf[0],ibuf[1]);


  setData(rc);

}

RpcCccReadReg::RpcCccReadReg(RpcCccServer* r,std::string name) : DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}

void RpcCccReadReg::rpcHandler()
{
  uint32_t adr=getInt();
  LOG4CXX_INFO(_logDDIF," CMD: READREG called reg["<<adr<<"]");
  int32_t rc=0;
  CCCManager* ccc= _server->getManager();
  if (ccc==NULL)
    {
      LOG4CXX_ERROR(_logDDIF,"Please open CCC DCCCCC01 first");
      _server->publishState("READREG_FAILED");
      rc=-1;
      setData(rc);
      return;
    }
  rc=ccc->getCCCReadout()->DoReadRegister(adr);


  setData(rc);

}


RpcCccServer::RpcCccServer() : _manager(NULL)
{

  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);

  s0<<"/DCS/"<<hname<<"/STATE";
  _state="CREATED";
  _csState = new DimService(s0.str().c_str(),(char*) _state.c_str());

  allocateCommands();
  s0.str(std::string());
  gethostname(hname,80);
  s0<<"RpcCccServer-"<<hname;
  DimServer::start(s0.str().c_str()); 


	

}

void RpcCccServer::allocateCommands()
{
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0<<"/DCS/"<<hname<<"/OPEN";
  _openCommand = new RpcCccOpen(this,s0.str());
  s0.str(std::string());
  s0<<"/DCS/"<<hname<<"/INITIALISE";
  _initialiseCommand=new RpcCccInitialise(this,s0.str());
  s0.str(std::string());
  s0<<"/DCS/"<<hname<<"/CONFIGURE";
  _configureCommand=new RpcCccConfigure(this,s0.str());
  s0.str(std::string());
  s0<<"/DCS/"<<hname<<"/START";
  _startCommand=new RpcCccStart(this,s0.str());
  s0.str(std::string());
  s0<<"/DCS/"<<hname<<"/STOP";
  _stopCommand=new RpcCccStop(this,s0.str());
  s0.str(std::string());
  s0<<"/DCS/"<<hname<<"/PAUSE";
  _pauseCommand=new RpcCccPause(this,s0.str());
  s0.str(std::string());
  s0<<"/DCS/"<<hname<<"/RESUME";
  _resumeCommand=new RpcCccResume(this,s0.str());
  s0.str(std::string());
  s0<<"/DCS/"<<hname<<"/DIFRESET";
  _difresetCommand=new RpcCccDIFReset(this,s0.str());
  s0.str(std::string());
  s0<<"/DCS/"<<hname<<"/CCCRESET";
  _cccresetCommand=new RpcCccCCCReset(this,s0.str());
  s0.str(std::string());
  s0<<"/DCS/"<<hname<<"/WRITEREG";
  _writeregCommand=new RpcCccWriteReg(this,s0.str());
  s0.str(std::string());
  s0<<"/DCS/"<<hname<<"/READREG";
  _readregCommand=new RpcCccReadReg(this,s0.str());


}

void RpcCccServer::Open(std::string s)
{
  if (_manager!=NULL)
    delete _manager;
  _manager= new CCCManager(s);
}
