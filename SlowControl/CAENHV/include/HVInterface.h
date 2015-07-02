#ifndef HVINTERFACE_H
#define HVINTERFACE_H
#include <stdint.h>
class HVInterface {
 public:
  virtual void Connect()=0;
  virtual void Disconnect()=0;
  virtual void SetCurrent(uint32_t channel,float imax)=0;
  virtual void SetVoltage(uint32_t channel,float v0)=0;
  virtual float GetCurrentSet(uint32_t channel)=0;
  virtual float GetVoltageSet(uint32_t channel)=0;
  virtual float GetCurrentRead(uint32_t channel)=0;
  virtual float GetVoltageRead(uint32_t channel)=0;
  virtual uint32_t GetStatus(uint32_t channel)=0;
};
#endif
