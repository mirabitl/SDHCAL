#ifndef _AMAS_H

#define _AMAS_H
#include <limits.h>
#include <vector>
#include <stdint.h>
#include "RecoHit.h"

class Amas
{

public:
	Amas(RecoHit* h);
	
	Amas(std::vector<RecoHit*>& vh);
	
	void add(RecoHit* h);
	bool append(RecoHit* h,uint32_t del=1);
	void compute();
	
	double getComponents(uint32_t i); 
	double* Components();
	uint32_t* Hits();
	double X();
	uint32_t size();
	std::vector<RecoHit*> &getHits();


	bool operator> (const Amas& other) const;
	
	bool operator>=(const Amas& other) const;
	
	bool operator< (const Amas& other) const;
	
	bool operator<=(const Amas& other) const;
	

	void copyTo(std::vector<RecoHit*>& vh);
	
	void copyFrom(std::vector<RecoHit*>& vh);
	
private:
	std::vector<RecoHit*> theHits_;
	double theComponents_[21];
	uint32_t nh[3];
};
#endif
