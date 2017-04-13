#ifndef _FullDaq_h
#define _FullDaq_h
#include "fsmwebCaller.hh"
#include "baseApplication.hh"
#include <string>
#include <vector>
#include <json/json.h>

class FullDaq : public levbdim::baseApplication
{
public:
  FullDaq(std::string name);
  ~FullDaq();
 
  void  discover(levbdim::fsmmessage* m);
  void  prepare(levbdim::fsmmessage* m);
  std::string  difstatus();
  void  singlescan(fsmwebCaller* d);
  void  singleinit(fsmwebCaller* d);
  void  singleregisterdb(fsmwebCaller* d);
  void  singleconfigure(fsmwebCaller* d);
  void  singlestart(fsmwebCaller* d);
  void  singlestop(fsmwebCaller* d);
  Json::Value  toJson(std::string s);
  void  initialise(levbdim::fsmmessage* m);
  void  configure(levbdim::fsmmessage* m);
  void  start(levbdim::fsmmessage* m);
  void  stop(levbdim::fsmmessage* m);
  void  destroy(levbdim::fsmmessage* m);


  void forceState(Mongoose::Request &request, Mongoose::JsonResponse &response);
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
  void registerDataSource(Mongoose::Request &request, Mongoose::JsonResponse &response);

  //Calibration
  void triggerSpillRegister(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void triggerCalibCount(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void triggerCalibOn(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void triggerReloadCalib(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void tdcSet6bDac(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void tdcSetVthTime(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void tdcSetMask(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void setRunHeader(Mongoose::Request &request, Mongoose::JsonResponse &response);

  
  std::string state(){return _fsm->state();}
  void forceState(std::string s){_fsm->setState(s);}
  // Virtual from baseAPplication
  virtual void  userCreate(levbdim::fsmmessage* m);
private:
  fsmweb* _fsm;
  fsmwebCaller* _dbClient,*_zupClient,*_cccClient,*_mdccClient,*_builderClient,*_gpioClient;
  std::vector<fsmwebCaller*> _DIFClients;
    std::vector<fsmwebCaller*> _tdcClients;
 
  std::string _dbstate;
  uint32_t _ctrlreg,_run;
  Json::Value _jConfigContent;
};
#endif
