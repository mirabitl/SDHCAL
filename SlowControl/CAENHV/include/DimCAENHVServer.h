#ifndef _DimCAENHVServer_h

#define _DimCAENHVServer_h
#include <iostream>
#include <string>
#include <string.h>
#include <semaphore.h>
#include <stdio.h>
#include "/opt/dhcal/dim/dim/dis.hxx"
#include "HVCaenInterface.h"
#include <sstream>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "MyInterface.h"
#include "HVMONDescription.h"
#include "HVRACKMyProxy.h"
#include "SETUPMyProxy.h"
#include "DETECTORMyProxy.h"
#include "HVMONMyProxy.h"
#include "PTMONMyProxy.h"
#include "DS1820MONMyProxy.h"
using namespace std;

class DimCAENHVServer: public DimServer
{
public:
  DimCAENHVServer();

  void Initialise(std::string account,std::string setup);
  void ReadChannel(uint32_t chan);
  void setV0(uint32_t chan,float v);
  void setI0(uint32_t chan,float v);
  void setOn(uint32_t chan);
  void setOff(uint32_t chan);
  ~DimCAENHVServer();
  void monitorStart(uint32_t period);


  void readout(uint32_t period);
  void monitorStop();
  void regulationStart(uint32_t period);

  void storeCurrentChannel();
  void regulate(uint32_t period);
  void regulationStop();
  void commandHandler();

private:

  HVMONDescription currentChannel_;
  HVRACKMyProxy*   theHVRACKMyProxy_;
  SETUPMyProxy*   theSETUPMyProxy_;
  DETECTORMyProxy*   theDETECTORMyProxy_;
  HVMONMyProxy*   theHVMONMyProxy_;
  PTMONMyProxy*   thePTMONMyProxy_;
  DS1820MONMyProxy*   theDS1820MONMyProxy_;


  int32_t theSetupId_,theHvrackId_;
  MyInterface* my_;

 
  DimService* channelReadService_; 

  //boost::thread    m_Thread_s;
  boost::thread_group g_d;
  boost::thread_group g_r;

  int32_t theMonitorPeriod_;
  DimCommand *startMonitorCommand_;
  DimCommand *stopMonitorCommand_;
  int32_t theRegulationPeriod_;
  DimCommand *startRegulationCommand_;
  DimCommand *stopRegulationCommand_;
  DimCommand *setOffCommand_;
  DimCommand *setOnCommand_;
  DimCommand *setI0Command_;
  DimCommand *setV0Command_;
  DimCommand *readChannelCommand_;
  DimCommand *initialiseCommand_;
  std::string theAccount_;
  std::string theSetup_;

  bool monitorRunning_;
  bool regulationRunning_;
  HVCaenInterface* theHV_;

 sem_t theMutex_;
};
#endif

