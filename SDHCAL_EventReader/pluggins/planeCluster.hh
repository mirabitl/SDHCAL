#ifndef _PlaneCLUSTER_HH

#define _PlaneCLUSTER_HH
#include <limits.h>

#include <stdint.h>
#include <vector>




#include "RecoHit.hh"
#include "recoPoint.hh"
/**
   \class planeCluster
   \author L.Mirabito
   \date May 2010
   \version 1.0
   \brief Vector of RecoHit. The position is the mean of X and Y. No usage of threshold
*/
class planeCluster : public recoPoint
{
public:
  planeCluster(){;}
  planeCluster(RecoHit* h);
  ~planeCluster();
  double dist(RecoHit* h1,RecoHit* h2);
  bool Append(RecoHit* h,double cut=2.0);
  std::vector<RecoHit*> *getHits();
  bool isAdjacent(planeCluster &c);
  
  void Print();
  virtual double dX();
  virtual double dY();
  inline uint32_t chamber(){return hits_[0]->chamber();} 
  virtual uint32_t plan(){return hits_[0]->plan();} 
  inline uint32_t size(){return hits_.size();}
  inline std::vector<RecoHit*>& hits(){return hits_;}
  
private:
  void add(RecoHit*);
  void calcPos();
  std::vector<RecoHit*> hits_;
  double dx_,dy_;
};
#endif
