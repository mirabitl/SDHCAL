#ifndef _LCccServer_h

#define _LCccServer_h
#include <iostream>

#include <string.h>
#include <stdio.h>


#include "CCCManager.h"

using namespace std;
#include <sstream>
#include "LdaqLogger.hh"
#include "fsm.hh"
class LCccServer 
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
private:
  std::string _state;
  levbdim::fsm* _fsm;
  CCCManager* _manager;
};
#endif

