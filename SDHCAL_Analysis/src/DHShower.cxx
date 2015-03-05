#include "UtilDefs.h"
#include "DHShower.h"
#include "RecoHit.h"

#include <Eigen/Dense>
#include "TPolyLine3D.h"
#include "TVirtualPad.h"
using namespace Eigen;
#include "DCHistogramHandler.h"

#include <limits.h>
#include <sys/timeb.h>
#include <float.h>

void DHShower::transverseProfile(uint32_t plan,uint32_t &nh,double &xb,double &yb, double &l0, double &l1,double* v0,double *v1,double &n9,double &n25)
{
  nh=0;xb=0;yb=0;l0=0;l1=0;
  v0[0]=0;v0[1]=0;v0[0]=0;v0[1]=0;
  std::map<uint32_t,std::vector<RecoHit*> >::iterator ipl=thePlans_.find(plan);
  if (ipl==thePlans_.end()) return;
  double wt=0.;


  for (std::vector<RecoHit*>::iterator iht=ipl->second.begin();iht!=ipl->second.end();iht++)
    {
      double w=1.;
      int ithr= (*iht)->getAmplitude()&0x3;
      if (ithr==2) w=1;
      if (ithr==1) w=1;
      if (ithr==3) w=2.;
      xb+=(*iht)->X()*w;
      yb+=(*iht)->Y()*w;
      wt+=w;
      nh++;
    }
  xb=xb/wt;
  yb=yb/wt;
  if (nh<3) return;
  //DEBUG_PRINT("1\n");
  Matrix<double,Dynamic,2> m(nh,2);
  //DEBUG_PRINT("2\n");
  Matrix<double,2,Dynamic> mt(2,nh);
  Matrix<double,Dynamic,Dynamic> D(nh,nh);
  for (int i=0;i<nh;i++)
    for (int j=0;j<nh;j++)
      D(i,j)=0.;
  //DEBUG_PRINT("3 %d\n",nh);


  nh=0;
  wt=0.;
  //firstPlan_=99;
  n9=0;
  n25=0;
  for (std::vector<RecoHit*>::iterator iht=ipl->second.begin();iht!=ipl->second.end();iht++)
    {
      if (abs((*iht)->X()-xb)<2 && abs((*iht)->Y()-yb)<2 ) n9=n9+1;
      if (abs((*iht)->X()-xb)<3 && abs((*iht)->Y()-yb)<3 ) n25=n25+1;
      m(nh,0)=(*iht)->X()-xb;
      mt(0,nh) =(*iht)->X()-xb;
      m(nh,1)=(*iht)->Y()-yb;
      mt(1,nh) =(*iht)->Y()-yb;
      int ithr= (*iht)->getAmplitude()&0x3;
      double w=1.;
      if (ithr==2) w=1;
      if (ithr==1) w=1;
      if (ithr==3) w=2.;
      //w=1.;
      D(nh,nh)=w;
      wt+=w;
      nh++;
    }
	
  //DEBUG_PRINT("4\n");
  D *=1./wt;
  //std::cout<<" Here it is "<<std::endl<<D<<std::endl;

  Matrix<double,Dynamic,Dynamic> V(nh,nh);
  // Matrix<double,Dynamic,Dynamic> V1(nh,3);
  //DEBUG_PRINT("5\n");
  //V1= D*m;
  //std::cout<<" Here it is "<<std::endl<<V1<<std::endl;
  //DEBUG_PRINT("=6\n");
  V=mt*D*m;
  //std::cout<<" Here it is "<<std::endl<<V<<std::endl;
  // DEBUG_PRINT("7\n");
  //V *=1./nh;
  //std::cout<<" Here it is "<<std::endl<<V<<std::endl;


  SelfAdjointEigenSolver< Matrix<double,Dynamic,Dynamic> > eigensolver(V);
  if (eigensolver.info() != Success) abort();
  // std::cout << "The eigenvalues of V are:\n" << eigensolver.eigenvalues() << std::endl;
  // std::cout << "Here's a matrix whose columns are eigenvectors of A \n"
  // 	    << "corresponding to these eigenvalues:\n"
  // 	    << eigensolver.eigenvectors() << std::endl;

  Matrix<double,2,1> va=eigensolver.eigenvalues();
  l0=va(0);
  l1=va(1);


  Matrix<double,2,2> vv=eigensolver.eigenvectors();
  v0[0]=vv(0,0);
  v0[1]=vv(1,0);
  v1[0]=vv(0,1);
  v1[1]=vv(1,1);



  return;


}
void DHShower::PlayMatrix(uint32_t fp,uint32_t lp)
{
  uint32_t nh=0;
  double xb=0,yb=0,zb=0;
  double wt=0.;
  theAmas_.clear();

  for (std::map<uint32_t,std::vector<RecoHit*> >::iterator ipl=thePlans_.begin();ipl!=thePlans_.end();ipl++)
    {
      if (ipl->first<fp || ipl->first>lp) continue;
      for (std::vector<RecoHit*>::iterator iht=ipl->second.begin();iht!=ipl->second.end();iht++)
	{
	  double w=1.;
	  int ithr= (*iht)->getAmplitude()&0x3;
	  if (ithr==2) w=1;
	  if (ithr==1) w=1;
	  if (ithr==3) w=2.;
	  xb+=(*iht)->X()*w;
	  yb+=(*iht)->Y()*w;
	  zb+=(*iht)->Z()*w;
	  wt+=w;
	  nh++;

	  if ((*iht)->getFlag(RecoHit::CORE))
	    {
	      bool appended=false;
	      for (std::vector<Amas>::iterator ia=theAmas_.begin();ia!=theAmas_.end();ia++)
		  {
		    appended=(ia->append((*iht),2));
		    if (appended) break;
		  }
		if (!appended)
		  {
		    Amas a((*iht));
		    theAmas_.push_back(a);
		  }
	    }
	}
    }
  if (nh<5) return;
  //DEBUG_PRINT("1\n");
  Matrix<double,Dynamic,3> m(nh,3);
  //DEBUG_PRINT("2\n");
  Matrix<double,3,Dynamic> mt(3,nh);
  Matrix<double,Dynamic,Dynamic> D(nh,nh);
  for (int i=0;i<nh;i++)
    for (int j=0;j<nh;j++)
      D(i,j)=0.;
  //DEBUG_PRINT("3 %d\n",nh);
  xb=xb/wt;
  yb=yb/wt;
  zb=zb/wt;
  xm_[0]=xb;
  xm_[1]=yb;
  xm_[2]=zb;
  nh=0;
  wt=0.;
  //firstPlan_=99;
  for (std::map<uint32_t,std::vector<RecoHit*> >::iterator ipl=thePlans_.begin();ipl!=thePlans_.end();ipl++)
    {
		
      //if (ipl->second.size()>=5 && ipl->first<firstPlan_) firstPlan_=ipl->first;
      if (ipl->first<fp || ipl->first>lp) continue;
      for (std::vector<RecoHit*>::iterator iht=ipl->second.begin();iht!=ipl->second.end();iht++)
	{
	  m(nh,0)=(*iht)->X()-xb;
	  mt(0,nh) =(*iht)->X()-xb;
	  m(nh,1)=(*iht)->Y()-yb;
	  mt(1,nh) =(*iht)->Y()-yb;
	  m(nh,2)=(*iht)->Z()-zb;
	  mt(2,nh) =(*iht)->Z()-zb;
	  int ithr= (*iht)->getAmplitude()&0x3;
	  double w=1.;
	  if (ithr==2) w=1;
	  if (ithr==1) w=1;
	  if (ithr==3) w=2.;
	  //w=1.;
	  D(nh,nh)=w;
	  wt+=w;
	  nh++;
	}
    }
  //DEBUG_PRINT("4\n");
  D *=1./wt;
  //std::cout<<" Here it is "<<std::endl<<D<<std::endl;

  Matrix<double,Dynamic,Dynamic> V(nh,nh);
  // Matrix<double,Dynamic,Dynamic> V1(nh,3);
  //DEBUG_PRINT("5\n");
  //V1= D*m;
  //std::cout<<" Here it is "<<std::endl<<V1<<std::endl;
  //DEBUG_PRINT("=6\n");
  V=mt*D*m;
  //std::cout<<" Here it is "<<std::endl<<V<<std::endl;
  // DEBUG_PRINT("7\n");
  //V *=1./nh;
  //std::cout<<" Here it is "<<std::endl<<V<<std::endl;


  SelfAdjointEigenSolver< Matrix<double,Dynamic,Dynamic> > eigensolver(V);
  if (eigensolver.info() != Success) abort();
  // std::cout << "The eigenvalues of V are:\n" << eigensolver.eigenvalues() << std::endl;
  // std::cout << "Here's a matrix whose columns are eigenvectors of A \n"
  // 	    << "corresponding to these eigenvalues:\n"
  // 	    << eigensolver.eigenvectors() << std::endl;

  Matrix<double,3,1> va=eigensolver.eigenvalues();
  l1_=va(0);
  l2_=va(1);
  l3_=va(2);

  Matrix<double,3,3> vv=eigensolver.eigenvectors();
  v1_[0]=vv(0,0)*sqrt(l1_);
  v1_[1]=vv(1,0)*sqrt(l1_);
  v1_[2]=vv(2,0)*sqrt(l1_);
  v2_[0]=vv(0,1)*sqrt(l2_);
  v2_[1]=vv(1,1)*sqrt(l2_);
  v2_[2]=vv(2,1)*sqrt(l2_);
  v3_[0]=vv(0,2)*sqrt(l3_);
  v3_[1]=vv(1,2)*sqrt(l3_);
  v3_[2]=vv(2,2)*sqrt(l3_);



  //getchar();
  firstPlan_=99;
  lastPlan_=0;
  for (std::map<uint32_t,std::vector<RecoHit*> >::iterator ipl=thePlans_.begin();ipl!=thePlans_.end();ipl++)
    {
      if (ipl->second.size()>1 && ipl->first<firstPlan_) firstPlan_=ipl->first;
      lastPlan_=ipl->first;
    }


  // now compute First and Last good amas
  zfirst_=999999.;
  zlast_=-9999999.;
  ng_=0;
  for (std::vector<Amas>::iterator ia=theAmas_.begin();ia!=theAmas_.end();ia++)
    {
      if (ia->size()<4) continue;
      ia->compute();
      ng_++;
      if (ia->getComponents(2)<zfirst_) zfirst_=ia->getComponents(2);
      if (ia->getComponents(2)>zlast_) zlast_=ia->getComponents(2);
    }

}

