#ifndef ChamberPos_HH
#define ChamberPos_HH
#include <math.h>
/**
   \class ChamberPos
   \author L.Mirabito
   \date May 2010
   \version 1.0
   \brief DIF Geometry: only PAD shift and polarity is stored. It allows to swap from local DIF numbering to local chamber numbering
 */
class ChamberPos
{
 public:
 ChamberPos(uint32_t id=0,double x0=0,double y0=0, double z0=0,double x1=0,double y1=0, double z1=0,uint32_t plan=0,uint32_t type=0) : id_(id),x0_(x0),y0_(y0),z0_(z0),x1_(x1),y1_(y1),z1_(z1),plan_(plan),type_(type)
  {
    if (type==0)
      {
	xsize_=96;
	ysize_=96;
	plan_=id_;
      }
    if (type==1)
      {
	xsize_=48;
	ysize_=32;
      }

    
  }
  inline  uint32_t getId(){return id_;}
  inline  uint32_t getPlan(){return plan_;}
  inline  uint32_t getXsize(){return xsize_;}
  inline  uint32_t getYsize(){return ysize_;}
  inline  double getX0(){return x0_;}
  inline  double getY0(){return y0_;}
  inline  double getZ0(){return z0_;}
  inline  double getX1(){return x1_;}
  inline  double getY1(){return y1_;}
  inline  double getZ1(){return z1_;}

  inline  void setId(uint32_t id){id_=id;}
  inline  void setX0(double x) {x0_=x;}
  inline  void setY0(double x) {y0_=x;}
  inline  void setZ0(double x) {z0_=x;}
  inline  void setX1(double x) {x1_=x;}
  inline  void setY1(double x) {y1_=x;}
  inline  void setZ1(double x) {z1_=x;}
  void calculateGlobal(double x,double y,double& xg,double& yg, double& zg)
  {

    xg= x0_+(x1_-x0_)*x/xsize_;
    yg= y0_+(y1_-y0_)*y/ysize_;
    zg = z0_; // A corriger pour des plans inclines
  }
  void calculateLocal(double xg,double yg, double zg, double& x,double& y, double& z)
  {
    x = (xg-x0_)*xsize_/(x1_-x0_);
    y = (yg-y0_)*ysize_/(y1_-y0_);
    zg = z;
  }

 private:
  uint32_t id_,plan_,xsize_,ysize_,type_;
  double x0_,y0_,z0_;
  double x1_,y1_,z1_;
  
};
#endif
