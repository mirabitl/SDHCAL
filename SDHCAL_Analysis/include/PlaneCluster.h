#ifndef _PlaneCLUSTER_H

#define _PlaneCLUSTER_H
#include <limits.h>

#include <stdint.h>
#include <vector>




#include "RecoHit.h"
#include "ChamberGeom.h"

/**
\class PlaneCluster
\author L.Mirabito
\date May 2010
\version 1.0
\brief Vector of RecoHit. The position is the mean of X and Y. No usage of threshold
*/
class PlaneCluster
{
public:
	PlaneCluster(){valid_=true;}
	PlaneCluster(RecoHit* h);
	~PlaneCluster();
	double dist(RecoHit* h1,RecoHit* h2);
	bool Append(RecoHit* h,double cut=2.0);
	std::vector<RecoHit*> *getHits();
	bool isAdjacent(PlaneCluster &c);
	void setValidity(bool t){valid_=t;}
	bool isValid() {return valid_;}
	double Pos(int p);
	void Print();
	double X();
	double Y();
	double dX();
	double dY();
	inline double Z(){return hits_[0]->Z();}
	inline uint32_t chamber(){return hits_[0]->chamber();} 
	inline uint32_t plan(){return hits_[0]->plan();} 
	inline uint32_t size(){return hits_.size();}
private:
	void calcPos();
	std::vector<RecoHit*> hits_;
	double x_,y_,dx_,dy_;
	bool valid_;
};
#endif
