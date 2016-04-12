#ifndef _RECOHIT_HH

#define _RECOHIT_HH
#include <limits.h>

#include <bitset>

#include "IMPL/LCTOOLS.h"
#include "EVENT/RawCalorimeterHit.h" 
#include "IMPL/LCGenericObjectImpl.h"
#include "IMPL/RawCalorimeterHitImpl.h"
#include "UTIL/CellIDDecoder.h"
#include "jsonGeo.hh"
#include <stdint.h>
#include <Math/Point3Dfwd.h>
class pcaComponents
{
public:
  pcaComponents();
  double& operator[](uint32_t i) { return _components[i];}
private:
  double _components[21];
};

using namespace ROOT::Math;
class RecoHit : public ROOT::Math::XYZPoint
{
public:
  enum Type {THR0=0,THR1=1,THR2=2,CORE=3,EDGE=4,ISOLATED=5,HOUGH=6,MIP=7};

  RecoHit();
  RecoHit(const RecoHit& obj)
  {
    _raw=obj.raw();
    _geo=obj.geo();
    initialise(_geo,_raw);
  }

  RecoHit(jsonGeo* d,IMPL::RawCalorimeterHitImpl* h);
  void initialise(jsonGeo* d,IMPL::RawCalorimeterHitImpl* h);
  uint16_t dif(){return _raw->getCellID0()&0xFF;}
  uint16_t asic(){ return 0xFF & (_raw->getCellID0()&0xFF00)>>8;}
  uint16_t channel(){ return 0xFF & (_raw->getCellID0()&0x3F0000)>>16;}
  uint16_t amplitude(){return _raw->getAmplitude();}
  uint32_t chamber(){return _geo->difGeo(this->dif())["chamber"].asUInt();}
  uint32_t plan(){ return _geo->chamberGeo(this->chamber())["plan"].asUInt();}
  std::bitset<8> Tag(){return _tag;}
  void setTag(RecoHit::Type t,bool v){_tag.set(t,v);}
  bool isTagged(RecoHit::Type t){return _tag[t];}
  void clearTag(){_tag.set(0);}

  uint16_t I(){return int(X()/1.04125);}
  uint16_t  J(){return int(Y()/1.04125);}
  IMPL::RawCalorimeterHitImpl* raw() const {return _raw;}
  jsonGeo* geo() const {return _geo;}
  

  template <class T>
  static pcaComponents calculateComponents(std::vector<T*> v);

private:
  jsonGeo* _geo;
  IMPL::RawCalorimeterHitImpl* _raw;
  std::bitset<8> _tag;

};
#endif
