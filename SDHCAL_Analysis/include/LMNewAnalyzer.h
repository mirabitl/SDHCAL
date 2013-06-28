#ifndef _LMNEWANALYZER_H
#define _LMNEWANALYZER_H
#include "DHCALAnalyzer.h"
#include "DHCalEventReader.h"
#include "DCHistogramHandler.h"
#include <iostream>
#include "RecoDIF.h"

class LMNewAnalyzer : public DHCALAnalyzer
{
 public:
  LMNewAnalyzer(DHCalEventReader* r,DCHistogramHandler* h);
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
  void mask(unsigned int difid,unsigned int ix,unsigned int iy);
  bool isMasked(unsigned int difid,unsigned int ix,unsigned int iy);
  unsigned long long getExternalTriggerTime() { return externalTriggerTime_;}
  unsigned long long getAbsoluteFrameTime(int bc) { return externalTriggerTime_-bc;}
  bool isFirstPowerPulsed(int cutms);
 private:
  DHCalEventReader* reader_;
  DCHistogramHandler* handler_;
  unsigned int nAnalyzed_,nInSynch_,run_;
  unsigned long long externalTriggerTime_,lastSpill_,lastPowerPulsedTime_;
  std::map<unsigned int, std::map<unsigned int,RecoDIF> > recevents_;
  std::map<unsigned int, std::pair<unsigned int,unsigned int> > noiseSummaryMap_;
  std::vector<unsigned int> padMask_;


  // Control
  std::string collectionName_;
  bool dropFirstSpillEvent_,dropFirstPowerPulsed_;
  float spillSize_;
  unsigned int clockSynchCut_;
};
#endif
