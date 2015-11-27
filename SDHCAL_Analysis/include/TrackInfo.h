#ifndef _TRACKINFO_H
#define _TRACKINFO_H
#define TK_MAX_POINT 128 
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <bitset>
#include <math.h>
#include "Droite3D.h"
class TrackInfo : public Droite3D
{
 public:
  inline double ax(){return ax_;}
  inline double bx(){return bx_;}
  inline double ay(){return ay_;}
  inline double by(){return by_;}
  inline double chi2x(){return chi2x_;}
  inline double chi2y(){return chi2y_;}
  inline uint32_t size(){return np_;}
  inline double zmin(){return zmin_;}
  inline double zmax(){return zmax_;}
  inline double x(uint8_t i){return x_[i];}
  inline double y(uint8_t i){return y_[i];}
  inline double z(uint8_t i){return z_[i];}
  inline std::bitset<128> planes(){return planes_;}
  inline uint32_t layer(uint8_t i){return layer_[i];}
  inline bool plane(uint8_t i){return planes_[i];}
  inline double xext(double z){return ax_*z+bx_;}
  inline double yext(double z){return ay_*z+by_;}
  inline double dx(double x,double z) { return (ax_*z+bx_)-x;}
  inline double dy(double y,double z) { return (ay_*z+by_)-y;}
  inline double distance(double x,double y,double z) { return sqrt(dx(x,z)*dx(x,z)+dy(y,z)*dy(y,z));}
  inline double dx(uint32_t i) { return (ax_*z_[i]+bx_)-x_[i];}
  inline double dy(uint32_t i) { return (ay_*z_[i]+by_)-y_[i];}
  inline double distance(uint32_t i) { return sqrt(dx(i)*dx(i)+dy(i)*dy(i));}
  inline void set_ax(double x) {ax_=x;}
  inline void set_bx(double x) {bx_=x;}
  inline void set_ay(double x) {ay_=x;}
  inline void set_by(double x) {by_=x;}
  inline void set_chi2x(double x) {chi2x_=x;}
  inline void set_chi2y(double x) {chi2y_=x;}
  inline void set_size(uint32_t s){np_=s;}
  inline void set_zmin(double x) {zmin_=x;}
  inline void set_zmax(double x) {zmax_=x;}
  inline void set_x(uint8_t i,double x){x_[i]=x;}
  inline void set_y(uint8_t i,double y){y_[i]=y;}
  inline void set_z(uint8_t i,double z){z_[i]=z;}
  inline void set_layer(uint8_t i,uint32_t l){layer_[i]=l;}
  inline void set_plane(uint8_t i){planes_.set(i);}
  inline void clear(){np_=0;ax_=bx_=ay_=by_=chi2x_=chi2y_=zmin_=zmax_=0;
    memset(layer_,0,TK_MAX_POINT*sizeof(uint32_t));
    memset(x_,0,TK_MAX_POINT*sizeof(double));
    memset(y_,0,TK_MAX_POINT*sizeof(double));
    memset(z_,0,TK_MAX_POINT*sizeof(double));
    planes_.reset();
    
  }
  inline void add_point(double x,double y,double z,uint32_t l){x_[np_]=x;y_[np_]=y;z_[np_]=z;layer_[np_]=l;np_++;}

  void regression();
  void exclude_layer(uint32_t layer,TrackInfo& ti);
  virtual double m(int i);
  virtual double v(int i);
  double closestApproach(double x,double y,double z);
 private:
  uint32_t np_;
  uint32_t layer_[TK_MAX_POINT];
  double ax_,ay_,bx_,by_,chi2x_,chi2y_,zmin_,zmax_;
  double x_[TK_MAX_POINT];
  double y_[TK_MAX_POINT];
  double z_[TK_MAX_POINT];
  std::bitset<128> planes_;
};
#endif
