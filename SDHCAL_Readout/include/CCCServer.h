#ifndef _CCCServer_h

#define _CCCServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "NMServer.h"
#include "CCCManager.h"
using namespace std;
#include <sstream>
class CCCServer: public NMServer
{
public:
  CCCServer(std::string host,uint32_t port);
  NetMessage* commandHandler(NetMessage* m);
 
  void doInitialise(std::string device);
  void startServices();
 
  void joinServices();
  
  void services();
 
private:
  bool running_;
  boost::thread    m_Thread_s;
  uint32_t nessai_;
  CCCManager* theManager_;
};
#endif