void DHShower::computePrincipalComponents(std::vector<RecoHit*> &v, double result[21])
{
  double resultc[21];

  uint32_t nh=0;
  double xb=0,yb=0,zb=0;
  double wt=0.;

  double fp=DBL_MAX;
  double lp=-DBL_MAX;
  double fx=DBL_MAX;
  double lx=-DBL_MAX;
  double fy=DBL_MAX;
  double ly=-DBL_MAX;

  memset(result,0,21*sizeof(double));
  //INFO_PRINT("%d vector size\n",v.size());
  for (std::vector<RecoHit*>::iterator it=v.begin();it!=v.end();it++)
    {
      RecoHit* iht=(*it);
      if (iht==NULL) continue;
      //INFO_PRINT("%x %d %d \n",iht,iht->I(),iht->J());
      //INFO_PRINT("%f %f \n",iht->x(),iht->y());
      //INFO_PRINT("%f %f \n",iht->X(),iht->Y());
      double w=1.;
      int ithr= iht->getAmplitude()&0x3;
      if (ithr==2) w=1;
      if (ithr==1) w=1;
      if (ithr==3) w=2.;
      xb+=iht->X()*w;
      yb+=iht->Y()*w;
      zb+=iht->Z()*w;
      wt+=w;
      if (iht->Z()<fp) fp=iht->Z();
      if (iht->Z()>lp) lp=iht->Z();
      if (iht->I()<fx) fx=iht->I();
      if (iht->I()>lx) lx=iht->I();
      if (iht->J()<fy) fy=iht->J();
      if (iht->J()>ly) ly=iht->J();
      nh++;
    }
	
  if (nh<3) return;
  //INFO_PRINT("%d hits\n",nh);
  Matrix<double,Dynamic,3> m(nh,3);
  //DEBUG_PRINT("2\n");
  Matrix<double,3,Dynamic> mt(3,nh);
  Matrix<double,Dynamic,Dynamic> D(nh,nh);
  for (int i=0;i<nh;i++)
    for (int j=0;j<nh;j++)
      D(i,j)=0.;
  //DEBUG_PRINT("3 %d\n",nh);
  xb=xb/wt;
  yb=yb/wt;
  zb=zb/wt;
  // store barycenter
  result[0]=xb;
  result[1]=yb;
  result[2]=zb;
  nh=0;
  wt=0.;
  //firstPlan_=99;

  for (std::vector<RecoHit*>::iterator it=v.begin();it!=v.end();it++)
    {
      RecoHit* iht=(*it);	  
      m(nh,0)=iht->X()-xb;
      mt(0,nh) =iht->X()-xb;
      m(nh,1)=iht->Y()-yb;
      mt(1,nh) =iht->Y()-yb;
      m(nh,2)=iht->Z()-zb;
      mt(2,nh) =iht->Z()-zb;
      int ithr= iht->getAmplitude()&0x3;
      double w=1.;
      if (ithr==2) w=1;
      if (ithr==1) w=1;
      if (ithr==3) w=2.;
      w=1.; //test
      D(nh,nh)=w;
      wt+=w;
      nh++;
    }
	
  //INFO_PRINT("%d %f\n",nh,wt);
  D *=1./wt;
  //std::cout<<" Here it is "<<std::endl<<D<<std::endl;

  Matrix<double,Dynamic,Dynamic> V(nh,nh);
  // Matrix<double,Dynamic,Dynamic> V1(nh,3);
  //DEBUG_PRINT("5\n");
  //V1= D*m;
  //std::cout<<" Here it is "<<std::endl<<V1<<std::endl;
  //DEBUG_PRINT("=6\n");
  // TEST V=mt*D*m;
	
  V=mt*D*m;
  //std::cout<<" Here it is "<<std::endl<<V<<std::endl;
  // DEBUG_PRINT("7\n");
  //V *=1./nh;
  //std::cout<<" Here it is "<<std::endl<<V<<std::endl;


  SelfAdjointEigenSolver< Matrix<double,Dynamic,Dynamic> > eigensolver(V);
  if (eigensolver.info() != Success) abort();
	
  //store eigen values
  Matrix<double,3,1> va=eigensolver.eigenvalues();
  result[3]=va(0);
  result[4]=va(1);
  result[5]=va(2);

  // store principal axis
  Matrix<double,3,3> vv=eigensolver.eigenvectors();
  result[6]=vv(0,0)*sqrt(result[3]);
  result[7]=vv(1,0)*sqrt(result[3]);
  result[8]=vv(2,0)*sqrt(result[3]);
  result[9]=vv(0,1)*sqrt(result[4]);
  result[10]=vv(1,1)*sqrt(result[4]);
  result[11]=vv(2,1)*sqrt(result[4]);
  result[12]=vv(0,2)*sqrt(result[5]);
  result[13]=vv(1,2)*sqrt(result[5]);
  result[14]=vv(2,2)*sqrt(result[5]);

  // Store First and last Z
  result[15]=fp;
  result[16]=lp;
  result[17]=fx;
  result[18]=lx;
  result[19]=fy;
  result[20]=ly;

}



