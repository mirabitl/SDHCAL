#ifndef _LDbServer_h

#define _LDbServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "fsm.hh"
#include "OracleDIFDBManager.h"
#include "DIFReadoutConstant.h"
#include "LdaqLogger.hh"
using namespace std;
#include <sstream>

class LDbServer
{
public:
  LDbServer(std::string name);
  void download(levbdim::fsmmessage* m);
  void destroy(levbdim::fsmmessage* m);
  void newrun(levbdim::fsmmessage* m);
  void doInitialise();
  void doDownload(std::string dbstate);
  void doDelete();
  uint32_t getRunFromDb();

private:
  levbdim::fsm* _fsm;

  int32_t runFromDb_;
  RunInfo* theRunInfo_;
  DimService* _runService; //State of the process

  // DB state
  std::string _dbState;
  DimService* _dbstateService;

  DIFDbInfo _difInfos[255];
  DimService* _difServices[255]; //State of the last register read


 
  OracleDIFDBManager* theDBManager_;
};
#endif

