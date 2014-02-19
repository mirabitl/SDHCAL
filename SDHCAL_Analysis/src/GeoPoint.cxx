#include "UtilDefs.h"
#include "GeoPoint.h"
#include "Shower.h"
#include <Eigen/Dense>
using namespace Eigen;

const double posError=0.5;

const double pad2cm=1.04125;


GeoPoint::GeoPoint(unsigned int ch,double x,double y,double z,double dx,double dy) : inTrack_(false)
{

	x_=x*pad2cm;
	y_=y*pad2cm;
	dx_=dx*pad2cm;
	dy_=dy*pad2cm;

	z_=z;
	chId_=ch;
	//	vnear_.reserve(100);
	//	vnear_.clear();
}

void GeoPoint::Print()
{
	printf("%d %f %f %f \n",chId_,x_,y_,z_);

}
/*
bool GeoPoint::addNearby(GeoPoint* h,float distcut)
{
  float x0=X(),y0=Y(),z0=Z(),x1=h->X(),y1=h->Y(),z1=h->Z();
  float dist=sqrt((x1-x0)*(x1-x0)+(y1-y0)*(y1-y0)+(z1-z0)*(z1-z0));
  if (dist<distcut) {vnear_.push_back(h);return true;}
  return false;
}
void GeoPoint::calculateComponents()
*/
  void GeoPoint::calculateComponents(std::vector<GeoPoint*> vnear_)
{
  memset(components_,0,21*sizeof(double));
  if (vnear_.size()<3) return;
  uint32_t nh=0;
  double xb=0,yb=0,zb=0;
  double wt=0.;
  
  double fp=DBL_MAX;
  double lp=-DBL_MAX;
  double fx=DBL_MAX;
  double lx=-DBL_MAX;
  double fy=DBL_MAX;
  double ly=-DBL_MAX;

  memset(components_,0,21*sizeof(double));
  //INFO_PRINT("%d vector size\n",v.size());
  for (std::vector<GeoPoint*>::iterator it=vnear_.begin();it!=vnear_.end();it++)
    {
      GeoPoint* iht=(*it);
      if (iht==NULL) continue;
      //INFO_PRINT("%x %d %d \n",iht,iht->I(),iht->J());
      //INFO_PRINT("%f %f \n",iht->x(),iht->y());
      //INFO_PRINT("%f %f \n",iht->X(),iht->Y());
      double w=1.;
      xb+=iht->X()*w;
      yb+=iht->Y()*w;
      zb+=iht->Z()*w;
      wt+=w;
      if (iht->Z()<fp) fp=iht->Z();
      if (iht->Z()>lp) lp=iht->Z();
      if (iht->X()<fx) fx=iht->X();
      if (iht->X()>lx) lx=iht->X();
      if (iht->Y()<fy) fy=iht->Y();
      if (iht->Y()>ly) ly=iht->Y();
      nh++;
    }
	
  if (nh<3) return;
  //INFO_PRINT("%d hits\n",nh);
  Matrix<double,Dynamic,3> m(nh,3);
  //DEBUG_PRINT("2\n");
  Matrix<double,3,Dynamic> mt(3,nh);
  Matrix<double,Dynamic,Dynamic> D(nh,nh);
  for (int i=0;i<nh;i++)
    for (int j=0;j<nh;j++)
      D(i,j)=0.;
  //DEBUG_PRINT("3 %d\n",nh);
  xb=xb/wt;
  yb=yb/wt;
  zb=zb/wt;
	// store barycenter
  components_[0]=xb;
  components_[1]=yb;
  components_[2]=zb;
  nh=0;
  wt=0.;
	//firstPlan_=99;

  for (std::vector<GeoPoint*>::iterator it=vnear_.begin();it!=vnear_.end();it++)
    {
      GeoPoint* iht=(*it);	  
      m(nh,0)=iht->X()-xb;
      mt(0,nh) =iht->X()-xb;
      m(nh,1)=iht->Y()-yb;
      mt(1,nh) =iht->Y()-yb;
      m(nh,2)=iht->Z()-zb;
      mt(2,nh) =iht->Z()-zb;
      double w=1.;
      w=1.; //test
      D(nh,nh)=w;
      wt+=w;
      nh++;
    }
	
  //INFO_PRINT("%d %f\n",nh,wt);
  D *=1./wt;
  //std::cout<<" Here it is "<<std::endl<<D<<std::endl;
  
  Matrix<double,Dynamic,Dynamic> V(nh,nh);
  
  V=mt*D*m;
  
  
  SelfAdjointEigenSolver< Matrix<double,Dynamic,Dynamic> > eigensolver(V);
  if (eigensolver.info() != Success) abort();
  
  
  //store eigen values
  Matrix<double,3,1> va=eigensolver.eigenvalues();
  components_[3]=va(0);
  components_[4]=va(1);
  components_[5]=va(2);
  
  // store principal axis
  Matrix<double,3,3> vv=eigensolver.eigenvectors();
  components_[6]=vv(0,0)*sqrt(components_[3]);
  components_[7]=vv(1,0)*sqrt(components_[3]);
  components_[8]=vv(2,0)*sqrt(components_[3]);
  components_[9]=vv(0,1)*sqrt(components_[4]);
  components_[10]=vv(1,1)*sqrt(components_[4]);
  components_[11]=vv(2,1)*sqrt(components_[4]);
  components_[12]=vv(0,2)*sqrt(components_[5]);
  components_[13]=vv(1,2)*sqrt(components_[5]);
  components_[14]=vv(2,2)*sqrt(components_[5]);
  
  // Store First and last Z
  components_[15]=fp;
  components_[16]=lp;
  components_[17]=fx;
  components_[18]=lx;
  components_[19]=fy;
  components_[20]=ly;

}
