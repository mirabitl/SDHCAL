#include "planeCluster.hh"
#include <Math/Vector3Dfwd.h>
#include <Math/DisplacementVector3D.h>
#include <TMath.h>
#define posError 0.5
planeCluster::planeCluster(RecoHit *h) : valid_(true),_used(false)
{
	hits_.clear();
	add(h);
	//  this->calcPos();
}
planeCluster::~planeCluster(){hits_.clear();}
void planeCluster::add(RecoHit* h)
{
  hits_.push_back(h);
  calcPos();
  h->setUse(true);
}
double planeCluster::dist(RecoHit* h1,RecoHit* h2)
{
	if (h1->chamber()!=h2->chamber()) return 1E12;
	ROOT::Math::XYZVector d=(*h1)-(*h2);
	double distx = TMath::Abs(d.X());
	double disty = TMath::Abs(d.Y());

	//std::cout<<h1.X()<<" "<<h2.X()<<" "<<distx<<std::endl;
	if (distx>disty) 
	return distx;
	else
	return disty;
}

bool planeCluster::isAdjacent(planeCluster &c)
{
  ROOT::Math::XYZVectorD d=(*this)-c;
  if (abs(d.X())<2 && abs(d.Y()<2)) return true;
  return false;
}
bool planeCluster::Append(RecoHit* h,double cut)
{	
	bool append=false;
	for (std::vector<RecoHit*>::iterator it= hits_.begin();it!=hits_.end();it++)
	{
	  if (h->chamber()!=(*it)->chamber()) return false;
	  if (dist(h,(*it))<cut) 
		{
			

			//  this->calcPos();
			append= true;
			break;
		}
	}
	if (append) add(h);
	return append;
}
std::vector<RecoHit*>* planeCluster::getHits(){ return &hits_;}
void planeCluster::Print()
{
  std::cout<<(*hits_.begin())->chamber()<<":"<<X()<<"/"<<Y()<<" "<<hits_.size()<<std::endl;
	for (std::vector<RecoHit*>::iterator it= hits_.begin();it!=hits_.end();it++)
	{
	  std::cout<<"\t "<<(int) (*it)->X()<<" "<<(int) (*it)->Y()<<std::endl; 
	}
}
void planeCluster::calcPos()
{
	// DEBUG_PRINT("On rentre dans calcpos %d \n",hits_.size());
  int n=0;double x=0,x2=0,y=0,y2=0,x_=0,y_=0;

	for (std::vector<RecoHit*>::iterator it= hits_.begin();it!=hits_.end();it++)
	{
		//  std::cout<<"\t "<<(int) (*it)->X()<<" "<<(int) (*it)->Y()<<std::endl; 
		n++;
		x+=(*it)->X();
		x2+=((*it)->X()*(*it)->X());
		y+=(*it)->Y();
		y2+=((*it)->Y()*(*it)->Y());
	}
	if (n>0) 
	{
		x_=x/n;
		y_=y/n;
		dx_=sqrt(x2/n-x_*x_+n*posError*posError);
		dy_=sqrt(y2/n-y_*y_+n*posError*posError);
		dx_=posError;
		dy_=posError;
		//      DEBUG_PRINT("%f %f %f %f \n",x_,dx_,y_,dy_);
	}
	else
	{
		x_=-10000.;
		y_=-10000.;
	}
	this->SetXYZ(x_,y_,hits_[0]->Z());

	
	return;

}



double planeCluster::dX(){return dx_;}
double planeCluster::dY(){return dy_;}



