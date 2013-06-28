#ifndef _RECOCLUSTER_H

#define _RECOCLUSTER_H
#include <limits.h>

#include <stdint.h>
#include <vector>




#include "RecoHit.h"
#include "ChamberGeom.h"

/**
\class RECOCluster
\author L.Mirabito
\date May 2010
\version 1.0
\brief Vector of RecoHit. The position is the mean of X and Y. No usage of threshold
*/
class RECOCluster
{
public:
	RECOCluster(){valid_=true;}
	RECOCluster(RecoHit h);
	~RECOCluster();
	double dist(RecoHit h1,RecoHit h2);
	bool Append(RecoHit h);
	std::vector<RecoHit>* getHits();
	bool isAdjacent(RECOCluster &c);
	void setValidity(bool t){valid_=t;}
	bool isValid() {return valid_;}
	double Pos(int p);
	void Print();
	double X();
	double Y();
	double dX();
	double dY();
private:
	void calcPos();
	std::vector<RecoHit> hits_;
	double x_,y_,dx_,dy_;
	bool valid_;
};
#endif
