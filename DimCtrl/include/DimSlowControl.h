#ifndef _DimSlowControl_h

#define _DimSlowControl_h
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


#include "json/json.h"

#include <string>
#include "CtrlLogger.h"

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

class DimSlowControl : public DimClient
{
public:
  DimSlowControl();
  ~DimSlowControl();
  void infoHandler(); //done
  void initialiseDB(std::string s); //done
  void loadReferences(); //done
  void readChannel(uint32_t ch); // done
  void setReadoutPeriod(uint32_t tr); //done
  void startStore(uint32_t tempo);
  void doStore();
  void stopStore();
  void startCheck(uint32_t tempo);
  void doCheck();
  void stopCheck();
  void setVoltage(uint32_t ch,float v); //done
  void setCurrentLimit(uint32_t ch,float imicro); //done
  void HVON(uint32_t ch); //done
  void HVOFF(uint32_t ch); //done
  bool getPTMean(); //done
  float vset(uint32_t chan); //done 
  float iset(uint32_t chan); //done
  float vout(uint32_t chan); //done
  float iout(uint32_t chan); //done
  std::string hvinfoChannel(uint32_t ichan);//done
  std::string hvinfoCrate();//done
private:
  //
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

