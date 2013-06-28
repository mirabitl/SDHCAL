#include "RecoDIF.h"
#include <stdlib.h>
RecoCluster::RecoCluster(RecoHitAddress h)
{
  hits_.clear();
  hits_.push_back(h);
}
RecoCluster::~RecoCluster(){hits_.clear();}
unsigned int RecoCluster::dist(RecoHitAddress h1,RecoHitAddress h2)
  {
    unsigned int distx = abs(h1.first-h2.first);
    unsigned int disty = abs(h1.second-h2.second);
    if (distx>disty) 
      return distx;
    else
      return disty;
  }
bool RecoCluster::Append(RecoHitAddress h)
  {						
    for (std::vector<RecoHitAddress>::iterator it= hits_.begin();it!=hits_.end();it++)
      {
	if (dist(h,*it)<2) 
	  {
	    hits_.push_back(h);
	    return true;
	  }
      }
    return false;
  }
std::vector<RecoHitAddress>* RecoCluster::getHits(){ return &hits_;}
void RecoCluster::Print()
  {
    std::cout<<X()<<"/"<<Y()<<std::endl;
    for (std::vector<RecoHitAddress>::iterator it= hits_.begin();it!=hits_.end();it++)
      {
	std::cout<<"\t "<<(int) it->first<<" "<<(int) it->second<<std::endl; 
      }
  }
float RecoCluster::Pos(int p)
  {
    int n=0;float x=0;
    for (std::vector<RecoHitAddress>::iterator it= hits_.begin();it!=hits_.end();it++)
      {
	//	std::cout<<"\t "<<(int) it->first<<" "<<(int) it->second<<std::endl; 
	n++;
	if (p==0) 
	  x+=it->first;
	else
	  x+=it->second;
      }
    if (n>0) 
      return x/n;
    else
      return -1.;
  }
float RecoCluster::X(){return Pos(0);}
float RecoCluster::Y(){return Pos(1);}


RecoDIF::RecoDIF(unsigned int id) : difid_(id) { clusters_.clear();}
RecoDIF::~RecoDIF() {clusters_.clear();}
void RecoDIF::AddHit(RecoHitAddress h)
{
  for (std::vector<RecoCluster>::iterator it= clusters_.begin();it!=clusters_.end();it++)
    {
      if (it->Append(h)) return;
    } 
  RecoCluster a(h);
  clusters_.push_back(a);
}
std::vector<RecoCluster>* RecoDIF::getClusters(){ return &clusters_;}
unsigned int RecoDIF::getDifId(){return difid_;}
void RecoDIF::Print()
{
  std::cout<<"DIF Reco "<<difid_<<std::endl;
  for (std::vector<RecoCluster>::iterator it= clusters_.begin();it!=clusters_.end();it++)
    {
      std::cout<<"New cluster "<<std::endl;
      it->Print();
    }
}
