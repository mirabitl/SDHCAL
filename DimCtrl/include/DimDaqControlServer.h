#ifndef _DimDaqControlServer_h

#define _DimDaqControlServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "dis.hxx"
#include "dic.hxx"

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

class DimDaqControlServer: public DimServer
{
public:
  DimDaqControlServer(DimDaqControl* c);

  ~DimDaqControlServer();
  virtual void commandHandler();
  void allocateCommands();
  void services();

  enum {ALIVED=1,BROWSED=2,SCANNED=3,INITIALISED=4,DBREGISTERED=5,CONFIGURED=6,STARTED=7,STOPPED=8,DESTROYED=9};
private:
  std::string theDNS_;
  DimDaqControl* theControl_;
  DimService* aliveService_;
  int32_t processStatus_;
  DimCommand* browseCommand_;
  DimCommand* scanCommand_;
  DimCommand* initialiseCommand_;
  DimCommand* registerstateCommand_;
  DimCommand* configureCommand_;
  DimCommand* startCommand_;
  DimCommand* stopCommand_;
  DimCommand* destroyCommand_;
  DimCommand* printCommand_;
  boost::thread    m_Thread_s;
  uint32_t ctrlreg_;
  std::string state_;
  std::vector<std::string> todo_;
  
};
#endif

