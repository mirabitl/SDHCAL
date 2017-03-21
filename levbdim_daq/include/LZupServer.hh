#ifndef _LZupServer_h

#define _LZupServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "fsmweb.hh"
#include "baseApplication.hh"
#include "Zup.h"
using namespace std;
#include <sstream>
#include "LdaqLogger.hh"

class LZupServer : public levbdim::baseApplication
{
public:
  LZupServer(std::string name);
  inline void publishStatus(){_zsStatus->updateService(_status,3*sizeof(float));}

  void configure(levbdim::fsmmessage* m);
  void on(levbdim::fsmmessage* m);
  void off(levbdim::fsmmessage* m);
  void read(levbdim::fsmmessage* m);
  // getters
  void setZup(Zup*z ){_zup =z;}
  Zup* getZup(){return _zup;}
  // action
  void Open(std::string,uint32_t m);
  void Read();
  void Switch(uint32_t m);
  float* readstatus(){return _status;}

  void c_joblog(Mongoose::Request &request, Mongoose::JsonResponse &response);
private:
  float _status[3];
  DimService* _zsStatus; //State of the last register read
  fsmweb* _fsm;
 
  Zup* _zup;
};
#endif

