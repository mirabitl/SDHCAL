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
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

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



class DimDIFServer: public DimServer
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
  //  void startServices();
  void startReadout();
  void readout(uint32_t difid);

  void clearServices();

  void allocateServices(int32_t id);
  enum State {ALIVED=0,SCANNED=1,INITIALISED=2,PRECONFIGURED=3,CONFIGURED=4,RUNNING=5,STOPPED=6,DESTROYED=7};
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
	int32_t devicesStatus_[255];
  DIFStatus difStatus_[255];
  uint32_t difData_[255*32*1024];

  DimService* aliveService_; //State of the process 
  DimService* devicesService_; // List of FTDI devices

	DimService* infoServicesMap_[255]; //FtdiDeviceInfo services map
	DimService* dataServicesMap_[255]; // data services map

  DimCommand *scanCommand_;
  DimCommand *initialiseCommand_;
  uint32_t theSlowBuffer_[8192];
  DimCommand *preconfigureCommand_;
  DimCommand *configurechipsCommand_;
  
  
  DimCommand *startCommand_;
  DimCommand *stopCommand_;
  DimCommand *destroyCommand_;
  
};
#endif

