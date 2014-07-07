#ifndef WEBDIFDBSERVER_H
#define WEBDIFDBSERVER_H
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "NMServer.h"
#include "OracleDIFDBManager.h"
using namespace std;
#include <sstream>
class WebDIFDBServer: public NMServer
{
public:
  WebDIFDBServer(std::string host,uint32_t port);
  NetMessage* commandHandler(NetMessage* m);
  void doDownload(std::string state);

 
 
private:
  bool running_;
  OracleDIFDBManager* theManager_;
};



#endif
