#ifndef _DimZupServer_h

#define _DimZupServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "dis.hxx"
#include "Zup.h"
using namespace std;
#include <sstream>
class DimZupServer: public DimServer
{
public:
  DimZupServer();
  void allocateCommands();
  void commandHandler();
  enum State {ALIVED=1,OPENED=2,ON=3,OFF=4,CLOSED=5};
 
  
private:
  int32_t processStatus_;
  float readValues_[3];
  DimService* aliveService_; //State of the process 
  DimService* readService_; //State of the last register read 
  DimCommand *onCommand_;
  DimCommand *openCommand_;
  DimCommand *offCommand_;
  DimCommand *readCommand_;
  DimCommand *closeCommand_;
 
  Zup* theZup_;
};
#endif

