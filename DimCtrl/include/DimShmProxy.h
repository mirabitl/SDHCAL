#ifndef _DimShmProxy_h

#define _DimShmProxy_h
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

class DimDaqControl;

class DimShmProxy: public DimServer,public DimClient
{
public:
  DimShmProxy();

  ~DimShmProxy();
  virtual void commandHandler();
  virtual void infoHandler();
  void allocateCommands();
  void clearInfo();
  void registerDifs();
  void svc();
  enum {ALIVED=1,INITIALISED=2,STARTED=3,STOPPED=4};
private:
  ShmProxy* theProxy_;
  boost::thread    theThread_;  
  DimService* aliveService_;
  DimService* runService_;
  DimService* eventService_;
  int32_t processStatus_,run_,event_;
  DimCommand* initialiseCommand_;
  DimCommand* setupCommand_;
  DimCommand* directoryCommand_;
  DimCommand* startCommand_;
  DimCommand* stopCommand_;
  DimCommand* destroyCommand_;
  int32_t theRun_;
  char theState_[255];
  DIFStatus theInfo_;
  uint32_t theBuffer_[32*1024];
  
  DimInfo* difInfo_[255];
  DimInfo* difState_[255];
  DimInfo* difData_[255];
  DimInfo* runInfo_;
  std::string  dbState_;
  DimInfo* dbstateInfo_;
};
#endif

