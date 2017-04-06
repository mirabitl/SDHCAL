#ifndef _TDC_MANAGER_HH
#define _TDC_MANAGER_HH
#include "TdcMessageHandler.hh"
#include "baseApplication.hh"
#include "PRSlow.hh"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>
struct evs {
  uint64_t abcid;
  double ltdc,rtdc;
  uint32_t bcid,idx,event,time,mezzanine,strip;
    
};

class TdcManager : public levbdim::baseApplication
{
public:
  TdcManager(std::string name);
  ~TdcManager(){;}
  void initialise(levbdim::fsmmessage* m);
  void configure(levbdim::fsmmessage* m);
  void start(levbdim::fsmmessage* m);
  void stop(levbdim::fsmmessage* m);
  void destroy(levbdim::fsmmessage* m);
  void c_joblog(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_status(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_diflist(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_set6bdac(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setMask(Mongoose::Request &request, Mongoose::JsonResponse &response);
  
  void parseConfig(std::string name);
  void writeRamAvm();
  void queryCRC();
  void startAcquisition( bool start);
  void listen();
  void dolisten();
  void doStart();
  void createTrees(std::string s);
  void closeTrees();
  void process(std::string fullpath);
  void rm(std::string fullpath);
  void ls(std::string sourcedir,std::vector<std::string>& res);

  void loop6BDac(uint8_t dacmin,uint8_t dacmax,uint8_t dacstep,uint8_t ngtc);
  void loopVthTime(uint32_t dacmin,uint32_t dacmax,uint32_t dacstep,uint8_t ngtc);
  void storeSlowControl(uint32_t code);
  void set6bDac(uint8_t dac);
  void setMask(uint8_t mask);
  void sendTrigger(uint32_t nt);
  void setVthTime(uint32_t dac);
private:
  NL::Socket* _sCtrl;
  NL::Socket* _sTDC1;
  NL::Socket* _sTDC2;
  NL::SocketGroup* _group;
 
  TdcMessageHandler* _msh;
  lytdc::OnRead* _onRead;
  lytdc::OnAccept* _onAccept;
  lytdc::OnClientDisconnect* _onClientDisconnect;
  lytdc::OnDisconnect* _onDisconnect;
  boost::thread_group g_store;
  boost::thread_group g_run;


  uint32_t disconnected_;
  fsmweb* _fsm;
  uint16_t _slcBuffer[0x1000];
  uint16_t _slcAddr[0x1000];    
  uint32_t _slcBytes;
  uint16_t _slcAck[0x1000];
  uint32_t _slcAckLength;
  PRSlow _s1,_s2;
  uint32_t _run,_type;
  std::string _directory;
  int32_t _fdOut;
  struct evs _eventStruct;
  uint32_t _t0;
  bool _running;

  uint32_t _firstEvent[2],_currentGTC[2];
  uint64_t _currentABCID[2];
  bool _loop;
};
#endif
