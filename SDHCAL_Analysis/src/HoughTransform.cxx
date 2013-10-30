#include <HoughTransform.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TEllipse.h>
#include <sstream>
#include "RecoHit.h"
#include "RecoPoint.h"
HoughTransform::HoughTransform(double thmin,double thmax,double rmin,double rmax,uint32_t nbintheta,uint32_t nbinr) : theThetaMin_(thmin),theThetaMax_(thmax),theRMin_(rmin),theRMax_(rmax),theNbinTheta_(nbintheta),theNbinR_(nbinr)
{
  theThetaBin_=(theThetaMax_-theThetaMin_)*1./theNbinTheta_;
  theRBin_=(theRMax_-theRMin_)*1./theNbinR_;
	
  for (uint32_t i=0;i<theNbinTheta_;i++)
    {
      double theta=theThetaMin_+(i+0.5)*theThetaBin_;
      theSin_[i]=sin(theta);
      theCos_[i]=cos(theta);
      for (uint32_t j=0;j<theNbinR_;j++)
	{
	  theHoughMap_[i][j].reserve(64);
	}
    }
  theX_.reserve(1024);
  theY_.reserve(1024);
  
  this->clear();

}
void HoughTransform::initialise(double thmin,double thmax,double rmin,double rmax,uint32_t nbintheta,uint32_t nbinr)
{
  theThetaMin_=thmin;theThetaMax_=thmax;theRMin_=rmin;theRMax_=rmax;theNbinTheta_=nbintheta;theNbinR_=nbinr;
   this->clear();
  theThetaBin_=(theThetaMax_-theThetaMin_)*1./theNbinTheta_;
  theRBin_=(theRMax_-theRMin_)*1./theNbinR_;
	
  for (uint32_t i=0;i<theNbinTheta_;i++)
    {
      double theta=theThetaMin_+(i+0.5)*theThetaBin_;
      theSin_[i]=sin(theta);
      theCos_[i]=cos(theta);

    }
}
  HoughTransform::~HoughTransform(){this->clear();}