double DHShower::closestDistance(DHShower& sh)
{
  //  produit vectoriel des 2 vecteurs directeurs
  double *pv3=sh.getv3();
  double *pxm=sh.getxm();
  double wx= (v3_[1]*pv3[2]-v3_[2]*pv3[1])/sqrt(l3_)/sqrt(sh.getl3());
  double wy= (v3_[2]*pv3[0]-v3_[0]*pv3[2])/sqrt(l3_)/sqrt(sh.getl3());
  double wz= (v3_[0]*pv3[1]-v3_[1]*pv3[0])/sqrt(l3_)/sqrt(sh.getl3());

  if (sqrt(wx*wx+wy*wy+wz*wz)<1E-2) return 99999.;


  DEBUG_PRINT("%f %f %f \n",wx,wy,wz);
  // Plan P1

  double p1=-1*(wx*xm_[0]+wy*xm_[1]+wz*xm_[2]);
  double p2=-1*(wx*pxm[0]+wy*pxm[1]+wz*pxm[2]);

  // Le point (0,-p2/wy,0) appartient a P2  et sa distance a P1 = |a1 x +b1 y + c1 z + p1|/ sqrt(a^2+b^2+c^2)
  double dist = abs(-p2 + p1)/sqrt(wx*wx+wy*wy+wz*wz);
  return dist;

}
DHShower::DHShower(RecoHit *h) : selected_(false)
{
  //h.setDHShower(this);
  std::vector<RecoHit*> v;
  v.push_back(h);
  std::pair<uint32_t,std::vector<RecoHit*> > p(h->chamber(),v);
  thePlans_.insert(p);
  firstPlan_=h->chamber();
  lastPlan_=h->chamber();

}
DHShower::~DHShower(){
  
}
void DHShower::clear() {thePlans_.clear();}
bool DHShower::append(RecoHit* h,float dist_cut)
{
  bool  append=false;

  for (std::map<uint32_t,std::vector<RecoHit*> >::iterator ipl=thePlans_.begin();ipl!=thePlans_.end();ipl++)
    {
      if (abs(h->chamber()-1.*ipl->first)>8) continue;

      for (std::vector<RecoHit*>::iterator iht=ipl->second.begin();iht!=ipl->second.end();iht++)
	{
	  uint32_t iDist=abs((*iht)->chamber()-h->chamber())+2*(abs(h->I()-(*iht)->I())+abs(h->J()-(*iht)->J()));
	  if (iDist<dist_cut)
	    {append=true;break;}
			
	  
	}
      if (append) break;
    }

  if (!append) return false;
  std::map<uint32_t,std::vector<RecoHit*> >::iterator ipl=thePlans_.find(h->chamber());
   //h.setDHShower(this);
  if (ipl!=thePlans_.end())
    ipl->second.push_back(h);
  else
    {
      std::vector<RecoHit*> v;
      v.push_back(h);
      std::pair<uint32_t, std::vector<RecoHit*> >  p(h->chamber(),v);
      thePlans_.insert(p);
    }

  return true;
}

