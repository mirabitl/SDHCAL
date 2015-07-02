#ifndef HVCAENINTERFACE_H
#define HVCAENINTERFACE_H
#include "HVInterface.h"
#include <string>
#include "CAENHVWrapper.h"

class HVCaenInterface: public HVInterface 
{
 public:
  HVCaenInterface(std::string host,std::string user,std::string pwd);
  virtual ~HVCaenInterface();
  virtual void Connect();
  virtual void Disconnect();
  virtual void SetOn(uint32_t channel);
  virtual void SetOff(uint32_t channel);
  virtual void SetCurrent(uint32_t channel,float imax);
  virtual void SetVoltage(uint32_t channel,float v0);
  virtual float GetCurrentSet(uint32_t channel);
  virtual float GetVoltageSet(uint32_t channel);
  virtual float GetCurrentRead(uint32_t channel);
  virtual float GetVoltageRead(uint32_t channel);
  virtual uint32_t GetStatus(uint32_t channel);
 private:
  std::string theHost_,theUser_,thePassword_;
  int32_t theID_;
  int32_t theHandle_;
};
#endif
