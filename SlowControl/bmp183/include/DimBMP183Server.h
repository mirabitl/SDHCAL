#ifndef _DimBMP183Server_h

#define _DimBMP183Server_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "/opt/dhcal/dim/dim/dis.hxx"
#include "BMP183.h"
#include <sstream>

using namespace std;

class DimBMP183Server: public DimServer
{
public:
  DimBMP183Server();
  ~DimBMP183Server();
	void getTemperature();
	void getPression();
  
private:
  int32_t processStatus_;
  float TemperatureReadValues_;
  float PressionReadValues_;
  DimService* TemperatureReadService_; 
  DimService* PressionReadService_; 
 
  BMP183* theBMP183_;
};
#endif

