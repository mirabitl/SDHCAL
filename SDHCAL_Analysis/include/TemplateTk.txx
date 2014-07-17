#include "UtilDefs.h"
//#include "TemplateTk.h"
#include "TMath.h"
template <class A>
TemplateTk<A>::TemplateTk() : ax_(0),bx_(0),ay_(0),by_(0),valid_(true),zmin_(1500),zmax_(-1500.)
{
	list_.clear();
	for (uint32_t i=0;i<=61;i++) dmin_[i]=1E9;
}
template <class A>
TemplateTk<A>::~TemplateTk() {list_.clear();}
template <class A>
void TemplateTk<A>::clear(){list_.clear();}
template <class A>
void TemplateTk<A>::removeDistantPoint(float zcut)
{
	for (typename std::vector< A* >::iterator ipt=list_.begin();ipt!=list_.end();)
	{
		float zdist=60.;
		for (typename std::vector< A* >::iterator jpt=list_.begin();jpt!=list_.end();jpt++)
		{
			if (ipt==jpt) continue;
			if (TMath::Abs((*ipt)->getChamberId()*1.-(*jpt)->getChamberId())<zdist) zdist=TMath::Abs((*ipt)->getChamberId()*1.-(*jpt)->getChamberId());
		}
		if (zdist>zcut)
		list_.erase(ipt);
		else
		++ipt;
	}
}


