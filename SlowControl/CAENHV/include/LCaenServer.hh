#ifndef _LCaenServer_h

#define _LCaenServer_h
#include <iostream>

#include <string.h>
#include <stdio.h>
#include "baseApplication.hh"
#include "HVCaenInterface.h"
using namespace std;
#include <sstream>
#include "LdaqLogger.hh"

class LCaenServer : public levbdim::baseApplication
{
public:
  LCaenServer(std::string name);
  void configure(levbdim::fsmmessage* m);
 
  void destroy(levbdim::fsmmessage* m);
  // action
  void Open(std::string ip);
  void c_setOutputVoltage(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setCurrentLimit(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setOutputVoltageRiseRate(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setOutputSwitch(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_getStatus(Mongoose::Request &request, Mongoose::JsonResponse &response);
  Json::Value Read(uint32_t i);
private:
  //float _status[3];
  //DimService* _zsStatus; //State of the last register read
 
  fsmweb* _fsm;
 
  HVCaenInterface* _hv;
};
#endif