uint32_t HoughTransform::getVoteMax(){return theVoteMax_;}
void HoughTransform::addMeasurement(void* s,HoughTransform::Type type,HoughTransform::Projection proj)
{
  //theVoteMax_=0;
  double z,x;
  uint32_t chamber;
  if (type == RECOPOINT)
    {
      RecoPoint* p =(RecoPoint*) s;
      chamber=p->chamber();
      if (proj==ZX)
	{
	  z=p->Z();
	  x=p->X();
	}
      else 
	if (proj==ZY)
	{
	  z=p->Z();
	  x=p->Y();
	}
    }
  else
    if (type == RECOHIT)
      {
	RecoHit* p =(RecoHit*) s;
	chamber=p->chamber();
	if (proj==ZX)
	  {
	    z=p->Z();
	    x=p->X();
	  }
	else 
	  if (proj==ZY)
	    {
	      z=p->Z();
	      x=p->Y();
	    }
      }
  theX_.push_back(z);
  theY_.push_back(x);
  double zp=z;
  double xp=x;
	
  //printf("x %f y %f \n",zp,xp);
  for (uint32_t i=0;i<theNbinTheta_;i++)
    {
      double r = zp*theCos_[i]+xp*theSin_[i];
      //printf("R %f Cos %f \n",r,theCos_[i]);
      if (r>theRMax_ || r<theRMin_) continue;
      uint32_t ir=int(floor((r-theRMin_)/theRBin_));
      theHoughPlanes_[i][ir].set(chamber);
      theHoughMap_[i][ir].push_back(s);
      theHoughImage_[i][ir]+=1;
					
      if (theHoughImage_[i][ir]>theVoteMax_) theVoteMax_=theHoughImage_[i][ir];
    }
}
void HoughTransform::clear() 
{
  memset(theHoughImage_,0,1024*1024*sizeof(uint16_t));
  theX_.clear();
  theY_.clear();
  theVoteMax_=0;
  for (uint32_t i=0;i<1024;i++)
    for (uint32_t j=0;j<1024;j++) 
      {theHoughMap_[i][j].clear();theHoughPlanes_[i][j].reset();}
}
double HoughTransform::getTheta(int32_t i) {return theThetaMin_+(i+0.5)*theThetaBin_;}
double HoughTransform::getR(int32_t i) {return theRMin_+(i+0.5)*theRBin_;}
  uint16_t HoughTransform::getValue(uint32_t i,uint32_t j){return theHoughImage_[i][j];}

  uint16_t HoughTransform::isALocalMaximum(uint32_t i,uint32_t j,uint32_t count)
{
  if (count>0 &&  theHoughImage_[i][j]<count) return 0;
bool notmax=false;
 for (int ic=-1;ic<=1;ic++)
   for (int jc=-1;jc<=1;jc++)
     {
       if (ic+i<0) continue;
       if (ic+i>=this->getNbinTheta()) continue;
       if (jc+j<0) continue;
       if (jc+j>=this->getNbinR()) continue;
       notmax=(this->getHoughImage(i,j)<this->getHoughImage(i+ic,j+jc));
       if (notmax) break;
     }
 if (notmax) return 0;
  return theHoughImage_[i][j];
}
static TCanvas* CanvasHough=NULL;
void HoughTransform::draw(DCHistogramHandler* h,std::vector< std::pair<uint32_t,uint32_t> > *maxval)
{
  std::cout<<"On entre dans draw"<<CanvasHough<<std::endl;
  if (CanvasHough==NULL)
    {
      CanvasHough=new TCanvas("CanvasHough","hough",800,900);
      CanvasHough->Modified();
      CanvasHough->Draw();
      CanvasHough->Divide(1,2);
      TVirtualPad* pd=CanvasHough->cd(2);
      pd->Divide(2,1);
    }
  CanvasHough->cd();
  std::stringstream htname;
  htname<<"HoughTransform"<<theNbinTheta_<<"_"<<theNbinR_;
  TH2F* hhtx = (TH2F*) h->GetTH2(htname.str());
  TH2F* hx = (TH2F*) h->GetTH2("LocalInverse");
  TH2F* hox = (TH2F*) h->GetTH2("LocalImage");

  if (hhtx==NULL)
    {
      hhtx =(TH2F*)h->BookTH2(htname.str(),theNbinTheta_,theThetaMin_,theThetaMax_,theNbinR_,theRMin_,theRMax_);
    }
  else
    hhtx->Reset();

  if (hx==NULL)
    {
      hx =(TH2F*)h->BookTH2("LocalInverse",100,-0.05,0.05,100,-0.05,0.05);
      hox =(TH2F*)h->BookTH2("LocalImage",190,-10.,180.,190,-10.,180);
      hx->SetMarkerColor(4);
      hx->SetMarkerSize(.2);
      hx->SetMarkerStyle(25);
      hox->SetMarkerColor(4);
      hox->SetMarkerSize(.2);
      hox->SetMarkerStyle(25);
    }
  else
    {
      hx->Reset();
      hox->Reset();
    }
  for (int ip=0;ip<theX_.size();ip++)
    {
      double xp=theX_[ip]/(theX_[ip]*theX_[ip]+theY_[ip]*theY_[ip]);
      double yp=theY_[ip]/(theX_[ip]*theX_[ip]+theY_[ip]*theY_[ip]);
      hx->Fill(xp,yp);
      hox->Fill(theX_[ip],theY_[ip]);
		
    }
		
	
  for (uint32_t i=0;i<theNbinTheta_;i++)
    for (uint32_t j=0;j<theNbinR_;j++)
      {
	hhtx->SetBinContent(i+1,j+1,theHoughImage_[i][j]*1.);

		
      }
	
  CanvasHough->cd(1);
  hhtx->Draw("COLZ");
  //hw->Draw();
  TVirtualPad* pd=CanvasHough->cd(2);
  pd->cd(1);
	

  hox->Draw("p");
  std::vector<TLine*> vline;vline.clear();
  if (maxval!=NULL)
    {
      uint32_t il=0;
      for (std::vector < std::pair<uint32_t,uint32_t> >::iterator ihb=maxval->begin();ihb<maxval->end();ihb++)
	{
	  uint32_t ith=(*ihb).first;
	  uint32_t ir=(*ihb).second;
	  double theta=0,r=0,w=0,nb=0;
		
	  for (int ithb=-1;ithb<=1;ithb++)
	    for (int irb=-1;irb<=1;irb++)
	      {
		if ((ith+ithb)<0 || (ith+ithb)>theNbinTheta_) continue;
		if ((ir+irb)<0 || (ir+irb)>theNbinR_) continue;
		double thetab = (this->getTheta(ith+ithb)+this->getTheta(ith+ithb+1))/2;
		double rb = (this->getR(ir+irb)+this->getR(ir+irb+1))/2;
		double wb=theHoughImage_[ith+ithb][ir+irb]*1.;
		//printf("%d %d %f \n",ithb,irb,wb);
		w+=wb;
		theta+=thetab*wb;
		r+=rb*wb;
		nb+=1;
	      }
	  r=r/w;
	  theta=theta/w;
	  theta=this->getTheta(ith);
	  r=this->getR(ir);
	  double a=-1./tan(theta);
	  double b=r/sin(theta);
		
		
	  //
	  double R=1./2./TMath::Abs(r);
	  double xi=-a/2./b;
	  double yi=1./2./b;
	  printf("%f %f %f %f %f %f Rayon R= %f => %f GeV/c\n",nb,w,r,theta,a,b,R,0.3*3.8*R/100.);

	  std::vector<void*> v= this->getHoughMap(ith,ir);
	  float zmin=9999.,zmax=-9999.;
	  for(std::vector<void*>::iterator ih=v.begin();ih!=v.end();ih++)
	    {
	      RecoPoint* h=(RecoPoint*) (*ih);
	      if (h->Z()>zmax) zmax=h->Z();
	      if (h->Z()<zmin) zmin=h->Z();

	    }
	  TLine* l =new TLine(zmin,a*zmin+b,zmax,a*zmax+b);
	  l->SetLineColor(2+il);il++;
	  l->Draw("SAME");
	  vline.push_back(l);
	  if (vline.size()>5) break;
	}
    }
  std::vector<TEllipse*> vel;vel.clear();
  /*
  pd->cd(2);
  hox->Draw("p");

  if (maxval!=NULL)
    {
      for (std::vector < std::pair<uint32_t,uint32_t> >::iterator ihb=maxval->begin();ihb<maxval->end();ihb++)
	{
	  uint32_t ith=(*ihb).first;
	  uint32_t ir=(*ihb).second;
	  double theta=0,r=0,w=0,nb=0;
		
	  for (int ithb=-1;ithb<=1;ithb++)
	    for (int irb=-1;irb<=1;irb++)
	      {
		if ((ith+ithb)<0 || (ith+ithb)>theNbinTheta_) continue;
		if ((ir+irb)<0 || (ir+irb)>theNbinR_) continue;
		double thetab = (this->getTheta(ith+ithb)+this->getTheta(ith+ithb+1))/2;
		double rb = (this->getR(ir+irb)+this->getR(ir+irb+1))/2;
		double wb=theHoughImage_[ith+ithb][ir+irb]*1.;
		//printf("%d %d %f \n",ithb,irb,wb);
		w+=wb;
		theta+=thetab*wb;
		r+=rb*wb;
		nb+=1;
	      }
	  r=r/w;
	  theta=theta/w;
	  double a=-1./tan(theta);
	  double b=r/sin(theta);
		
		
	  //
	  double R=1./2./TMath::Abs(r);
	  double xi=-a/2./b;
	  double yi=1./2./b;
	  printf("%f %f %f %f %f %f Rayon R= %f => %f GeV/c\n",nb,w,r,theta,a,b,R,0.3*3.8*R/100.);
	  TEllipse* l =new TEllipse(xi,yi,R,R);
	  l->SetLineColor(3);
	  l->SetFillStyle(0);
	  l->Draw("SAME");
	  vel.push_back(l);
	  if (vel.size()>5) break;
	}
    }
*/
  CanvasHough->Modified();
  CanvasHough->Draw();

  CanvasHough->Update();
  //CanvasHough->WaitPrimitive();

  char c;c=getchar();putchar(c); if (c=='.') exit(0);
  for (std::vector<TLine*>::iterator il=vline.begin();il!=vline.end();il++) delete (*il);
  for (std::vector<TEllipse*>::iterator il=vel.begin();il!=vel.end();il++) delete (*il);

}

