#ifndef _DIMDIFDataHandler_h

#define _DIMDIFDataHandler_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "dis.hxx"
#include "dic.hxx"
#include "DIFReadoutConstant.h"
using namespace std;
#include <sstream>
#include <map>
#include <string>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "CtrlLogger.h"


class DimDIFDataHandler: public DimClient
{
public:
  DimDIFDataHandler(uint32_t id,std::string prefix);

  ~DimDIFDataHandler();
  void infoHandler();
  DIFStatus& getStatus(){return difStatus_;}
  char* getState();
private:
  uint32_t theId_;
  std::string thePrefix_;
  DimInfo* theDIFInfo_;
  DimInfo* theDIFData_;
  DimInfo* theDIFState_;
  uint32_t difData_[32*1024];
  DIFStatus difStatus_;
  char difState_[128];
  std::string theShmPrefix_;

};
#endif

