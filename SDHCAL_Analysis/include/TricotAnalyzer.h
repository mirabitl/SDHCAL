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
#include "TrackInfo.h"
#include "UtilDefs.h"
typedef  std::pair<DIFPtr*,uint32_t> ptrDifFrame,pdf;


class TStripCluster {
 private:
  std::vector<uint8_t> _strips;
  uint8_t _asic;
  double _x,_a,_b;
  bool _used;

public:
  TStripCluster(){_strips.clear();_x=0;_asic=0;_used=false;}
  TStripCluster(const TStripCluster& obj)
  {

    for (int i=0;i<obj.nstrips();i++)
      {_strips.push_back(obj.strips(i));}
    _asic= obj.asic();
    _x=obj.x();
    _a=obj.a();
    _b=obj.b();
    _used= obj.isUsed();
    }
  TStripCluster(uint8_t asic,uint8_t i){_strips.push_back(i);_x=i;_asic=asic; this->slope();_used=false;}
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
  uint8_t nstrips() const {return _strips.size();}
  uint8_t strips(uint8_t i) const {return _strips[i];}
  uint8_t asic() const {return _asic;}
  double x() const {return _x;}
  double a() const {return _a;}
  double b() const {return _b;}
  void setUsed(bool t=true){_used=t;}
  bool isUsed() const {return _used;}
};
class PadCluster {
public:
  PadCluster(){_pads.clear();_plan=-1;_x=0;_y=0;_used=false;}
  PadCluster(uint8_t plan,uint8_t x,uint8_t y){_plan=plan;_pads.push_back(std::make_pair(x,y)) ; this->calpos();_used=false;}
  PadCluster(const PadCluster& obj)
  {

    for (int i=0;i<obj.npads();i++)
      {_pads.push_back(std::make_pair(obj.padI(i),obj.padJ(i)));}
    _plan= obj.plan();
    _x=obj.x();
    _y=obj.y();
    _used= obj.isUsed();
    }
  bool append(uint8_t plan,uint8_t x,uint8_t y)
  {
    if (plan!=_plan) return false;
    
    bool ap=false;
    for (std::vector<std::pair<uint8_t,uint8_t> >::iterator ipc=_pads.begin();ipc!=_pads.end();ipc++)
      {
	double distx=abs(x-(*ipc).first);
	double disty=abs(y-(*ipc).second);
	if (distx>disty && distx<2) ap=true;
	if (disty>=distx && disty<2) ap=true;
      }
    if (ap){_pads.push_back(std::make_pair(x,y)) ; this->calpos();}
    return ap;
  }
  void calpos()
  {
    _x=_y=0;
    for (int i=0;i<_pads.size();i++)
      {_x+=_pads[i].first;_y+=_pads[i].second;}
    _x/=this->size();
    _y/=this->size();
  }
  void dump()
  {
    printf("Position %d %f %f \n",npads(),x(),y());
    for (int i=0;i<npads();i++)
      {
	printf("%d %d %d \n",i,padI(i),padJ(i)); 
      }
  }
  uint32_t size() const {return _pads.size();}
  uint32_t npads() const {return _pads.size();}
  std::vector<std::pair<uint8_t,uint8_t> >& pads(){return _pads;}
  uint8_t padI(uint32_t i) const {return _pads[i].first;}
  uint8_t padJ(uint32_t i) const {return _pads[i].second;}
  uint8_t plan() const {return _plan;}
  double x() const {return _x;}
  double y() const {return _y;}
  void setUsed(bool t=true){_used=t;}
  bool isUsed() const {return _used;}
private:
  std::vector<std::pair<uint8_t,uint8_t> >  _pads;
  double _x,_y;
  bool _used;
  uint8_t _plan;
};

class TricotCluster
{
  TricotCluster(TStripCluster* s1,TStripCluster* s2,TStripCluster* s3) : _s1(s1),_s2(s2),_s3(s3),_valid(false)
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
  TricotCluster(TStripCluster* s1,TStripCluster* s2) : _s1(s1),_s2(s2),_s3(NULL),_valid(false)
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
  TStripCluster* _s1;
  TStripCluster* _s2;
  TStripCluster* _s3;
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

  void processOneSeed(std::vector<ptrDifFrame> &v);
  void dumpSeed(std::vector<ptrDifFrame> &v);
  void ShowTracks();
private:


  int nAnalyzed_;
  int32_t _fdOut;
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
  uint32_t _neff,_neff2,_neff3,_neff4,_nall,_currentSeed;
  float _x[65532];
  float _y[65532];
  float _z[65532];
  uint32_t _layer[65532];
  uint32_t _npBuf;
  std::map<uint8_t,uint8_t> _plid,_pldif,_pltyp;
  std::map<uint8_t,float> _pldx;
  std::map<uint8_t,float> _pldy;
  std::map<uint8_t,float> _plz;

  std::map<uint8_t,float> _plaxi,_playi,_plaxa,_playa;

  std::map<uint32_t,std::vector<ptrDifFrame> > _FrameMap;
  TrackInfo _tk;
	
};
#endif
