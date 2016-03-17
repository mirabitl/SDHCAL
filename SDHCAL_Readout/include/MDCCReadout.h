#ifndef _MDCCReadout_h_
#define _MDCCReadout_h_

#include "LocalHardwareException.h"

#define USE_FTDI
#ifdef USE_FTDI

#include "FtdiUsbDriver.h"

#endif
#include "ReadoutLogger.h"
class MDCCReadout 
{
public:
  MDCCReadout (std::string name,uint32_t productid=0x6001);
  ~MDCCReadout();
  void open();
  void close();
  void writeRegister(uint32_t addr,uint32_t data);
  uint32_t readRegister(uint32_t addr);
  void maskTrigger();
  void unmaskTrigger();
  void resetCounter();
  uint32_t mask();
  uint32_t spillCount();
  uint32_t busyCount();
  uint32_t version();
  uint32_t id();

private : 
  FtdiUsbDriver* _driver;
  std::string _name;
  uint32_t _productid,_version,_id;

};
#endif
