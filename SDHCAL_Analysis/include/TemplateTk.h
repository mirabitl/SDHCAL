#ifndef _TEMPLATETK_H

#define _TEMPLATETK_H
#include <limits.h>
#include <stdint.h>
#include <vector>

//#include "RecoPoint.h"
class A;
template <class A>
class TemplateTk
{
public:
  TemplateTk();
  ~TemplateTk();
  void clear();
  void addPoint(A& p);
  bool addNearestPoint(A& p);
  bool addPoint(A& p,double xcut, double ycut);
  bool addPoints(std::vector<A> &v, double zref,double xcut, double ycut);
  bool addPoints(std::vector<A> &v, double dcut);
  bool addChi2Points(std::vector<A> &v, double dcut,
		      std::vector< typename std::vector<A>::iterator>* used=NULL);
  void removeDistantPoint(float zcut);

  void Refit(TemplateTk &t,float c);
  void regression();
  void regression1D(std::vector<double> &vx,std::vector<double> &weight,std::vector<double> &y,double &chi2, double &alpha,double &beta);
  void calculateChi2();
  void clean();
  void Print();
  std::vector<A*>& getList(){return list_;}
  uint32_t getNumberOfHits(){return list_.size();}
  std::vector<double>& getChi2(){return dist_;}
  double getXext(double z) { return ax_*z+bx_;}
  double getYext(double z) { return ay_*z+by_;}  
  double calculateDistance(A& p);
  double ax_,bx_,ay_,by_,chi2_;
  double prChi2_;
  int32_t firstChamber_,lastChamber_;
  inline bool isValid(){return valid_;}
  inline void setValid(bool t){valid_=t;}
  float zmin_,zmax_;
private:
  bool valid_;
  std::vector<A*> list_;
  std::vector<double> dist_;
  float dmin_[61];

};
#include "TemplateTk.txx"
#endif
