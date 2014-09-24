#ifndef _LCIOWritterInterface_h_
#define _LCIOWritterInterface_h_
#include "DIFWritterInterface.h"
#include "DHCalEventReader.h"
#include <stdint.h>
class LCIOWritterInterface : public DIFWritterInterface
{
public:
  LCIOWritterInterface();
  std::string getOutputFileName(uint32_t run,uint32_t seq);

  virtual void openFile(uint32_t run=0,std::string dir="/tmp",std::string setup="default");
  virtual void writeEvent(uint32_t gtc,std::vector<unsigned char*> vbuf);
  virtual void closeFile();
 private:
  uint32_t theEventNumber_,theRunNumber_,theSequence_;
  DHCalEventReader* theMonitor_;
};
#endif
