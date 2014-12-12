#include <stdint.h>
#include <stdio.h> 
#include "ComputerTrack.h"
#include <math.h>
#include "DCHistogramHandler.h"
#include "TCanvas.h"
#include "GeoPoint.h"
#include "TemplateTk.h"
#include "Amas.h"
//#include "TemplateTk.txx"

ComputerTrack::ComputerTrack(HoughCut* cuts) :theCuts_(cuts)
{
  theNStub_=0;
  theX_=NULL;
  theY_=NULL;
  theZ_=NULL;
  theLayer_=NULL;

}
ComputerTrack::~ComputerTrack()
{
}
void ComputerTrack::DefaultCuts()
{
  theCuts_->RhoMin=-150.;
  theCuts_->RhoMax=150;
  theCuts_->NRho=48;
  theCuts_->NTheta=48;
  theCuts_->NStubLow=4;
  theCuts_->NLayerRow=4;
  theCuts_->NStubLowCandidate=3;
  theCuts_->NBins3GeV=64;
  theCuts_->NDelBarrel=2.;
  theCuts_->NStubHigh=5;
  theCuts_->NLayerHigh=5;
  theCuts_->NStubHighCandidate=5;
}
void ComputerTrack::muonFinder(uint32_t nstub,float* x,float* y,float* z,uint32_t* layer)
{
  theTrackVector_.clear();
  // Look for first and last plane
  int32_t fpl=9999,lpl=-1;
  for (int i=0;i<nstub;i++)
    {
      if (layer[i]<fpl) fpl=layer[i];
      if (layer[i]>fpl) lpl=layer[i];
    }
  int nplans=lpl-fpl+1;
  if (nplans<3) return;
  //std::cout<<"Seraching track from "<<fpl<<"to "<<lpl<<std::endl;

  std::vector<TrackInfo> tracks;
  /*
  std::map<uint32_t,std::vector<GeoPoint> > chmap;
  for (uint32_t i=0;i<nstub;i++)
    {
      if (layer[i]>nplans) continue;
      GeoPoint p(layer[i],x[i],y[i],z[i]);
      //  DEBUG_PRINT("%d %d %f %f %f \n",i,layer[i],x[i],y[i],z[i]);
      p.setUsed(false);
      std::map<uint32_t,std::vector<GeoPoint> >::iterator ich=chmap.find(layer[i]);
      if (ich==chmap.end())
	{
	  std::vector<GeoPoint> v;
	  v.push_back(p);
	  std::pair<uint32_t,std::vector<GeoPoint> > pm(layer[i],v);
	  chmap.insert(pm);
	}
      else
	ich->second.push_back(p);
    }
  */
  bool used[1024];memset(used,0,1024*sizeof(bool));
  for (uint32_t ip1=fpl;ip1<lpl;ip1++)
    {
      for (uint32_t ip2=ip1+1;ip2<lpl;ip2++)
	{

	  for (uint32_t i=0;i<nstub;i++)
	    {
	      if (used[i]) continue;
	      if (layer[i]!=ip1) continue;

	    
	      for (uint32_t j=0;j<nstub;j++)
		{
		  if (used[j]) continue;
		  if (layer[j]!=ip2) continue;
		  TrackInfo t;
		  t.clear();

		  t.add_point(x[i],y[i],z[i],layer[i]);
		  t.add_point(x[j],y[j],z[j],layer[j]);


		  t.regression();
		  // DEBUG_PRINT(" (%d,%f,%f,%f)  (%d,%f,%f,%f) ==> %f %f %f %f  \n",layer[i],x[i],y[i],z[i],layer[j],x[j],y[j],z[j],t.ax_,t.bx_,t.ay_,t.by_);
		  for (uint32_t k=0;k<nstub;k++)
		    {
		      if (used[k]) continue;
		      if (layer[k]==ip1) continue;
		      if (layer[k]==ip2) continue;
		      
		      if (abs(t.closestApproach(x[k],y[k],z[k]))<2.)
			{
			  // 3 hits on tag
			  used[i]=true;
			  used[j]=true;
			  used[k]=true;
			  t.add_point(x[k],y[k],z[k],layer[k]);
			  t.regression();
			}
		    }
		  
		  if (t.size()>=3)
		    {
		      //t.tagPoints();
		      tracks.push_back(t);
		    }
		}
	    }
	}
    }
  theTrackVector_.clear();
  for (std::vector<TrackInfo>::iterator itk=tracks.begin();itk!=tracks.end();itk++)
    {
      if (itk->size()<3) {
	// DEBUG_PRINT("small tk %ld \n",itk->getList().size());
	continue;
      }
      itk->regression();
     
     //  for (int jp=0;jp<51;jp++)
// 	if (itk->plane(jp))
// 	   DEBUG_PRINT("%.1d",1);
// 	else
// 	   DEBUG_PRINT("%.1d",0);
//        DEBUG_PRINT("\n");
      // DEBUG_PRINT("tracks %d %f %f \n",itk->getList().size(),itk->zmin_,itk->zmax_);
      float xmin=itk->xext(itk->zmin());
      float xmax=itk->xext(itk->zmax());
      float ymin=itk->yext(itk->zmin());
      float ymax=itk->yext(itk->zmax());
      float l=sqrt((xmax-xmin)*(xmax-xmin)+(ymax-ymin)*(ymax-ymin)+(itk->zmax()-itk->zmin())*(itk->zmax()-itk->zmin()));
      // DEBUG_PRINT(" l %f \n ",l);
      theLength_+=l;
      //itk->Print();
      theTrackVector_.push_back((*itk));
    }

}




