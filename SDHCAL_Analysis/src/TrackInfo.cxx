#include "TrackInfo.h"
double TrackInfo::v(int i)
{
  switch (i)
    {
    case 0:
      return ax_;
    case 1:
      return ay_;
    case 2:
      return 1;
    default:
      return 999999;
    }
}
double TrackInfo::m(int i)
{
  switch (i)
    {
    case 0:
      return bx_;
    case 1:
      return by_;
    case 2:
      return 0;
    default:
      return 999999;
    }
}
double TrackInfo::closestApproach(double x,double y,double z)
{
  //
  double m0m1[3];
  // printf("1 %f %f %f \n",this->m(0),this->m(1),this->m(2));getchar();
  //printf("1 %f %f %f \n",this->v(0),this->v(1),this->v(2));getchar();
  m0m1[0]=this->m(0)-x;
  m0m1[1]=this->m(1)-y;
  m0m1[2]=this->m(2)-z;
  //printf("2");
  // produit direct
  double v0v1[3];
  v0v1[0]=m0m1[1]*this->v(2)-m0m1[2]*this->v(1);
  v0v1[1]=m0m1[2]*this->v(0)-m0m1[0]*this->v(2);
  v0v1[2]=m0m1[0]*this->v(1)-m0m1[1]*this->v(0);
  // printf("3");
  double norm_v0v1=sqrt(v0v1[0]*v0v1[0]+v0v1[1]*v0v1[1]+v0v1[2]*v0v1[2]);
  double norm_v1=sqrt(this->v(0)*this->v(0)+this->v(1)*this->v(1)+this->v(2)*this->v(2));
  // printf("4");
  return norm_v0v1/norm_v1;
  
}
void TrackInfo::regression()
{

  double z2=0,z=0,zy=0,y=0,zx=0,x=0,w=0;
  zmin_=9999.;zmax_=0;
  if (np_<2) return;
  planes_.reset();
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
      planes_.set(layer_[i]);
      
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
  // calculate closest approach for all points
  for  (uint32_t i=0;i<np_;i++)
    cla_[i]=closestApproach(x_[i],y_[i],z_[i]);
  return;


}
void TrackInfo::exclude_layer(uint32_t layer,TrackInfo& ti)
{
  ti.clear();
  for (uint32_t i=0;i<np_;i++)
    {
      if (layer_[i]==layer) continue;
      ti.add_point(x_[i],y_[i],z_[i],layer_[i]);
    }
  ti.regression();
}
void  TrackInfo::add_point(double x,double y,double z,uint32_t l)
{
  if (np_<2)
    {
      x_[np_]=x;y_[np_]=y;z_[np_]=z;layer_[np_]=l;np_++;
      return;
    }
  if (cla_[0]<0)
    this->regression();
  double d=closestApproach(x,y,z);

  for  (uint32_t i=0;i<np_;i++)
    {
      if (layer_[i]==l && d>=cla_[i]) return;
      if (layer_[i]==l && d<cla_[i])
	{
	  x_[i]=x;
	  y_[i]=y;
	  z_[i]=z;
	  return;
	}
    }
  x_[np_]=x;y_[np_]=y;z_[np_]=z;layer_[np_]=l;np_++;
}
