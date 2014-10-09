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



class DimDaqControl : public DimClient
{
public:
  DimDaqControl(std::string dns);

  ~DimDaqControl();
  virtual void infoHandler();
  
  void scandns();
  void scan();
  void print();
  
  void initialise();
  void initialiseWriter(std::string dir);
  void registerstate(uint32_t ctr,std::string sta);
  void configure();
  void start();
  void on();
  void off();
  void stop();
  void download(std::string s);
  void destroy();
  int getCurrentRun();
  char* getCurrentState();

  void setGain(uint32_t g);
  void setThresholds(uint32_t b0,uint32_t b1,uint32_t b2);

  void doScan(DimDDSClient* c);
  void doInitialise(DimDDSClient* c);
  void doRegisterstate(DimDDSClient* c);
  void doConfigure(DimDDSClient* c);
  void doStart(DimDDSClient* c);
  void doStop(DimDDSClient* c);
  void doDestroy(DimDDSClient* c);
  void doSetGain(DimDDSClient* c);
  void doSetThresholds(DimDDSClient* c);
  enum {ALIVED=1,BROWSED=2,SCANNED=3,INITIALISED=4,DBREGISTERED=5,CONFIGURED=6,STARTED=7,STOPPED=8,DESTROYED=9};
private:
  std::string theDNS_;
  std::string theDBPrefix_;
  std::string theCCCPrefix_;
  std::string theWriterPrefix_;
  std::string theZupPrefix_;
  
  std::map<std::string,DimDDSClient*> theDDSCMap_;
  uint32_t theCtrlReg_;
  std::string theState_;
  boost::interprocess::interprocess_mutex bsem_;

  int theCurrentRun_;
  char theCurrentState_[255];
  DimInfo* runInfo_;
  DimInfo* dbstateInfo_;

  int32_t theCalibrationGain_;
  int32_t theCalibrationThresholds_[3];
};
#endif

