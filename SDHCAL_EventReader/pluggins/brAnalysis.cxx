#define NX 36
#define NY 36

#include "brAnalysis.hh"
#include "DIFUnpacker.h"
#include <TLine.h>
#include <TGraphErrors.h>
#include <TFitResult.h>
#include <TFitter.h>
#include <TF1.h>
#include <TPluginManager.h>
#include <stdint.h>
#include <math.h>
#include "TPolyLine3D.h"
#include "TVirtualPad.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
//#include <lapacke.h>
#include "DifGeom.h"
#include "jsonGeo.hh"
#include "RecoHit.hh"
#include "recoPoint.hh"
#include "recoTrack.hh"
#include "rCluster.hh"
#include "TCanvas.h"

class framePoint : public recoPoint
{
public:
  framePoint(uint32_t p) : _plan(p){;}
   virtual double dX(){return 0.3;}
  virtual double dY(){return 0.3;}
  virtual uint32_t plan(){ return _plan;}
private:
  uint32_t _plan;
};


brAnalysis::brAnalysis() :_geo(NULL),_t0(2E50),_t(0),_tspill(0)
,_readoutTotalTime(0),_numberOfMuon(0),_numberOfShower(0)
{
  reader_=DHCalEventReader::instance();
  rootHandler_ =DCHistogramHandler::instance();
  this->initialise();
}



void brAnalysis::initialise()
{

 
}
void brAnalysis::initJob(){
  presetParameters();
 
}
void brAnalysis::endJob(){
   rootHandler_->writeSQL();
	
}
void brAnalysis::presetParameters()
{
  std::map<std::string,MarlinParameter> m=reader_->getMarlinParameterMap();
  std::map<std::string,MarlinParameter>::iterator it;
  try
    {
      //if ((it=m.find("Interactif"))!=m.end()) draw_=it->second.getBoolValue();
      //if ((it=m.find("SkipEvents"))!=m.end()) theSkip_=it->second.getIntValue();
      //if ((it=m.find("MonitoringPath"))!=m.end()) theMonitoringPath_=it->second.getStringValue();
      //if ((it=m.find("MonitoringPeriod"))!=m.end()) theMonitoringPeriod_=it->second.getIntValue();

      //INFO_PRINT("Interactif %d \n",draw_);
      //std::string geofile;
      if ((it=m.find("geometry"))!=m.end())
	_geo=new jsonGeo(it->second.getStringValue());
      

    }
  catch (std::string s)
    {
      std::cout<<__PRETTY_FUNCTION__<<" error "<<s<<std::endl;
    }
	
}


