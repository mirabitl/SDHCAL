#ifndef _RECOPOINT_HH

#define _RECOPOINT_HH
#include <limits.h>

#include <stdint.h>
#include <vector>




#include "RecoHit.hh"

/**
   \class recoPoint
   \author L.Mirabito
   \date May 2010
   \version 1.0
   \brief Vector of RecoHit. The position is the mean of X and Y. No usage of threshold
*/
class recoPoint : public ROOT::Math::XYZPoint
{
public:
  recoPoint(){valid_=true;_used=false;}
  ~recoPoint(){;}
  virtual double dX()=0;
  virtual double dY()=0;
  virtual uint32_t plan()=0;
  inline bool isUsed() const {return _used;}
  inline void setUse(bool t ){_used=t;}
  bool operator < (const recoPoint& str) const
  {
    return (Z() < str.Z());
  }
  void setValidity(bool t){valid_=t;}
  bool isValid() const {return valid_;}
protected:
  bool valid_,_used;
};
#endif
