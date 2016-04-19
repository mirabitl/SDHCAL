#include "RecoHit.hh"

#include <stdio.h>
#include <iostream>


pcaComponents::pcaComponents(){memset(_components,0,21*sizeof(double));}


RecoHit::RecoHit(jsonGeo *g,IMPL::RawCalorimeterHitImpl* h) : _raw(h),_geo(g),_used(false)
{
  _geo->convert(this->dif(),this->asic(),this->channel(),this);
  int ithr= amplitude()&0x3;
  _tag.reset();
  if (ithr ==1) this->setTag(RecoHit::THR1,true);
  if (ithr ==2) this->setTag(RecoHit::THR0,true);
  if (ithr ==3) this->setTag(RecoHit::THR2,true);
 
}
							      


void RecoHit::initialise(jsonGeo* g,IMPL::RawCalorimeterHitImpl* h) 
{
 _geo->convert(this->dif(),this->asic(),this->channel(),this);
  int ithr= amplitude()&0x3;
  _tag.reset();
  if (ithr ==1) this->setTag(RecoHit::THR1,true);
  if (ithr ==2) this->setTag(RecoHit::THR0,true);
  if (ithr ==3) this->setTag(RecoHit::THR2,true);
}
