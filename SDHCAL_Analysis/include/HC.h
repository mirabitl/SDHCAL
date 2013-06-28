#ifndef _HOUGHCANDIDATE_H

#define _HOUGHCANDIDATE_H
#include <limits.h>

#include <stdint.h>

#include <vector>

#include <RecoPoint.h>
class HC
{
public:
	HC(uint32_t m,float th,float r); ;
	 float Pos(float z);
	 void setDmin(float d,uint32_t ch);
	 void add(std::vector<RecoPoint>::iterator i);
	 void Dump();
	 uint32_t common(HC& other,std::vector<std::vector<RecoPoint>::iterator > &v);
	
	uint32_t m_;
	float th_,r_,a_,b_,dmin_[61];

	std::vector<std::vector<RecoPoint>::iterator> points_;
};


#endif
