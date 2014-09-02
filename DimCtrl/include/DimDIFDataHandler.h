#ifndef _DIMDIFDataHandler_h

#define _DIMDIFDataHandler_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "dis.hxx"
#include "dic.hxx"
#include "DIFReadout.h"
#include "DIFReadoutConstant.h"
using namespace std;
#include <sstream>
#include <map>
#include <vector>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>


typedef struct
{
  uint32_t id;
  uint32_t status;
  uint32_t slc;
  uint32_t gtc;
  uint64_t bcid;
  uint64_t bytes;
} DIFStatus;



class DimDIFDataHandler: public DimClient
{
public:
  DimDIFDataHandler(uint32_t id,std::string prefix);

  ~DimDIFDataHandler();
  void infoHandler();

private:
  uint32_t theId_;
  std::string thePrefix_
  DimInfo* theDIFInfo_;
  DimInfo* theDIFData_;
  uint32_t difData_[32*1024];
  DIFStatus difStatus_;
  std::string theShmPrefix_;

};
#endif

