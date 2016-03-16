#ifndef _RpcShmServer_h

#define _RpcShmServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "dis.hxx"
#include "dic.hxx"

#include "ShmProxy.h"
#include "DIFReadoutConstant.h"
using namespace std;
#include <sstream>
#include <map>
#include <vector>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <boost/interprocess/sync/interprocess_mutex.hpp>

#include <string>
#include "CtrlLogger.h"

class RpcShmServer;

class RpcShmInitialise : public DimRpc
{
 public:
  RpcShmInitialise(RpcShmServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcShmServer* _server;
};
class RpcShmStart : public DimRpc
{
 public:
  RpcShmStart(RpcShmServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcShmServer* _server;
};

class RpcShmStop : public DimRpc
{
 public:
  RpcShmStop(RpcShmServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcShmServer* _server;
};

class RpcShmDirectory : public DimRpc
{
 public:
  RpcShmDirectory(RpcShmServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcShmServer* _server;
};

class RpcShmSetup : public DimRpc
{
 public:
  RpcShmSetup(RpcShmServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcShmServer* _server;
};

class RpcShmDestroy : public DimRpc
{
 public:
  RpcShmDestroy(RpcShmServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcShmServer* _server;
};


class RpcShmStatus : public DimRpc
{
 public:
  RpcShmStatus(RpcShmServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcShmServer* _server;
};


class RpcShmServer: public DimServer,public DimClient
{
public:
  RpcShmServer();

  ~RpcShmServer();

  virtual void infoHandler();
  void allocateCommands();
  void clearInfo();
  void registerDifs();
  void svc();
  void initialise();
  void destroy();
  // getters
  inline ShmProxy* getProxy() {return theProxy_;}
  inline std::string getDbState(){return _dbState;}
  inline uint32_t getDbRun(){return _dbRun;}

  inline void setState(std::string s){_state.assign(s);}
  inline std::string state() const {return _state;}
  // Publish DIM services
  inline void publishState(std::string s){setState(s);_shmState->updateService((char*) _state.c_str());}
  inline void publishRun(uint32_t r){_run=r;_runService->updateService();}
  inline uint32_t run(){return _run;}
  inline uint32_t event(){return _event;}
private:
  ShmProxy* theProxy_;
  boost::thread    theThread_;
  std::string _state;
  DimService* _shmState;
  DimService* _runService;
  DimService* _eventService;
  int32_t _run,_event;
  RpcShmInitialise* _initialiseCommand;
  RpcShmSetup* _setupCommand;
  RpcShmDirectory* _directoryCommand;
  RpcShmStart* _startCommand;
  RpcShmStop* _stopCommand;
  RpcShmDestroy* _destroyCommand;
  RpcShmStatus* _statusCommand;


  uint32_t _buffer[32*1024];
  DimInfo* _difData[255];
  
  int32_t _dbRun;
  DimInfo* _runInfo;
  std::string  _dbState;
  DimInfo* _dbstateInfo;
};
#endif

