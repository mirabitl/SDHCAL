#ifndef _DimGPIOServer_h

#define _DimGPIOServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "dis.hxx"
#include "GPIO.h"
#include <sstream>

using namespace std;

class DimGPIOServer: public DimServer
{
public:
  DimGPIOServer();
  void allocateCommands();
  void commandHandler();
  enum State {ALIVED=1,OPENED=2,DIF_ON=3,VME_ON=4,BOTH_ON=5, BOTH_OFF=6,CLOSED=7};
  
private:
  int32_t processStatus_;
  float readValues_[3];
  DimService* aliveService_; //State of the process 
  DimService* readService_; //State of the last register read 
  DimCommand *DIFonCommand_;
  DimCommand *VMEonCommand_;
  DimCommand *openCommand_;
  DimCommand *DIFoffCommand_;
  DimCommand *VMEoffCommand_;
  DimCommand *readCommand_;
  DimCommand *closeCommand_;
 
  GPIO* theGPIO_;
};
#endif

