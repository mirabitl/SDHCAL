#ifndef _DHCALONLINEREADER_H
#define _DHCALONLINEREADER_H
#include "DHCalEventReader.h"
#include "DCHistogramHandler.h"

class DHCalOnlineReader : public DHCalEventReader, public DCHistogramHandler
{
 public:
  virtual ~DHCalOnlineReader(){;}
  virtual void initHistograms()=0;
  virtual void processEvent()=0;
};
#endif