void HoughTransform::draw(DCHistogramHandler* h,std::vector< std::pair<double,double> > *maxval)
{
  if (CanvasHough==NULL)
    {
      CanvasHough=new TCanvas("CanvasHough","hough",800,900);
      CanvasHough->Modified();
      CanvasHough->Draw();
      CanvasHough->Divide(1,2);
      TVirtualPad* pd=CanvasHough->cd(2);
      pd->Divide(2,1);
    }
  CanvasHough->cd();
  std::stringstream htname;
  htname<<"HoughTransform"<<theNbinTheta_<<"_"<<theNbinR_;
  TH2F* hhtx = (TH2F*) h->GetTH2(htname.str());
  TH2F* hx = (TH2F*) h->GetTH2("LocalInverse");
  TH2F* hox = (TH2F*) h->GetTH2("LocalImage");

  if (hhtx==NULL)
    {
      hhtx =(TH2F*)h->BookTH2(htname.str(),theNbinTheta_,theThetaMin_,theThetaMax_,theNbinR_,theRMin_,theRMax_);
    }
  else
    hhtx->Reset();

  if (hx==NULL)
    {
      hx =(TH2F*)h->BookTH2("LocalInverse",100,-0.05,0.05,100,-0.05,0.05);
      hox =(TH2F*)h->BookTH2("LocalImage",600,-150.,150.,200,-110.,110);
      hx->SetMarkerColor(4);
      hx->SetMarkerSize(.2);
      hx->SetMarkerStyle(25);
      hox->SetMarkerColor(4);
      hox->SetMarkerSize(.2);
      hox->SetMarkerStyle(25);
    }
  else
    {
      hx->Reset();
      hox->Reset();
    }
  for (int ip=0;ip<theX_.size();ip++)
    {
      double xp=theX_[ip]/(theX_[ip]*theX_[ip]+theY_[ip]*theY_[ip]);
      double yp=theY_[ip]/(theX_[ip]*theX_[ip]+theY_[ip]*theY_[ip]);
      hx->Fill(xp,yp);
      hox->Fill(theX_[ip],theY_[ip]);
		
    }
		
	
  for (uint32_t i=0;i<theNbinTheta_;i++)
    for (uint32_t j=0;j<theNbinR_;j++)
      {
	hhtx->SetBinContent(i+1,j+1,theHoughImage_[i][j]*1.);

		
      }
	
  CanvasHough->cd(1);
  hhtx->Draw("COLZ");
  //hw->Draw();
  TVirtualPad* pd=CanvasHough->cd(2);
  pd->cd(1);
  

  hx->Draw("p");
  std::vector<TLine*> vline;vline.clear();
  if (maxval!=NULL)
    {
      for (std::vector < std::pair<double,double> >::iterator ihb=maxval->begin();ihb<maxval->end();ihb++)
	{
	  double theta=(*ihb).first;
	  double r=(*ihb).second;
	  
	  double a=-1./tan(theta);
	  double b=r/sin(theta);
	  
	  
	  //
	  double R=1./2./TMath::Abs(r);
	  double xi=-a/2./b;
	  double yi=1./2./b;
	  if (0.3*3.8*R/100.<1.5) continue;
	  printf("%f %f %f %f Rayon R= %f \n => %f GeV/c  Phi0 %f \n",r,theta,a,b,R,0.3*3.8*R/100.,atan(a));
	  
	  TLine* l =new TLine(0.,b,0.05,a*0.05+b);
	  l->SetLineColor(2);
	  l->Draw("SAME");
	  vline.push_back(l);
	  if (vline.size()>5) break;
	}
    }
  pd->cd(2);
  hox->Draw("p");
  
  
  CanvasHough->Modified();
  CanvasHough->Draw();
  //CanvasHough->WaitPrimitive();
  
  CanvasHough->Update();
  //CanvasHough->WaitPrimitive();
  char c;c=getchar();putchar(c); if (c=='.') exit(0);
  if (c=='s')
    {
      CanvasHough->SaveAs("Last.png");
    }
  for (std::vector<TLine*>::iterator il=vline.begin();il!=vline.end();il++) delete (*il);

  
}


