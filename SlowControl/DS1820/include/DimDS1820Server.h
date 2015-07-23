#ifndef _DimDS1820Server_h

#define _DimDS1820Server_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "dis.hxx"
#include "DS1820.h"
#include <sstream>
#include <iostream>
#include <string>
#include "/opt/dhcal/dim/dim/dis.hxx"
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "MyInterface.h"

using namespace std;

class DimDS1820Server: public DimServer
{
public:
  DimDS1820Server();
  ~DimDS1820Server();
	void getTemperature();
  void Loop();
  void readout();
  void store();

  void commandHandler();

private:
  int32_t processStatus_;
  float readValues_[16];
  DimService* readService_; //State of the last register read 
  boost::thread_group g_d;

  int32_t thePeriod_;
  DimCommand *periodCommand_;
  std::string theAccount_;
  bool storeDb_;
  MyInterface* my_;
  DimCommand *storeCommand_;

  DS1820* theDS1820_;
};
#endif

