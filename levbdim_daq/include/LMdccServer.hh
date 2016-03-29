#ifndef _LMdccServer_h

#define _LMdccServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "fsm.hh"

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
  void reset(levbdim::fsmmessage* m);
  void cmd(levbdim::fsmmessage* m);
  void doOpen(std::string s);

  MDCCReadout* getMDCCReadout(){  std::cout<<" get Ptr "<<_mdcc<<std::endl;
return _mdcc;}
private:
  levbdim::fsm* _fsm;
 
  MDCCReadout* _mdcc;
};
#endif

