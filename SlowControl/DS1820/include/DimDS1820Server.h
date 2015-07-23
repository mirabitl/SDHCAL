#ifndef _DimDS1820Server_h

#define _DimDS1820Server_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "dis.hxx"
#include "DS1820.h"
#include <sstream>

using namespace std;

class DimDS1820Server: public DimServer
{
public:
  DimDS1820Server();
  ~DimDS1820Server();
	void getTemperature();
  
private:
  int32_t processStatus_;
  float readValues_[16];
  DimService* readService_; //State of the last register read 
 
  DS1820* theDS1820_;
};
#endif

