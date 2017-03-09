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
  _valid=(_points.size()>=2);
  this->regression();
}

void recoTrack::cap(recoTrack& t,double &dist, ROOT::Math::XYZPoint &p1, ROOT::Math::XYZPoint &p2)
{
  // Closets time approach

  
  ROOT::Math::XYZVector dv = dir() - t.dir();
  float cpatime=0;
  float    dv2 = dv.Mag2();
  if (dv2 > 1E-6)      // the  tracks are almost parallel
    {
      ROOT::Math::XYZVector  w0 = orig() - t.orig();
      cpatime = -1*w0.Dot(dv) / dv2;
    }

  p1=orig()+cpatime*dir();
   p2=t.orig()+cpatime*t.dir();

  ROOT::Math::XYZVector d=p1-p2;

  //std::cout<<p1.X()<<" "<<p1.Y()<<" "<<p1.Z()<<std::endl;
  dist=sqrt(d.Mag2());

  
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
  /* uint32_t nc=0;
    for (std::vector<planeCluster*>::iterator ic=pc.begin();ic!=pc.end();ic++)
    {
	  planeCluster* c0=(*ic);
	  std::cout<<"Z["<<nc++<<"]="<<c0->Z()<<std::endl; 
    }
  */
  for (std::vector<planeCluster*>::iterator ic=pc.begin();ic!=pc.end();ic++)
    {
	  planeCluster* c0=(*ic);
	  //std::cout<<"Z["<<nc<<"]="<<c0->Z()<<std::endl; 
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
	      tk->addPoint((*ic));
	      tk->addPoint((*jc));
	      for (std::vector<planeCluster*>::iterator kc=pc.begin();kc!=pc.end();kc++)
		{
		  if ((*kc)->isUsed()) continue;
		  if ((*kc)->Z()<=(*jc)->Z()) continue;
		  if ((*kc)->Z()<=(*ic)->Z()) continue;
		   if (tk->distance((*kc))<tkDistCut)
			{
			  //std::cout<<" "<<abs((*kc)->Z()-tk->zmin())
			  //		   <<" "<<abs((*kc)->Z()-tk->zmax())<<std::endl;
			  if (((*kc)->Z()<tk->zmin()-3)||
			      ((*kc)->Z()>tk->zmax()+3)) continue;
			  else
			    {
			      //std::cout<<(*kc)->Z()<<" add to  "<<tk->zmax()<<std::endl;
			      
			      tk->addPoint((*kc));
			      (*kc)->setUse(true);
			      good=true;
			      break;
			    }
			}
		      //std::cout<<tk->distance((*kc))<<std::endl;
		      /*
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
		      //std::cout<<"seed "<<(*ic)->Z()<<" "<<(*jc)->Z()<<" "<<(*kc)->Z()<<" min "<<tk->zmin()<<" "<<tk->zmax()<<std::endl;
		      good=true;
		      break;
		    }
		    */
		}
	      if (good)
		{
		  for (std::vector<planeCluster*>::iterator kc=pc.begin();kc!=pc.end();kc++)
		    {
		      if ((*kc)->isUsed()) continue;
		      //if ((*kc)->Z()<tk->zmax()) continue;
		      if (tk->distance((*kc))<tkDistCut)
			{
			  std::cout<<" ZDIST  "<<abs((*kc)->Z()-tk->zmin())
			  		   <<" "<<abs((*kc)->Z()-tk->zmax())<<std::endl;
			  if (((*kc)->Z()<tk->zmin()-6)||
			      ((*kc)->Z()>tk->zmax()+6)) continue;
			  else
			    {
			      //std::cout<<(*kc)->Z()<<" add to  "<<tk->zmax()<<std::endl;
			      
			      tk->addPoint((*kc));
			      (*kc)->setUse(true);
			    }
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
		    else
		      delete tk;


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

void recoTrack::getChi2(std::vector<planeCluster*> clusters)
{
  double chi2=0,paderr=100./96./sqrt(12.);
  for (std::vector<ROOT::Math::XYZPoint*>::iterator ip=this->points().begin();ip!=this->points().end();ip++)
    {
      double cont=this->distance((*ip));
      double err=0;
      bool found=false;
      std::vector<planeCluster*>::iterator ic=std::find(clusters.begin(),clusters.end(),(planeCluster*) (*ip));
      
      if (ic!=clusters.end())
	{
	  double errx=1./sqrt((*ic)->hits().size())*paderr;
	  double erry=1./sqrt((*ic)->hits().size())*paderr;
	  err=sqrt(errx*errx+erry*erry);
	  chi2+=cont*cont/err/err;
	  //nc++;

	}
      else
	std::cout<<"Cluster noit found !!!"<<std::endl;

    }

//std::cout<<"chi2 "<<chi2<<" ndf"<<this->points().size()*2-4<<" "<<TMath::Prob(chi2,this->points().size()*2-4)<<std::endl;
  this->setChi2(chi2);

}

void recoTrack::getChi2(std::vector<recoPoint*> clusters)
{
  double chi2=0;
  for (std::vector<ROOT::Math::XYZPoint*>::iterator ip=this->points().begin();ip!=this->points().end();ip++)
    {
      double cont=this->distance((*ip));
      double err=0;
      bool found=false;
      std::vector<recoPoint*>::iterator ic=std::find(clusters.begin(),clusters.end(),(recoPoint*) (*ip));
      
      if (ic!=clusters.end())
	{
	  double errx=(*ic)->dX();
	  double erry=(*ic)->dY();
	  err=sqrt(errx*errx+erry*erry);
	  chi2+=cont*cont/err/err;
	  //nc++;

	}
      else
	std::cout<<"Cluster noit found !!!"<<std::endl;

    }

//std::cout<<"chi2 "<<chi2<<" ndf"<<this->points().size()*2-4<<" "<<TMath::Prob(chi2,this->points().size()*2-4)<<std::endl;
  this->setChi2(chi2);

}

#undef old_method
void recoTrack::combinePoint(std::vector<recoPoint*> pc,jsonGeo* g,  std::vector<recoTrack*> &vtk)
{
  for (std::vector<recoTrack*>::iterator itk=vtk.begin();itk!=vtk.end();itk++) delete (*itk);
  vtk.clear();
  float maxDistSeed=g->cuts()["maxDistSeed"].asFloat();//10
  float maxDistSeed2=maxDistSeed*maxDistSeed;
  float cosSeed=g->cuts()["cosSeed"].asFloat();//0.01
  float tkDistCut=g->cuts()["tkDistCut"].asFloat();//1.5
  float tkDzMax=g->cuts()["tkDzMax"].asFloat();//1.5
  
  uint32_t minTkPoint=g->cuts()["tkMinPoint"].asUInt();
  //std::cout<<maxDistSeed2<<" "<<cosSeed<<" "<<tkDistCut<<" "<<minTkPoint<<std::endl;
  /* uint32_t nc=0;
   *   for (std::vector<planeCluster*>::iterator ic=pc.begin();ic!=pc.end();ic++)
   *   {
   *     planeCluster* c0=(*ic);
   *     std::cout<<"Z["<<nc++<<"]="<<c0->Z()<<std::endl; 
}
*/
  for (std::vector<recoPoint*>::iterator ic=pc.begin();ic!=pc.end();ic++)
  {
    recoPoint* c0=(*ic);
    //std::cout<<"Z= "<<c0->Z()<<std::endl; 
    if ((*ic)->isUsed()) continue;
    for (std::vector<recoPoint*>::iterator jc=pc.begin();jc!=pc.end();jc++)
    {
      if ((*jc)->isUsed()) continue;
      if ((*jc)->Z()<=(*ic)->Z()) continue;
      recoPoint* c1=(*jc);
      ROOT::Math::XYZVector d=(*c1)-(*c0);
      //std::cout<<d.Mag2()<<" distance"<<std::endl;
      if (d.Mag2()>maxDistSeed2) continue;
      
      
      bool good=false;
      recoTrack* tk=new recoTrack();

#ifndef old_method
      tk->addPoint((*ic));
      tk->addPoint((*jc));
#endif
      for (std::vector<recoPoint*>::iterator kc=pc.begin();kc!=pc.end();kc++)
      {
        if ((*kc)->isUsed()) continue;
        if ((*kc)->Z()<=(*jc)->Z()) continue;
	if ((*kc)->Z()<=(*ic)->Z()) continue;
#ifdef old_method
        recoPoint* c2=(*kc);
        ROOT::Math::XYZVector d1=(*c2)-(*c1);
        if (d1.Mag2()>maxDistSeed2) continue;
        double s=d.Dot(d1)/sqrt(d.Mag2()*d1.Mag2());
        //std::cout<<"angle : "<<s<<std::endl;
        if (abs(s-1.)<cosSeed)
        {
          (*ic)->setUse(true);
          (*jc)->setUse(true);
          (*kc)->setUse(true);
           tk->addPoint((*ic));
           tk->addPoint((*jc));
        
          tk->addPoint((*kc));
          //std::cout<<"seed "<<(*ic)->Z()<<" "<<(*jc)->Z()<<" "<<(*kc)->Z()<<" min "<<tk->zmin()<<" "<<tk->zmax()<<std::endl;
          good=true;
          break;
        }
#else
	if (tk->distance((*kc))<tkDistCut)
          {
            //std::cout<<" "<<abs((*kc)->Z()-tk->zmin())
            //           <<" "<<abs((*kc)->Z()-tk->zmax())<<std::endl;
            if (((*kc)->Z()<tk->zmin()-6*3)||
              ((*kc)->Z()>tk->zmax()+6*3)) continue;
            else
            {
              //std::cout<<(*kc)->Z()<<" add to  "<<tk->zmax()<<std::endl;
              (*ic)->setUse(true);
	      (*jc)->setUse(true);
              tk->addPoint((*kc));
              (*kc)->setUse(true);
	      good=true;
	      break;
            }
	  }
#endif
      }
      if (good)
      {
        for (std::vector<recoPoint*>::iterator kc=pc.begin();kc!=pc.end();kc++)
        {
          if ((*kc)->isUsed()) continue;
	  bool planuse=false;
	  for ( std::vector<ROOT::Math::XYZPoint*>::iterator ip=tk->points().begin();ip!=tk->points().end();ip++)
	    if ((*ip)->Z()==(*kc)->Z()) {planuse=true;break;}
	  if (planuse) continue;
          //if ((*kc)->Z()<tk->zmax()) continue;
          if (tk->distance((*kc))<tkDistCut)
          {
            //std::cout<<" "<<abs((*kc)->Z()-tk->zmin())
            //           <<" "<<abs((*kc)->Z()-tk->zmax())<<std::endl;
            if (((*kc)->Z()<tk->zmin()-tkDzMax)||
              ((*kc)->Z()>tk->zmax()+tkDzMax)) continue;
            else
            {
              //std::cout<<(*kc)->Z()<<" add to  "<<tk->zmax()<<std::endl;
              
              tk->addPoint((*kc));
              (*kc)->setUse(true);
            }
          }
          //std::cout<<tk->distance((*kc))<<std::endl;
        }
        // Now clean a few
        tk->clean(tkDistCut);
        if (tk->size()>=minTkPoint)
        {
          //std::cout<<tk;
          //tk.Dump();
	  tk->getChi2(pc);
          vtk.push_back(tk);
          break;
        }
        else
	  delete tk;
        
        
      }
    }
  }
  return;
}



void recoTrack::combinePoint1(std::vector<recoPoint*> pc,jsonGeo* g,  std::vector<recoTrack*> &vtk)
{
  for (std::vector<recoTrack*>::iterator itk=vtk.begin();itk!=vtk.end();itk++) delete (*itk);
  vtk.clear();
  
  float maxDistSeed=g->cuts()["maxDistSeed"].asFloat();//10
  float maxDistSeed2=maxDistSeed*maxDistSeed;
  float cosSeed=g->cuts()["cosSeed"].asFloat();//0.01
  float tkDistCut=g->cuts()["tkDistCut"].asFloat();//1.5
  float tkDzMax=g->cuts()["tkDzMax"].asFloat();//1.5
  
  uint32_t minTkPoint=g->cuts()["tkMinPoint"].asUInt();
  //std::cout<<maxDistSeed2<<" "<<cosSeed<<" "<<tkDistCut<<" "<<minTkPoint<<std::endl;
  /* uint32_t nc=0;
   *   for (std::vector<planeCluster*>::iterator ic=pc.begin();ic!=pc.end();ic++)
   *   {
   *     planeCluster* c0=(*ic);
   *     std::cout<<"Z["<<nc++<<"]="<<c0->Z()<<std::endl; 
}
*/
  for (std::vector<recoPoint*>::iterator ic=pc.begin();ic!=pc.end();ic++)
  {
    recoPoint* c0=(*ic);
    //std::cout<<"Z= "<<c0->Z()<<std::endl; 
    if ((*ic)->isUsed()) continue;
    for (std::vector<recoPoint*>::iterator jc=pc.begin();jc!=pc.end();jc++)
    {
      if ((*jc)->isUsed()) continue;
      if ((*jc)->Z()>=(*ic)->Z()) continue;
      recoPoint* c1=(*jc);
      ROOT::Math::XYZVector d=(*c1)-(*c0);
      //std::cout<<d.Mag2()<<" distance"<<std::endl;
      //      if (d.Mag2()>maxDistSeed2) continue;
      
      
      bool good=false;
      recoTrack* tk=new recoTrack();

 
      for (std::vector<recoPoint*>::iterator kc=pc.begin();kc!=pc.end();kc++)
      {
        if ((*kc)->isUsed()) continue;
        if ((*kc)->Z()>=(*jc)->Z()) continue;
	if ((*kc)->Z()>=(*ic)->Z()) continue;
        recoPoint* c2=(*kc);
        ROOT::Math::XYZVector d1=(*c2)-(*c1);
        //if (d1.Mag2()>maxDistSeed2) continue;
        double s=d.Dot(d1)/sqrt(d.Mag2()*d1.Mag2());
        //std::cout<<"angle : "<<s<<std::endl;
        if (abs(s-1.)<cosSeed)
        {
          (*ic)->setUse(true);
          (*jc)->setUse(true);
          (*kc)->setUse(true);
           tk->addPoint((*ic));
           tk->addPoint((*jc));
        
          tk->addPoint((*kc));
          //std::cout<<"seed "<<(*ic)->Z()<<" "<<(*jc)->Z()<<" "<<(*kc)->Z()<<" min "<<tk->zmin()<<" "<<tk->zmax()<<std::endl;
          good=true;
          break;
        }

      }
      if (good)
      {
        for (std::vector<recoPoint*>::iterator kc=pc.begin();kc!=pc.end();kc++)
        {
          if ((*kc)->isUsed()) continue;
	  bool planuse=false;
	  for ( std::vector<ROOT::Math::XYZPoint*>::iterator ip=tk->points().begin();ip!=tk->points().end();ip++)
	    if ((*ip)->Z()==(*kc)->Z()) {planuse=true;break;}
	  if (planuse) continue;
          //if ((*kc)->Z()<tk->zmax()) continue;
          if (tk->distance((*kc))<tkDistCut)
          {
            //std::cout<<" "<<abs((*kc)->Z()-tk->zmin())
            //           <<" "<<abs((*kc)->Z()-tk->zmax())<<std::endl;
            if (((*kc)->Z()<tk->zmin()-tkDzMax)||
              ((*kc)->Z()>tk->zmax()+tkDzMax)) continue;
            else
            {
              //std::cout<<(*kc)->Z()<<" add to  "<<tk->zmax()<<std::endl;
              
              tk->addPoint((*kc));
              (*kc)->setUse(true);
            }
          }
          //std::cout<<tk->distance((*kc))<<std::endl;
        }
        // Now clean a few
        tk->clean(tkDistCut);
        if (tk->size()>=minTkPoint)
        {
          //std::cout<<tk;
          //tk.Dump();
	  tk->getChi2(pc);
          vtk.push_back(tk);
          break;
        }
        else
	  delete tk;
        
        
      }
    }
  }
  return;
}
