#ifndef _RootShmProcessor_h_
#define _RootShmProcessor_h_
#include "shmdriver.hh"

#include <stdint.h>
class DHCalEventReader;
class RootShmProcessor : public levbdim::shmprocessor
{
public:
  RootShmProcessor(std::string dir="/tmp",std::string setup="default");

  virtual void start(uint32_t run);
  virtual void stop();
  virtual  void processEvent(uint32_t key,std::vector<levbdim::buffer*> dss);
  virtual  void processRunHeader(std::vector<uint32_t> header);

  std::string getOutputFileName(uint32_t run,uint32_t seq);
  void close();
  std::string getSetup(){return _setup;}
 private:
  std::string _filepath,_setup;
  uint32_t theEventNumber_,theRunNumber_,theSequence_;
  DHCalEventReader* _der;
  bool _started;
};
#endif
