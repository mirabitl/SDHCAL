#ifndef _DimDaqControl_h

#define _DimDaqControl_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "dis.hxx"
#include "dic.hxx"
#include "DimDDSClient.h"
#include "DimDaqControlServer.h"
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



class DimDaqControl
{
public:
  DimDaqControl(std::string dns);

  ~DimDaqControl();
  void scandns();
  void scan();
  void initialise();
  void registerstate(uint32_t ctr,std::string sta);
  void configure();
  void start();
  void stop();
  void destroy();
  void doScan(DimDDSClient* c);
  void doInitialise(DimDDSClient* c);
  void doRegisterstate(DimDDSClient* c);
  void doConfigure(DimDDSClient* c);
  void doStart(DimDDSClient* c);
  void doStop(DimDDSClient* c);
  void doDestroy(DimDDSClient* c);
  enum {ALIVED=1,BROWSED=2,SCANNED=3,INITIALISED=4,DBREGISTERED=5,CONFIGURED=6,STARTED=7,STOPPED=8,DESTROYED=9};
private:
  std::string theDNS_;
  std::string theDBPrefix_;
  std::string theCCCPrefix_;
  std::map<std::string,DimDDSClient*> theDDSCMap_;
  uint32_t theCtrlReg_;
  std::string theState_;
  boost::interprocess::interprocess_mutex bsem_;
  
};
#endif

