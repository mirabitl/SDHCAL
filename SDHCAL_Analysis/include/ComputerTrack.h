#ifndef ComputerTrack_H_
#define ComputerTrack_H_
#include <vector>
#include <stdint.h>
#include "libhoughStruct.h"

#include "libhoughCPU.h"
#include "RecoCandTk.h"

typedef struct
{
  float RhoMin;
  float RhoMax;
  uint32_t NRho;
  uint32_t NTheta;
  uint32_t NStubLow;
  uint32_t NLayerRow;
  uint32_t NStubLowCandidate;
  uint32_t NBins3GeV;
  uint32_t NBins5GeV; 
  uint32_t NBins15GeV;
  uint32_t NBins30GeV;
  uint32_t NBins100GeV;
  uint32_t NStubHigh;
  uint32_t NLayerHigh;
  uint32_t NStubHighCandidate;
  float NDelBarrel;
  float NDelInter;
  float NDelEndcap;
  
} HoughCut;



class ComputerTrack
{
public:
  ComputerTrack(HoughCut* cuts);
  ~ComputerTrack();
  void DefaultCuts();
  void associate(uint32_t nstub,float* x,float* y,float* z,uint32_t* layer);
  void telescope(uint32_t nstub,float* x,float* y,float* z,uint32_t* layer,uint32_t nplans);

  std::vector<RecoCandTk> &getCandidates(){return theCandidateVector_;}
  float Length(){return theLength_;}
protected:
  std::vector<RecoCandTk> theCandidateVector_;
  HoughCut* theCuts_;
  uint32_t theNStub_;
  float* theX_;
  float* theY_;
  float* theZ_;
  uint32_t* theLayer_;
  float theLength_;
};
#endif
