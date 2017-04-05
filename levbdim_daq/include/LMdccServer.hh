#ifndef _LMdccServer_h

#define _LMdccServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "baseApplication.hh"

#include "MDCCHandler.hh"

using namespace std;
#include <sstream>
#include "LdaqLogger.hh"

class LMdccServer : public levbdim::baseApplication
{
public:
  LMdccServer(std::string name);
  void open(levbdim::fsmmessage* m);
  void close(levbdim::fsmmessage* m);
  void pause(levbdim::fsmmessage* m);
  void resume(levbdim::fsmmessage* m);
  void ecalpause(levbdim::fsmmessage* m);
  void ecalresume(levbdim::fsmmessage* m);
  void reset(levbdim::fsmmessage* m);
  void cmd(levbdim::fsmmessage* m);
  void doOpen(std::string s);

  MDCCHandler* getMDCCHandler(){  //std::cout<<" get Ptr "<<_mdcc<<std::endl;
    return _mdcc;}
  void c_joblog(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_status(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_pause(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_resume(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_ecalpause(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_ecalresume(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_reset(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_readreg(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_writereg(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_spillon(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_spilloff(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_beamon(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setcalibcount(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_reloadcalib(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_calibon(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_caliboff(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_resettdc(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setspillregister(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_settrigext(Mongoose::Request &request, Mongoose::JsonResponse &response);
private:
  //levbdim::fsm* _fsm;
  fsmweb* _fsm;
 
  MDCCHandler* _mdcc;
};
#endif

