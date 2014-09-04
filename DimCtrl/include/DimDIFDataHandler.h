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



class DimDIFDataHandler: public DimClient
{
public:
  DimDIFDataHandler(uint32_t id,std::string prefix);

  ~DimDIFDataHandler();
  void infoHandler();

private:
  uint32_t theId_;
  std::string thePrefix_;
  DimInfo* theDIFInfo_;
  DimInfo* theDIFData_;
  uint32_t difData_[32*1024];
  DIFStatus difStatus_;
  std::string theShmPrefix_;

};
#endif

