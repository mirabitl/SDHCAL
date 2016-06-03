#include "RecoHit.hh"

#include <stdio.h>
#include <iostream>


pcaComponents::pcaComponents(){memset(_components,0,21*sizeof(double));}


RecoHit::RecoHit(jsonGeo *g,IMPL::RawCalorimeterHitImpl* h) : _raw(h),_geo(g),_used(false)
{

  uint8_t cham= (_geo->difGeo(this->dif())["chamber"].asUInt()&0xFF);
  uint8_t plan=(_geo->chamberGeo(cham)["plan"].asUInt()&0xFF);
  _id=(plan<<8) | cham;
  _geo->convert(this->dif(),this->asic(),this->channel(),this);
  int ithr= amplitude()&0x3;
  _tag.reset();
  if (ithr ==1) this->setTag(RecoHit::THR1,true);
  if (ithr ==2) this->setTag(RecoHit::THR0,true);
  if (ithr ==3) this->setTag(RecoHit::THR2,true);
 
}
							      


void RecoHit::initialise(jsonGeo* g,IMPL::RawCalorimeterHitImpl* h) 
{
  _raw=h;
  _geo=g;
  _used=false;
  uint8_t cham= (_geo->difGeo(this->dif())["chamber"].asUInt()&0xFF);
  uint8_t plan=(_geo->chamberGeo(cham)["plan"].asUInt()&0xFF);
  _id=(plan<<8) | cham;
  // std::cout<<"avant convert "<<(int64_t) h<<" "<<(int64_t) g<<std::endl;
 _geo->convert(this->dif(),this->asic(),this->channel(),this);
  int ithr= amplitude()&0x3;
  _tag.reset();
  if (ithr ==1) this->setTag(RecoHit::THR1,true);
  if (ithr ==2) this->setTag(RecoHit::THR0,true);
  if (ithr ==3) this->setTag(RecoHit::THR2,true);
}
