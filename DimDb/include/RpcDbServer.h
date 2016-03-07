#ifndef _RpcDbServer_h

#define _RpcDbServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "dis.hxx"
#include "OracleDIFDBManager.h"
#include "DIFReadoutConstant.h"
#include "DbLogger.h"
using namespace std;
#include <sstream>

class RpcDbServer;

class RpcDbDownload : public DimRpc
{
 public:
  RpcDbDownload(RpcDbServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcDbServer* _server;
  std::string _state;
};

class RpcDbDelete : public DimRpc
{
 public:
  RpcDbDelete(RpcDbServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcDbServer* _server;
};
class RpcDbGetRun : public DimRpc
{
 public:
  RpcDbGetRun(RpcDbServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcDbServer* _server;
};


class RpcDbServer: public DimServer
{
public:
  RpcDbServer();
  void allocateCommands();


  void doInitialise();
  void doDownload(std::string dbstate);
  void doDelete();
  uint32_t getRunFromDb();
  void setState(std::string s){_state.assign(s);}
  inline std::string state() const {return _state;}
  // Publish DIM services
  inline void publishState(std::string s){setState(s);_ddbState->updateService((char*) _state.c_str());}

private:
  std::string _state;
  std::string _dbState;
  int32_t runFromDb_;
  DIFDbInfo _difInfos[255];
  DimService* _ddbState; //State of the process 
  DimService* _runService; //State of the process 
  DimService* _dbstateService;
  DimService* _difServices[255]; //State of the last register read

  RpcDbDownload* _downloadCommand;
  RpcDbDelete* _deleteCommand;
  RpcDbGetRun* _getrunCommand;
  RunInfo* theRunInfo_;
 
  OracleDIFDBManager* theDBManager_;
};
#endif

