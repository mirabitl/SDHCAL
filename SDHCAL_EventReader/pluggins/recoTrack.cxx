#include "recoTrack.hh"

recoTrack::recoTrack() :_valid(false)
{
  clear();
}




void recoTrack::clear()
{
  _points.clear();
  _orig.SetXYZ(0,0,0);
  _dir.SetXYZ(0,0,0);
}

void recoTrack::addPoint(ROOT::Math::XYZPoint* p)
{
  _points.push_back(p);
  _valid=(_points.size()>2);
  this->regression();
}
double recoTrack::distance(ROOT::Math::XYZPoint* p)
{
  if (_dir.Mag2()<1E-3)
    return 999999.;
  ROOT::Math::XYZVector ba=(*p)-_orig;
  ROOT::Math::XYZVector cross=_dir.Cross(ba);
  return sqrt(cross.Mag2()/_dir.Mag2());
}
ROOT::Math::XYZPoint recoTrack::extrapolate(double z)
{
  ROOT::Math::XYZPoint e;
  e.SetXYZ(_dir.X()*z+_orig.X(),_dir.Y()*z+_orig.Y(),z);
  return e;
}
void recoTrack::regression()
{
  unsigned int n = _points.size();

  if (!_valid) return;
  double zbar=0;
  double xbar=0;
  double ybar=0;
  double z2bar =0;
  double zxbar=0;
  double zybar=0;
  zmin_=1000000;
  zmax_=-100000;
  double wxt=0;
  double wyt=0;
  for (std::vector<ROOT::Math::XYZPoint*>::iterator ip=_points.begin();ip!=_points.end();ip++)
    {
      if ((*ip)->Z()<zmin_) zmin_=(*ip)->Z();
      if ((*ip)->Z()>zmax_) zmax_=(*ip)->Z();
      zbar+=(*ip)->Z();
      z2bar+=(*ip)->Z()*(*ip)->Z();
      zxbar+=(*ip)->Z()*(*ip)->X();
      zybar+=(*ip)->Z()*(*ip)->Y();
      xbar+=(*ip)->X();
      ybar+=(*ip)->Y();
    }
  zbar /=n;
  z2bar /=n;
  zxbar /=n;
  zybar /=n;
  ybar /=n;
  xbar /=n;
  double s2z = z2bar-zbar*zbar;
  double szx = zxbar-zbar*xbar;
  double szy = zybar-zbar*ybar;
  double ax_ = szx/s2z;double bx_=xbar -ax_*zbar;
  double ay_ = szy/s2z;double by_=ybar -ay_*zbar;
  _orig.SetXYZ(bx_,by_,0);
  _dir.SetXYZ(ax_,ay_,1.0);


}
void recoTrack::Dump(std::ostream &os) 
{
    for (std::vector<ROOT::Math::XYZPoint*>::iterator ip=_points.begin();ip!=_points.end();ip++)
    {
      os<<"\t ptr:"<<(*ip)<<" ("<<(*ip)->X()<<":"<<(*ip)->Y()<<":"<<(*ip)->Z()<<")"<<" ==>"<<distance((*ip))<<std::endl;
    }

}

TLine* recoTrack::linex()
{
  return new TLine(zmin_,_orig.X()+_dir.X()*zmin_,zmax_,_orig.X()+_dir.X()*zmax_);
}
TLine* recoTrack::liney()
{
  return new TLine(zmin_,_orig.Y()+_dir.Y()*zmin_,zmax_,_orig.Y()+_dir.Y()*zmax_);
}
