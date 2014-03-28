#ifndef _GEOPOINT_H

#define _GEOPOINT_H
#include <limits.h>

#include <stdint.h>
#include <vector>






/**
\class RecoPoint
\author L.Mirabito
\date May 2010
\version 1.0
\brief Vector of RecoHit. The position is the mean of X and Y. No usage of threshold
*/
class GeoPoint 
{
public:
	GeoPoint(unsigned int ch,double x , double y, double z,double dx=0.5,double dy=0.5);
	void Print();
	 double X(){ return x_;}
	 double Y(){return y_;}
	double dX(){ return dx_;}
	double dY(){return dy_;}
	 double Z(){return z_;}
	unsigned int getChamberId(){ return chId_;}
	 int chamber(){return chId_;}
	void setUsed(bool t){inTrack_=t;}
	bool isUsed(){return inTrack_;}
	void setPointId(uint32_t i){ptid_=i;}
	uint32_t getPointId(){return ptid_;}
	//bool addNearby(GeoPoint* h,float distcut);
	void calculateComponents(std::vector<GeoPoint*> vnear);
	double* Components(){return components_;}
	//std::vector<GeoPoint*>& Voisins(){return vnear_;}
private:
	unsigned int chId_;

	double x_,y_,z_,x2_,y2_;
	double dx_,dy_;
	bool inTrack_;
	uint32_t ptid_;
	//std::vector<GeoPoint*> vnear_;
	double components_[21];
};

#endif
