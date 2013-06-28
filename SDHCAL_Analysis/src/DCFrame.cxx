
#include "DCFrame.h"
#include <bitset>
DCFrame::DCFrame()
{
  difId_=0;
  asicId_=0;
  timeToTrigger_=0;
  timeThreshold_=0;
  bc_=0;
  reco_bc_=0;
  nHit0_ =0; nHit1_=0;
  
  synchronised_=false;
}
void DCFrame::setAll(unsigned short dif,unsigned short asic,double bc,bool* l0,bool* l1,double ti,double threshold) 
{
  difId_=dif;asicId_=asic;timeToTrigger_=ti; timeThreshold_=threshold;
  bc_=bc;

  nHit0_ =0; nHit1_=0;
  
  for (unsigned int i=0;i<64;i++)
    {
      level0_[i]=l0[i];
      level1_[i]=l1[i];
      if (level0_[i]) nHit0_++;
      if (level1_[i]) nHit1_++;
      synchronised_ = (timeToTrigger_<timeThreshold_);
    }
}
DCFrame::DCFrame(unsigned short dif,unsigned short asic,double bc,bool* l0,bool* l1,double ti,double threshold) : difId_(dif),asicId_(asic),timeToTrigger_(ti), timeThreshold_(threshold)
{
  bc_=bc;

  nHit0_ =0; nHit1_=0;
  
  for (unsigned int i=0;i<64;i++)
    {
      level0_[i]=l0[i];
      level1_[i]=l1[i];
      if (level0_[i]) nHit0_++;
      if (level1_[i]) nHit1_++;
      synchronised_ = (timeToTrigger_<timeThreshold_);
    }
}
DCFrame::~DCFrame()
{

}
void DCFrame::Dump()
{


 std::bitset<64> l0;
  std::bitset<64> l1;
  l0.reset();
  l1.reset();
  for (unsigned int i=0;i<64;i++)
    {
      l0.set(i,getLevel0(i));
      l1.set(i,getLevel1(i));
    }
  
  std::cout<<bc_<<"-";
  std::cout<<reco_bc_<<"-";
  std::cout<<timeToTrigger_<<": ";
  std::cout<<"("<<difId_<<","<<asicId_<<")"<<std::endl;
  std::cout<<"\t "<<l0<<std::endl;
  std::cout<<"\t "<<l1<<std::endl;

}
void DCFrame::setSynchronised(bool t) {synchronised_=t;}
unsigned short DCFrame::getDifId() { return difId_;}
unsigned short DCFrame::getAsicId() { return asicId_;}
double DCFrame::getBunchCrossingTime() { return bc_;}
double DCFrame::getRecoBunchCrossingTime() { return reco_bc_;}
void DCFrame::setRecoBunchCrossingTime(double t) { reco_bc_=t;}

double DCFrame::getTimeToTrigger() { return timeToTrigger_;}

bool  DCFrame::getLevel0(unsigned int i)  { return  level0_[i]; }
bool  DCFrame::getLevel1(unsigned int i)  { return  level1_[i]; }
bool DCFrame::isSynchronised() { return synchronised_;}
unsigned short DCFrame::getNumberOfHit(unsigned short level) { if ( level == 0) return nHit0_; if (level == 1) return nHit1_; return 0;}
