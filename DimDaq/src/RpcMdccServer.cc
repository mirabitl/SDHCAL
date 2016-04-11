
#include "RpcMdccServer.h"

RpcMdccOpen::RpcMdccOpen(RpcMdccServer* r,std::string name) : DimRpc(name.c_str(),"C","I:1"),_server(r) {}

void RpcMdccOpen::rpcHandler()
{
  LOG4CXX_INFO(_logDDIF," CMD: OPEN called called"<<getData()<<" "<<getSize());

  unsigned char* cbuf= (unsigned char*) getData();
  char name[80];
  memset(name,0,80);
  memcpy(name,cbuf,getSize());
  std::string s;s.assign(name);
  LOG4CXX_INFO(_logDDIF," Device "<<s);
  _server->doOpen(s);
  _server->publishState("OPENED");
  int32_t rc=0;
   setData(rc);
}

RpcMdccClose::RpcMdccClose(RpcMdccServer* r,std::string name) : DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}

void RpcMdccClose::rpcHandler()
{
  LOG4CXX_INFO(_logDDIF," CMD: CLOSE called");
  int32_t rc=0;
  MDCCReadout* mdc=_server->getMDCCReadout();
  if (mdc==NULL)
    {
      LOG4CXX_ERROR(_logDDIF,"Please open MDC01 first");
      _server->publishState("CLOSE_FAILED");
      rc=-1;
      setData(rc);
      return;
    }
  mdc->close();
  _server->publishState("CLOSED");

   setData(rc);
}
RpcMdccMask::RpcMdccMask(RpcMdccServer* r,std::string name) : DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}

void RpcMdccMask::rpcHandler()
{
  LOG4CXX_INFO(_logDDIF," CMD: Mask called with "<<getInt());
  int32_t rc=0;
  MDCCReadout* mdc=_server->getMDCCReadout();
  if (mdc==NULL)
    {
      LOG4CXX_ERROR(_logDDIF,"Please open MDCCC01 first");
      _server->publishState("MASK_FAILED");
      rc=-1;
      setData(rc);
      return;
    }
  if (getInt()==1)
    {
      mdc->maskTrigger();
      _server->publishState("MASKED");
    }
  else
    {
      mdc->unmaskTrigger();
      _server->publishState("UNMASKED");
    }
    


   setData(rc);

}
RpcMdccReset::RpcMdccReset(RpcMdccServer* r,std::string name) : DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}

void RpcMdccReset::rpcHandler()
{
  LOG4CXX_INFO(_logDDIF," CMD: RESET called");
  int32_t rc=0;
  MDCCReadout* mdc=_server->getMDCCReadout();
  if (mdc==NULL)
    {
      LOG4CXX_ERROR(_logDDIF,"Please open MDCC01 first");
      _server->publishState("RESET_FAILED");
      rc=-1;
      setData(rc);
      return;
    }
  mdc->resetCounter();


   setData(rc);

}

RpcMdccStatus::RpcMdccStatus(RpcMdccServer* r,std::string name) : DimRpc(name.c_str(),"I:1","I:5"),_server(r) {}

void RpcMdccStatus::rpcHandler()
{
  LOG4CXX_INFO(_logDDIF," CMD: STATUS called");
  int32_t rc[5];
  memset(rc,0,5*sizeof(int32_t));
  MDCCReadout* mdc=_server->getMDCCReadout();
  if (mdc==NULL)
    {
      LOG4CXX_ERROR(_logDDIF,"Please open MDC DMDCCC01 first");
      _server->publishState("STATUS_FAILED");
      setData(rc,5*sizeof(int32_t));
      return;
    }
  rc[0]=mdc->version();
  rc[1]=mdc->id();
  rc[2]=mdc->mask();
  rc[3]=mdc->spillCount();
  rc[4]=mdc->busy1Count();


   setData(rc,5*sizeof(int32_t));

}

RpcMdccWriteReg::RpcMdccWriteReg(RpcMdccServer* r,std::string name) : DimRpc(name.c_str(),"I:2","I:1"),_server(r) {}

void RpcMdccWriteReg::rpcHandler()
{
  uint32_t* ibuf=(uint32_t*) getData();
  LOG4CXX_INFO(_logDDIF," CMD: WRITEREG called reg["<<ibuf[0]<<"]<-"<<ibuf[1]);
  int32_t rc=0;
  MDCCReadout* mdc=_server->getMDCCReadout();
  if (mdc==NULL)
    {
      LOG4CXX_ERROR(_logDDIF,"Please open MDCC01 first");
      _server->publishState("WRITEREG_FAILED");
      rc=-1;
      setData(rc);
      return;
    }
  mdc->writeRegister(ibuf[0],ibuf[1]);


  setData(rc);

}

RpcMdccReadReg::RpcMdccReadReg(RpcMdccServer* r,std::string name) : DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}

void RpcMdccReadReg::rpcHandler()
{
  uint32_t adr=getInt();
  LOG4CXX_INFO(_logDDIF," CMD: READREG called reg["<<adr<<"]");
  int32_t rc=0;
  MDCCReadout* mdc=_server->getMDCCReadout();
  if (mdc==NULL)
    {
      LOG4CXX_ERROR(_logDDIF,"Please open MDCCC01 first");
      _server->publishState("READREG_FAILED");
      rc=-1;
      setData(rc);
      return;
    }
  rc=mdc->readRegister(adr);


  setData(rc);

}


RpcMdccServer::RpcMdccServer() : _mdcc(NULL)
{

  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);

  s0<<"/MDCS/STATE";
  _state="CREATED";
  _csState = new DimService(s0.str().c_str(),(char*) _state.c_str());

  allocateCommands();
  s0.str(std::string());
  gethostname(hname,80);
  s0<<"RpcMdccServer-"<<hname;
  DimServer::start(s0.str().c_str()); 


	

}

void RpcMdccServer::allocateCommands()
{
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0<<"/MDCS/OPEN";
  _openCommand = new RpcMdccOpen(this,s0.str());
  s0.str(std::string());
  s0<<"/MDCS/CLOSE";
  _closeCommand=new RpcMdccClose(this,s0.str());
  s0.str(std::string());
  s0<<"/MDCS/MASK";
  _maskCommand=new RpcMdccMask(this,s0.str());
  s0.str(std::string());
  s0<<"/MDCS/RESET";
  _resetCommand=new RpcMdccReset(this,s0.str());
  s0.str(std::string());
  s0<<"/MDCS/STATUS";
  _statusCommand=new RpcMdccStatus(this,s0.str());
 s0.str(std::string());
  s0<<"/MDCS/WRITEREG";
  _writeregCommand=new RpcMdccWriteReg(this,s0.str());
  s0.str(std::string());
  s0<<"/MDCS/READREG";
  _readregCommand=new RpcMdccReadReg(this,s0.str());


}

void RpcMdccServer::doOpen(std::string s)
{
  std::cout<<"calling open "<<std::endl;
  if (_mdcc!=NULL)
    delete _mdcc;
  _mdcc= new MDCCReadout(s);
  _mdcc->open();
  //std::cout<<" Open Ptr "<<_mdcc<<std::endl;
}
