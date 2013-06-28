#ifndef _CCCClient_h
#define _CCCClient_h
#include <iostream>
#include <string.h>
#include<stdio.h>
#include "NMClient.h"
using namespace std;
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include <netlink/socket.h>
#include <netlink/socket_group.h>
#include <string>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <sstream>

class CCCClient: public NMClient
{
public:
  CCCClient(std::string host,uint32_t port);

  NetMessage* serviceHandler(NetMessage* m);

  void sendSecureCommand(std::string s,NetMessage* m);
 
  NetMessage* answerHandler(NetMessage* m);
  void doRegistration();
  void doInitialise(std::string dcc);
  void doConfigure();
  void doStart();
  void doStop();
  void doPause();
  void doResume();
  void doDIFReset();
  void doCCCReset();
  void doTestRegisterWrite(uint32_t r);
  uint32_t doTestRegisterRead();

  
private:
  uint32_t nessai_;
  double nbytes_;
  std::string prefix_;
  boost::interprocess::interprocess_mutex bsem_;
  uint32_t aSize_;
  unsigned char aBuf_[0x10000];
};
#endif
