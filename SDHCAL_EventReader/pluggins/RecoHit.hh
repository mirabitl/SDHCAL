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
#include <TPrincipal.h>
#include <cfloat>
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

  RecoHit(){_geo=NULL;_raw=NULL;_used=false;}
  RecoHit(const RecoHit& obj)
  {
    _raw=obj.raw();
    _geo=obj.geo();
    _weight=obj.weight();
    _used=obj.isUsed();
    initialise(_geo,_raw);
  }

  RecoHit(jsonGeo* d,IMPL::RawCalorimeterHitImpl* h);
  void initialise(jsonGeo* d,IMPL::RawCalorimeterHitImpl* h);
  uint16_t dif(){return _raw->getCellID0()&0xFF;}
  uint16_t asic(){ return 0xFF & (_raw->getCellID0()&0xFF00)>>8;}
  uint16_t channel(){ return 0xFF & (_raw->getCellID0()&0x3F0000)>>16;}
  uint16_t amplitude(){return _raw->getAmplitude();}
  //uint32_t chamber(){return _geo->difGeo(this->dif())["chamber"].asUInt();}
  //uint32_t plan(){ return _geo->chamberGeo(this->chamber())["plan"].asUInt();}
  uint32_t chamber() {return _id&0xFF;}
  uint32_t plan(){return (_id>>8)&0xFF;}
  std::bitset<8> Tag(){return _tag;}
  void setTag(RecoHit::Type t,bool v){_tag.set(t,v);}
  bool isTagged(RecoHit::Type t){return _tag[t];}
  void clearTag(){_tag.set(0);}

  uint16_t I(){return int(X()/1.04125);}
  uint16_t  J(){return int(Y()/1.04125);}
  IMPL::RawCalorimeterHitImpl* raw() const {return _raw;}
  inline jsonGeo* geo() const {return _geo;}
  inline void setWeight(float t)  { _weight=t;}
  inline float weight() const {return _weight;}
  inline void setUse(bool t)  { _used=t;}
  inline bool isUsed() const {return _used;}

  template <class T>
  static pcaComponents calculateComponents(std::vector<T*> vnear_)
{
  pcaComponents c;

  if (vnear_.size()<3) return c;
  TPrincipal tp(3,"");
  double xp[3];
  uint32_t nh=0;
  double xb=0,yb=0,zb=0;
  double wt=0.;
  
  double fp=DBL_MAX;
  double lp=-DBL_MAX;
  double fx=DBL_MAX;
  double lx=-DBL_MAX;
  double fy=DBL_MAX;
  double ly=-DBL_MAX;


  //INFO_PRINT("%d vector size\n",v.size());
  for (typename std::vector<T*>::iterator it=vnear_.begin();it!=vnear_.end();it++)
    {
      T* iht=(*it);
 
      double w=1.;
      xb+=iht->X()*w;
      yb+=iht->Y()*w;
      zb+=iht->Z()*w;
      wt+=w;
      xp[0]=iht->X();
      xp[1]=iht->Y();
      xp[2]=iht->Z();
      tp.AddRow(xp);
      if (iht->Z()<fp) fp=iht->Z();
      if (iht->Z()>lp) lp=iht->Z();
      if (iht->X()<fx) fx=iht->X();
      if (iht->X()>lx) lx=iht->X();
      if (iht->Y()<fy) fy=iht->Y();
      if (iht->Y()>ly) ly=iht->Y();
      nh++;
    }
  
  if (nh<3) return c;
  tp.MakePrincipals();
  c[0]=(*tp.GetMeanValues())[0];
  c[1]=(*tp.GetMeanValues())[1];
  c[2]=(*tp.GetMeanValues())[2];



  c[3]=(*tp.GetEigenValues())[2];
  c[4]=(*tp.GetEigenValues())[1];
  c[5]=(*tp.GetEigenValues())[0];


  c[6]=(*tp.GetEigenVectors())(0,0);
  c[7]=(*tp.GetEigenVectors())(1,0);
  c[8]=(*tp.GetEigenVectors())(2,0);	
  c[9]=(*tp.GetEigenVectors())(0,1);
  c[10]=(*tp.GetEigenVectors())(1,1);
  c[11]=(*tp.GetEigenVectors())(2,1);
  c[12]=(*tp.GetEigenVectors())(0,2);
  c[13]=(*tp.GetEigenVectors())(1,2);
  c[14]=(*tp.GetEigenVectors())(2,2);

  // Store First and last Z
  c[15]=fp;
  c[16]=lp;
  c[17]=fx;
  c[18]=lx;
  c[19]=fy;
  c[20]=ly;
  
  return c;
}

private:
  jsonGeo* _geo;
  uint32_t _id;
  IMPL::RawCalorimeterHitImpl* _raw;
  std::bitset<8> _tag;
  float _weight;
  bool _used;
};
#endif
