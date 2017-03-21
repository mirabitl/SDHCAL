#ifndef _LCccServer_h

#define _LCccServer_h
#include <iostream>

#include <string.h>
#include <stdio.h>


#include "CCCManager.h"

using namespace std;
#include <sstream>
#include "LdaqLogger.hh"
#include "baseApplication.hh"
class LCccServer : public levbdim::baseApplication
{
public:
  LCccServer(std::string name);
  void open(levbdim::fsmmessage* m);
  void initialise(levbdim::fsmmessage* m);
  void configure(levbdim::fsmmessage* m);
  void start(levbdim::fsmmessage* m);
  void stop(levbdim::fsmmessage* m);
  void cmd(levbdim::fsmmessage* m);
  void Open(std::string s);

  // getters

  CCCManager* getManager(){return _manager;}
  // Commands
  void pause(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void resume(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void difreset(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void cccreset(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void readreg(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void writereg(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void joblog(Mongoose::Request &request, Mongoose::JsonResponse &response);

private:
  std::string _state;
  //levbdim::fsm* _fsm;
  fsmweb* _fsm;
  CCCManager* _manager;
};
#endif

