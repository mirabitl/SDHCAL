#include <stdint.h>
#include <stdio.h> 
#include "ComputerHough.h"
#include <math.h>
#include "DCHistogramHandler.h"
#include "TCanvas.h"
#include "GeoPoint.h"
#include "TemplateTk.h"
#include "Amas.h"
//#include "TemplateTk.txx"

ComputerHough::ComputerHough(HoughCut* cuts) :theCuts_(cuts)
{
  theNStub_=0;
  theX_=NULL;
  theY_=NULL;
  theZ_=NULL;
  theLayer_=NULL;
  
  createHoughCPU(&ph_,768,3072,768);
  for (int i=0;i<96;i++)
    createHoughCPU(&phcand_[i],768,3072,768);
  for (int i=0;i<64;i++)
    createHoughCPU(&phrcand_[i]);
  

}
ComputerHough::~ComputerHough()
{
  //printf("DELTEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n");
  deleteHoughCPU(&ph_);
 for (int i=0;i<96;i++)
   deleteHoughCPU(&phcand_[i]);
  for (int i=0;i<64;i++)
    deleteHoughCPU(&phrcand_[i]);
 
}
void ComputerHough::DefaultCuts()
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
void ComputerHough::Compute(uint32_t nstub,float* x,float* y,float* z,uint32_t* layer)
{

  


  theNStub_=nstub;
  theX_=x;
  theY_=y;
  theZ_=z;  
  theLayer_=layer;
  theCandidateVector_.clear();
  // Initialisation depending on sector 
  float thmin=-PI/2,thmax=PI/2;
  float rhmin=theCuts_->RhoMin,rhmax=theCuts_->RhoMax;
  //printf("On appelle le GPU %d \n",theNStub_);
  int ntheta=theCuts_->NTheta;
  int nrho=theCuts_->NRho;//8//12//192;
  
  initialiseHoughCPU(&ph_,theNStub_,ntheta,nrho,thmin,thmax,rhmin,rhmax);
  // Rough process
  fillPositionHoughCPU(&ph_,theX_,theY_,theZ_);
  fillLayerHoughCPU(&ph_,theLayer_);
		  //clearHough(&ph);
  processHoughCPU(&ph_,theCuts_->NStubLow,theCuts_->NLayerRow,0);
  printf(" %d candidates Max val %d STubs %d\n",ph_.h_cand[0],ph_.max_val,ph_.nstub);
  dumpCPU(&ph_);
  // Precise HT filling
  uint32_t nc=(int)ph_.h_cand[0];
  if (nc>96) nc=96;
  for (int ic=0;ic<nc;ic++)
    {
      clearHoughCPU(&phcand_[ic]);
    }

  // Loop on candidate
  for (int ic=0;ic<nc;ic++)
    {
      phcand_[ic].h_reg[20]=0;
      int pattern=ph_.h_cand[ic+1]; // vcand[ic]
      int ith=pattern&0X3FF;
      int ir=(pattern>>10)&0x3FF;
      //ith=(vcand[ic])&0x3FF;
      //ir=(vcand[ic]>>10)&0x3FF;
      int ns=(pattern>>20)&0x3FF;
      if (ns<theCuts_->NStubLowCandidate) continue;//if (ns<3) continue;
      //printf("%f \n",fabs(GET_R_VALUE(ph,ir)));
      uint32_t nbinf=64;


      uint32_t nbinr=nbinf;


		  
      float ndel=theCuts_->NDelBarrel;
      float tmi=GET_THETA_VALUE(ph_,ith)-ndel*ph_.thetabin;
      
      float tma=GET_THETA_VALUE(ph_,ith)+ndel*ph_.thetabin;
      float rmi=GET_R_VALUE(ph_,ir)-ndel*ph_.rbin;
      float rma=GET_R_VALUE(ph_,ir)+ndel*ph_.rbin;
      
      initialiseHoughCPU(&phcand_[ic],theNStub_,nbinf,nbinr,tmi,tma,rmi,rma);	    

      copyPositionHoughCPU(&ph_,pattern,&phcand_[ic],0,false);
    }
		  

		
  //Precise HT processing
		 
  for (int ic=0;ic<nc;ic++)
    {
      if (phcand_[ic].h_reg[20]>0)
	{
	  phcand_[ic].nstub=int( phcand_[ic].h_reg[20]);
	  processHoughCPU(&phcand_[ic],theCuts_->NStubHigh,theCuts_->NLayerHigh,0);
	  
	}
    }

  // Finael analysis of High precision candidate

  for (int ic=0;ic<nc;ic++)
    {
      if (phcand_[ic].h_reg[20]>0)
	{
	  uint32_t nch=(int)phcand_[ic].h_cand[0];
	  if (nch>64) nch=64;
	  for (int ici=0;ici<nch;ici++)
	    {
	      int patterni=phcand_[ic].h_cand[ici+1]; 
	      int ithi=patterni&0X3FF;
	      int iri=(patterni>>10)&0x3FF;
	      
	      if (((patterni>>20)&0x3FF)<theCuts_->NStubHighCandidate) continue;


	      // RZ  & R Phi regression
	      initialiseHoughCPU(&phrcand_[ici],theNStub_,32,32,-PI/2,PI/2,-150.,150.);
	      copyPositionHoughCPU(&phcand_[ic],patterni,&phrcand_[ici],1,true);
	      phrcand_[ici].nstub=int( phrcand_[ici].h_reg[20]);

	      if ( phrcand_[ici].h_reg[20]<=0) continue;
			      
	      if ( phrcand_[ici].h_reg[70+9]<3.5) continue; //at least 4 YZ points

	      RecoCandTk tk;

	      tk.ax_=phrcand_[ici].h_reg[60+0];
	      tk.bx_=phrcand_[ici].h_reg[60+1];
	      tk.ay_=phrcand_[ici].h_reg[70+0];
	      tk.by_=phrcand_[ici].h_reg[70+1];
	      theCandidateVector_.push_back(tk);

			      
	    }
	}
    }
		 









		  
		
		  
  //  printf("Fin du CPU %ld \n",	theCandidateVector_.size() );



}
void ComputerHough::ComputeOneShot(uint32_t nstub,float* x,float* y,float* z,uint32_t* layer)
{
  theNStub_=nstub;
  theX_=x;
  theY_=y;
  theZ_=z;  
  theLayer_=layer;
  theCandidateVector_.clear();
  // Initialisation depending on sector 
 
  float thmin=0,thmax=PI;
  float rhmin=0.,rhmax=150.;
  //printf("On appelle le GPU %d \n",theNStub_);
  int ntheta=theCuts_->NTheta;
  int nrho=theCuts_->NRho;//8//12//192;
  //initialiseHough(&ph,gpu_nstub,ntheta,nrho,-PI/2,PI/2,-0.06,0.06);
  
  ntheta=128;
  nrho=128;
  theCuts_->NLayerRow=5;
  theCuts_->NStubLow=5;
  theCuts_->NStubLowCandidate=5;
  initialiseHoughCPU(&ph_,theNStub_,ntheta,nrho,thmin,thmax,rhmin,rhmax);
  // Rough process
  fillPositionHoughCPU(&ph_,theX_,theY_,theZ_);
  fillLayerHoughCPU(&ph_,theLayer_);
		  //clearHough(&ph);
  processHoughCPU(&ph_,6,5,0);
  //printf("SECTOR %d gives %d candidates Max val %d STubs %d\n",isel,ph_.h_cand[0],ph_.max_val,ph_.nstub);
  // Precise HT filling
  uint32_t nc=(int)ph_.h_cand[0];
  if (nc>512) nc=512;
  

  // Loop on candidate
  for (int ic=0;ic<nc;ic++)
    {
      clearHoughCPU(&phcand_[0]);

      phcand_[0].h_reg[20]=0;
      int pattern=ph_.h_cand[ic+1]; // vcand[ic]
      int ith=pattern&0X3FF;
      int ir=(pattern>>10)&0x3FF;
      //ith=(vcand[ic])&0x3FF;
      //ir=(vcand[ic]>>10)&0x3FF;
      int ns=(pattern>>20)&0x3FF;
      if (ns<6) continue;//if (ns<3) continue;
      // RZ  & R Phi regression

      initialiseHoughCPU(&phcand_[0],theNStub_,ntheta,nrho,thmin,thmax,rhmin,rhmax);
  
      copyPositionHoughCPU(&ph_,pattern,&phcand_[0],1,false);
      phcand_[0].nstub=int( phcand_[0].h_reg[20]);
      //dumpCPU(&phcand_[0]);

      printf(" Candidat %d => nstub %d \n", ic,phcand_[0].nstub);
      processHoughCPU(&phcand_[0],6,5,1);


      //dumpCPU(&phcand_[0]);
      //getchar();
      if (phcand_[0].h_cand[0]<1) continue;
      int nc1=(int) phcand_[0].h_cand[0];
      for (int ic1=0;ic1<nc1;ic1++)
	{

	  phcand_[1].h_reg[20]=0;

	  int pattern1=phcand_[0].h_cand[ic1+1]; // vcand[ic]
	  int ith1=pattern1&0X3FF;
	  int ir1=(pattern1>>10)&0x3FF;
	  int ns1=(pattern1>>20)&0x3FF;
	  if (ns1<5) continue;
	   initialiseHoughCPU(&phcand_[1],theNStub_,ntheta,nrho,thmin,thmax,rhmin,rhmax);
  
	   copyPositionHoughCPU(&phcand_[0],pattern1,&phcand_[1],1,true);
	   RecoCandTk tk;
	   drawphcand(1);
	   printf("%d stubs --------------------------> %d candidats \n",phcand_[0].nstub,int(phcand_[1].h_reg[20]));
	  tk.ax_=phcand_[1].h_reg[60+0];
	  tk.bx_=phcand_[1].h_reg[60+1];
	  tk.ay_=phcand_[1].h_reg[70+0];
	  tk.by_=phcand_[1].h_reg[70+1];
	  theCandidateVector_.push_back(tk);
	}
      
      
    }
}
static TCanvas* CanvasGPU1=NULL;
void ComputerHough::drawphcand(uint32_t ic)
{

  unsigned int h_hough[phcand_[ic].ntheta*phcand_[ic].nrho];

  copyHoughImageCPU(&phcand_[ic],h_hough);

  //TH2F* g_hough=(TH2F*) rootHandler_->GetTH2("GPUHough");
  printf("drawph==> %d %f %f %d %f %f \n",phcand_[ic].ntheta,phcand_[ic].thetamin,phcand_[ic].thetamax,phcand_[ic].nrho,phcand_[ic].rmin,phcand_[ic].rmax);
  TH2F* g_hough=new TH2F("GPUHough1","GPUHough1",phcand_[ic].ntheta,phcand_[ic].thetamin,phcand_[ic].thetamax,phcand_[ic].nrho,phcand_[ic].rmin,phcand_[ic].rmax);
  TH2F* g_houghxz=new TH2F("GPUHough1X","GPUHough1X",150,0.,150.,120,-10.,110.);
  TH2F* g_houghyz=new TH2F("GPUHough1Y","GPUHough1Y",150,0.,150.,120,-10.,110.);
  for (int i=0;i<phcand_[ic].nstub;i++)
    {
      g_houghxz->Fill(phcand_[ic].d_z[i],phcand_[ic].d_x[i]);
      g_houghyz->Fill(phcand_[ic].d_z[i],phcand_[ic].d_y[i]);
    }
  for (int ith=0;ith<phcand_[ic].ntheta;ith++)
    for (int ir=0;ir<phcand_[ic].nrho;ir++)
      if (h_hough[ith*phcand_[ic].nrho+ir]!=0)
	g_hough->SetBinContent(ith+1,ir+1,h_hough[ith*phcand_[ic].nrho+ir]*1.);
  if (CanvasGPU1==NULL)
    {
      CanvasGPU1=new TCanvas("CanvasGPU11","hough1",800,900);
      CanvasGPU1->Divide(2,2);
      CanvasGPU1->Modified();
      CanvasGPU1->Draw();
    }
  CanvasGPU1->cd(1);
  g_hough->Draw("COLZ");
  CanvasGPU1->cd(3);
  g_houghxz->Draw("BOX");
  CanvasGPU1->cd(4);
  g_houghyz->Draw("BOX");
  CanvasGPU1->Modified();
  CanvasGPU1->Draw();
  //CanvasGPU1->WaitPrimitive();
  
  CanvasGPU1->Update();
  //CanvasGPU->WaitPrimitive();
  char c;c=getchar();putchar(c); if (c=='.') exit(0);
  delete g_hough;
  delete g_houghxz;
  delete g_houghyz;
}
 
