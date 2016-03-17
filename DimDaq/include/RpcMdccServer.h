#ifndef _RpcMdccServer_h

#define _RpcMdccServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "dis.hxx"

#include "MDCCReadout.h"

using namespace std;
#include <sstream>
#include "DaqLogger.h"
class RpcMdccServer;

class RpcMdccOpen : public DimRpc
{
 public:
  RpcMdccOpen(RpcMdccServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcMdccServer* _server;
};

class RpcMdccClose : public DimRpc
{
 public:
  RpcMdccClose(RpcMdccServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcMdccServer* _server;
};
class RpcMdccMask : public DimRpc
{
 public:
  RpcMdccMask(RpcMdccServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcMdccServer* _server;
};
class RpcMdccReset : public DimRpc
{
 public:
  RpcMdccReset(RpcMdccServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcMdccServer* _server;
};
class RpcMdccStatus : public DimRpc
{
 public:
  RpcMdccStatus(RpcMdccServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcMdccServer* _server;
};
class RpcMdccWriteReg : public DimRpc
{
 public:
  RpcMdccWriteReg(RpcMdccServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcMdccServer* _server;
};

class RpcMdccReadReg : public DimRpc
{
 public:
  RpcMdccReadReg(RpcMdccServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcMdccServer* _server;
};

class RpcMdccServer: public DimServer
{
public:
  RpcMdccServer();
  void allocateCommands();
  void doOpen(std::string s);
  void setState(std::string s){_state.assign(s);}
  inline std::string state() const {return _state;}
  // Publish DIM services
  inline void publishState(std::string s){setState(s);_csState->updateService((char*) _state.c_str());}

  // getters

  MDCCReadout* getMDCCReadout(){  std::cout<<" get Ptr "<<_mdcc<<std::endl;
return _mdcc;}
private:
  std::string _state;
  DimService* _csState; 
  RpcMdccOpen* _openCommand;
  RpcMdccClose*  _closeCommand;
  RpcMdccMask* _maskCommand;
  RpcMdccReset*  _resetCommand;
  RpcMdccStatus* _statusCommand;
  RpcMdccWriteReg*  _writeregCommand;
  RpcMdccReadReg* _readregCommand;

 
  MDCCReadout* _mdcc;
};
#endif

