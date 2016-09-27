#ifndef TSTRIPCLUSTER_HH
#define TSTRIPCLUSTER_HH
#include <stdint.h>
#include<vector>
#include "jsonGeo.hh"

class TStripCluster {
private:
  std::vector<uint8_t> _strips;
  uint8_t _asic,_dif;
  uint32_t _id;
  double _x,_a,_b;
  bool _used;
  jsonGeo* _geo;
public:
  TStripCluster(){_strips.clear();_x=0;_asic=0;_dif=0;_used=false;_geo=NULL;}
  TStripCluster(const TStripCluster& obj)
  {
    
    for (int i=0;i<obj.nstrips();i++)
    {_strips.push_back(obj.strips(i));}
    _asic= obj.asic();
    _dif=obj.dif();
    _x=obj.x();
    _a=obj.a();
    _b=obj.b();
    _geo=obj.geo();
    _used= obj.isUsed();
    _id=obj.id();
  }
  TStripCluster(uint8_t dif,uint8_t asic,uint8_t i,jsonGeo* g)
  {
    _strips.push_back(i);_x=i;_asic=asic; _dif=dif;this->slope();_used=false;_geo=g;
    
    uint8_t cham= (_geo->difGeo(this->dif())["chamber"].asUInt()&0xFF);
    uint8_t plan=(_geo->chamberGeo(cham)["plan"].asUInt()&0xFF);
    _id=(plan<<8) | cham;
  }
  bool append(uint8_t dif,uint8_t asic,uint8_t i)
  {
    if (dif!=_dif) return false;
    if (asic!=_asic) return false;
    bool ap=false;
    //std::cout<<"append " <<_strips.size()<<std::endl;
    for (uint16_t j=0;j<_strips.size();j++)
      if (abs(i-_strips[j])<2.1){ap=true;break;}
      if (!ap) return false;
      _strips.push_back(i);
    
    _x=0;for (uint16_t j=0;j<_strips.size();j++) _x+=_strips[j];_x/=_strips.size();
    //std::cout<<"append f " <<_strips.size()<<std::endl;
    
    slope();
    //std::cout<<"append f1 " <<_strips.size()<<std::endl;
    
    return ap;
  }
  void slope()
  {
#undef LMVERSION

#ifdef LMVERSION
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
#else
switch (_asic)
  {
  case 1:
  { 
    _a=0;
    _b=(_x)*0.64+0.2;
    break;
  }
  case 2:
  {
    _a=1.732;
    if(_x<31)
      _b = abs(31-_x)*1.255;
    else
     _b = -(_x-31)*1.255;
  
    break;
  }
  case 3:
  {
   _a = -1.732;
    _b = (63-_x)*1.255+0.6;
    break;
  }
  }
#endif
  }   
  std::vector<uint8_t>& strips(){return _strips;}
  uint8_t nstrips() const {return _strips.size();}
  uint8_t strips(uint8_t i) const {return _strips[i];}
  uint8_t asic() const {return _asic;}
  uint8_t dif() const {return _dif;}
  uint32_t id() const { return _id;}
  double x() const {return _x;}
  double a() const {return _a;}
  double b() const {return _b;}
  void setUsed(bool t=true){_used=t;}
  bool isUsed() const {return _used;}
  inline jsonGeo* geo() const {return _geo;}
  uint32_t chamber() {return _id&0xFF;}
  uint32_t plan(){return (_id>>8)&0xFF;}
};
#endif