void brAnalysis::processEvent()
{
  if (_geo==NULL) return;
  if (reader_->getEvent()==0) return;
  evt_=reader_->getEvent();
  if (_geo->cuts()["daqType"].asString().compare("LEVBDIM")!=0)
    reader_->parseRawEvent();
  else
    reader_->parseLevbdimEvent();
     
  //if (evt_->getEventNumber()<=4) return;
  std::vector<DIFPtr*>::iterator itb =reader_->getDIFList().begin();
  if (itb==reader_->getDIFList().end()) return;
				      
  DIFPtr* dbase= (*itb);
  
  _bxId=dbase->getAbsoluteBCID();
  _gtc=dbase->getGTC(); 

   if (_t0>1E50)
     _t0=_bxId*2E-7;
   double ct=_bxId*2E-7-_t0;
   if ((ct-_t)>5.)
     {
       _tspill=ct;
       std::cout<<" New Spill====>"<<_tspill<<std::endl;
     }
   _t=ct;
   std::cout<<"Event  "<<_bxId<<" "<<_gtc<<" "<<_t<<std::endl; 
  
 // Minimal histos
  
  TH1* hacqtim= rootHandler_->GetTH1("/BR/AcquistionTime");	
  if (hacqtim==NULL)
  {
    hacqtim=rootHandler_->BookTH1("/BR/AcquistionTime",2000.,0.,2.);
  }
  // Filling frame and selecting events
  _tframe.clear();
  _tcount.clear();
  for (std::vector<recoTrack*>::iterator it=_vtk.begin();it!=_vtk.end();it++) delete (*it);
  _vtk.clear();
  
  std::map<uint32_t,std::vector<std::pair<DIFPtr*,uint32_t> > >::iterator ifm=_tframe.end();
  std::map<uint32_t,std::bitset<64> >::iterator im=_tcount.end();
  int32_t window=_geo->cuts()["timeWindow"].asInt();
  for (std::vector<DIFPtr*>::iterator it = reader_->getDIFList().begin();it!=reader_->getDIFList().end();it++)
  {
    DIFPtr* d = (*it);
    //uint32_t chid= getChamber(d->getID());
    // LMTest      uint32_t bc = rint(f->getBunchCrossingTime()/DCBufferReader::getDAQ_BC_Period());
    uint32_t chid = _geo->difGeo(d->getID())["chamber"].asUInt();
    
    // Loop on Frames
    for (uint32_t ifra=0;ifra<d->getNumberOfFrames();ifra++)
    {
      uint32_t bc=d->getFrameTimeToTrigger(ifra);
      bool found=false;
      for (int dt=-window;dt<=window;dt++)
	{
	  im=_tcount.find(bc+dt);
	  

	  if (im!=_tcount.end())
	    {
	      im->second.set(chid);
	      ifm=_tframe.find(bc+dt);
	      ifm->second.push_back(std::pair<DIFPtr*,uint32_t>(d,ifra));
	      found=true;
	      break;
	    }
	}
	  if (found) continue;
	  std::bitset<64> v(0);
	  v.set(chid);
	  std::pair<uint32_t,std::bitset<64> > p(bc,v);
	  _tcount.insert(p);
	  std::vector<std::pair<DIFPtr*,uint32_t> > vf;
	  vf.push_back(std::pair<DIFPtr*,uint32_t>(d,ifra));
	  std::pair<uint32_t,std::vector<std::pair<DIFPtr*,uint32_t> > > pf(bc,vf);
	  _tframe.insert(pf);
	  //std::cout<<"New seed "<<bc<<std::endl;
       
      }
    }
    
  std::vector<uint32_t> seeds;seeds.clear();
  uint32_t npmin=_geo->cuts()["minPlans"].asUInt();
  uint32_t itmin=0xFFFFFFFF;
  uint32_t itmax=0;
  uint32_t lastseed=0;
  for (std::map<uint32_t,std::bitset<64> >::iterator im=_tcount.begin();im!=_tcount.end();im++)
    {
      //
      //if (im->second[57] || im->second.count()>12)
      

      if (im->first<itmin) itmin=im->first;
      if (im->first>itmax) itmax=im->first;
      if (im->second.count()>=npmin)
	{
	  ifm=_tframe.find(im->first);
	  
	  //std::cout<<im->first<<" "<<im->second.count()<<" "<<ifm->second.size()<<" "<<im->second<<std::endl;
	  if ( im->first-lastseed<(2*window+2)) //remove buggy seeds in 1st event of spill
	    {
	      lastseed=im->first;
	      continue;
	    }
	  lastseed=im->first;
	  this->processPhysicEvent(im->first);
	seeds.push_back(im->first);
	}
    }
  //getchar();
    _readoutTime=(itmax-itmin)*2E-7;
    hacqtim->Fill(_readoutTime);
    if (_readoutTime>0 && _readoutTime<5.)
      _readoutTotalTime+=_readoutTime;
    std::cout<<"seeds : "<<seeds.size()<<" readout time :"<<_readoutTime<<" absolute "<<_t<<" in spill "<<_t-_tspill<<" Sh (n:rate) "<<_numberOfShower<<":"<<_numberOfShower/_readoutTotalTime<<" Mu(n:rate) "<<_numberOfMuon<<":"<<_numberOfMuon/_readoutTotalTime<<" total time "<<_readoutTotalTime<<std::endl;
  if (evt_->getEventNumber()%100==0 &&evt_->getEventNumber()>0)
    rootHandler_->writeSQL();
 
}