void DHShower::Add(RecoHit* h)
{
  std::map<uint32_t,std::vector<RecoHit*> >::iterator ipl=thePlans_.find(h->chamber());
  //h.setDHShower(this);
  if (ipl->second.size()>1 && h->chamber()>lastPlan_) lastPlan_=h->chamber();
  if (ipl!=thePlans_.end())
    {
      ipl->second.push_back(h);
      if (ipl->second.size()>1 &&ipl->first<firstPlan_) firstPlan_=ipl->first;
    }
  else
    {
      std::vector<RecoHit*> v;
      v.push_back(h);
      std::pair<uint32_t, std::vector<RecoHit*> >  p(h->chamber(),v);
      thePlans_.insert(p);
    }

  return;
}
double DHShower::Distance(RecoHit* h)
{
  double dist=9999.;
  for (std::map<uint32_t,std::vector<RecoHit*> >::iterator ipl=thePlans_.begin();ipl!=thePlans_.end();ipl++)
    {
      for (std::vector<RecoHit*>::iterator iht=ipl->second.begin();iht!=ipl->second.end();iht++)
	{
			
	  //double d=sqrt((iht->X()-h.X())*(iht->X()-h.X())+(iht->Y()-h.Y())*(iht->Y()-h.Y())+(iht->Z()-h.Z())*(iht->Z()-h.Z()));
	  double d=abs((*iht)->chamber()-h->chamber())+2*(abs(h->I()-(*iht)->I())+abs(h->J()-(*iht)->J()));
	  if (d<dist) dist=d;

	}
    }
  return dist;
}