void ComputerTrack::telescope(uint32_t nstub,float* x,float* y,float* z,uint32_t* layer,uint32_t nplans)
{

  std::vector<TemplateTk<GeoPoint> > tracks;
  /*
  std::map<uint32_t,std::vector<GeoPoint> > chmap;
  for (uint32_t i=0;i<nstub;i++)
    {
      if (layer[i]>nplans) continue;
      GeoPoint p(layer[i],x[i],y[i],z[i]);
      //  DEBUG_PRINT("%d %d %f %f %f \n",i,layer[i],x[i],y[i],z[i]);
      p.setUsed(false);
      std::map<uint32_t,std::vector<GeoPoint> >::iterator ich=chmap.find(layer[i]);
      if (ich==chmap.end())
	{
	  std::vector<GeoPoint> v;
	  v.push_back(p);
	  std::pair<uint32_t,std::vector<GeoPoint> > pm(layer[i],v);
	  chmap.insert(pm);
	}
      else
	ich->second.push_back(p);
    }
  */
  GeoPoint* gp[1024];
  for (int i=0;i<nstub;i++)
    gp[i]=new GeoPoint(layer[i],x[i],y[i],z[i]);
  bool used[1024];memset(used,0,1024*sizeof(bool));
  for (uint32_t ip1=1;ip1<=nplans/2;ip1++)
    {
      for (uint32_t ip2=nplans;ip2>nplans/2;ip2--)
	{
	  if (ip2==ip1) continue;
	  for (uint32_t i=0;i<nstub;i++)
	    {
	      if (used[i]) continue;
	      if (layer[i]!=ip1) continue;

	    
	      for (uint32_t j=0;j<nstub;j++)
		{
		  if (used[j]) continue;
		  if (layer[j]!=ip2) continue;
		  TemplateTk<GeoPoint> t;
		  t.planes_.reset();

		  t.addPoint((*gp[i]));
		  t.addPoint((*gp[j]));

		  t.planes_[layer[i]]=1;
		  t.planes_[layer[j]]=1;
		  t.regression();
		  // DEBUG_PRINT(" (%d,%f,%f,%f)  (%d,%f,%f,%f) ==> %f %f %f %f  \n",layer[i],x[i],y[i],z[i],layer[j],x[j],y[j],z[j],t.ax_,t.bx_,t.ay_,t.by_);
		  for (uint32_t k=0;k<nstub;k++)
		    {
		      if (used[k]) continue;
		      if (layer[k]==ip1) continue;
		      if (layer[k]==ip2) continue;
		      
		      if (t.calculateDistance((*gp[k]))<2.)
			{
			  // 3 hits on tag
			  used[i]=true;
			  used[j]=true;
			  used[k]=true;
			  t.addPoint((*gp[k]));

			  t.planes_[layer[k]]=1;
			  t.regression();
			}
		    }
		  
		  if (t.getNumberOfHits()>=3)
		    {
		      //t.tagPoints();
		      tracks.push_back(t);
		    }
		}
	    }
	}
    }
  theCandidateVector_.clear();
  theTrackVector_.clear();
  for (std::vector<TemplateTk<GeoPoint> >::iterator itk=tracks.begin();itk!=tracks.end();itk++)
    {
      if (itk->getNumberOfHits()<3) {
	// DEBUG_PRINT("small tk %ld \n",itk->getList().size());
	continue;
      }
      itk->regression();
      // DEBUG_PRINT("tracks %d %f %f \n",itk->getList().size(),itk->zmin_,itk->zmax_);
      float xmin=itk->getXext(itk->zmin_);
      float xmax=itk->getXext(itk->zmax_);
      float ymin=itk->getYext(itk->zmin_);
      float ymax=itk->getYext(itk->zmax_);
      float l=sqrt((xmax-xmin)*(xmax-xmin)+(ymax-ymin)*(ymax-ymin)+(itk->zmax_-itk->zmin_)*(itk->zmax_-itk->zmin_));
      // DEBUG_PRINT(" l %f \n ",l);
      theLength_+=l;
      //itk->Print();
        RecoCandTk tk;
	tk.planes_=itk->planes_;
	tk.ax_=itk->ax_;
	tk.bx_=itk->bx_;
	tk.ay_=itk->ay_;
	tk.by_=itk->by_;
	tk.np_=itk->getNumberOfHits();
	tk.zmin_=itk->zmin_;
	tk.zmax_=itk->zmax_;
	tk.chi2_=itk->chi2_;
	theCandidateVector_.push_back(tk);


	TrackInfo t;
	uint32_t np=0;
	for (std::vector<GeoPoint*>::iterator ip=itk->getList().begin();ip!=itk->getList().end();ip++)
	  {
	    t.set_x(np,(*ip)->X());
	    t.set_y(np,(*ip)->Y());
	    t.set_z(np,(*ip)->Z());
	    t.set_layer(np,(*ip)->chamber());
	    // DEBUG_PRINT("%d %f %f %f \n",np,(*ip)->X(),(*ip)->Y(),(*ip)->Z());
	    np++;
	  }
	t.set_size(np);
	t.regression();
	// DEBUG_PRINT(" l %f \n ",t.ax());
	theTrackVector_.push_back(t);
    }
  for (int i=0;i<nstub;i++)
    delete gp[i];

}
 
