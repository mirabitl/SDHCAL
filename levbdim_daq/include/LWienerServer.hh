#ifndef _LWienerServer_h

#define _LWienerServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "baseApplication.hh"
#include "wienersnmp.hh"
using namespace std;
#include <sstream>
#include "LdaqLogger.hh"
struct
{
  uint32_t channel;
  float vset;
  float iset;
  float vout;
  float iout;
  char  status[64];
} hvchannel;
class LWienerServer : public levbdim::baseApplication
{
public:
  LWienerServer(std::string name);
  inline void publishStatus(){_zsStatus->updateService(_status,3*sizeof(float));}

  void configure(levbdim::fsmmessage* m);
  void startMonitoring(levbdim::fsmmessage* m);
  void stopMonitoring(levbdim::fsmmessage* m);
  void startAutoControl(levbdim::fsmmessage* m);
  void stopAutoControl(levbdim::fsmmessage* m);
  void destroy(levbdim::fsmmessage* m);
  // action
  void Open(std::string ip);
  void c_joblog(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setOutputVoltage(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setCurrentLimit(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setOutputVoltageRiseRate(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setOutputSwitch(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_getStatus(Mongoose::Request &request, Mongoose::JsonResponse &response);
private:
  //float _status[3];
  //DimService* _zsStatus; //State of the last register read
  struct hvchannel _chan;
  std::vector<DimService*> _vecHvStatus;
  fsmweb* _fsm;
 
  WienerSnmp* _hv;
};
#endif

