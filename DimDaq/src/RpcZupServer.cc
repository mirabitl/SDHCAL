
#include "RpcZupServer.h"


RpcZupOpen::RpcZupOpen(RpcZupServer* r,std::string name) : DimRpc(name.c_str(),"I:1;C","I:1"),_server(r) {}

void RpcZupOpen::rpcHandler()
{
  LOG4CXX_INFO(_logDDIF," CMD: OPEN called called"<<getData()<<" "<<getSize());
  uint32_t* ibuf=(uint32_t*) getData();
  unsigned char* cbuf= (unsigned char*) &ibuf[1];
  uint32_t port=ibuf[0];
  char name[80];
  memset(name,0,80);
  memcpy(name,&ibuf[1],getSize()-sizeof(int32_t));
  std::string s;s.assign(name);
  _server->Open(s,port);
  _server->publishState("OPENED");
  int32_t rc=0;
   setData(rc);
}
RpcZupRead::RpcZupRead(RpcZupServer* r,std::string name) : DimRpc(name.c_str(),"I:1","F:3"),_server(r) {}

void RpcZupRead::rpcHandler()
{
  LOG4CXX_INFO(_logDDIF," CMD: Read called");
  _server->Read();
  float rc[3];
  memcpy(rc,_server->readstatus(),3*sizeof(float));
  setData(rc,3*sizeof(float));
}
RpcZupSwitch::RpcZupSwitch(RpcZupServer* r,std::string name) : DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}

void RpcZupSwitch::rpcHandler()
{
  LOG4CXX_INFO(_logDDIF," CMD: Switch called"<<getInt());
  int32_t sta=getInt();
  
  _server->Switch(sta);
  int32_t rc=0;
   setData(rc);
}

RpcZupServer::RpcZupServer() : _zup(NULL)
{

  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0<<"/DZUP/"<<hname<<"/STATE";
  _state="CREATED";
  _zsState = new DimService(s0.str().c_str(),(char*) _state.c_str());
  s0.str(std::string());
  s0<<"/DZUP/"<<hname<<"/STATUS";
  memset(_status,0,3*sizeof(float));
  _zsStatus = new DimService(s0.str().c_str(),"F:3",_status,3*sizeof(float));
  allocateCommands();
  s0.str(std::string());
  gethostname(hname,80);
  s0<<"RpcZupServer-"<<hname;
  DimServer::start(s0.str().c_str()); 


	

}
void RpcZupServer::allocateCommands()
{
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0<<"/DZUP/"<<hname<<"/OPEN";
  _openCommand=new RpcZupOpen(this,s0.str());
  s0.str(std::string());
  s0<<"/DZUP/"<<hname<<"/SWITCH";
  _switchCommand=new RpcZupSwitch(this,s0.str());
  s0.str(std::string());
  s0<<"/DZUP/"<<hname<<"/READ";
  _readCommand=new RpcZupRead(this,s0.str());

}

void RpcZupServer::Open(std::string s, uint32_t port)
{
  
  if (_zup!=NULL)
    delete _zup;
  _zup= new Zup(s,port);

  this->Read();
}
void RpcZupServer::Read()
{
  if (_zup==NULL)
    {
      LOG4CXX_ERROR(_logDDIF," Zup not created ");
      return;
    }
  sleep((unsigned int) 1);
  _status[0]=_zup->ReadVoltageSet();
  _status[1]=_zup->ReadVoltageUsed();
  _status[2]=_zup->ReadCurrentUsed();
  this->publishStatus();
}

void RpcZupServer::Switch(uint32_t mode)
{
  if (_zup==NULL)
    {
      LOG4CXX_ERROR(_logDDIF," Zup not created ");
      return;
    }
  if (mode==0)
    {
      LOG4CXX_INFO(_logDDIF,"Switching OFF "<<mode);
      _zup->OFF();this->publishState("OFF");}
  else
    {
      LOG4CXX_INFO(_logDDIF,"Switching ON "<<mode);
      _zup->ON();this->publishState("ON");}
  this->Read();
}
