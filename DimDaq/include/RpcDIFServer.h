#ifndef _RpcDIFServer_h

#define _RpcDIFServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "dis.hxx"
#include "dic.hxx"
#include "DimDIF.h"
#include "DIFReadoutConstant.h"
using namespace std;
#include <sstream>
#include <map>
#include <vector>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "DaqLogger.h"

class RpcDIFServer;

class RpcDIFRegisterDB : public DimRpc
{
 public:
  RpcDIFRegisterDB(RpcDIFServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcDIFServer* _server;
};

class RpcDIFScan : public DimRpc
{
 public:
  RpcDIFScan(RpcDIFServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcDIFServer* _server;
  int _ndif;
  int _buf[256];
};
class RpcDIFInitialise : public DimRpc
{
 public:
  RpcDIFInitialise(RpcDIFServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcDIFServer* _server;
};

class RpcDIFConfigure : public DimRpc
{
 public:
  RpcDIFConfigure(RpcDIFServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcDIFServer* _server;
};
class RpcDIFStart : public DimRpc
{
 public:
  RpcDIFStart(RpcDIFServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcDIFServer* _server;
};
class RpcDIFStop : public DimRpc
{
 public:
  RpcDIFStop(RpcDIFServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcDIFServer* _server;
};

class RpcDIFDestroy : public DimRpc
{
 public:
  RpcDIFDestroy(RpcDIFServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcDIFServer* _server;
};



class RpcDIFServer: public DimServer,public DimClient
{
  
public:
  RpcDIFServer();
  void allocateCommands();
  void commandHandler();
  void prepareDevices();
  void startDIFThread(DimDIF* d);
  void registerDB(std::string state);
  void infoHandler();
  /*
  void setThreshold(uint32_t B0,uint32_t B1,uint32_t B2,SingleHardrocV2ConfigurationFrame& ConfigHR2);
  void setGain(uint32_t gain,SingleHardrocV2ConfigurationFrame& ConfigHR2);
  void setThreshold(uint32_t B0,uint32_t B1,uint32_t B2,DIFDbInfo& s);
  void setGain(uint32_t gain,DIFDbInfo& s);
  void setThreshold(uint32_t B0,uint32_t B1,uint32_t B2);
  void setGain(uint32_t gain);
  */
  // DimRpc interface
  std::map<uint32_t,FtdiDeviceInfo*>& getFtdiMap(){ return theFtdiDeviceInfoMap_;}
  std::map<uint32_t,DimDIF*>& getDIFMap(){ return theDIFMap_;}
      
  FtdiDeviceInfo* getFtdiDeviceInfo(uint32_t i) { if ( theFtdiDeviceInfoMap_.find(i)!=theFtdiDeviceInfoMap_.end()) return theFtdiDeviceInfoMap_[i]; else return NULL;}

  void joinThreads(){g_d.join_all();}
private:
  std::map<uint32_t,FtdiDeviceInfo*> theFtdiDeviceInfoMap_;	
  std::map<uint32_t,DimDIF*> theDIFMap_;

  RpcDIFScan *scanCommand_;
  RpcDIFInitialise *initialiseCommand_;
  RpcDIFConfigure *configureCommand_;
  RpcDIFRegisterDB *registerdbCommand_;
  RpcDIFStart *startCommand_;
  RpcDIFStop *stopCommand_;
  RpcDIFDestroy *destroyCommand_;
  boost::thread_group g_d;

  DimInfo* theDBDimInfo_[255];

  // Dim Part
  int32_t processStatus_;
  DimService* aliveService_; //State of the process 
  /*
  uint32_t theCalibrationGain_;
  uint32_t theCalibrationThresholds_[3];

  DimCommand *gainCommand_;
  DimCommand *thresholdCommand_;
  DimCommand *loopConfigureCommand_;
  */





};
#endif