uint32_t DHShower::getFDHitsN(uint32_t* v,uint32_t thr)
{
  memset(v,0,8*sizeof(uint32_t));
  uint32_t vscale[8]={1,2,3,4,6,8,12,16};
  for (uint32_t iscale=0;iscale<8;iscale++)
    {
      uint32_t scale=vscale[iscale];
      std::vector<uint32_t> vcell;
      vcell.clear();
      for (std::map<uint32_t,std::vector<RecoHit*> >::iterator ipl=thePlans_.begin();ipl!=thePlans_.end();ipl++)
	{
	  for (std::vector<RecoHit*>::iterator ih=ipl->second.begin();ih!=ipl->second.end();ih++)
	    {
	      int ithr= (*ih)->getAmplitude()&0x3;
	      if (ithr==2 && thr!=0) continue;
	      if (ithr==1 && thr!=1) continue;
	      if (ithr==3 && thr!=2) continue;
	      uint32_t idx=(*ih)->I()-1;
	      uint32_t jdx=(*ih)->J()-1;
	      uint32_t kdx=ipl->first-1;
	      if (idx<0) continue;
	      if (idx>95) continue;
	      if (jdx<0) continue;
	      if (jdx>95) continue;
	      idx=idx/scale;
	      jdx=jdx/scale;
	      kdx=kdx/scale;
	      uint32_t code=(idx<<24)|(jdx<<16)|kdx;
	      if (std::find(vcell.begin(), vcell.end(), code)!=vcell.end()) continue;
	      vcell.push_back(code);
	    }
	}
      v[iscale]=vcell.size();
    }
  // 10 mm

  return v[0];
}



