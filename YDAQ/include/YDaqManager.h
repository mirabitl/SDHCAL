#ifndef YAMI_YDAQMANAGER_INCLUDE
#define YAMI_YDAQMANAGER_INCLUDE
#include <yami4-cpp/yami.h>

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <string>
#include <map>
#include <vector>
#include "difhw.h"
#include "ccc.h"
#include "onedifhandler.h"
#include "evb.h"
#include "odb.h"
#include "zuplv.h"
#include "browser.h"

class YDaqManager
{
public:
  YDaqManager(std::string nameserver,std::string configuration);
  // Parsing
  void Parse(std::string conf);

  // Server managment
  void StopServer(std::string name,std::string remoteip);
  void StartServer(std::string name,std::string remoteip);

  // General command
  void Discover();
  void LVOn();
  void LVOff();
  void Initialise();
  void Download();
  void LoadSlowControl();
  void Start();
  void Pause();
  void Resume();
  void EVBStatus();
  void DestroyDIF();
  void Stop();
  void Clear();
  // Getters Statemachine
  Evb::Statemachine* getEvb(){return evbs;}
  Odb::Statemachine* getOdb(){return odbs;}
  Ccc::Statemachine* getCcc(){return cccs;}
  Zuplv::Statemachine* getZup(){return zups;}
  Difhw::onedifhandler* getDifHandler(std::string name){return difsmap[name];}

  // Getter Configuration
  Evb::Config& getEVBConfiguration() {return evbconf;}
  Zuplv::Config& getZUPConfiguration() {return zupconf;}
  Ccc::Config& getCCCConfiguration() {return cccconf;}
  Difhw::Config& getDIFConfiguration() {return difconf;}
  // Getter Configuration
  Evb::Status& getEVBStatus() {return evbstatus;}
  Zuplv::Status& getZUPStatus() {return zupstatus;}
  Ccc::Status& getCCCStatus() {return cccstatus;}
  Difhw::Difstatus& getDIFStatus(std::string name) {return difsmap[name]->getStatus();}
  Ccc::Registeraccess& getCCCRegisterAccess() {return cccreg;}

  
private:
  yami::agent client_agent;

  //hosts
  std::string name_server,daqconfig;
  std::string evbhost,ccchost,odbhost,zuphost;
  std::vector<std::string> difhosts;
  //State Machine 
  Evb::Statemachine *evbs;
  Odb::Statemachine *odbs;
  Zuplv::Statemachine *zups;
  Ccc::Statemachine *cccs;
  std::map<std::string,Difhw::onedifhandler*> difsmap;
  // Configuration
  Evb::Config evbconf;
  Evb::Runconfig evbrun;
  Evb::Status evbstatus;

  Odb::Config odbconf;
  Odb::Status odbstatus;
   
  Zuplv::Config zupconf;
  Zuplv::Status zupstatus;

  Ccc::Config cccconf;
  Ccc::Registeraccess cccreg;
  Ccc::Status cccstatus;

  Difhw::Config difconf;
   
};
#endif
