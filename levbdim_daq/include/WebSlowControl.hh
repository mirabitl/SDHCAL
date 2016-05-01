#ifndef _WebSlowControl_h
#define _WebSlowControl_h
#include "fsmweb.hh"
#include <string>
#include <vector>
#include <json/json.h>

#include <iostream>

#include <string.h>
#include<stdio.h>
#include "dis.hxx"
#include "dic.hxx"
#include "DIFReadoutConstant.h"
using namespace std;
#include <sstream>
#include <map>
#include <vector>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "MyInterface.h"
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include "LdaqLogger.hh"

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

class chamberRef
{
public:
  uint32_t channel;
  float vref;
  float iref;
  float p0;
  float t0;
};

class WebSlowControl : public DimClient
{
public:
  WebSlowControl(std::string name,uint32_t port);
  ~WebSlowControl();
  std::string state(){return _fsm->state();}
  void forceState(std::string s){_fsm->setState(s);}
  
  void discover(levbdim::fsmmessage* m);

  void infoHandler();
  void setReadoutPeriod(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void readChannel(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void setVoltage(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void setCurrentLimit(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void HVON(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void HVOFF(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void loadReferences(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void ptRead(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void destroy(levbdim::fsmmessage* m);
  void initialise(levbdim::fsmmessage* m);
  bool  getPTMean();
  float pression();
  float temperature();
  float vset(uint32_t i);
  float iset(uint32_t i);
  float vout(uint32_t i);
  float iout(uint32_t i);
  std::string hvinfoChannel(uint32_t ch);
  std::string hvinfoCrate();
  void startMonitor(levbdim::fsmmessage* m);

  void doStore();
  void stopMonitor(levbdim::fsmmessage* m);
  void startCheck(levbdim::fsmmessage* m);

  void doCheck();
  void stopCheck(levbdim::fsmmessage* m);
  void dimsetVoltage(int ch,float v);
  void dimreadChannel(int ch);
private:
  fsmweb* _fsm;
    std::string _BMPPrefix;
  //Hardware
  wienerChannel _hvchannels[56];
  DimInfo* _wienerInfo[56];
  float _PRead,_TRead;
  DimInfo* _pressionInfo;
  DimInfo* _temperatureInfo;
  // DB
  std::string _mysqlAccount;
  MyInterface* _my;
  // Store
  boost::interprocess::interprocess_mutex _bsem;
  boost::thread_group g_store;
  bool _storeRunning;
  uint32_t _storeTempo;
  std::vector<chamberRef> _chambers;
  // Check
  boost::thread_group g_check;
  bool _checkRunning;
  uint32_t _checkTempo;
  float _PMean,_TMean;

};
#endif
