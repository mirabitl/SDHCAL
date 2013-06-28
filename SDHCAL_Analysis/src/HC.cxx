
#include <HC.h>
#include <stdio.h>
#include <algorithm>
HC::HC(uint32_t m,float th,float r) : m_(m),th_(th),r_(r)
{
	a_= -1./tan(th_);
	b_= r_/sin(th_)-50.*a_;
	points_.clear();
	for (int i=0;i<61;i++) dmin_[i]=1E9;
}
float HC::Pos(float z) {return a_*z+b_;}
void HC::setDmin(float d,uint32_t ch){if (d<dmin_[ch]) dmin_[ch]=d;}
void HC::add(std::vector<RecoPoint>::iterator i){points_.push_back(i);}
void HC::Dump(){printf("=====> %d %f %f %d  \n",m_,a_,b_,(int) points_.size());}
uint32_t HC::common(HC& other,std::vector<std::vector<RecoPoint>::iterator > &v)
{
	uint32_t nc=0;
	for (std::vector< std::vector<RecoPoint>::iterator >::iterator ip=other.points_.begin();ip!=other.points_.end();ip++)
	if (std::find(points_.begin(),points_.end(),(*ip))!=points_.end()) 
	{nc++; v.push_back(*ip);}
	return nc;
}