void ComputerHough::associate(uint32_t nstub,float* x,float* y,float* z,uint32_t* layer)
{
  std::map<uint32_t,std::vector<GeoPoint> > chmap;
  std::vector<TemplateTk<GeoPoint> > tracks;
  for (uint32_t i=0;i<nstub;i++)
    {
      if (layer[i]>50) continue;
      GeoPoint p(layer[i],x[i],y[i],z[i]);
      // printf("%d %d %f %f %f \n",i,layer[i],x[i],y[i],z[i]);
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
	  std::vector<GeoPoint*> vnear_;vnear_.clear();
	  for (int32_t iz=izmin;iz<=izmax;iz++)
	    {
	      if (iz<0) continue;
	      std::map<uint32_t,std::vector<GeoPoint> >::iterator ichn=chmap.find(iz);
	      if (ichn==chmap.end()) continue;
	      for (std::vector<GeoPoint>::iterator ipn=ichn->second.begin();ipn!=ichn->second.end();ipn++)
		{
		  float dx=ip->X()-ipn->X(),dy=ip->Y()-ipn->Y(),dz=ip->Z()-ipn->Z();
		  if (2*(abs(dx)+abs(dy))+abs(dz)/2.8>7) continue;
		  //ip->addNearby(&(*ipn),12.);
		  vnear_.push_back(&(*ipn));
		}
	    }
	  ip->calculateComponents(vnear_);
	  Components* c=(Components*) ip->Components();
	  double w=0;
	  if (c->l2!=0) w=sqrt((c->l1)/c->l2);
	  //printf("Voisins %ld l2=> %f  w=> %f\n",ip->Voisins().size(),c->l2,w);
	  if (hweight!=NULL) hweight->Fill(w);
	  if (w<0.35) hxy->Fill(ip->Z(),ip->X());
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
	  if (c->ymax>0.35) continue;
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
			//printf("%f (%f,%f,%f) \n",c->l2,c->v2x,c->v2y,c->v2z);
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
      if (itk->getNumberOfHits()>=3 && abs(itk->zmax_-itk->zmin_)>3*2.7) 
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

    //    printf("All %d  unassociated cluster %d %d\n",nstub,nuna,nass);
  TH2F* g_houghxz[100];
  TH2F* g_houghyz[100];
  uint32_t nh=0;
  theLength_=0;
  theCandidateVector_.clear();
#undef DRAW_IT
#ifdef DRAW_IT
  if (CanvasGPU1==NULL)
    {
      CanvasGPU1=new TCanvas("CanvasGPU11","hough1",800,400);
      CanvasGPU1->Divide(2,1);
      CanvasGPU1->Modified();
      CanvasGPU1->Draw();
    }
#endif
  int nin=0;
  for (std::vector<TemplateTk<GeoPoint> >::iterator itk=tracks.begin();itk!=tracks.end();itk++)
    {
      if (itk->getNumberOfHits()<3) {
	printf("small tk %ld \n",itk->getList().size());
	continue;
      }
      float xmin=itk->getXext(itk->zmin_);
      float xmax=itk->getXext(itk->zmax_);
      float ymin=itk->getYext(itk->zmin_);
      float ymax=itk->getYext(itk->zmax_);
      float l=sqrt((xmax-xmin)*(xmax-xmin)+(ymax-ymin)*(ymax-ymin)+(itk->zmax_-itk->zmin_)*(itk->zmax_-itk->zmin_));
      printf(" l %f \n ",l);
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
	//printf("Track tk: %f %f %f %f \n",itk->ax_,itk->bx_,itk->ay_,itk->by_);
	for (std::vector<GeoPoint*>::iterator ip=itk->getList().begin(); ip!=itk->getList().end();ip++)
	  {
	    if ((*ip)->isUsed())
	      {
		float dx=itk->getXext((*ip)->Z())-(*ip)->X();
		float dy=itk->getYext((*ip)->Z())-(*ip)->Y();
		//	printf ("\t Point (%f,%f,%f) %f %f \n",(*ip)->X(),(*ip)->Y(),(*ip)->Z(),dx,dy);
		chi2+= (dx*dx+dy*dy);ndf+=2.;
	    }
	    
	}
	printf("%f %f DIST %f\n",chi2,ndf,chi2/(ndf-4));
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
  hweight->Draw();
  CanvasGPU1->Modified();
  CanvasGPU1->Draw();
#endif
  printf("All %d   unassociated cluster %d %d intk %d\n",nstub,nuna,nass,nin);

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
  printf("Total lengh %f\n",theLength_);

}
