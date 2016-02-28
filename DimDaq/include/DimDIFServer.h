#ifndef _DimDIFServer_h

#define _DimDIFServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "dis.hxx"
#include "dic.hxx"
#include "DIFReadout.h"
#include "DIFReadoutConstant.h"
using namespace std;
#include <sstream>
#include <map>
#include <vector>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "DaqLogger.h"
typedef struct 
{
  uint32_t vendorid;
  uint32_t productid;
  char name[12];
  uint32_t id;
  uint32_t type;
} FtdiDeviceInfo;



class DimDIFServer: public DimServer,public DimClient
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
  void registerDBService(const char* state);

  void infoHandler();

  void setThreshold(uint32_t B0,uint32_t B1,uint32_t B2,SingleHardrocV2ConfigurationFrame& ConfigHR2);
  void setGain(uint32_t gain,SingleHardrocV2ConfigurationFrame& ConfigHR2);
  void setThreshold(uint32_t B0,uint32_t B1,uint32_t B2,DIFDbInfo& s);
  void setGain(uint32_t gain,DIFDbInfo& s);
  void setThreshold(uint32_t B0,uint32_t B1,uint32_t B2);
  void setGain(uint32_t gain);


  enum State {ALIVED=1,SCANNED=2,INITIALISED=3,PRECONFIGURED=4,CONFIGURED=5,RUNNING=6,STOPPED=7,DESTROYED=8,FAILED=99};
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
  std::string difState_[255];
  DimService* aliveService_; //State of the process 
  DimService* devicesService_; // List of FTDI devices
  
  DimService* infoServicesMap_[255]; //FtdiDeviceInfo services map
  DimService* dataServicesMap_[255]; // data services map
  DimService* stateServicesMap_[255]; // data services map

  DimCommand *scanCommand_;
  DimCommand *initialiseCommand_;
  uint32_t theSlowBuffer_[8192];
  DimCommand *preconfigureCommand_;
  DimCommand *registerstateCommand_;
  DimCommand *configurechipsCommand_;
  
   
  DimCommand *startCommand_;
  DimCommand *stopCommand_;
  DimCommand *destroyCommand_;


  uint32_t theCalibrationGain_;
  uint32_t theCalibrationThresholds_[3];
  DimCommand *gainCommand_;
  DimCommand *thresholdCommand_;
  DimCommand *loopConfigureCommand_;




  DimInfo* theDBDimInfo_[255];
  DIFDbInfo theDIFDbInfo_[255];
};
#endif

