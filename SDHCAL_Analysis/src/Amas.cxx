#include "UtilDefs.h"
#include "Amas.h"

#include "RecoHit.h"
#include "Shower.h"

Amas::Amas(RecoHit* h)
{
	theHits_.push_back(h);
}
Amas::Amas(std::vector<RecoHit*>& vh)
{
	theHits_.clear();
	this->copyFrom(vh);
	this->compute();
	//theHits_.push_back(h);
}
void Amas::add(RecoHit* h) {theHits_.push_back(h);}
bool Amas::append(RecoHit* h,uint32_t del)
{
	bool appended=false;
	for (std::vector<RecoHit*>::iterator it=theHits_.begin();it!=theHits_.end();it++)
	{
		uint32_t iDist=abs(2.8*abs((*it)->chamber()-h->chamber())+1*(abs((*it)->I()-h->I())+abs((*it)->J()-h->J())));
		//iht->chamber()-h.chamber())+2*(abs(h.I()-iht->I())+abs(h.J()-iht->J()));
		if (iDist>5) continue; // c'etait 7 avant
		//if (abs((*it)->chamber()-h->chamber())>del) continue;
		//if (abs((*it)->I()-h->I())>del) continue;
		//if (abs((*it)->J()-h->J())>del) continue;
		appended=true;
	}
	if (appended) theHits_.push_back(h);
	return appended;
}
void Amas::compute()
{
	Shower::computePrincipalComponents(theHits_,theComponents_);
	memset(nh,0,3*sizeof(uint32_t));
	for (std::vector<RecoHit*>::iterator it=theHits_.begin();it!=theHits_.end();it++)
	{
		int ithr= (*it)->getAmplitude()&0x3;
		if (ithr==1) nh[1]++;
		if (ithr==2) nh[0]++;
		if (ithr==3) nh[2]++;
	}
}
void Amas::Print()
{
  printf("-> %d %d %d : %f ==>",nh[0],nh[1],nh[2],getVolume());
  for (int i=0;i<21;i++) printf("%f ",theComponents_[i]);printf("\n");
}
double Amas::getVolume(){return 4/3*3.14159265359*sqrt(theComponents_[3]*theComponents_[4]*theComponents_[5])*10.1/2.8;}
double Amas::getComponents(uint32_t i) {return theComponents_[i];}
double* Amas::Components() {return &theComponents_[0];}
uint32_t* Amas::Hits(){return &nh[0];}
double Amas::X(){return theComponents_[0];}
uint32_t Amas::size(){return theHits_.size();}
std::vector<RecoHit*>& Amas::getHits(){return theHits_;}


bool Amas::operator> (const Amas& other) const
{
	return this->theHits_.size() > other.theHits_.size();
}
bool Amas::operator>=(const Amas& other) const
{
	return this->theHits_.size() >= other.theHits_.size();
}
bool Amas::operator< (const Amas& other) const
{
	return this->theHits_.size() < other.theHits_.size();
}
bool Amas::operator<=(const Amas& other) const
{
	return this->theHits_.size() <= other.theHits_.size();
}

void Amas::copyTo(std::vector<RecoHit*>& vh)
{
	for (std::vector<RecoHit*>::iterator it=theHits_.begin();it!=theHits_.end();it++)
	{
		vh.push_back((*it));
	}
}
void Amas::copyFrom(std::vector<RecoHit*>& vh)
{
	for (std::vector<RecoHit*>::iterator it=vh.begin();it!=vh.end();it++)
	{
		theHits_.push_back((*it));
	}
}


