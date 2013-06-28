#ifndef _LMANALYZER_H
#define _LMANALYZER_H
#include "DHCALAnalyzer.h"
#include "DHCalEventReader.h"
#include "DCHistogramHandler.h"
#include <iostream>
#include "RecoDIF.h"

class LMAnalyzer : public DHCALAnalyzer
{
 public:
  LMAnalyzer(DHCalEventReader* r,DCHistogramHandler* h);
  virtual void processEvent();
  virtual void initHistograms(){;}
  virtual void processRunHeader(){;}
  virtual void initJob(){;}
  virtual void endJob();
  virtual void initRun(){;}
  virtual void endRun(){;}
  bool decodeTrigger(LCCollection* rhcol, float tcut);
  void fillHistos(unsigned int synchcut);
  void setCollectionName(std::string s){ collectionName_=s;}
  void registerDIF(int id);
 private:
  DHCalEventReader* reader_;
  DCHistogramHandler* handler_;
  unsigned int nAnalyzed_,nInSynch_;
  float lasttime_;
  std::map<unsigned int, std::map<unsigned int,RecoDIF> > recevents_;
  std::map<unsigned int, std::pair<unsigned int,unsigned int> > noisesum_;
  std::string collectionName_;
};
#endif