uint32_t DHShower::getFDHits(uint32_t* v,uint32_t thr)
{
  memset(v,0,8*sizeof(uint32_t));

  uint32_t n10=0,n20=0,n30=0,n40=0,n60=0,n80=0,n120=0,n150=0;
  for (std::map<uint32_t,std::vector<RecoHit*> >::iterator ipl=thePlans_.begin();ipl!=thePlans_.end();ipl++)
    {
      unsigned char v10[96][96];
      unsigned char v20[48][48];
      unsigned char v30[32][32];
      unsigned char v40[24][24];
      unsigned char v60[16][16];
      unsigned char v80[12][12];
      unsigned char v120[8][8];
      unsigned char v150[6][6];
      memset(v10,0,96*96*sizeof(unsigned char));
      memset(v20,0,48*48*sizeof(unsigned char));
      memset(v30,0,32*32*sizeof(unsigned char));
      memset(v40,0,24*24*sizeof(unsigned char));
      memset(v60,0,16*16*sizeof(unsigned char));
      memset(v80,0,12*12*sizeof(unsigned char));
      memset(v120,0,8*8*sizeof(unsigned char));
      memset(v150,0,6*6*sizeof(unsigned char));

      for (std::vector<RecoHit*>::iterator ih=ipl->second.begin();ih!=ipl->second.end();ih++)
	{
	  int ithr= (*ih)->getAmplitude()&0x3;
	  if (ithr==2 && thr!=0) continue;
	  if (ithr==1 && thr!=1) continue;
	  if (ithr==3 && thr!=2) continue;
	  uint32_t idx=(*ih)->I()-1;
	  uint32_t jdx=(*ih)->J()-1;
	  if (idx<0) continue;
	  if (idx>95) continue;
	  if (jdx<0) continue;
	  if (jdx>95) continue;
	  v10[idx][jdx]=1;
	  v20[idx/2][jdx/2]=1;
	  v30[idx/3][jdx/3]=1;
	  v40[idx/4][jdx/4]=1;
	  v60[idx/6][jdx/6]=1;
	  v80[idx/8][jdx/8]=1;
	  v120[idx/12][jdx/12]=1;
	  v150[idx/16][jdx/16]=1;
	}
      // 10 mm
      for (uint32_t i=0;i<96;i++)
	for (uint32_t j=0;j<96;j++)
	  if (v10[i][j]) n10++;
      // 20 mm
      for (uint32_t i=0;i<48;i++)
	for (uint32_t j=0;j<48;j++)
	  if (v20[i][j]) n20++;
      // 30 mm
      for (uint32_t i=0;i<32;i++)
	for (uint32_t j=0;j<32;j++)
	  if (v30[i][j]) n30++;
      // 40 mm
      for (uint32_t i=0;i<24;i++)
	for (uint32_t j=0;j<24;j++)
	  if (v40[i][j]) n40++;
      // 60 mm
      for (uint32_t i=0;i<16;i++)
	for (uint32_t j=0;j<16;j++)
	  if (v60[i][j]) n60++;
      // 80 mm
      for (uint32_t i=0;i<12;i++)
	for (uint32_t j=0;j<12;j++)
	  if (v80[i][j]) n80++;

      // 120 mm
      for (uint32_t i=0;i<8;i++)
	for (uint32_t j=0;j<8;j++)
	  if (v120[i][j]) n120++;
      // 150 mm
      for (uint32_t i=0;i<6;i++)
	for (uint32_t j=0;j<6;j++)
	  if (v150[i][j]) n150++;

    }
  v[0]=n10;
  v[1]=n20;
  v[2]=n30;
  v[3]=n40;
  v[4]=n60;
  v[5]=n80;
  v[6]=n120;
  v[7]=n150;
  return n10;
}


