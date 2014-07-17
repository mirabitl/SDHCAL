#ifndef PlanShift_HH
#define PlanShift_HH
#include <math.h>
/**
   \class PlanShift
   \author L.Mirabito
   \date May 2010
   \version 1.0
   \brief DIF Geometry: only PAD shift and polarity is stored. It allows to swap from local DIF numbering to local chamber numbering
 */
class PlanShift
{
 public:
 PlanShift(uint32_t id=0,double x0=0,double y0=0,double z0=0) : id_(id),x0_(x0),y0_(y0)
  {
  }
  inline  uint32_t getId(){return id_;}
  inline  double getX0(){return x0_;}
  inline  double getY0(){return y0_;}
  inline  double getZ0(){return z0_;}
  inline  void setId(uint32_t id){id_=id;}
  inline  void setX0(double x) {x0_=x;}
  inline  void setY0(double x) {y0_=x;}
  inline  void setZ0(double x) {z0_=x;}

 private:
  uint32_t id_;  double x0_,y0_,z0_;

  
};
#endif
