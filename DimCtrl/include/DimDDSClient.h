#ifndef _DimDDSClient_h

#define _DimDDSClient_h
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





class DimDDSClient: public DimClient
{
public:
  DimDDSClient(std::string name,std::string prefix);

  ~DimDDSClient();
  void infoHandler();

  void scanDevices();

  void initialise();
  void setDBState(std::string state);
  void configure();
  void start();
  void stop();
  void destroy();
private:
  std::string theName_;
  std::string thePrefix_
  DimInfo* theDDSStatus_;
  uint32_t theStatus_;
  DimInfo* theDDSDevices_;
  uint32_t theDevices_[255];
  std::map<uint32_t,DimDIFDataHandler*> theDDDHMap_;

};
#endif

