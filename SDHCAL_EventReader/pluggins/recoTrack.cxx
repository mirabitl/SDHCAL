#include "recoTrack.hh"
#include "TMath.h"
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
void recoTrack::clean(float cut)
{
  for (std::vector<ROOT::Math::XYZPoint*>::iterator ip=_points.begin();ip!=_points.end();)
    {
      if (distance((*ip))>cut)
	_points.erase(ip);
      else
	ip++;
    }
  regression();		

}
void recoTrack::remove(ROOT::Math::XYZPoint* p)
{
  for (std::vector<ROOT::Math::XYZPoint*>::iterator ip=_points.begin();ip!=_points.end();)
    {
      if ((*ip)==p)
	_points.erase(ip);
      else
	ip++;
    }
  regression();		

}
 void recoTrack::combine(std::vector<planeCluster*> pc,jsonGeo* g,  std::vector<recoTrack*> &vtk)
{
  for (std::vector<recoTrack*>::iterator itk=vtk.begin();itk!=vtk.end();itk++) delete (*itk);
  vtk.clear();
  float maxDistSeed=g->cuts()["maxDistSeed"].asFloat();//10
  float maxDistSeed2=maxDistSeed*maxDistSeed;
  float cosSeed=g->cuts()["cosSeed"].asFloat();//0.01
  float tkDistCut=g->cuts()["tkDistCut"].asFloat();//1.5
  uint32_t minTkPoint=g->cuts()["tkMinPoint"].asUInt();
  //std::cout<<maxDistSeed2<<" "<<cosSeed<<" "<<tkDistCut<<" "<<minTkPoint<<std::endl;
  for (std::vector<planeCluster*>::iterator ic=pc.begin();ic!=pc.end();ic++)
    {
	  planeCluster* c0=(*ic);
	  if ((*ic)->isUsed()) continue;
	  for (std::vector<planeCluster*>::iterator jc=pc.begin();jc!=pc.end();jc++)
	    {
	      if ((*jc)->isUsed()) continue;
	      if ((*jc)->Z()<=(*ic)->Z()) continue;
	      planeCluster* c1=(*jc);
	      ROOT::Math::XYZVector d=(*c1)-(*c0);
	      if (d.Mag2()>maxDistSeed2) continue;


	      bool good=false;
	      recoTrack* tk=new recoTrack();
	      for (std::vector<planeCluster*>::iterator kc=pc.begin();kc!=pc.end();kc++)
		{
		  if ((*kc)->isUsed()) continue;
		  if ((*kc)->Z()<=(*jc)->Z()) continue;
		  planeCluster* c2=(*kc);
		  ROOT::Math::XYZVector d1=(*c2)-(*c1);
		  if (d1.Mag2()>maxDistSeed2) continue;
		  double s=d.Dot(d1)/sqrt(d.Mag2()*d1.Mag2());
		  //std::cout<<s<<std::endl;
		  if (abs(s-1.)<cosSeed)
		    {
		      (*ic)->setUse(true);
		      (*jc)->setUse(true);
		      (*kc)->setUse(true);

		      tk->addPoint((*ic));
		      tk->addPoint((*jc));
		      tk->addPoint((*kc));
		      good=true;
		      break;
		    }
		}
	      if (good)
		{
		  for (std::vector<planeCluster*>::iterator kc=pc.begin();kc!=pc.end();kc++)
		    {
		      if ((*kc)->isUsed()) continue;
		      if (tk->distance((*kc))<tkDistCut)
			{
			  tk->addPoint((*kc));
			  (*kc)->setUse(true);
			}
		      //std::cout<<tk->distance((*kc))<<std::endl;
		    }
		  // Now clean a few
		  tk->clean(tkDistCut);
		  if (tk->size()>=minTkPoint)
		    {
		      //std::cout<<tk;
		      //tk.Dump();

		      vtk.push_back(tk);
		      break;
		    }


		}
	    }
	}
      return;
}
void recoTrack::setChi2(double chi2)
{
  _chi2=chi2;
  _pchi2=TMath::Prob(_chi2,_points.size()*2-4);
}
