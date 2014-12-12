#ifndef _RECOHIT_H

#define _RECOHIT_H
#include <limits.h>

#include <bitset>

#include "IMPL/LCTOOLS.h"
#include "EVENT/RawCalorimeterHit.h" 
#include "IMPL/LCGenericObjectImpl.h"
#include "IMPL/RawCalorimeterHitImpl.h"
#include "UTIL/CellIDDecoder.h"
#include "DifGeom.h"
#include "ChamberGeom.h"
#include <stdint.h>
#include <list>
class Shower;
class RecoHit
{
public:
  enum Type {THR0=0,THR1=1,THR2=2,CORE=3,EDGE=4,ISOLATED=5,HOUGH=6,MIP=7};

  RecoHit(){;}
  RecoHit(DifGeom& d, ChamberGeom& c,IMPL::RawCalorimeterHitImpl* h,uint32_t hrtype=2);
  void initialise(DifGeom& d, ChamberGeom& c,IMPL::RawCalorimeterHitImpl* h,uint32_t hrtype=2);
  double X();
  double Y();
  double Z();
  int chamber();
  int plan();
  std::bitset<8> Flag(){return theFlag_;}
  void setFlag(RecoHit::Type t,bool v){theFlag_.set(t,v);}
  bool getFlag(RecoHit::Type t){return theFlag_[t];}
  void clearFlag(){theFlag_.set(0);}

  uint8_t I(){return chamberLocalI_;}
  uint8_t  J(){return chamberLocalJ_;}

  unsigned short dif(){return dg_.getId();}
  uint8_t getDifI(){ return difLocalI_;}
  uint8_t getDifJ(){ return difLocalJ_;}
  uint8_t getAsic(){ return 0xFF & (raw_->getCellID0()&0xFF00)>>8;}
  uint8_t getChannel(){ return 0xFF & (raw_->getCellID0()&0x3F0000)>>16;}
  uint16_t getAmplitude(){return raw_->getAmplitude();}
  //  inline void setIndices(uint32_t i,uint32_t n) { index_=i;next_=n;}
  //inline uint32_t getIndex(){return index_;}
  //inline uint32_t getNext(){return next_;}
  Shower* getShower(){return shower_;}
  void setShower(Shower* s){shower_=s;}
  //bool addNearby(RecoHit* h,float distcut);
  void calculateComponents(std::vector<RecoHit*> v);
  double* Components(){return components_;}
  //std::list<RecoHit*>& Voisins(){return vnear_;}
  //uint32_t getNumberOfVoisins(){return nnear_;}
  void clear();
  static double pad2cm(){return 1.04125;}

private:
  DifGeom dg_;
  ChamberGeom cg_;
  IMPL::RawCalorimeterHitImpl* raw_;
  uint8_t difLocalI_,difLocalJ_;
  uint8_t chamberLocalI_,chamberLocalJ_;
  // uint32_t index_,next_;
  double x_,y_;
  std::bitset<8> theFlag_;
  Shower* shower_;
  uint32_t nnear_;
  //std::list<RecoHit*> vnear_;
  double components_[21];
};
#endif
