#ifndef _FullSlow_h
#define _FullSlow_h
#include "baseApplication.hh"
#include <string>
#include <vector>
#include <json/json.h>

#include <iostream>

#include <string.h>
#include<stdio.h>
using namespace std;
#include <sstream>
#include <map>
#include <vector>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include "LdaqLogger.hh"
#include "fsmwebCaller.hh"
#include "json/json.h"

#include <string>


class wienerChannel
{
public:
  uint32_t channel;
  float vset;
  float iset;
  float vout;
  float iout;
};


class FullSlow : levbdim::baseApplication
{
public:
  FullSlow(std::string name);
  ~FullSlow();
  std::string state(){return _fsm->state();}
  void forceState(std::string s){_fsm->setState(s);}
  
  void discover(levbdim::fsmmessage* m);
  void configure(levbdim::fsmmessage* m);
  void destroy(levbdim::fsmmessage* m);
 // Virtual from baseAPplication
  virtual void  userCreate(levbdim::fsmmessage* m);

  void LVStatus(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void LVON(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void LVOFF(Mongoose::Request &request, Mongoose::JsonResponse &response);
  
  void setVoltage(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void setCurrentLimit(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void HVStatus(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void HVON(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void HVOFF(Mongoose::Request &request, Mongoose::JsonResponse &response);


private:
  fsmweb* _fsm;
  fsmwebCaller* _caenClient,*_zupClient,*_genesysClient,*_bmpClient,*_gpioClient;
  Json::Value _jConfigContent;

 
};
#endif