void HoughTransform::Convert(double theta,double r,double &a,double &b)
{
   a=-1./tan(theta);
   b=r/sin(theta);
}
void HoughTransform::getPattern(int i,int j,uint32_t& nc,uint32_t& fp,uint32_t& lp)
{
std::string sn[64]={
  "1",
  "11",
  "111",
  "1111",
  "11111",
  "111111",
  "1111111",
  "11111111",
  "111111111",
  "1111111111",
  "11111111111",
  "111111111111",
  "1111111111111",
  "11111111111111",
  "111111111111111",
  "1111111111111111",
  "11111111111111111",
  "111111111111111111",
  "1111111111111111111",
  "11111111111111111111",
  "111111111111111111111",
  "1111111111111111111111",
  "11111111111111111111111",
  "111111111111111111111111",
  "1111111111111111111111111",
  "11111111111111111111111111",
  "111111111111111111111111111",
  "1111111111111111111111111111",
  "11111111111111111111111111111",
  "111111111111111111111111111111",
  "1111111111111111111111111111111",
  "11111111111111111111111111111111",
  "111111111111111111111111111111111",
  "1111111111111111111111111111111111",
  "11111111111111111111111111111111111",
  "111111111111111111111111111111111111",
  "1111111111111111111111111111111111111",
  "11111111111111111111111111111111111111",
  "111111111111111111111111111111111111111",
  "1111111111111111111111111111111111111111",
  "11111111111111111111111111111111111111111",
  "111111111111111111111111111111111111111111",
  "1111111111111111111111111111111111111111111",
  "11111111111111111111111111111111111111111111",
  "111111111111111111111111111111111111111111111",
  "1111111111111111111111111111111111111111111111",
  "11111111111111111111111111111111111111111111111",
  "111111111111111111111111111111111111111111111111",
  "1111111111111111111111111111111111111111111111111",
  "11111111111111111111111111111111111111111111111111",
  "111111111111111111111111111111111111111111111111111",
  "1111111111111111111111111111111111111111111111111111",
  "11111111111111111111111111111111111111111111111111111",
  "111111111111111111111111111111111111111111111111111111",
  "1111111111111111111111111111111111111111111111111111111",
  "11111111111111111111111111111111111111111111111111111111",
  "111111111111111111111111111111111111111111111111111111111",
  "1111111111111111111111111111111111111111111111111111111111",
  "11111111111111111111111111111111111111111111111111111111111",
  "111111111111111111111111111111111111111111111111111111111111",
  "1111111111111111111111111111111111111111111111111111111111111",
  "11111111111111111111111111111111111111111111111111111111111111",
  "111111111111111111111111111111111111111111111111111111111111111",
  "1111111111111111111111111111111111111111111111111111111111111111"};

 nc=0;fp=0;lp=0;
 nc=theHoughPlanes_[i][j].count();
 while (nc>4)
   {
     std::size_t found=theHoughPlanes_[i][j].to_string().find(sn[nc-1]);
     /*if (theHTx_->getHoughPlanes(i,j).count()<=5) found = theHTx_->getHoughPlanes(i,j).to_string().find(str4);
       if (theHTx_->getHoughPlanes(i,j).count()<=6) found = theHTx_->getHoughPlanes(i,j).to_string().find(str5);
       if (theHTx_->getHoughPlanes(i,j).count()>6) found = theHTx_->getHoughPlanes(i,j).to_string().find(str6);*/
     if (found!=std::string::npos)
       {
	 lp=63-found;
	 fp=63-(found+nc-1);
	 break;
       }
     nc--;
   }
 if (lp==0 && fp ==0) nc=0;
 return;
}
