#include "TrackInfo.h"
void TrackInfo::regression()
{

  double z2=0,z=0,zy=0,y=0,zx=0,x=0,w=0,zmin_=0,zmax_=0;
  if (np_<2) return;
  for (uint32_t i=0;i<np_;i++)
    {

      z+=z_[i];
      z2+=z_[i]*z_[i];
      zy+=z_[i]*y_[i];
      y+=y_[i];
      zx+=z_[i]*x_[i];
      x+=x_[i];
      if (z_[i]<zmin_) zmin_=z_[i];
      if (z_[i]>zmax_) zmax_=z_[i];
      
    }
  
  //std::cout<<x<<" "<<x2<<" "<<xy<<" "<<y<<" "<<w<<" "<<vx.size()<<std::endl;
  double a=z2,b=z,c=z,d=np_;
  double det=(a*d-b*c);
  //std::cout<<"Det ="<<det<<std::endl;
  double m11= d/det;
  double m12=-b/det;
  double m21=-c/det;
  double m22=a/det;
  ay_=m11*zy+m12*y;
  by_=m21*zy+m22*y;
  ax_=m11*zx+m12*x;
  bx_=m21*zx+m22*x;
  // std::cout<<alpha<<" "<<beta<<std::endl;
  chi2x_=0;
  chi2y_=0;
  for  (uint32_t i=0;i<np_;i++)
    {
      chi2x_+=(x_[i]-ax_*z_[i]-bx_)*(x_[i]-ax_*z_[i]-bx_);
      chi2y_+=(y_[i]-ay_*z_[i]-by_)*(y_[i]-ay_*z_[i]-by_);
    }
  return;


}
void TrackInfo::exclude_layer(uint32_t layer,TrackInfo& ti)
{
  memset(&ti,0,sizeof(TrackInfo));
  uint32_t np=0;
  double zmin=9999999.,zmax=-9999999;
  for (uint32_t i=0;i<np_;i++)
    {
      if (layer_[i]==layer) continue;
      ti.set_z(np,z_[i]);
      ti.set_x(np,x_[i]);
      ti.set_y(np,y_[i]);
      ti.set_layer(np,layer_[i]);
      if (z_[i]<zmin) zmin=z_[i];
      if (z_[i]>zmax) zmax=z_[i];
      np++;
    }
  ti.set_size(np);
  ti.set_zmin(zmin);
  ti.set_zmax(zmax);
  ti.regression();
}
