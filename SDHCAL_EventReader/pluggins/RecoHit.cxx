#include "RecoHit.hh"
#include <TPrincipal.h>
#include <stdio.h>
#include <iostream>
#include <cfloat>

pcaComponents::pcaComponents(){memset(_components,0,21*sizeof(double));}


RecoHit::RecoHit(jsonGeo *g,IMPL::RawCalorimeterHitImpl* h) : _raw(h),_geo(g)
{
  _geo->convert(this->dif(),this->asic(),this->channel(),this);
  int ithr= amplitude()&0x3;
  _tag.reset();
  if (ithr ==1) this->setTag(RecoHit::THR1,true);
  if (ithr ==2) this->setTag(RecoHit::THR0,true);
  if (ithr ==3) this->setTag(RecoHit::THR2,true);
 
}
							      


template <class T>
pcaComponents RecoHit::calculateComponents(std::vector<T*> vnear_)
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
void RecoHit::initialise(jsonGeo* g,IMPL::RawCalorimeterHitImpl* h) 
{
 _geo->convert(this->dif(),this->asic(),this->channel(),this);
  int ithr= amplitude()&0x3;
  _tag.reset();
  if (ithr ==1) this->setTag(RecoHit::THR1,true);
  if (ithr ==2) this->setTag(RecoHit::THR0,true);
  if (ithr ==3) this->setTag(RecoHit::THR2,true);
}