void ComputerTrack::associate(uint32_t nstub,float* x,float* y,float* z,uint32_t* layer)
{
  std::map<uint32_t,std::vector<GeoPoint> > chmap;
  std::vector<TemplateTk<GeoPoint> > tracks;
  for (uint32_t i=0;i<nstub;i++)
    {
      if (layer[i]>50) continue;
      GeoPoint p(layer[i],x[i],y[i],z[i]);
      //  DEBUG_PRINT("%d %d %f %f %f \n",i,layer[i],x[i],y[i],z[i]);
      p.setUsed(false);
      std::map<uint32_t,std::vector<GeoPoint> >::iterator ich=chmap.find(layer[i]);
      if (ich==chmap.end())
	{
	  std::vector<GeoPoint> v;
	  v.push_back(p);
	  std::pair<uint32_t,std::vector<GeoPoint> > pm(layer[i],v);
	  chmap.insert(pm);
	}
      else
	ich->second.push_back(p);
    }

  // Calculate PC

  TH1F* hweight=new TH1F("Weight","weight",100,-0.1,0.9);
  TH2F* hxy=new TH2F("xz","xz",150,0.,150.,120,-10.,110.);
  TH2F* hxyb=new TH2F("xzb","xzb",150,0.,150.,120,-10.,110.);
  for (uint32_t ipl=0;ipl<64;ipl++)
    {
      std::map<uint32_t,std::vector<GeoPoint> >::iterator ich=chmap.find(ipl);
      if (ich==chmap.end()) continue;
      for (std::vector<GeoPoint>::iterator ip=ich->second.begin();ip!=ich->second.end();ip++)
	{
	  int izmin=ipl-2;
	  int izmax=ipl+2;
	  if (ipl==0) {izmin=0;izmax=4;}
	  if (ipl==1) {izmin=0;izmax=4;}
	  izmin=ipl-2;
	  izmax=ipl+2;
	  if (ipl==0) {izmin=0;izmax=4;}
	  if (ipl==1) {izmin=0;izmax=4;}
	  std::vector<GeoPoint*> vnear_;vnear_.clear();
	  for (int32_t iz=izmin;iz<=izmax;iz++)
	    {
	      if (iz<0) continue;
	      std::map<uint32_t,std::vector<GeoPoint> >::iterator ichn=chmap.find(iz);
	      if (ichn==chmap.end()) continue;
	      for (std::vector<GeoPoint>::iterator ipn=ichn->second.begin();ipn!=ichn->second.end();ipn++)
		{
		  float dx=ip->X()-ipn->X(),dy=ip->Y()-ipn->Y(),dz=ip->Z()-ipn->Z();
		  // if (2*(abs(dx)+abs(dy))+abs(dz)/2.8>7) continue; // On etait a 7
		  if (dz==0 && sqrt(dx*dx+dy*dy)>6) continue;
		  /////if (sqrt(dx*dx+dy*dy+dz*dz)>15) continue;
		  if (sqrt(dx*dx+dy*dy+dz*dz)>9) continue;
		  //ip->addNearby(&(*ipn),12.);
		  vnear_.push_back(&(*ipn));
		}
	    }
	  ip->calculateComponents(vnear_);
	  Components* c=(Components*) ip->Components();
	  double w=0;
	  if (c->l2!=0) w=sqrt((c->l1)/c->l2);
	  // DEBUG_PRINT("Voisins %ld l2=> %f %f %f  w=> %f\n",vnear_.size(),c->l0,c->l1,c->l2,w);
	  if (hweight!=NULL) hweight->Fill(w);
	  if (w<0.3 && w!=0) 
	    hxy->Fill(ip->Z(),ip->X());
	  else
	    if (w!=0)
	      hxyb->Fill(ip->Z(),ip->X());
	  c->ymax=w;
	}
    }
  // Loop on plane
#define FORWARD
#ifdef FORWARD
  for (uint32_t ipl=0;ipl<64;ipl++)
    {
      std::map<uint32_t,std::vector<GeoPoint> >::iterator ich=chmap.find(ipl);
      if (ich==chmap.end()) continue;
      for (std::vector<GeoPoint>::iterator ip=ich->second.begin();ip!=ich->second.end();ip++)
	{
	  // first loop on tracks 
	  Components* c=(Components*) ip->Components();
	  ////@ Un test if (c->ymax>0.35) continue;
	  std::vector<TemplateTk<GeoPoint> >::iterator itk_assoc=tracks.end();
	  float distmax=9999;
	  for (std::vector<TemplateTk<GeoPoint> >::iterator itk=tracks.begin();itk!=tracks.end();itk++)
	    {
	      if (itk->getList().size()<2) continue;
	      if (itk->zmax_<ip->Z()-6) continue;
	      if (itk->zmax_>ip->Z()) continue;
	      if (itk->getList().size()>=2)
		{
		  float dx=(itk->getXext(ip->Z())-ip->X()),dy=(itk->getYext(ip->Z())-ip->Y());
		  float dist=sqrt(dx*dx+dy*dy);
		  float err=5*0.5/sqrt(itk->getList().size());
		  //err=4.;
		  if (dist<distmax && dist<err) //4 before
		    {
		      distmax=dist;itk_assoc=itk;
		    }
		}
	    }
	  if (itk_assoc==tracks.end())
	    for (std::vector<TemplateTk<GeoPoint> >::iterator itk=tracks.begin();itk!=tracks.end();itk++)
	      {
		if (itk->getList().size()!=1) continue;
		if (itk->zmax_<ip->Z()-3) continue;
		if (itk->zmax_>=ip->Z()) continue;
		if (itk->getList().size()==1)
		  {
		    
		    
		    Components* c=(Components*) itk->getList()[0]->Components();
		    if (c->l2>0)
		      {
		    	RecoCandTk t;
		    	t.ax_ =c->v2x/c->v2z;
		    	t.ay_ =c->v2y/c->v2z;
		    	t.bx_=c->xb-t.ax_*c->zb;
		    	t.by_=c->yb-t.ay_*c->zb;
		    	float dx=(t.getXext(ip->Z())-ip->X()),dy=(t.getYext(ip->Z())-ip->Y());
		    	float dist=sqrt(dx*dx+dy*dy);
			// DEBUG_PRINT("%f (%f,%f,%f) \n",c->l2,c->v2x,c->v2y,c->v2z);
		    	if (dist<distmax && dist<6) //4 before
		    	  {
		    	    distmax=dist;itk_assoc=itk;
		    	  }
		      }
		     else
		      {
			float dx=(itk->getList()[0]->X()-ip->X());
			float dy=(itk->getList()[0]->Y()-ip->Y());
			float dz=(itk->getList()[0]->Z()-ip->Z());
			float dist=sqrt(dx*dx+dy*dy+dz*dz);
			if (dist<6 && dist<distmax) {distmax=dist;itk_assoc=itk;} // coupure a 6 avant

		      }
		  }
		}
	  if (itk_assoc!=tracks.end())
	    {
	      itk_assoc->addPoint((*ip));
	      itk_assoc->regression();
	    }
	  else
	    {
	      TemplateTk<GeoPoint> t;
	      t.addPoint((*ip));
	      tracks.push_back(t);
	    }
	}
	
    }
#else
  for (int32_t ipl=63;ipl>=0;ipl--)
    {
      std::map<uint32_t,std::vector<GeoPoint> >::iterator ich=chmap.find(ipl);
      if (ich==chmap.end()) continue;
      for (std::vector<GeoPoint>::iterator ip=ich->second.begin();ip!=ich->second.end();ip++)
	{
	  // first loop on tracks 
	  std::vector<TemplateTk<GeoPoint> >::iterator itk_assoc=tracks.end();
	  float distmax=9999;
	  for (std::vector<TemplateTk<GeoPoint> >::iterator itk=tracks.begin();itk!=tracks.end();itk++)
	    {
	      if (itk->zmin_>ip->Z()+6) continue;
	      if (itk->zmin_<ip->Z()) continue;
	      if (itk->getList().size()>=2)
		{
		  float dx=(itk->getXext(ip->Z())-ip->X()),dy=(itk->getYext(ip->Z())-ip->Y());
		  float dist=sqrt(dx*dx+dy*dy);
		  if (dist<distmax && dist<5*0.5/sqrt(itk->getList().size())) //4 before
		    {
		      distmax=dist;itk_assoc=itk;
		    }
		}
	    }
	  if (distmax==9999)
	    for (std::vector<TemplateTk<GeoPoint> >::iterator itk=tracks.begin();itk!=tracks.end();itk++)
	      {
		if (itk->zmin_>ip->Z()+6) continue;
		if (itk->zmin_<=ip->Z()) continue;
		if (itk->getList().size()==1)
		  {
		    float dx=(itk->getList()[0]->X()-ip->X());
		    float dy=(itk->getList()[0]->Y()-ip->Y());
		    float dz=(itk->getList()[0]->Z()-ip->Z());
		    float dist=sqrt(dx*dx+dy*dy+dz*dz);
		    if (dist<16 && dist<distmax) {distmax=dist;itk_assoc=itk;} // coupure a 6 avant
		  }
		}
	  if (itk_assoc!=tracks.end())
	    {

	      itk_assoc->addPoint((*ip));
	      itk_assoc->regression();
	    }
	  else
	    {
	      TemplateTk<GeoPoint> t;

	      t.addPoint((*ip));
	      tracks.push_back(t);
	    }
	}
	
    }

#endif
    for (std::vector<TemplateTk<GeoPoint> >::iterator itk=tracks.begin();itk!=tracks.end();)
    {
      if (itk->getNumberOfHits()>=2) 
	{
	  std::vector<GeoPoint*> v=itk->getList();
	  for (std::vector<GeoPoint*>::iterator ip=v.begin();ip!=v.end();ip++)
	    (*ip)->setUsed(true);
	  ++itk;
	}
      else
	{
	  //++itk;
	  tracks.erase(itk);
	}
    }


    // Now try to add non associated
    uint32_t nuna=0,nass=0;
    for (int32_t ipl=64;ipl>=0;ipl--)
      {
	std::map<uint32_t,std::vector<GeoPoint> >::iterator ich=chmap.find(ipl);
	if (ich==chmap.end()) continue;
	for (std::vector<GeoPoint>::iterator ip=ich->second.begin();ip!=ich->second.end();ip++)
	  {
	    if (ip->isUsed()) continue;
	    float distmax=9999.; std::vector<TemplateTk<GeoPoint> >::iterator itmax=tracks.end();
	    for (std::vector<TemplateTk<GeoPoint> >::iterator itk=tracks.begin();itk!=tracks.end();itk++)
	      {
		if (itk->getNumberOfHits()<2) continue; 
		if (itk->zmin_>ip->Z()+3) continue;
		if (itk->zmax_<ip->Z()-3) continue;
		float dx=(itk->getXext(ip->Z())-ip->X());
		float dy=(itk->getYext(ip->Z())-ip->Y());

		float dist=sqrt(dx*dx+dy*dy);
		if (dist< 5*0.5/sqrt(itk->getList().size()) && dist<distmax) {distmax=dist;itmax=itk;}
	      }
	    if (itmax!=tracks.end())
	      {
		(*ip).setUsed(true);
		itmax->addPoint((*ip));
		itmax->regression();
		nass++;
	      }
	    else
	      nuna++;
	  }
      }
    for (std::vector<TemplateTk<GeoPoint> >::iterator itk=tracks.begin();itk!=tracks.end();)
    {
      if (itk->getNumberOfHits()>=3 && abs(itk->zmax_-itk->zmin_)>3*2.7) // 3 avant
	{
	  std::vector<GeoPoint*> v=itk->getList();
	  for (std::vector<GeoPoint*>::iterator ip=v.begin();ip!=v.end();ip++)
	    (*ip)->setUsed(true);
	  ++itk;
	}
      else
	{
	  //++itk;
	  tracks.erase(itk);
	}
    }

    //     DEBUG_PRINT("All %d  unassociated cluster %d %d\n",nstub,nuna,nass);
  TH2F* g_houghxz[100];
  TH2F* g_houghyz[100];
  uint32_t nh=0;
  theLength_=0;
  theCandidateVector_.clear();
#undef DRAW_IT
#ifdef DRAW_IT
  if (CanvasGPU1==NULL)
    {
      CanvasGPU1=new TCanvas("CanvasGPU11","hough1",800,600);
      CanvasGPU1->Divide(2,2);
      CanvasGPU1->Modified();
      CanvasGPU1->Draw();
    }
#endif
  int nin=0;
  for (std::vector<TemplateTk<GeoPoint> >::iterator itk=tracks.begin();itk!=tracks.end();itk++)
    {
      if (itk->getNumberOfHits()<3) {
	 DEBUG_PRINT("small tk %ld \n",itk->getList().size());
	continue;
      }
      float xmin=itk->getXext(itk->zmin_);
      float xmax=itk->getXext(itk->zmax_);
      float ymin=itk->getYext(itk->zmin_);
      float ymax=itk->getYext(itk->zmax_);
      float l=sqrt((xmax-xmin)*(xmax-xmin)+(ymax-ymin)*(ymax-ymin)+(itk->zmax_-itk->zmin_)*(itk->zmax_-itk->zmin_));
       DEBUG_PRINT(" l %f \n ",l);
      theLength_+=l;
      //itk->Print();
        RecoCandTk tk;
	tk.ax_=itk->ax_;
	tk.bx_=itk->bx_;
	tk.ay_=itk->ay_;
	tk.by_=itk->by_;
	tk.zmin_=itk->zmin_;
	tk.zmax_=itk->zmax_;
	tk.chi2_=itk->chi2_;
	theCandidateVector_.push_back(tk);
	float chi2=0,ndf=0;
	// DEBUG_PRINT("Track tk: %f %f %f %f \n",itk->ax_,itk->bx_,itk->ay_,itk->by_);
	for (std::vector<GeoPoint*>::iterator ip=itk->getList().begin(); ip!=itk->getList().end();ip++)
	  {
	    if ((*ip)->isUsed())
	      {
		float dx=itk->getXext((*ip)->Z())-(*ip)->X();
		float dy=itk->getYext((*ip)->Z())-(*ip)->Y();
		//	 DEBUG_PRINT ("\t Point (%f,%f,%f) %f %f \n",(*ip)->X(),(*ip)->Y(),(*ip)->Z(),dx,dy);
		chi2+= (dx*dx+dy*dy);ndf+=2.;
	    }
	    
	}
	 DEBUG_PRINT("%f %f DIST %f\n",chi2,ndf,chi2/(ndf-4));
#ifdef DRAW_IT
      std::stringstream s;
      s<<"GPUHough1X"<<nh;
      g_houghxz[nh]=new TH2F(s.str().c_str(),s.str().c_str(),150,0.,150.,120,-10.,110.);
      s<<"y";
      g_houghyz[nh]=new TH2F(s.str().c_str(),s.str().c_str(),150,0.,150.,120,-10.,110.);

     

      for (std::vector<GeoPoint*>::iterator ip=itk->getList().begin(); ip!=itk->getList().end();ip++)
	{
	  if ((*ip)->isUsed())
	    {
	      g_houghxz[nh]->Fill((*ip)->Z(),(*ip)->X());
	      g_houghyz[nh]->Fill((*ip)->Z(),(*ip)->Y());
	      
	      nin++;
	    }

	}
      g_houghxz[nh]->SetLineColor(nh+1);
      g_houghyz[nh]->SetLineColor(nh+1);
      CanvasGPU1->cd(1);
      //  g_hough->Draw("COLZ");
      CanvasGPU1->cd(1);
      if (nh==0)
	g_houghxz[nh]->Draw("BOX");
      else
	g_houghxz[nh]->Draw("BOXSAME");
      CanvasGPU1->cd(2);
      if (nh==0)
	g_houghyz[nh]->Draw("BOX");
      else
	g_houghyz[nh]->Draw("BOXSAME");

      CanvasGPU1->Modified();
      CanvasGPU1->Draw();
#endif
      nh++;
    }
#ifdef DRAW_IT
  CanvasGPU1->cd(1);
  hxy->Draw("BOX");
 CanvasGPU1->cd(2);
  hxyb->Draw("BOX");
  CanvasGPU1->cd(3);
  hweight->Draw();
  CanvasGPU1->Modified();
  CanvasGPU1->Draw();
#endif
   DEBUG_PRINT("All %d   unassociated cluster %d %d intk %d\n",nstub,nuna,nass,nin);

  //CanvasGPU1->WaitPrimitive();
#ifdef DRAW_IT
  if ( CanvasGPU1!=NULL)
    CanvasGPU1->Update();
  //CanvasGPU->WaitPrimitive();
  char c;c=getchar();putchar(c); if (c=='.') exit(0);
  //  delete g_hough;
  for (int i=0;i<nh;i++)
    {
      delete g_houghxz[i];
      delete g_houghyz[i];
    }

#endif
  delete hweight;
  delete hxy;
  delete hxyb;
   DEBUG_PRINT("Total lengh %f\n",theLength_);

}
