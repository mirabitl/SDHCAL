#ifndef _LMdccServer_h

#define _LMdccServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "fsmweb.hh"

#include "MDCCReadout.h"

using namespace std;
#include <sstream>
#include "LdaqLogger.hh"

class LMdccServer
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

  MDCCReadout* getMDCCReadout(){  //std::cout<<" get Ptr "<<_mdcc<<std::endl;
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

private:
  //levbdim::fsm* _fsm;
  fsmweb* _fsm;
 
  MDCCReadout* _mdcc;
};
#endif

