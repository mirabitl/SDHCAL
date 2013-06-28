#ifndef _FILTERANALYZER_H
#define _FILTERANALYZER_H
#include "DHCALAnalyzer.h"
#include "DHCalEventReader.h"
#include "DCHistogramHandler.h"
#include <iostream>
class FilterAnalyzer : public DHCALAnalyzer
{
 public:
  FilterAnalyzer(DHCalEventReader* r,DCHistogramHandler* h);
  virtual void processEvent();
  virtual void initHistograms();
  virtual void processRunHeader()
  {
    if (writing_)
      reader_->writeRunHeader();
  }
  void setWriting(bool t){writing_=t;}
  virtual void initJob(){;}
  virtual void endJob(){;}
  virtual void initRun(){;}
  virtual void endRun(){;}
  inline void setminChambersInTime(int t){minChambersInTime_=t;}
  inline void setuseSynchronized(bool t){useSynchronized_=t;}
 private:
  DHCalEventReader* reader_;
  DCHistogramHandler* handler_;
  bool writing_;
  bool headerWritten_;
  int minChambersInTime_;
  bool useSynchronized_;
};
#endif
