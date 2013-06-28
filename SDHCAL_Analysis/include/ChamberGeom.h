#ifndef ChamberGeom_HH
#define ChamberGeom_HH
#include <math.h>
/**
   \class ChamberGeom
   \author L.Mirabito
   \date May 2010
   \version 1.0
   \brief DIF Geometry: only PAD shift and polarity is stored. It allows to swap from local DIF numbering to local chamber numbering
 */
class ChamberGeom
{
 public:
 ChamberGeom(unsigned int id=0,double x=0,double y=0, double z=0,double angle=0) : id_(id),x_(x),y_(y),z_(z),angle_(angle) 
  {
    double ang=  acos(-1)/180.*angle;
    cos_=cos(ang);
    sin_=sin(ang);
  }
  inline  unsigned int getId(){return id_;}
  inline  double getX(){return x_;}
  inline  double getY(){return y_;}
  inline  double getZ(){return z_;}
  inline  double getAngle(){return angle_;}
  inline  void setId(unsigned int id){id_=id;}
  inline  void setX(double x) {x_=x;}
  inline  void setY(double x) {y_=x;}
  inline  void setZ(double x) {z_=x;}
  void calculateGlobal(double x,double y, double z,double& xg,double& yg, double& zg)
  {
    xg = x*cos_-sin_*y+x_;
    yg = x*sin_+y*cos_+y_;
    zg = z+z_;
  }
  void calculateLocal(double x,double y, double z,double& xg,double& yg, double& zg)
  {
    xg = (x-x_)*cos_+sin_*(y-y_);
    yg = -1*(x-x_)*sin_+(y-y_)*cos_;
    zg = z-z_;
  }

  inline  double toGlobalX(double x){return x+x_;}
  inline  double toGlobalY(double x){return x+y_;}
  inline  double toLocalX(double x){return x-x_;}
  inline  double toLocalY(double x){return x-y_;}
 private:
  unsigned int id_;
  double x_,y_,z_,angle_,cos_,sin_;

};
#endif
