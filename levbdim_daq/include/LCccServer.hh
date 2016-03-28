#ifndef _RpcCccServer_h

#define _RpcCccServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "dis.hxx"

#include "CCCManager.h"

using namespace std;
#include <sstream>
#include "DaqLogger.h"
class RpcCccServer;

class RpcCccOpen : public DimRpc
{
 public:
  RpcCccOpen(RpcCccServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcCccServer* _server;
};
class RpcCccInitialise : public DimRpc
{
 public:
  RpcCccInitialise(RpcCccServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcCccServer* _server;
};
class RpcCccConfigure : public DimRpc
{
 public:
  RpcCccConfigure(RpcCccServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcCccServer* _server;
};
class RpcCccStart : public DimRpc
{
 public:
  RpcCccStart(RpcCccServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcCccServer* _server;
};
class RpcCccStop : public DimRpc
{
 public:
  RpcCccStop(RpcCccServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcCccServer* _server;
};
class RpcCccPause : public DimRpc
{
 public:
  RpcCccPause(RpcCccServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcCccServer* _server;
};
class RpcCccResume : public DimRpc
{
 public:
  RpcCccResume(RpcCccServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcCccServer* _server;
};
class RpcCccDIFReset : public DimRpc
{
 public:
  RpcCccDIFReset(RpcCccServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcCccServer* _server;
};
class RpcCccCCCReset : public DimRpc
{
 public:
  RpcCccCCCReset(RpcCccServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcCccServer* _server;
};

class RpcCccWriteReg : public DimRpc
{
 public:
  RpcCccWriteReg(RpcCccServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcCccServer* _server;
};

class RpcCccReadReg : public DimRpc
{
 public:
  RpcCccReadReg(RpcCccServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcCccServer* _server;
};

class RpcCccServer: public DimServer
{
public:
  RpcCccServer();
  void allocateCommands();
  void Open(std::string s);
  void setState(std::string s){_state.assign(s);}
  inline std::string state() const {return _state;}
  // Publish DIM services
  inline void publishState(std::string s){setState(s);_csState->updateService((char*) _state.c_str());}

  // getters

  CCCManager* getManager(){return _manager;}
private:
  std::string _state;
  DimService* _csState; 
  RpcCccOpen* _openCommand;
  RpcCccInitialise*  _initialiseCommand;
  RpcCccConfigure* _configureCommand;
  RpcCccStart*  _startCommand;
  RpcCccStop* _stopCommand;
  RpcCccPause*  _pauseCommand;
  RpcCccResume* _resumeCommand;
  RpcCccDIFReset*  _difresetCommand;
  RpcCccCCCReset* _cccresetCommand;
  RpcCccWriteReg*  _writeregCommand;
  RpcCccReadReg* _readregCommand;

 
  CCCManager* _manager;
};
#endif

