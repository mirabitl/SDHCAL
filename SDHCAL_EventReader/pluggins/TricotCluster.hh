#ifndef TRICOTCLUSTER_HH
#define TRICOTCLUSTER_HH
#include "TStripCluster.hh"
#include <vector>
#include <stdint.h>
#include <Math/Point3Dfwd.h>
#include <TPrincipal.h>
#include <cfloat>
#include "recoPoint.hh"
class TricotCluster : public recoPoint
{
private:
  std::vector<TStripCluster> _strips;
  double _dx,_dy;
  
public:
  TricotCluster(double x,double y,double z,TStripCluster &a,TStripCluster &b, TStripCluster &c)
    {
      _strips.clear();
      _strips.push_back(a);_strips.push_back(b);_strips.push_back(c);
      this->SetXYZ((x+28.8)/1.25,(y+1.)/1.25,z);
      //this->SetXYZ(x+21,(y-4.1)/0.92,z);
      //this->SetXYZ(1.0*x,1.0*y,z);
      
        //std::cout<<(int) a.dif()<<" "<<x<<" "<<y<<" "<<z<<std::endl;
    setUse(false);
      
    }
  TricotCluster(double x,double y,double z,TStripCluster &a,TStripCluster &b)
    {
      _strips.clear();
      _strips.push_back(a);_strips.push_back(b);
      //this->SetXYZ((x+18.22)/0.82,(y+1.04)/0.9,z);
      //this->SetXYZ(x+21,(y-4.1)/0.92,z);
      
      //this->SetXYZ(1.0*x,1.0*y,z);
      this->SetXYZ((x+28.8)/1.25,(y+1.)/1.25,z);
      
      //this->SetXYZ((x+17.65)/0.9,(y-4.4)/0.9,z);
      
      //std::cout<<(int) a.dif()<<" "<<x<<" "<<y<<" "<<z<<std::endl;
      setUse(false);
      
      
    }
  TricotCluster(const TricotCluster& obj)
  {
    
    for (int i=0;i<obj.nstrips();i++)
    {_strips.push_back(obj.strips(i));}
    _used= obj.isUsed();
    this->SetXYZ(obj.X(),obj.Y(),obj.Z());
    
  }
  std::vector<TStripCluster>& strips(){return _strips;}
  uint8_t nstrips() const {return _strips.size();}
  TStripCluster strips(uint8_t i) const {return _strips[i];}

  uint8_t dif() const {return strips(0).dif();}
  virtual double dX(){return 0.1;};
  virtual double dY(){return 0.1;}
  virtual uint32_t plan(){ 
      //std::cout<<"ID first strip "<<std::hex<<strips(0).id()<<std::dec<<std::endl;
      
    return strips(0).plan();}

};
#endif
