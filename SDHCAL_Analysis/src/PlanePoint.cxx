#include "UtilDefs.h"
#include "PlanePoint.h"

const double posError=0.5;

const double pad2cm=1.04125;


PlanePoint::PlanePoint(PlaneCluster& h,unsigned int ch,double x,double y,double z,double dx,double dy) : inTrack_(false)
{
	h_=h;
	x_=x*pad2cm;
	y_=y*pad2cm;
	dx_=dx*pad2cm;
	dy_=dy*pad2cm;
	calcPos();
	z_=z;
	chId_=ch;
}
void PlanePoint::calcPos()
{
	int n=0;double x=0,x2=0,y=0,y2=0; 
	weight_=0,weight2_=0;
	for (std::vector<RecoHit*>::iterator it= h_.getHits()->begin();it!=h_.getHits()->end();it++)
	{
		//std::cout<<"\t "<<(int) it->X()<<" "<<(int) it->Y()<<std::endl; 
		n++;
		double w=0;
		int ithr= (*it)->getAmplitude()&0x3;
		if (ithr==1) w=3;
		if (ithr==2) w=1;
		if (ithr==3) w=15;
		x+=(*it)->X()*w;
		x2+=((*it)->X()*(*it)->X())*w;
		y+=(*it)->Y()*w;
		y2+=((*it)->Y()*(*it)->Y())*w;
		weight_+=w;
		weight2_+=(w*w);
	}
	if (n>0) 
	{
		x_=x/weight_;
		y_=y/weight_;
		x2_=x2/weight_;
		y2_=y2/weight_;
		//dx_=sqrt(x2/n-x_*x_+n*posError*posError);
		//dy_=sqrt(y2/n-y_*y_+n*posError*posError);
		dx_ = posError*posError/n + x2_-x_*x_;
		dx_=sqrt(dx_);
		//dx_=posError;
		dy_=  posError*posError/n + y2_-y_*y_;
		dy_=sqrt(dy_);
		//	    if (n>4)
		//  DEBUG_PRINT(" Cluster Pos %f %f %f %f \n",x_,dx_,y_,dy_);
	}
	return;

}

void PlanePoint::Print()
{
	printf("%d %f %f %f \n",chId_,x_,y_,z_);
	h_.Print();
}
