#ifndef _DimCCCServer_h

#define _DimCCCServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "dis.hxx"
#include "CCCManager.h"
using namespace std;
#include <sstream>
#include "DaqLogger.h"
class DimCCCServer: public DimServer
{
public:
  DimCCCServer();
  void allocateCommands();
  void commandHandler();
  enum State {ALIVED=1,INITIALISED=2,CONFIGURED=3,RUNNING=4,STOPPED=5,PAUSED=6,RESUMED=7,FAILED=99};
 
  void doInitialise(std::string device);
  
private:
  int32_t processStatus_;
  int32_t register_;
  DimService* aliveService_; //State of the process 
  DimService* registerService_; //State of the last register read 
  DimCommand *initialiseCommand_;
  DimCommand *configureCommand_;
  DimCommand *startCommand_;
  DimCommand *stopCommand_;
  DimCommand *pauseCommand_;
  DimCommand *resumeCommand_;
  DimCommand *difresetCommand_;
  DimCommand *cccresetCommand_;
  DimCommand *testregisterreadCommand_;
  DimCommand *testregisterwriteCommand_;
  DimCommand *registerreadCommand_;
  DimCommand *registerwriteCommand_;
 
  CCCManager* theManager_;
};
#endif

