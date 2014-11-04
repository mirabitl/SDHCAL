#ifndef _DimDDSClient_h

#define _DimDDSClient_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "dis.hxx"
#include "dic.hxx"
#include "DimDIFDataHandler.h"
#include "DIFReadoutConstant.h"
using namespace std;
#include <sstream>
#include <map>
#include <vector>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <boost/interprocess/sync/interprocess_mutex.hpp>




class DimDDSClient: public DimClient
{
public:
  DimDDSClient(std::string name,std::string prefix);

  ~DimDDSClient();
  void infoHandler();

  void scanDevices();
  bool checkState(std::string str);
  void waitState(std::string str,uint32_t max_wait=5);
  void initialise();
  void setDBState(uint32_t ctrlreg,std::string state);
  void configure();
  void start();
  void stop();
  void destroy();
  void print();
  std::map<uint32_t,DimDIFDataHandler*>& getDIFMap(){return theDDDHMap_;}
  void setGain(uint32_t g);
  void setThresholds(uint32_t b0,uint32_t b1,uint32_t b2);

private:
  std::string theName_;
  std::string thePrefix_;
  DimInfo* theDDSStatus_;
  uint32_t theStatus_;
  DimInfo* theDDSDevices_;
  uint32_t theDevices_[255];
  std::map<uint32_t,DimDIFDataHandler*> theDDDHMap_;
  uint32_t theCtrlReg_;
  std::string theState_;
  boost::interprocess::interprocess_mutex bsem_;

  int32_t theCalibrationGain_;
  int32_t theCalibrationThresholds_[3];
};
#endif

