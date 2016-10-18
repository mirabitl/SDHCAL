#ifndef _RCLUSTER_HH

#define _RCLUSTER_HH
#include <limits.h>

#include <stdint.h>
#include <vector>



#include "recoPoint.hh"
/**
 *   \class planeCluster
 *   \author L.Mirabito
 *   \date May 2010
 *   \version 1.0
 *   \brief Vector of RecoHit. The position is the mean of X and Y. No usage of threshold
 */
template <class T> 
class rCluster : public recoPoint
{
public:
  rCluster() :dx_(0),dy_(0) {;}
  rCluster(T* h)
  {hits_.clear();
    add(h);
  }
  ~rCluster()
  {hits_.clear();
  }
  double dist(T* h1,T* h2)
  {
    if (abs(h1->Z()-h2->Z())>0.5) return 1E12;
    double distx=abs(h1->X()-h2->X());
    double disty=abs(h1->Y()-h2->Y());
    //std::cout<<h1.X()<<" "<<h2.X()<<" "<<distx<<std::endl;
    if (distx>disty) 
      return distx;
    else
      return disty;
  }
  
  bool Append(T* h,double cut=2.0)
  {
    bool append=false;
    for (typename std::vector<T*>::iterator it= hits_.begin();it!=hits_.end();it++)
    {
      if (h->plan()!=(*it)->plan()) return false;
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
  std::vector<T*> *getHits() { return &hits_;}
  bool isAdjacent(T &c)
  {
    ROOT::Math::XYZVectorD d=(*this)-c;
    if (abs(d.X())<2 && abs(d.Y()<2)) return true;
    return false;
  }
  
  void Print()
  {
    std::cout<<(*hits_.begin())->plan()<<":"<<X()<<"/"<<Y()<<" "<<hits_.size()<<std::endl;
    for (typename std::vector<T*>::iterator it= hits_.begin();it!=hits_.end();it++)
    {
      std::cout<<"\t "<<" "<<(int) (*it)->X()<<" "<<(int) (*it)->Y()<<std::endl; 
    }
  }
  virtual double dX(){ return dx_;}
  virtual double dY(){ return dy_;}
  inline uint32_t chamber(){return hits_[0]->plan();} 
  virtual uint32_t plan(){return hits_[0]->plan();} 
  inline uint32_t size(){return hits_.size();}
  inline std::vector<T*>& hits(){return hits_;}
  
private:
  void add(T* h)
  {
    hits_.push_back(h);
    calcPos();
    h->setUse(true);
  }
  void calcPos()
  {
    int n=0;double x=0,x2=0,y=0,y2=0,x_=0,y_=0;
    double posError=0.3;
    for (typename std::vector<T*>::iterator it= hits_.begin();it!=hits_.end();it++)
    {
      //  std::cout<<"\t "<<(int) (*it)->X()<<" "<<(int) (*it)->Y()<<std::endl; 
      n++;
      x+=(*it)->X();
      x2+=((*it)->X()*(*it)->X());
      y+=(*it)->Y();
      y2+=((*it)->Y()*(*it)->Y());
      posError=(*it)->dX();
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
  std::vector<T*> hits_;
  double dx_,dy_;
};
#endif
