#ifndef _DimDIFServer_h

#define _DimDIFServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "dis.hxx"
#include "DIFReadout.h"
using namespace std;
#include <sstream>
#include <map>
#include <vector>

typedef struct 
{
  uint32_t vendorid;
  uint32_t productid;
  char name[12];
  uint32_t id;
  uint32_t type;
} FtdiDeviceInfo;

typedef struct
{
  uint32_t id;
  uint32_t slc;
  uint32_t gtc;
  uint64_t bcid;
  uint64_t bytes;
} DIFStatus;



class DimDIFServer: public DimDIFServer
{
public:
  DimDIFServer();
  void allocateCommands();
  void commandHandler();
  uint32_t configureChips(uint32_t difid,SingleHardrocV2ConfigurationFrame* slow,uint32_t nasic=48) throw (LocalHardwareException);
  uint32_t configureChips(std::string path,uint32_t difid) throw (LocalHardwareException);

  void preConfigure(uint32_t difid,uint32_t ctrlreg) throw (LocalHardwareException);
  void initialise(uint32_t difid) throw (LocalHardwareException);
  std::vector<uint32_t>& scanDevices();
  void prepareDevices();
  void UsbPrepare();
  void startServices();
  void startReadout();
  void readout(uint32_t difid);

  void joinServices();

  void services();
  enum State {ALIVED=0,SCANNED=1,INITIALISED=2,CONFIGURED=3,RUNNING=4,STOPPED=5};
private:
  bool running_,readoutStarted_;
  boost::thread    m_Thread_s;
  std::map<uint32_t,DIFReadout*> theDIFMap_;
  std::vector<uint32_t> theListOfDIFFounds_;
  boost::thread  m_Thread_d[255];
  boost::thread_group g_d;
  std::map<uint32_t,FtdiDeviceInfo> theFtdiDeviceInfoMap_;	
  
  // Dim Part
  int32_t processStatus_;
  DimService* aliveService_; //State of the process 
  DimCommand *scanCommand_;
  int32_t devicesStatus_[255];
  DimService* devicesService_; // List of difid (int)
  DimCommand *initialiseCommand_;

  DimCommand *configureCommand_;
  
  DIFStatus difStatus_[255];
  DimService* infoServicesMap_[255]; //FtdiDeviceInfo services map
  DimCommand *startCommand_;
  uint32_t difData_[255*32*1024];
  DimServices dataServicesMap_[255]; // data services map
  DimCommand *stopCommand_;
  DimCommand *destroyCommand_;
  
};
#endif

