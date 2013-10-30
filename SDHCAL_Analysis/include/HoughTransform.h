#ifndef _HOUGHTRANSFORM_H

#define _HOUGHTRANSFORM_H
#include <vector>
#include <bitset>
#include<stdint.h>
#include <DCHistogramHandler.h>
#define PI 3.141592653589793
class RecoHit;
class RecoPoint;
class HoughTransform
{
public:
  enum Type {RECOPOINT=1,RECOHIT=2};
  enum Projection {ZX=1,ZY=2};
  HoughTransform(double thmin,double thmax,double rmin,double rmax,uint32_t nbintheta=8,uint32_t nbinr=8);
  
  ~HoughTransform();
  void initialise(double thmin,double thmax,double rmin,double rmax,uint32_t nbintheta=8,uint32_t nbinr=8);
  void clear();
  double getTheta(int32_t i);
  double getR(int32_t i);
  uint16_t getValue(uint32_t i,uint32_t j);
  void addMeasurement(void* s,HoughTransform::Type type,HoughTransform::Projection proj);	

  void draw(DCHistogramHandler* h,std::vector< std::pair<uint32_t,uint32_t> > *maxval=NULL);
  void draw(DCHistogramHandler* h,std::vector< std::pair<double,double> > *maxval);
  uint32_t getVoteMax();
  static void Convert(double theta,double r,double &a,double &b);

  void addXPoint(RecoPoint* s);
  void addYPoint(RecoPoint* s);
  void addXHit(RecoHit* s);
  void addYHit(RecoHit* s);
  uint16_t isALocalMaximum(uint32_t i,uint32_t j,uint32_t count=0);
  void getPattern(int i,int j,uint32_t& nc,uint32_t& fp,uint32_t& lp);

  // Inline getters
  inline double getThetaBin(){return  theThetaBin_;}
  inline double getRBin(){return theRBin_;}
  inline double getThetaMin(){return  theThetaMin_;}
  inline double getRMin(){return theRMin_;}
  inline double getThetaMax(){return  theThetaMax_;}
  inline double getRMax(){return theRMax_;}
  inline uint32_t getNbinTheta(){ return theNbinTheta_;}
  inline uint32_t getNbinR(){ return theNbinR_;}
	
  inline std::vector<void*> getHoughMap(uint16_t i,uint16_t j) { return theHoughMap_[i][j];}
  inline uint16_t getHoughImage(uint16_t i,uint16_t j) { return theHoughImage_[i][j];}
  inline std::bitset<64> getHoughPlanes(uint16_t i,uint16_t j) { return theHoughPlanes_[i][j];}
private:
  double theSin_[1024];
  double theCos_[1024];
  std::vector<double> theX_;
  std::vector<double> theY_;
  uint16_t theHoughImage_[1024][1024];
  std::vector<void*> theHoughMap_[1024][1024];
  std::bitset<64> theHoughPlanes_[1024][1024];
  double theThetaMin_;
  double theThetaMax_;
  double theRMin_,theRMax_;
  double theThetaBin_,theRBin_;
  uint32_t theNbinTheta_;
  uint32_t theNbinR_;
  uint16_t theVoteMax_;
};



#endif
