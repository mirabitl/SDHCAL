#ifndef _RECOFRAME_HH
#define _RECOFRAME_HH
#include "jsonGeo.hh"
#include <stdint.h>
#include <Math/Point3Dfwd.h>
using namespace ROOT::Math;

class RecoFrame : public ROOT::Math::XYZPoint
  {
  public:
    RecoFrame(){_frame=0;_geo=0;}
    RecoFrame(uint64_t f){_frame=f;_geo=0;}
    RecoFrame(uint8_t dif,uint8_t asic,uint8_t channel,uint8_t threshold,uint32_t bc){ this->set(dif,asic,channel,threshold,bc);}
    void initialise(jsonGeo* g) {_geo=g;g->convert(this->dif(),this->asic(),this->channel(),this);}
      
    inline uint16_t I(){return int(X()/1.04125);}
    inline uint16_t  J(){return int(Y()/1.04125);}
    inline uint16_t chamber(){return (_geo==NULL)?0:_geo->difInfo(this->dif()).chamber;}
    inline uint32_t dif(){return (_frame&0xFF);}
    inline uint32_t asic( ){return ((_frame>>8)&0x3F);}
    inline uint32_t channel( ){return ((_frame>>14)&0x3F);}
    inline uint32_t threshold(){return ((_frame>>20)&0x3);}
    inline uint32_t bc(){ return ((_frame>>32)&0xFFFFFFFF);}
    inline void set(uint8_t dif,uint8_t asic,uint8_t channel,uint8_t threshold,uint32_t bc)
    {
      uint64_t d=dif,a=asic,c=channel,t=threshold,b=bc;
      _frame=((b&0xFFFFFFFF)<<32)|((t&0x3)<<20)|((c&0x3f)<<14)|((a&0x3F)<<8)|
        (d&0xFF);}
    inline uint64_t value() const {return _frame;}
    inline void set( uint64_t f){_frame=f;}
  private:
    uint64_t _frame;
    jsonGeo* _geo;
  };
#endif
