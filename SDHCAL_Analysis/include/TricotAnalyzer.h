#ifndef _TRICOTANALYZER_H

#define _TRICOTANALYZER_H
#include <limits.h>
#include <vector>
#include "DHCALAnalyzer.h"
#include "DHCalEventReader.h"
#include "DCHistogramHandler.h"
#include <iostream>
#include <sys/timeb.h>



#include "IMPL/LCTOOLS.h"
#include "EVENT/RawCalorimeterHit.h" 
#include "IMPL/LCGenericObjectImpl.h"
#include "IMPL/RawCalorimeterHitImpl.h"
#include "UTIL/CellIDDecoder.h"

#include "UtilDefs.h"
typedef  std::pair<DIFPtr*,uint32_t> ptrDifFrame,pdf;


class StripCluster {
public:
  StripCluster(){_strips.clear();_x=0;_asic=0;_used=false;}
  StripCluster(uint8_t asic,uint8_t i){_strips.clear();_strips.push_back(i);_x=i;_asic=asic; slope();_used=false;}
  bool append(uint8_t asic,uint8_t i)
  {
    if (asic!=_asic) return false;
    bool ap=false;
    for (uint8_t j=0;j<_strips.size();j++)
      if (abs(i-_strips[j])<2.1){ap=true;break;}
    if (!ap) return false;
    _strips.push_back(i);
    _x=0;for (uint8_t j=0;j<_strips.size();j++) _x+=_strips[j];_x/=_strips.size();
    slope();
    return ap;
  }
  void slope()
  {
    double s,c;
    switch (_asic)
      {
      case 1:
	{
	  _a=0;
	  _b=(_x-0.5)*30./64.;
	  break;
	}
      case 2:
	{
	  c=cos(2*M_PI/3.);
	  s=sin(2*M_PI/3.);
	  _a=-s/c;
	  _b=(_x-0.5)*30./64./c+30.;
	  break;
	}
      case 3:
	{
	  c=cos(M_PI/3.);
	  s=sin(M_PI/3.);
	  _a=-s/c;
	  _b=(65-_x+0.5)*30./64./c;
	  break;
	}
      }
  }	  
  std::vector<uint8_t>& strips(){return _strips;}
  uint8_t asic(){return _asic;}
  double x(){return _x;}
  double a(){return _a;}
  double b(){return _b;}
  void setUsed(bool t=true){_used=t;}
  bool isUsed(){return _used;}
private:
  std::vector<uint8_t> _strips;
  uint8_t _asic;
  double _x,_a,_b;
  bool _used;
};

class TricotCluster
{
  TricotCluster(StripCluster* s1,StripCluster* s2,StripCluster* s3) : _s1(s1),_s2(s2),_s3(s3),_valid(false)
  {
    if (_s1->isUsed()) return;
    if (_s2->isUsed()) return;
    if (_s3->isUsed()) return;
    if (_s1->asic()==_s2->asic()) return;
    if (_s1->asic()==_s3->asic()) return;
    if (_s3->asic()==_s2->asic()) return;
    float x1=(_s2->b()-_s1->b())/(_s1->a()-_s2->a());
    float y1=_s1->a()*x1+_s1->b();
    float x2=(_s3->b()-_s1->b())/(_s1->a()-_s3->a());
    float y2=_s1->a()*x2+_s1->b();
    float x3=(_s3->b()-_s2->b())/(_s2->a()-_s3->a());
    float y3=_s2->a()*x3+_s2->b();
    float _x=(x1+x2+x3)/3.;
    float _y=(y1+y2+y3)/3.;
    if (sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2))>2) return;
    if (sqrt((x1-x3)*(x1-x3)+(y1-y3)*(y1-y3))>2) return;
    if (sqrt((x3-x2)*(x3-x2)+(y3-y2)*(y3-y2))>2) return;
    _s1->setUsed();
    _s2->setUsed();
    _s3->setUsed();
    _valid=true;
  }
  TricotCluster(StripCluster* s1,StripCluster* s2) : _s1(s1),_s2(s2),_s3(NULL),_valid(false)
  {
    if (_s1->isUsed()) return;
    if (_s2->isUsed()) return;
    if (_s1->asic()==_s2->asic()) return;
    float x1=(_s2->b()-_s1->b())/(_s1->a()-_s2->a());
    float y1=_s1->a()*x1+_s1->b();
    float _x=x1;
    float _y=y1;
    _s1->setUsed();
    _s2->setUsed();
    _valid=true;
  }
  
private:
  StripCluster* _s1;
  StripCluster* _s2;
  StripCluster* _s3;
  double _x,_y;
  bool _valid;
};

class TricotAnalyzer : public DHCALAnalyzer
{
public:
  TricotAnalyzer();

  virtual ~TricotAnalyzer(){;}
  void initialise();
  virtual void processEvent();
  virtual void initHistograms(){;}
  virtual void processRunHeader()
  {
    if (writing_)
      reader_->writeRunHeader();
  }
  void presetParameters();
  void setWriting(bool t){writing_=t;}
  virtual void setReader(DHCalEventReader* r){reader_=r;rootHandler_=DCHistogramHandler::instance();}

  virtual void initJob();
  virtual void endJob();
  virtual void initRun(){;}
  virtual void endRun(){;}

  uint32_t buildPrincipal(std::string v);

  void buildFrameMap(std::vector<uint32_t> &seeds);

  void processSeeds();
private:


  int nAnalyzed_;

  DCHistogramHandler* rootHandler_;


  // Reader
  DHCalEventReader* reader_;

  bool writing_;
  bool draw_;

  IMPL::LCEventImpl* evt_;

  unsigned long long theBCID_;
  unsigned long long theBCIDSpill_;
  unsigned long long theAbsoluteTime_;


  uint32_t theMonitoringPeriod_;
  uint32_t theSkip_,theCount_[255][49],theTotalCount_[255][49];
  double theTotalTime_,theEventTotalTime_;
  std::string theMonitoringPath_;
	
  unsigned long long theStartBCID_;
  uint32_t _neff,_neff2,_neff3,_neff4,_nall;
  float _x[65532];
  float _y[65532];
  float _z[65532];
  uint32_t _layer[65532];
  uint32_t _npBuf;
  std::map<uint8_t,uint8_t> _plid,_pldif;
  std::map<uint8_t,float> _pldx;
  std::map<uint8_t,float> _pldy;
  std::map<uint8_t,float> _plz;

  std::map<uint32_t,std::vector<ptrDifFrame> > _FrameMap;
  
	
};
#endif