void brAnalysis::processPhysicEvent(uint32_t iseed)
{
  std::vector<recoPoint*> point;
  //double bestX1=0.049985, bestY1 =0.049985,bestZ1 =0.049985,bestX2 =2.63409e-05, bestY2=3.00341e-05,bestZ2=7.91877e-05,bestX3=-1.75883e-08, bestY3=-9.99385e-09,bestZ3 =-9.99385e-09;
  double bestX1=0.033; double bestY1 =0.0905167;double bestZ1 =0.136332;double bestX2 =2.69635e-05; double bestY2=9.13179e-06;double bestZ2=3.056545e-05;double bestX3=-1.7897e-08; double bestY3= 1.49317e-09;double bestZ3 =2.599392e-08;
  double par[9];
  par[0]= 2.859E-02;// 1.889E-02 
par[1]= 4.376E-05;// 4.794E-05 
par[2]= -2.248E-08;// 2.660E-08 
par[3]= 1.533E-01;// 6.956E-02 
par[4]= -3.566E-04;// 1.819E-04 
par[5]= 2.174E-07;// 1.020E-07 
par[6]= -2.665E-01;// 8.000E-02 
par[7]= 1.456E-03;// 2.061E-04 
par[8]= -8.541E-07;// 1.186E-07 
par[0]= 4.609E-02;// 5.274E-03 
par[1]= -4.682E-06;// 7.414E-06 
par[2]= -1.366E-09;// 2.253E-09


par[3]= 3.461E-02;// 1.744E-02 
par[4]= 4.376E-05;// 2.096E-05 
par[5]= -1.419E-08;// 2.931E-08

 
par[6]= 7.173E-02;// 1.256E-02 
par[7]= 3.515E-04;// 1.073E-05 
par[8]= -1.723E-07;// 1.379E-08 

  uint32_t chbif=_geo->cuts()["bifChamber"].asUInt();
  float xbmin=_geo->cuts()["xBeamMin"].asFloat();
  float xbmax=_geo->cuts()["xBeamMax"].asFloat();
  float ybmin=_geo->cuts()["yBeamMin"].asFloat();
  float ybmax=_geo->cuts()["yBeamMax"].asFloat();
  //std::cout<<chbif<<std::endl;
  TH1* hnhit= rootHandler_->GetTH1("/BR/nhit");
  TH1* hhitparasic= rootHandler_->GetTH1("/BR/hitparasic");
  TH1* hnhitm= rootHandler_->GetTH1("/BR/nhitmuon");
  TH1* hnhits= rootHandler_->GetTH1("/BR/nhitshower");
  TH1* hmsi= rootHandler_->GetTH1("/BR/sizemuon");
  TH1* hssi= rootHandler_->GetTH1("/BR/sizeshower");

  TH2* hhitvsratio=rootHandler_->GetTH2("/BR/hitvsratio");
  TH1* hen= rootHandler_->GetTH1("/BR/energy");
    TH1* henb= rootHandler_->GetTH1("/BR/energyb");

  TProfile* hr0 = (TProfile*) rootHandler_->GetTH1("/BR/r0");
  TProfile* hr1 = (TProfile*) rootHandler_->GetTH1("/BR/r1");
  TProfile* hr2 = (TProfile*) rootHandler_->GetTH1("/BR/r2");
    TH2* hshe=rootHandler_->GetTH2("/BR/showerentry");
    
  
	  // if (hhtx==NULL)
	  //   {
	  //     hhtx = rootHandler_->BookProfile("HoughTransformX",100,0.,100,-50.,50.);
  if (hnhit==NULL)
  {
    hnhit=rootHandler_->BookTH1("/BR/nhit",1000.,0.,2000.);
    hnhitm=rootHandler_->BookTH1("/BR/nhitmuon",1000.,0.,2000.);
    hnhits=rootHandler_->BookTH1("/BR/nhitshower",1000.,0.,2000.);
    
       hmsi=rootHandler_->BookTH1("/BR/sizemuon",128.,0.1,128.1);
       hssi=rootHandler_->BookTH1("/BR/sizeshower",128.,0.1,128.1);

    hhitparasic=rootHandler_->BookTH1("/BR/hitparasic",512.,0.,64.);
    hhitvsratio=rootHandler_->BookTH2("/BR/hitvsratio",512,0.,10.,512,0.,0.5);
    hen=rootHandler_->BookTH1("/BR/energy",500.,0.,150.);
     henb=rootHandler_->BookTH1("/BR/energyb",500.,0.,150.);
    hr0 = rootHandler_->BookProfile("/BR/r0",30,0.,5.,0.,5000.);
    hr1 = rootHandler_->BookProfile("/BR/r1",30,0.,5.,0.,5000.);
    hr2 = rootHandler_->BookProfile("/BR/r2",30,0.,5.,0.,5000.);
    hshe=rootHandler_->BookTH2("/BR/showerentry",100,0.,100.,100,0.,100.);

  }
  std::map<uint32_t,std::vector<std::pair<DIFPtr*,uint32_t> > >::iterator ifm=_tframe.find(iseed);
  if (ifm==_tframe.end())
    return;
  std::map<uint32_t,std::bitset<64> >::iterator im=_tcount.find(iseed);
  if (im==_tcount.end())
      return;
  uint32_t nshower=0,nmuon=0;	
  uint32_t nhit=0,nh0=0,nh1=0,nh2=0;
  uint32_t np[60];memset(np,0,60*4);
  for (std::vector<std::pair<DIFPtr*,uint32_t> >::iterator itf=ifm->second.begin();itf!=ifm->second.end();itf++)
  {
    DIFPtr* d=itf->first;
    uint32_t ifra=itf->second;
    //std::cout<<hex<<(uint64_t) d<<" "<<dec<<ifra<<std::endl;
    uint32_t chid = _geo->difGeo(d->getID())["chamber"].asUInt();
    np[chid]++;
    for (uint32_t j=0;j<64;j++)
    {
      if (!(d->getFrameLevel(ifra,j,0) || d->getFrameLevel(ifra,j,1))) continue;
      nhit++;
      if (d->getFrameLevel(ifra,j,0) && d->getFrameLevel(ifra,j,1)) 
	nh2++;
      else
	if (d->getFrameLevel(ifra,j,0)) 
	  nh1++;
	else
	  if (d->getFrameLevel(ifra,j,1)) 
	    nh0++;
      
      framePoint* p= new framePoint(chid);
      
      _geo->convert(d->getID(),d->getFrameAsicHeader(ifra),j,p);
      std::stringstream s;
      s<<"/SH/CH"<<chid;
      TH2* hpos=rootHandler_->GetTH2(s.str()+"/pos");
      if (hpos==NULL)
      {
	hpos=rootHandler_->BookTH2(s.str()+"/pos",120.,-10.,110.,120,-10.,110.);
      }
      hpos->Fill(p->X(),p->Y());
      point.push_back(p);
    }
    //std::cout<<ifra<<" "<<nhit<<std::endl;
  }
  if (point.size()<4) return;
  pcaComponents cp=RecoHit::calculateComponents(point);
  //std::cout<<cp[3]<<" "<<cp[4]<<" "<<cp[5]<<std::endl;
  double bx=cp[0]-cp[6]*cp[2]/cp[8],by=cp[1]-cp[7]*cp[2]/cp[8];
  hshe->Fill(bx,by);
  hnhit->Fill(nhit*1.);
  double hitsparasic=(nh1+nh2+nh0)*1./ifm->second.size();
  hitsparasic=0;
  int nplane=0;
  for (int i=0;i<60;i++)
  {
    if (np[i]==0) continue;
    hitsparasic+=np[i];
    nplane++;
  }
  if (nplane>0) hitsparasic/=nplane;
  hhitparasic->Fill(hitsparasic);
  hhitvsratio->Fill(hitsparasic,(cp[3])/cp[5]);
  
  // Build clusters
  std::vector<rCluster<recoPoint>*> clusters;
  for (std::vector<recoPoint*>::iterator it=point.begin();it!=point.end();it++)
  {
    bool found=false;
    for (std::vector<rCluster<recoPoint>*>::iterator iv=clusters.begin();iv!=clusters.end();iv++)
      if ((*iv)->Append((*it),3.)){found=true;break;}
    if (found) continue;
    clusters.push_back(new rCluster<recoPoint>((*it)));
  }
  uint32_t nbad=0,ngood=0;
  for (std::vector<rCluster<recoPoint>*>::iterator iv=clusters.begin();iv!=clusters.end();iv++)
    {
      if ((*iv)->plan()==1)
	{
	  if ((*iv)->X()>xbmin && (*iv)->X()<xbmax  &&(*iv)->Y()>ybmin && (*iv)->X()<ybmax)
	    ngood++;
	  else
	    nbad++;
	}
    }
 // std::cout<<"Clusters :"<<clusters.size()<<" Ratio "<<point.size()*1./clusters.size()<<std::endl;
  
  /** Track */
   

   std::vector<recoPoint*> vrp;
   float zmin=1E20,zmax=-1E20;
   
    for (std::vector<rCluster<recoPoint>*>::iterator iv=clusters.begin();iv!=clusters.end();iv++) 
     {
       uint32_t nv=0;
       for (std::vector<rCluster<recoPoint>*>::iterator jv=clusters.begin();jv!=clusters.end();jv++) 
       {
	 if ((*iv)==(*jv)) continue;
	 ROOT::Math::XYZVector d1=(*(*iv))-(*(*jv));
	 if (d1.Mag2()>100) continue;
	 nv++;				    
       }
       
       //std::cout<<"Size "<<(*iv)->size()<<" vois "<<nv<<std::endl; 
       if ((*iv)->size()<3 || ((*iv)->size()<5 && nv<5) )
         vrp.push_back(*iv);
       //hmsi->Fill((*iv)->size()*1.);
       else
	 {
	   if ((*iv)->Z()<zmin) zmin=(*iv)->Z();
	   if ((*iv)->Z()>zmax) zmax=(*iv)->Z();
	     }
     }
    recoTrack::combinePoint(vrp,_geo,_vtk);
    //std::cout<<_vtk.size()<<" tracks found "<<std::endl;
    uint32_t ntk=_vtk.size();
    
    
    bool bif=false;
    if (chbif>0)
    {
    std::map<uint32_t,std::bitset<64> >::iterator ick=_tcount.find(iseed-5);
    if (ick!=_tcount.end())
	if (ick->second[chbif]!=0) bif=true;
	  
    ick=_tcount.find(iseed-6);
    if (ick!=_tcount.end()) 
	if (ick->second[chbif]!=0) bif=true;
    ick=_tcount.find(iseed-7); 
    if (ick!=_tcount.end())
	if (ick->second[chbif]!=0) bif=true;
    }
    bif=bif || (chbif==0);
    
    // if (bif)
    //this->draw(point);
  // Select shower and muons
    //getchar();
  // 2016 data
    //if (hitsparasic>1.5 && (cp[3])/cp[5]>1E-2 &&  point.size()*1./clusters.size()>3. && nbad<5 && bx>50 && bx< 70 && by>45 && by<60 && ntk>0 && zmin>2 && zmax<130.)
    // 2015 Oct
    if (hitsparasic>1.5 && (cp[3])/cp[5]>1E-2 &&  point.size()*1./clusters.size()>3. && nbad<5 && bx>xbmin && bx< xbmax && by>ybmin && by<ybmax && ntk>0 && zmin>2 && zmax<130.)
  {nshower++;_numberOfShower++;

    printf("%d,%d,%d,%lld,%d,%f,%f,%d,%d,%d,%d,%d,%d,%d,%d,%f,%f,%d\n",evt_->getRunNumber(),evt_->getEventNumber(),_gtc,_bxId,iseed,_t,_tspill,im->second.count(),ifm->second.size(),nhit,nh0,nh1,nh2,clusters.size(),ntk,zmin,zmax,bif);
    
      double n=nhit;
  double n2=n*n;
  double fe =(nh0*(par[0]+par[1]*n+par[2]*n2)+
	     nh1*(par[3]+par[4]*n+par[5]*n2)+
	     nh2*(par[6]+par[7]*n+par[8]*n2));

    
  //printf("%d %d %f %f %f \n",ngood,nbad,zmin,zmax,f);
    
    /** 733754 80 GeV */
    //  nh0=nh0+int((_t-_tspill-iseed*2E-7)*5.5);
    //nh1=nh1+int((_t-_tspill-iseed*2E-7)*3.1);
    //nh2=nh2+int((_t-_tspill-iseed*2E-7)*3.);

  //std::cout<<nhit<<" "<<nh0+nh1+nh2<<std::endl;

    /** 733724  40  GeV
  nh0=nh0+int((_t-_tspill-iseed*2E-7)*1.2);
  nh1=nh1+int((_t-_tspill-iseed*2E-7)*1.28);
  nh2=nh2+int((_t-_tspill-iseed*2E-7)*1.26);
    */
     hr0->Fill(_t-_tspill-iseed*2E-7,nh0);
    hr1->Fill(_t-_tspill-iseed*2E-7,nh1);
    hr2->Fill(_t-_tspill-iseed*2E-7,nh2);
  double en=(bestX1+bestX2*nhit+bestX3*nhit*nhit)*nh0;
 
  en=en+(bestY1+bestY2*nhit+bestY3*nhit*nhit)*nh1;
  en=en+(bestZ1+bestZ2*nhit+bestZ3*nhit*nhit)*nh2;
  en=en*1.;
      for (std::vector<rCluster<recoPoint>*>::iterator iv=clusters.begin();iv!=clusters.end();iv++) 
     {
       hssi->Fill((*iv)->size()*1.);
     }
    //double en=0.057*(nh0*0.2+(1+nhit/15000.)*nh1+5*(1-nhit/15000)*nh2);
    //  std::cout<<" CANDIDATE seed :"<<iseed<<" -> chambers: "<<im->second.count()<<" -> frames: "<<ifm->second.size()<<" -> Hits: "<<nhit<<" -> ratio:"<<hitsparasic<<" "<<nh0<<" "<<nh1<<" "<<nh2<<" "<<en<<" " <<ngood<<" "<<nbad<<" tk "<<ntk<<std::endl;
    hnhits->Fill(nh0+nh1+nh2);
    hen->Fill(fe);
    if (bif) henb->Fill(fe);
    //this->draw(point);

 
  }
  else
  {nmuon++;_numberOfMuon++;
    hnhitm->Fill(nhit);
    for (std::vector<rCluster<recoPoint>*>::iterator iv=clusters.begin();iv!=clusters.end();iv++) 
     {
  
       hmsi->Fill((*iv)->size()*1.);
     }
    //if (ntk==0 && point.size()>30)
    //  this->draw(point);
  }

  for (std::vector<recoPoint*>::iterator it=point.begin();it!=point.end();it++) delete (*it);
  for (std::vector<rCluster<recoPoint>*>::iterator iv=clusters.begin();iv!=clusters.end();iv++) delete((*iv));
  //std::cout<<" CANDIDATE seed :"<<im->first<<" -> chambers: "<<im->second.count()<<" -> frames: "<<ifm->second.size()<<" -> Hits: "<<nhit<<" -> ratio:"<<hitsparasic<<std::endl;
}

