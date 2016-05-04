#ifndef _LDIFServer_h

#define _LDIFServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "fsm.hh"
#include "fsmweb.hh"
#include "LDIF.hh"
#include "DIFReadoutConstant.h"
using namespace std;
#include <sstream>
#include <map>
#include <vector>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "LdaqLogger.hh"

class LDIFServer  : public DimClient
{
  
public:
  LDIFServer(std::string name);
  void registerdb(levbdim::fsmmessage* m);
  void scan(levbdim::fsmmessage* m);
  void initialise(levbdim::fsmmessage* m);
  void configure(levbdim::fsmmessage* m);
  void start(levbdim::fsmmessage* m);
  void stop(levbdim::fsmmessage* m);
  void destroy(levbdim::fsmmessage* m);
  void status(levbdim::fsmmessage* m);

  void cmdStatus(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void jobLog(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void setThreshold(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void setGain(Mongoose::Request &request, Mongoose::JsonResponse &response);

  void prepareDevices();
  void startDIFThread(LDIF* d);
  void registerDB(std::string state);
  void infoHandler();
  // DimRpc interface
  std::map<uint32_t,FtdiDeviceInfo*>& getFtdiMap(){ return theFtdiDeviceInfoMap_;}
  std::map<uint32_t,LDIF*>& getDIFMap(){ return theDIFMap_;}
      
  FtdiDeviceInfo* getFtdiDeviceInfo(uint32_t i) { if ( theFtdiDeviceInfoMap_.find(i)!=theFtdiDeviceInfoMap_.end()) return theFtdiDeviceInfoMap_[i]; else return NULL;}

  void joinThreads(){g_d.join_all();}

private:
  std::map<uint32_t,FtdiDeviceInfo*> theFtdiDeviceInfoMap_;	
  std::map<uint32_t,LDIF*> theDIFMap_;

  //levbdim::fsm* _fsm;
  fsmweb* _fsm;
  boost::thread_group g_d;

  DimInfo* theDBDimInfo_[255];

  /*
  uint32_t theCalibrationGain_;
  uint32_t theCalibrationThresholds_[3];

  DimCommand *gainCommand_;
  DimCommand *thresholdCommand_;
  DimCommand *loopConfigureCommand_;
  */





};
#endif

