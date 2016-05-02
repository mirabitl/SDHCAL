#ifndef _WebJobControl_h
#define _WebJobControl_h
#include "fsmweb.hh"
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
#include "DimJobInterface.h"
#include "LdaqLogger.hh"

#include "json/json.h"

#include <string>



class WebJobControl 
{
public:
  WebJobControl(std::string name,uint32_t port);
  ~WebJobControl();
  std::string state(){return _fsm->state();}
  void forceState(std::string s){_fsm->setState(s);}
  
  void status(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void killjob(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void restartjob(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void destroy(levbdim::fsmmessage* m);
  void initialise(levbdim::fsmmessage* m);
  void start(levbdim::fsmmessage* m);
  void kill(levbdim::fsmmessage* m);
private:
  fsmweb* _fsm;
  DimJobInterface* _djc;

};
#endif
