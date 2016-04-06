#ifndef _LDaq_h
#define _LDaq_h
#include "LClient.hh"
#include "fsm.hh"
#include <string>
#include <vector>
#include <json/json.h>

class LDaq
{
public:
  LDaq();
  ~LDaq();
  std::string  process(std::string command,std::string param);
  std::string  process(std::string command);
  LClient*  findFSM(DimBrowser* dbr,std::string pattern);
  void  discover(levbdim::fsmmessage* m);
  void  prepare(levbdim::fsmmessage* m);
  std::string  difstatus();
  void  singlescan(LClient* d);
  void  singleinit(LClient* d);
  void  singleregisterdb(LClient* d);
  void  singleconfigure(LClient* d);
  void  singlestart(LClient* d);
  void  singlestop(LClient* d);
  Json::Value  toJson(std::string s);
  void  initialise(levbdim::fsmmessage* m);
  void  configure(levbdim::fsmmessage* m);
  void  start(levbdim::fsmmessage* m);
  void  stop(levbdim::fsmmessage* m);
  void  destroy(levbdim::fsmmessage* m);
  void  doubleSwitchZup(unsigned int pause);
  std::string  LVStatus();
  void  LVON();
  void  LVOFF();
  void  setParameters(std::string jsonString);
  void  setDBState(std::string dbs);
  void  setControlRegister(uint32_t reg);
  std::string  builderStatus();
  std::string  status();
  void  pauseTrigger();
  void  resumeTrigger();
  void  pauseEcal();
  void  resumeEcal();
  void  resetTriggerCounters();
  std::string  triggerStatus();
  void  triggerSpillOn(uint32_t nc);
  void  triggerSpillOff(uint32_t nc);
  std::string state(){return _fsm->state();}
  void forceState(std::string s){_fsm->setState(s);}

private:
  levbdim::fsm* _fsm;
  LClient* _dbClient,*_zupClient,*_cccClient,*_mdccClient,*_builderClient;
  std::vector<LClient*> _DIFClients;
  std::string _strParam;
  Json::Value _jparam;
  Json::Value _proclist;
  std::string _dbstate,_dccname,_mdccname,_zupdevice,_writerdir,_memorydir,_proctype;
  uint32_t _zupport,_ctrlreg,_run;
};
#endif
