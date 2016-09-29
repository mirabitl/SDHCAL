#ifndef _LGpioServer_h

#define _LGpioServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "fsmweb.hh"
#include "GPIO.h"
using namespace std;
#include <sstream>
#include "LdaqLogger.hh"

class LGpioServer
{
public:
  LGpioServer(std::string name);
  inline void publishStatus(){_zsStatus->updateService(_status,3*sizeof(float));}

  void configure(levbdim::fsmmessage* m);
  void on(levbdim::fsmmessage* m);
  void off(levbdim::fsmmessage* m);
  void read(levbdim::fsmmessage* m);
  // getters
  void setGPIO(GPIO*z ){_gpio =z;}
  GPIO* getGPIO(){return _gpio;}
  // action
  void Open(std::string,uint32_t m);
  void Read();
  void Switch(uint32_t m);
  float* readstatus(){return _status;}

  void c_joblog(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_vmeoff(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_vmeon(Mongoose::Request &request, Mongoose::JsonResponse &response);
private:
  float _status[3];
  DimService* _zsStatus; //State of the last register read
  fsmweb* _fsm;
 
  GPIO* _gpio;
};
#endif