static TCanvas* TCHits=NULL;
void brAnalysis::draw(std::vector<recoPoint*> vp)
{
  TH2* hzx=rootHandler_->GetTH2("/BR/hzx");
  TH2* hzy=rootHandler_->GetTH2("/BR/hzy");
  if (hzx==NULL)
    {
      hzx=rootHandler_->BookTH2("/BR/zx",150,0.,150.,100,0.,100.);
      hzy=rootHandler_->BookTH2("/BR/zy",150,0.,150.,100,0.,100.);
    }
  else
    {
      hzx->Reset();
      hzy->Reset();
    }
  for (std::vector<recoPoint*>::iterator it=vp.begin();it!=vp.end();it++)
    {
      hzx->Fill((*it)->Z(),(*it)->X());
      hzy->Fill((*it)->Z(),(*it)->Y());
    }
  if (TCHits==NULL)
    {
      TCHits=new TCanvas("TCHits","tChits1",900,900);
      TCHits->Modified();
      TCHits->Draw();
      TCHits->Divide(1,2);
    }
  TCHits->cd(1);
  hzx->Draw("COLZ");
#define drawtk
#ifdef drawtk
  std::vector<TLine*> vl;
  for (std::vector<recoTrack*>::iterator itk=_vtk.begin();itk!=_vtk.end();itk++)
  {
    ROOT::Math::XYZPoint pmin=(*itk)->extrapolate((*itk)->zmin());
      ROOT::Math::XYZPoint pmax=(*itk)->extrapolate((*itk)->zmax());

  TLine* l = new TLine(pmin.Z(),pmin.X(),pmax.Z(),pmax.X());
    l->SetLineColor(2);
    l->Draw("SAME");
    vl.push_back(l);
    std::cout<<pmin.X()<<" "<<pmax.X()<<std::endl;
  }
#endif
  TCHits->Modified();
  TCHits->cd(2);
  hzy->Draw("COLZ");
  for (std::vector<recoTrack*>::iterator itk=_vtk.begin();itk!=_vtk.end();itk++)
  {
    ROOT::Math::XYZPoint pmin=(*itk)->extrapolate((*itk)->zmin());
      ROOT::Math::XYZPoint pmax=(*itk)->extrapolate((*itk)->zmax());

  TLine* l = new TLine(pmin.Z(),pmin.Y(),pmax.Z(),pmax.Y());
    l->SetLineColor(2);
    l->Draw("SAME");
    vl.push_back(l);
    std::cout<<pmin.X()<<" "<<pmax.X()<<std::endl;
  }
  TCHits->Modified();
  TCHits->Draw();
  TCHits->Update();
  getchar();
  for (std::vector<TLine*>::iterator il=vl.begin();il!=vl.end();il++) delete (*il);
  std::cout<<"fini "<<std::endl;
}


extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
    DHCALAnalyzer* loadAnalyzer(void)
    {
      return (new brAnalysis);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
    void deleteAnalyzer(DHCALAnalyzer* obj)
    {
      delete obj;
    }
}
