#ifndef _LBuilder_h
#define _LBuilder_h

#include "fsm.hh"
#include "datasocket.hh"
#include "shmdriver.hh"
#include "basicwriter.hh"
#include "LcioShmProcessor.hh"
#include "LdaqLogger.hh"

class LBuilder {
public:
  LBuilder(std::string name);
  void initialise(levbdim::fsmmessage* m);
  void configure(levbdim::fsmmessage* m);
  void start(levbdim::fsmmessage* m);
  void stop(levbdim::fsmmessage* m);
  void halt(levbdim::fsmmessage* m);
  void destroy(levbdim::fsmmessage* m);
  void status(levbdim::fsmmessage* m);


private:
  levbdim::fsm* _fsm;
  std::vector<levbdim::datasocket*> _sources;
  std::string _memorypath,_filepath,_proctype;
  levbdim::shmdriver* _evb;
  levbdim::shmprocessor* _writer;
};

#endif