void DHShower::summarizeNumberOfHits()
{

 memset(theHitNumber_,0,540*sizeof(uint32_t));
 for (std::map<uint32_t,std::vector<RecoHit*> >::iterator ipl=thePlans_.begin();ipl!=thePlans_.end();ipl++)
   {
     for (std::vector<RecoHit*>::iterator ih=ipl->second.begin();ih!=ipl->second.end();ih++)
	{
	  if ((*ih)->getFlag(RecoHit::THR0)!=0)
	    {
	      if ((*ih)->getFlag(RecoHit::MIP)!=0)
		{theHitNumber_[ipl->first*9+0]++;theHitNumber_[0]++;}
	      else
		if ((*ih)->getFlag(RecoHit::CORE)!=0)
		  {theHitNumber_[ipl->first*9+1]++;theHitNumber_[1]++;}
		else
		  {theHitNumber_[ipl->first*9+2]++;theHitNumber_[2]++;}
	    }
	  if ((*ih)->getFlag(RecoHit::THR1)!=0)
	    {
	      if ((*ih)->getFlag(RecoHit::MIP)!=0)
		{theHitNumber_[ipl->first*9+3]++;theHitNumber_[3]++;}
	      else
		if ((*ih)->getFlag(RecoHit::CORE)!=0)
		  {theHitNumber_[ipl->first*9+4]++;theHitNumber_[4]++;}
		else
		  {theHitNumber_[ipl->first*9+5]++;theHitNumber_[5]++;}
	    }
	  if ((*ih)->getFlag(RecoHit::THR2)!=0)
	    {
	      if ((*ih)->getFlag(RecoHit::MIP)!=0)
		{theHitNumber_[ipl->first*9+6]++;theHitNumber_[6]++;}
	      else
		if ((*ih)->getFlag(RecoHit::CORE)!=0)
		  {theHitNumber_[ipl->first*9+7]++;theHitNumber_[7]++;}
		else
		  {theHitNumber_[ipl->first*9+8]++;theHitNumber_[8]++;}
	    }
	}
   }

 


}

uint32_t DHShower::getNumberOfHits(uint32_t plan,uint32_t threshold)
{

  uint32_t idx=plan*9+threshold*3;
  return (theHitNumber_[idx]+theHitNumber_[idx+1]+theHitNumber_[idx+2]);
}
double DHShower::getCorrectedNumberOfHits(uint32_t plan,uint32_t threshold,std::map<uint32_t,double* > &correff)
{
  std::map<uint32_t,std::vector<RecoHit*> >::iterator ipl=thePlans_.find(plan);
  double nh[3];
  nh[0]=0;
  nh[1]=0;
  nh[2]=0;
  if (ipl!=thePlans_.end())
    {
      std::map<uint32_t,double* >::iterator itcor=correff.find(plan);
      double* c=(itcor->second);
      for (std::vector<RecoHit*>::iterator ih=ipl->second.begin();ih!=ipl->second.end();ih++)
	{
	  int ithr= (*ih)->getAmplitude()&0x3;
	  uint32_t I=((*ih)->I()-1)/16;
	  uint32_t J=((*ih)->J()-1)/16;
	  if (ithr==2) nh[0]+=c[I*8+J];
	  if (ithr==1) nh[1]+=c[I*8+J];
	  if (ithr==3) nh[2]+=c[I*8+J];
	}
    }
  return nh[threshold]; 
}
uint32_t DHShower::getMip(uint32_t threshold)
{
  uint32_t nh=0;
  for (uint32_t ip=1;ip<=50;ip++)
    {
       uint32_t idx=ip*9+threshold*3;
       nh+=theHitNumber_[idx];
    }
  
  return nh;
}
uint32_t DHShower::getCore(uint32_t threshold)
{
  uint32_t nh=0;
  for (uint32_t ip=1;ip<=50;ip++)
    {
       uint32_t idx=ip*9+threshold*3;
       nh+=theHitNumber_[idx+1];
    }
  
  return nh;
}
uint32_t DHShower::getEdge(uint32_t threshold)
{
  uint32_t nh=0;
  for (uint32_t ip=1;ip<=50;ip++)
    {
       uint32_t idx=ip*9+threshold*3;
       nh+=theHitNumber_[idx+2];
    }
  
  return nh;
}
uint32_t DHShower::getNumberOfHits(uint32_t threshold)
{
  return this->getReduceNumberOfHits(threshold,1,50);
}
uint32_t DHShower::getReduceNumberOfHits(uint32_t threshold,uint32_t firstp,uint32_t lastp)
{
  uint32_t nh=0;
  for (uint32_t ip=firstp;ip<=lastp;ip++)
    {
       uint32_t idx=ip*9+threshold*3;
       nh+=(theHitNumber_[idx]+theHitNumber_[idx+1]+theHitNumber_[idx+2]);
    }
  
  return nh;
}

