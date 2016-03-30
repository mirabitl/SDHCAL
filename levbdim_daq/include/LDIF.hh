#ifndef _LDIF_h

#define _LDIF_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "datasource.hh"
#include "DIFReadout.h"
#include "DIFReadoutConstant.h"
using namespace std;
#include <sstream>
#include <map>
#include <vector>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "LdaqLogger.hh"
typedef struct 
{
  uint32_t vendorid;
  uint32_t productid;
  char name[12];
  uint32_t id;
  uint32_t type;
} FtdiDeviceInfo;

class LDIF
{
public:
  LDIF(FtdiDeviceInfo *ftd);
  ~LDIF();
  // registration to Dim Info and creation of DimServices 
  void registration();
  // initialise
  void initialise();
  // configure
  void difConfigure(uint32_t ctrl);
  void chipConfigure();
  void configure(uint32_t ctrl);
  // Start Stop
  void start();
  void readout();
  void stop();
  // destroy
  void destroy();
  // register access
  void writeRegister(uint32_t adr,uint32_t reg);
  void readRegister(uint32_t adr,uint32_t &reg);
  // Getter and setters
  inline DIFStatus* status() const {return _status;}
  inline DIFReadout* rd() const {return _rd;}
  inline DIFDbInfo* dbdif() const {return _dbdif;}
  void setState(std::string s){_state.assign(s);}
  inline std::string state() const {return _state;}
  inline uint32_t* data()  {return (uint32_t*) _dsData->buffer()->ptr();}
  // Publish DIM services
  inline void publishState(std::string s){setState(s);_dsState->updateService((char*) _state.c_str());}
  inline void publishStatus(){_dsStatus->updateService(_status,sizeof(DIFStatus));}
  // run control
  inline void setReadoutStarted(bool t){_readoutStarted=t;}
  inline bool readoutStarted() const { return _readoutStarted;}
  inline bool running() const { return _running;}
  inline uint32_t detectorId() const {return _detid;}
private:
  FtdiDeviceInfo _ftd;
  DIFStatus* _status;
  DIFReadout* _rd;
  std::string _state;
  DIFDbInfo* _dbdif;
  uint32_t _data[32768];

  DimService* _dsStatus;
  DimService* _dsState;
  levbdim::datasource* _dsData;
  uint32_t _detid;

  bool _running,_readoutStarted,_readoutCompleted;
};
#endif
