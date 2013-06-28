#include "UtilDefs.h"
#include "RECOCluster.h"
#define posError 0.5
RECOCluster::RECOCluster(RecoHit h) : valid_(true)
{
	hits_.clear();
	hits_.push_back(h);
	//  this->calcPos();
}
RECOCluster::~RECOCluster(){hits_.clear();}
double RECOCluster::dist(RecoHit h1,RecoHit h2)
{
	if (h1.chamber()!=h2.chamber()) return 1E12;
	double distx = abs(h1.I()-h2.I());
	double disty = abs(h1.J()-h2.J());

	//std::cout<<h1.X()<<" "<<h2.X()<<" "<<distx<<std::endl;
	if (distx>disty) 
	return distx;
	else
	return disty;
}

bool RECOCluster::isAdjacent(RECOCluster &c)
{
	if (hits_.begin()->chamber()!=c.getHits()->begin()->chamber()) return false;
	for (std::vector<RecoHit>::iterator it= hits_.begin();it!=hits_.end();it++)
	for (std::vector<RecoHit>::iterator jt= c.getHits()->begin();jt!=c.getHits()->end();jt++)
	if (dist((*it),(*jt))<2) return true;
	return false;
}
bool RECOCluster::Append(RecoHit h)
{	
	bool append=false;
	for (std::vector<RecoHit>::iterator it= hits_.begin();it!=hits_.end();it++)
	{
		if (h.chamber()!=it->chamber()) return false;
		if (dist(h,*it)<2) 
		{
			

			//  this->calcPos();
			append= true;
			break;
		}
	}
	if (append) hits_.push_back(h);
	return append;
}
std::vector<RecoHit>* RECOCluster::getHits(){ return &hits_;}
void RECOCluster::Print()
{
	std::cout<<hits_.begin()->chamber()<<":"<<X()<<"/"<<Y()<<" "<<hits_.size()<<std::endl;
	for (std::vector<RecoHit>::iterator it= hits_.begin();it!=hits_.end();it++)
	{
		std::cout<<"\t "<<(int) it->X()<<" "<<(int) it->Y()<<std::endl; 
	}
}
double RECOCluster::Pos(int p)
{
	int n=0;double x=0;
	for (std::vector<RecoHit>::iterator it= hits_.begin();it!=hits_.end();it++)
	{
		//	std::cout<<"\t "<<(int) it->X()<<" "<<(int) it->Y()<<std::endl; 
		n++;
		if (p==0) 
		x+=it->X();
		else
		x+=it->Y();
	}
	if (n>0) 
	return x/n;
	else
	return -100000.;
}
void RECOCluster::calcPos()
{
	// DEBUG_PRINT("On rentre dans calcpos %d \n",hits_.size());
	int n=0;double x=0,x2=0,y=0,y2=0;
	for (std::vector<RecoHit>::iterator it= hits_.begin();it!=hits_.end();it++)
	{
		//  std::cout<<"\t "<<(int) it->X()<<" "<<(int) it->Y()<<std::endl; 
		n++;
		x+=it->X();
		x2+=(it->X()*it->X());
		y+=it->Y();
		y2+=(it->Y()*it->Y());
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
		x=-10000.;
		y=-10000.;
	}
	return;

}


double RECOCluster::X(){calcPos();return x_;}
double RECOCluster::Y(){calcPos();return y_;}
double RECOCluster::dX(){return dx_;}
double RECOCluster::dY(){return dy_;}



