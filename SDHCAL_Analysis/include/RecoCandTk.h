#ifndef _RECOCANDTK_H

#define _RECOCANDTK_H
#include <limits.h>
#include <stdint.h>
#include <vector>

#include "RecoPoint.h"
#include <bitset> 
class RecoCandTk
{
public:
	RecoCandTk();
	~RecoCandTk();
	void clear();
	void addPoint(RecoPoint& p);
	bool addNearestPoint(RecoPoint& p);
	bool addPoint(RecoPoint& p,double xcut, double ycut);
	bool addPoints(std::vector<RecoPoint> v, double zref,double xcut, double ycut);
	bool addPoints(std::vector<RecoPoint> v, double dcut);
	bool addChi2Points(std::vector<RecoPoint> v, double dcut,std::vector<std::vector<RecoPoint>::iterator>* used=NULL);
	void removeDistantPoint(float zcut);

	void Refit(RecoCandTk &t,float c);
	void regression();
	void regression1D(std::vector<double> vx,std::vector<double> weight,std::vector<double> y,double &chi2, double &alpha,double &beta);
	void calculateChi2();
	void clean();
	void Print();
	std::vector<RecoPoint*>& getList(){return list_;}
	std::vector<double>& getChi2(){return dist_;}
	double getXext(double z) { return ax_*z+bx_;}
	double getYext(double z) { return ay_*z+by_;}  
	double calculateDistance(RecoPoint& p);
	double ax_,bx_,ay_,by_,chi2_;
	double prChi2_;
	uint32_t firstChamber_,lastChamber_,np_;
	inline bool isValid(){return valid_;}
	inline void setValid(bool t){valid_=t;}
	float zmin_,zmax_;
	std::bitset<128> planes_;
private:
	bool valid_;
	std::vector<RecoPoint*> list_;
	std::vector<double> dist_;
	float dmin_[61];

};
#endif
