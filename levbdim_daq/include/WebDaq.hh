#ifndef _WebDaq_h
#define _WebDaq_h
#include "LClient.hh"
#include "fsmweb.hh"
#include <string>
#include <vector>
#include <json/json.h>

class WebDaq
{
public:
  WebDaq(std::string name,uint32_t port);
  ~WebDaq();
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


  void doubleSwitchZup(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void  LVStatus(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void LVON(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void LVOFF(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void setParameters(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void getParameters(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void downloadDB(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void setControlRegister(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void dbStatus(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void  builderStatus(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void status(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void pauseTrigger(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void resumeTrigger(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void pauseEcal(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void resumeEcal(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void resetTriggerCounters(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void triggerStatus(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void triggerSpillOn(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void triggerSpillOff(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void triggerBeam(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void setThreshold(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void setGain(Mongoose::Request &request, Mongoose::JsonResponse &response);

  std::string state(){return _fsm->state();}
  void forceState(std::string s){_fsm->setState(s);}

private:
  fsmweb* _fsm;
  LClient* _dbClient,*_zupClient,*_cccClient,*_mdccClient,*_builderClient;
  std::vector<LClient*> _DIFClients;
  std::string _strParam;
  Json::Value _jparam;
  Json::Value _proclist;
  std::string _dbstate,_dccname,_mdccname,_zupdevice,_writerdir,_memorydir,_proctype;
  uint32_t _zupport,_ctrlreg,_run;
};
#endif
