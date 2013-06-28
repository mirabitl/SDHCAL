#ifndef _LMBASICANALYZER_H
#define _LMBASICANALYZER_H
#include "DHCALAnalyzer.h"
#include "DHCalEventReader.h"
#include "DCHistogramHandler.h"
#include <iostream>
class LMBasicAnalyzer : public DHCALAnalyzer
{
 public:
  LMBasicAnalyzer(DHCalEventReader* r,DCHistogramHandler* h);
  virtual void processEvent();
  virtual void initHistograms();
  virtual void processRunHeader(){;}
  virtual void initJob(){;}
  virtual void endJob(){;}
  virtual void initRun(){;}
  virtual void endRun(){;}
 private:
  DHCalEventReader* reader_;
  DCHistogramHandler* handler_;
  unsigned int nAnalyzed_,nInSynch_;
};
#endif
