#ifndef _DimBMP183Server_h

#define _DimBMP183Server_h
#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include "/opt/dhcal/dim/dim/dis.hxx"
#include "BMP183.h"
#include <sstream>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "MyInterface.h"
#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;
using namespace std;
static LoggerPtr _logger(Logger::getLogger("BMP183"));

class DimBMP183Server: public DimServer
{
public:
  DimBMP183Server();
  ~DimBMP183Server();
  void getTemperature();
  void getPression();
  void Loop();
  void readout();
  void store();

  void commandHandler();
  void setLogger(LoggerPtr s){_logger=s;}
private:
  int32_t processStatus_;
  float TemperatureReadValues_;
  float PressionReadValues_;
  DimService* TemperatureReadService_; 
  DimService* PressionReadService_; 
  //boost::thread    m_Thread_s;
  boost::thread_group g_d;

  int32_t thePeriod_;
  DimCommand *periodCommand_;
  std::string theAccount_;
  bool storeDb_;
  MyInterface* my_;
  DimCommand *storeCommand_;
  BMP183* theBMP183_;
};
#endif

