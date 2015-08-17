#ifndef _DimJobControl_h

#define _DimJobControl_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "dis.hxx"
#include "dic.hxx"

#include "JobControl.h"
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
enum class DimProcessStatus {notcreated,running,killed};

class DimProcessData
{

 public:
  DimProcessData(std::string json_string);
  

  Json::Value _processInfo;
  pid_t _childPid;
  DimProcessStatus _status;

};

class DimJobControl: public DimServer
{
public:
  DimJobControl();

  ~DimJobControl();
  virtual void commandHandler();

  void allocateCommands();
  
  void startProcess(DimProcessData* p);
  void killProcess(DimProcessData* p,uint32_t sig);

  std::string status(pid_t pid);
  std::string log(pid_t pid);
  void clear();

private:

  std::map<pid_t,DimProcessData*> _processMap;
  std::map<pid_t,DimService*> _statusMap;
  std::map<pid_t,DimService*> _logMap;
  DimCommand* _startCommand;
  DimCommand* _killCommand;
  DimCommand* _statusCommand;
  DimCommand* _logCommand;
  
};
#endif