uint32_t DHShower::getNumberOfMips(uint32_t plan)
{
  uint32_t idx=plan*9;
  return (theHitNumber_[idx]+theHitNumber_[idx+3]+theHitNumber_[idx+6]);
}



static TCanvas* TCDHShower=NULL;

void DHShower::drawDisplay()
{
  DCHistogramHandler* rootHandler_=DCHistogramHandler::instance();
  TH3* hcgposcore = rootHandler_->GetTH3("DHShowerMap");
  TH3* hcgposedge = rootHandler_->GetTH3("DHShowerMapEdge");
  TH3* hcgposiso = rootHandler_->GetTH3("IsoMap");
  TH1F* hweight=(TH1F*) rootHandler_->GetTH1("showerweight");
  if (hcgposcore==NULL)
    {
      hcgposcore =rootHandler_->BookTH3("DHShowerMap",52,-2.8,145.6,100,0.,100.,100,0.,100.);
      hcgposedge =rootHandler_->BookTH3("DHShowerMapEdge",52,-2.8,145.6,100,0.,100.,100,0.,100.);
      hcgposiso =rootHandler_->BookTH3("IsoMap",52,-2.8,145.6,100,0.,100.,100,0.,100.);
    }
  else
    {
      hcgposcore->Reset();
      hcgposedge->Reset();
      hcgposiso->Reset();
    }

  if (hcgposcore!=0 )
    {
      hcgposcore->Reset();
      for (std::map<uint32_t,std::vector<RecoHit*> >::iterator ipl=thePlans_.begin();ipl!=thePlans_.end();ipl++)
	{
	  for (std::vector<RecoHit*>::iterator ih=ipl->second.begin();ih!=ipl->second.end();ih++)
	    
	    {
	      if (!((*ih)->getFlag(RecoHit::MIP)||(*ih)->getFlag(RecoHit::ISOLATED)  ))
		hcgposcore->Fill((*ih)->Z(),(*ih)->X(),(*ih)->Y());
	      else
		if ((*ih)->getFlag(RecoHit::ISOLATED))
		  hcgposiso->Fill((*ih)->Z(),(*ih)->X(),(*ih)->Y());
		else
		  hcgposedge->Fill((*ih)->Z(),(*ih)->X(),(*ih)->Y());
	  

	    }
	}
      if (TCDHShower==NULL)
	{
	  TCDHShower=new TCanvas("TCDHShower","test1",1300,900);
	  TCDHShower->Modified();
	  TCDHShower->Draw();
	  TCDHShower->Divide(1,2);
	}
      TCDHShower->cd(1);
      hcgposcore->SetMarkerStyle(21);
      hcgposedge->SetMarkerStyle(21);
      hcgposiso->SetMarkerStyle(21);
      hcgposcore->SetMarkerSize(.4);
      hcgposedge->SetMarkerSize(.4);
      hcgposiso->SetMarkerSize(.4);

      hcgposcore->SetMarkerColor(kRed);

      hcgposedge->SetMarkerColor(kGreen);
      hcgposiso->SetMarkerColor(kCyan);

      hcgposcore->Draw("p");
      hcgposedge->Draw("pSAME");

      hcgposiso->Draw("pSAME");

      TCDHShower->cd(2);
      if (hweight!=0) hweight->Draw();
      TCDHShower->Modified();
      TCDHShower->Draw();
      TCDHShower->Update();
      //::usleep(2);
      //std::stringstream ss("");
      //ss<<"/tmp/Display_"<<evt_->getRunNumber()<<"_"<<evt_->getEventNumber()<<"_"<<currentTime_<<".png";
      //TCDHShower->SaveAs(ss.str().c_str());
      //char cmd[256];
      //sprintf(cmd,"display %s",ss.str().c_str());
      // system(cmd)
      //delete c;
    }

}


