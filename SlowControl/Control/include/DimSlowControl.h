#ifndef _DimSlowControlControl_h

#define _DimSlowControlControl_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "dis.hxx"
#include "dic.hxx"
using namespace std;
#include <sstream>
#include <map>
#include <vector>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <boost/interprocess/sync/interprocess_mutex.hpp>

#include <string>



#include "HVMONDescription.h"
class DimSlowControl  : public DimClient
{
public:
  DimSlowControl(std::string dns);
  ~DimSlowControl();
  virtual void infoHandler();
  void scandns();
  int getCurrentRun();
  char* getCurrentState();
  uint32_t getCurrentHVChannel();
  float getCurrentHVVoltageSet();
  float getCurrentHVVoltageRead();
  uint32_t getCurrentHVCurrentRead();
  uint32_t getCurrentHVStatus();
  float getDS1820Temperature(uint32_t i);
  float getBMP183Temperature();
  float getBMP183Pressure();

  void CAENHVInitialise(std::string s);
  void CAENHVSetVoltage(uint32_t chan,float v);
  void CAENHVSetCurrent(uint32_t chan,float ic);
  void CAENHVSetOn(uint32_t chan);
  void CAENHVSetOff(uint32_t chan);
  void CAENHVRead(uint32_t chan);
  void OpenGPIO();
  void CloseGPIO();
  void VMEOn();
  void VMEOff();
  void DIFOn();
  void DIFOff();
  void BMP183SetPeriod(uint32_t p);
  void BMP183Store(std::string s);
  void DS1820SetPeriod(uint32_t p);
  void DS1820Store(std::string s);

private:
  std::string theDNS_;
  DimInfo* runInfo_;
  DimInfo* dbstateInfo_;
  DimInfo* CAENHVInfo_;
  DimInfo* BMP183PressureInfo_;
  DimInfo* BMP183TemperatureInfo_;
  DimInfo* DS1820Info_;
  std::string   theCAENHVPrefix_;
  std::string   theBMP183Prefix_;
  std::string   theDS1820Prefix_;
  std::string   theGPIOPrefix_;
  HVMONDescription theCAENHVChannel_;
  float theBMP183Pressure_,theBMP183Temperature_;
  float theDS1820Temperatures_[2];
  int32_t theCurrentRun_;
  char theCurrentState_[256];
};
#endif