#ifdef OLDWAY
template <class A>
bool TemplateTk<A>::addPoints(std::vector<A> v, double dcut)
{
	std::vector<double> zpos;
	// Find all Z plane
	for (unsigned int i=0;i<v.size();i++)
	{
		std::vector<double>::iterator it = std::find(zpos.begin(),zpos.end(),v[i].Z());
		if (it == zpos.end()) zpos.push_back(v[i].Z());
	}
	// Remove existing plane
	for (unsigned int i=0;i<list_.size();i++)
	{
		std::vector<double>::iterator it = std::find(zpos.begin(),zpos.end(),list_[i]->Z());
		if (it != zpos.end()) 
		zpos.erase(it++);
	}
	for (unsigned int i=0;i<zpos.size();i++)
	{
		addPoints(v,zpos[i],dcut,dcut);
	}    
	return true;
}
#else
template <class A>
bool TemplateTk<A>::addPoints(std::vector<A> &v, double dcut)
{
	std::vector<double> zpos;
	// Loop on point and 
	for (unsigned int i=0;i<v.size();i++)
	{
		
		uint32_t ch=v[i].getChamberId();
		float zch = v[i].Z();
		bool drop=false;
		// Check not in chamber list already
		for (uint32_t j=0;j<list_.size();j++)
		if (list_[j]->getChamberId()==ch) {drop=true;break;}
		if (drop) continue;
		//calculate extrapolation
		float xext= ax_*zch+bx_;
		float yext= ay_*zch+by_;
		// check dist cut
		float dist=sqrt((v[i].X()-xext)*(v[i].X()-xext)+(v[i].Y()-yext)*(v[i].Y()-yext));
		if (dist>dcut) continue;
		for (uint32_t j=0;j<v.size();j++)
		{
			if (j==i) continue;
			if (v[j].getChamberId()!=ch) continue;
			float dist1 = sqrt((v[j].X()-xext)*(v[j].X()-xext)+(v[j].Y()-yext)*(v[j].Y()-yext));
			if (dist1<dist) {drop=true;break;} 
		}
		if (drop) continue;
		list_.push_back(&v[i]);
		dist_.push_back(0);
		regression();
	}

	return true;
}
#endif
template <class A> 
bool TemplateTk<A>::addChi2Points(std::vector<A> &v, double dcut, std::vector< typename std::vector<A>::iterator>* used)
{
	std::vector<double> zpos;
	// Loop on point and 
	for (typename std::vector<A>::iterator iv=v.begin();iv!=v.end();iv++)
	{
		
		uint32_t ch=iv->getChamberId();
		float zch = iv->Z();
		bool drop=false;
		// Check not in chamber list already
		for (uint32_t j=0;j<list_.size();j++)
		if (list_[j]->getChamberId()==ch) {drop=true;break;}
		if (drop) continue;
		//calculate extrapolation
		double xext= ax_*zch+bx_;
		double yext= ay_*zch+by_;
		double wx= 1./iv->dX()/iv->dX();
		double wy= 1./iv->dY()/iv->dY();

		double chi2=wx*(iv->X()-xext)*(iv->X()-xext);
		chi2+=wy*(iv->Y()-yext)*(iv->Y()-yext);
		if (chi2>dcut) continue;
		if (used!=NULL) used->push_back(iv);
		list_.push_back(&(*iv));
		dist_.push_back(0);
		regression();
	}

	return true;
}
template <class A>
double TemplateTk<A>::calculateDistance(A& p)
{
	double xext = ax_*p.Z()+bx_;
	double yext = ay_*p.Z()+by_;
	double dist1 = sqrt((p.X()-xext)*(p.X()-xext)+(p.Y()-yext)*(p.Y()-yext));
	return dist1;
}
template <class A>
bool TemplateTk<A>::addPoints(std::vector<A> &v, double zref,double xcut, double ycut)
{
	double distmin=9999.; unsigned int imin=999999;
	double xext = ax_*zref+bx_;
	double yext = ay_*zref+by_;
	for (unsigned int j=0;j<v.size();j++)
	{
		if (fabs(v[j].Z()-zref)<0.1)
		{
			double dist1 = sqrt((v[j].X()-xext)*(v[j].X()-xext)+(v[j].Y()-yext)*(v[j].Y()-yext));
			if (dist1<=distmin) 
			{
				distmin=dist1;
				imin=j;
			}
		}
	}
	if (imin>v.size()) return false;
	if (distmin>xcut) return false;
	//  if (fabs(v[imin].X()-xext)> xcut) return false;
	// if (fabs(v[imin].Y()-yext)> ycut) return false;
	list_.push_back(&v[imin]);
	dist_.push_back(0);
	if (v[imin].Z()<zmin_) zmin_=v[imin].Z();
	if (v[imin].Z()>zmax_) zmax_=v[imin].Z();
	regression();
	return true; 

}  
template <class A>
bool TemplateTk<A>::addNearestPoint( A& p)
{
	//  std::cout<<__PRETTY_FUNCTION__<<p.X()<<" "<<p.Y()<<" "<<p.Z()<<std::endl;
	float d=this->calculateDistance(p);
	if (d<dmin_[p.getChamberId()])
	{
		if (p.Z()<zmin_) zmin_=p.Z();
		if (p.Z()>zmax_) zmax_=p.Z();
		list_.push_back(&p);
		dist_.push_back(d);
		dmin_[p.getChamberId()]=d;
		return true;
	}
	else
	return false;
} 
template <class A>
void TemplateTk<A>::addPoint( A& p)
{
  //std::cout<<__PRETTY_FUNCTION__<<p.X()<<" "<<p.Y()<<" "<<p.Z()<<" min "<<zmin_<<" max "<<zmax_<<std::endl;
	if (p.Z()<zmin_) zmin_=p.Z();
	if (p.Z()>zmax_) zmax_=p.Z();
	list_.push_back(&p);
	dist_.push_back(0);
} 
template <class A>
bool TemplateTk<A>::addPoint(A& p,double xcut, double ycut)
{
	double z = p.Z();
	double xext = ax_*z+bx_;
	double yext = ay_*z+by_;
	double dist = sqrt((p.X()-xext)*(p.X()-xext)+(p.Y()-yext)*(p.Y()-yext));

	for (unsigned int j=0;j<list_.size();j++)
	{
		if (fabs(list_[j]->Z()-p.Z())<0.1)
		{
			double dist1 = sqrt((list_[j]->X()-xext)*(list_[j]->X()-xext)+(list_[j]->Y()-yext)*(list_[j]->Y()-yext));
			if (dist1<=dist) 
			return false;
			else
			{
				list_[j]=&p;
				regression();
				return true;
			}
		}
	}

	if (fabs(p.X()-xext)> xcut) return false;
	if (fabs(p.Y()-yext)> ycut) return false;
	list_.push_back(&p);
	dist_.push_back(0);
	if (p.Z()<zmin_) zmin_=p.Z();
	if (p.Z()>zmax_) zmax_=p.Z();
	regression();
	return true;
} 
template <class A>
void TemplateTk<A>::Print()
{
  INFO_PRINT("%d hits X (%f,%f)  Y (%f,%f) Chi2 %f \n",list_.size(),ax_,bx_,ay_,by_,chi2_);
	// for (unsigned int i=0;i<list_.size();i++) list_[i]->Print();

}
template <class A>
void TemplateTk<A>::regression1D(std::vector<double> &vx,std::vector<double> &weight,std::vector<double> &vy,double &chi2, double &alpha,double &beta)
{
	double x2=0,x=0,xy=0,y=0,w=0;
	if (vx.size()<2) return;
	for (uint32_t i=0;i<vx.size();i++)
	{
	  weight[i]=1.;
		x+=vx[i]*weight[i];
		x2+=vx[i]*vx[i]*weight[i];
		xy+=vx[i]*vy[i]*weight[i];
		w+=weight[i];
		y+=vy[i]*weight[i];
		
	}

	//std::cout<<x<<" "<<x2<<" "<<xy<<" "<<y<<" "<<w<<" "<<vx.size()<<std::endl;
	double a=x2,b=x,c=x,d=w;
	double det=(a*d-b*c);
	//std::cout<<"Det ="<<det<<std::endl;
	double m11= d/det;
	double m12=-b/det;
	double m21=-c/det;
	double m22=a/det;
	alpha=m11*xy+m12*y;
	beta=m21*xy+m22*y;
	// std::cout<<alpha<<" "<<beta<<std::endl;
	chi2=0;
	for  (uint32_t i=0;i<vx.size();i++)
	{
		chi2+=weight[i]*(vy[i]-alpha*vx[i]-beta)*(vy[i]-alpha*vx[i]-beta);
	}
	return;
}
template <class A>
void TemplateTk<A>::regression()
{
	unsigned int n = list_.size();
	if (n<2) return;
	double zbar=0;
	double xbar=0;
	double ybar=0;
	double z2bar =0;
	double zxbar=0;
	double zybar=0;
	firstChamber_=1000;
	lastChamber_=-1000;
	double wxt=0;
	double wyt=0;
	std::vector<double> vx;
	std::vector<double> vy;
	std::vector<double> vz;
	std::vector<double> wgx;
	std::vector<double> wgy;
	vx.clear();
	vy.clear();
	vz.clear();
	wgx.clear();
	wgy.clear();
	for (unsigned int i=0;i<n;i++)
	{
		//      std::cout<<i<<"==>"<<list_[i]->X()<<" "<<list_[i]->Y()<<" "<<list_[i]->Z()<<std::endl;
		uint32_t ch=list_[i]->getChamberId();
		if (ch<firstChamber_) firstChamber_=ch;
		if (ch>lastChamber_) lastChamber_=ch;
#define Use_Error
#ifdef Use_Error
		wgx.push_back(1./list_[i]->dX()/list_[i]->dX());
		wgy.push_back(1./list_[i]->dY()/list_[i]->dY());
#else
		wgx.push_back(list_[i]->Charge()*list_[i]->Charge()/list_[i]->dX()/list_[i]->dX());
		wgy.push_back(list_[i]->Charge()*list_[i]->Charge()/list_[i]->dY()/list_[i]->dY());

#endif
		vx.push_back(list_[i]->X());
		vy.push_back(list_[i]->Y());
		vz.push_back(list_[i]->Z());

		zbar+=list_[i]->Z();
		z2bar+=list_[i]->Z()*list_[i]->Z();
		zxbar+=list_[i]->Z()*list_[i]->X();
		zybar+=list_[i]->Z()*list_[i]->Y();
		xbar+=list_[i]->X();
		ybar+=list_[i]->Y();
	}
	zbar /=n;
	z2bar /=n;
	zxbar /=n;
	zybar /=n;
	ybar /=n;
	xbar /=n;
	double s2z = z2bar-zbar*zbar;
	double szx = zxbar-zbar*xbar;
	double szy = zybar-zbar*ybar;
	ax_ = szx/s2z;bx_=xbar -ax_*zbar;
	ay_ = szy/s2z;by_=ybar -ay_*zbar;

	calculateChi2();
	//this->Print();
	double chi2x_=0,chi2y_=0;
	regression1D(vz,wgx,vx,chi2x_,ax_,bx_);
	//DEBUG_PRINT("Fit en X %f %f %f \n",chi2x_,ax_,bx_);
	regression1D(vz,wgy,vy,chi2y_,ay_,by_);
	// DEBUG_PRINT("Fit en Y %f %f %f \n",chi2y_,ay_,by_);
	
	chi2_=chi2x_+chi2y_;
	//this->Print();
	//getchar();

	



	return;
}
template <class A>
void TemplateTk<A>::Refit(TemplateTk &t,float cut)
{
	unsigned int n = list_.size();


	for (unsigned int i=0;i<n;i++)
	{
		if (dist_[i]>cut) 
		{
			//DEBUG_PRINT("%f %f removed \n",dist_[i],cut);
			continue;
		}
		t.addPoint((*list_[i]));
		
	}
	t.regression();

}
template <class A>
void TemplateTk<A>::calculateChi2()
{
	unsigned int n = list_.size();

	chi2_=0;
	for (unsigned int i=0;i<n;i++)
	{
		double dx = ax_*list_[i]->Z()+bx_ -list_[i]->X();
		double dy = ay_*list_[i]->Z()+by_ -list_[i]->Y();

		//dist_[i]=(dx*dx/list_[i]->dX()/list_[i]->dX() +dy*dy/list_[i]->dY()/list_[i]->dY());
		dist_[i]=(dx*dx+dy*dy)/(list_[i]->dX()*list_[i]->dX()+list_[i]->dY()*list_[i]->dY());
		chi2_ += dist_[i];
		// std::cout <<i<<" "<<dist_[i]<<std::endl;
	}

	prChi2_=TMath::Prob(chi2_,2*n-4);
}


template <class A>
void TemplateTk<A>::tagPoints()
{
	//  unsigned int n = list_.size();


	for (typename std::vector<A*>::iterator it=list_.begin();it!=list_.end();it++)
	{
	  
	  (*it)->setUsed(true);
		
	}

}

template <class A>
void TemplateTk<A>::clean()
{
	//  unsigned int n = list_.size();

	std::vector<double>::iterator ic=dist_.begin();
	for (typename std::vector<A*>::iterator it=list_.begin();it!=list_.end();)
	{
		
		if ((*ic)>10.) {
			//DEBUG_PRINT("%f %f %f %f \n",it->X(),it->Y(),it->Z(),(*ic));
			list_.erase(it++);
			dist_.erase(ic++);
		}
		else
		{
			it++;
			ic++;
			
		}
		
	}
	regression();
}
