#ifndef _LRootPublisher_h

#define _LRootPublisher_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "baseApplication.hh"

#include "datasource.hh"
#include "shmdriver.hh"

using namespace std;
#include <sstream>
#include "LdaqLogger.hh"
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <boost/interprocess/sync/interprocess_mutex.hpp>

class LRootPublisher : public levbdim::baseApplication
{
public:
  LRootPublisher(std::string name);
  
  void configure(levbdim::fsmmessage* m);
  void start(levbdim::fsmmessage* m);
  void stop(levbdim::fsmmessage* m);
  void destroy(levbdim::fsmmessage* m);
  void scanMemory();
  void cmdStatus(Mongoose::Request &request, Mongoose::JsonResponse &response);
private:
  //levbdim::fsm* _fsm;
  fsmweb* _fsm;
  boost::thread_group _gThread;

  std::vector<levbdim::datasource*> _sources;
  std::string _directory;
  uint32_t _run,_lastEvent,_detid;
  bool _running;
};
#endif

