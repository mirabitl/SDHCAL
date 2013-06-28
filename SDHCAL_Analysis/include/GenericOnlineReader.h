#ifndef _GenericONLINEREADER_H
#define _GenericONLINEREADER_H
#include "DHCalOnlineReader.h"
#include "DCHistogramHandler.h"

class GenericOnlineReader : public DHCalOnlineReader
{
 public:
  virtual ~GenericOnlineReader(){;}
  virtual void initHistograms();
  virtual void processEvent();
};
#endif
