#ifndef _LBmp183Server_h

#define _LBmp183Server_h
#include <iostream>

#include <string.h>
#include <stdio.h>
#include "fsmweb.hh"
#include "BMP183.h"
using namespace std;
#include <sstream>
#include "LdaqLogger.hh"

class LBmp183Server
{
public:
  LBmp183Server(std::string name);
  void configure(levbdim::fsmmessage* m);
 
  void destroy(levbdim::fsmmessage* m);
  // action
 
  void c_joblog(Mongoose::Request &request, Mongoose::JsonResponse &response);
  
  void c_getStatus(Mongoose::Request &request, Mongoose::JsonResponse &response);
  
private:
  //float _status[3];
  //DimService* _zsStatus; //State of the last register read
 
  fsmweb* _fsm;
 
  BMP183* _bmp;
};
#endif

