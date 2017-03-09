#include "hitMonitor.hh"
hitMonitor::hitMonitor(jsonGeo* g) : _geo(g),theTrackIndex_(0),theFirstChamber_(1),theLastChamber_(50),
									  theExtrapolationMinimumPoint_(6),theExtrapolationMinimumChi2_(0.001),theExtrapolationDistanceCut_(8),theTrackAngularCut_(0.1),theChamberEdgeCut_(5.),useTk4_(true),theIntegratedTime_(0)
{
  theAsicCount_.clear();
  rootHandler_ =DCHistogramHandler::instance();
}
void hitMonitor::clear()
{
  theAsicCount_.clear();
  theTrackIndex_=0;
  theIntegratedTime_=0;
}
void hitMonitor::setFirstChamber(uint32_t i) 
{theFirstChamber_=i;}

void hitMonitor::setLastChamber(uint32_t i)
{theLastChamber_=i;}
void hitMonitor::setExtrapolationMinimumPoint(uint32_t i) 
{theExtrapolationMinimumPoint_=i;}
void hitMonitor::setExtrapolationMinimumChi2(float i) 
{theExtrapolationMinimumChi2_=i;}
void hitMonitor::setChamberEdgeCut( float i) 
{theChamberEdgeCut_=i;} 
void hitMonitor::setUseTk4(bool t)
{useTk4_=t;};
	
void hitMonitor::FillTimeAsic(IMPL::LCCollectionVec* rhcol,bool difanalysis)
{
  //  std::map<uint32_t,uint32_t> count;
  //count.clear();
  TH1* hoccall= (TH1F*) rootHandler_->GetTH1("AsicOccupancy");
  TH1* hoccalldif= (TH1F*) rootHandler_->GetTH1("AsicOccupancyDIF");
  TH1* hoccallchamber= (TH1F*) rootHandler_->GetTH1("AsicOccupancyChamber");

  TH2* hasic2= (TH2F*) rootHandler_->GetTH2("DIFAsicCount");
	
  if (hoccall==0)
    {
      hoccall=rootHandler_->BookTH1("AsicOccupancy",255*48,0.,255*48.);
      hoccalldif=rootHandler_->BookTH1("AsicOccupancyDIF",255,0.,255.);
      hoccallchamber=rootHandler_->BookTH1("AsicOccupancyChamber",61,0.,61.);
      hasic2=rootHandler_->BookTH2("DIFAsicCount",256,0.1,256.1,48,0.1,48.1);
    }
  hoccalldif->Reset();
  hoccallchamber->Reset();
  hasic2->Reset();
  double tmin=99999999.;
  double tmax=0.;
  //	printf("%d %s\n",__LINE__,__PRETTY_FUNCTION__);
  if (difanalysis)
    this->DIFStudy(rhcol);
  //	printf("%d %s\n",__LINE__,__PRETTY_FUNCTION__);
  //IMPL::LCCollectionVec* rhcol=(IMPL::LCCollectionVec*) evt_->getCollection(collectionName_);
  std::bitset<255> dif_found(0);
  for (int i=0;i<rhcol->getNumberOfElements();i++)
    {
      IMPL::RawCalorimeterHitImpl* hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(i);
      if (hit==0) continue;
		
      // Decode
      unsigned int difid = hit->getCellID0()&0xFF;
      int asicid = (hit->getCellID0()&0xFF00)>>8;
      //hasic2->Fill(difid*1.,asicid*1);
      int channel= (hit->getCellID0()&0x3F0000)>>16;
      unsigned int bc = hit->getTimeStamp();
      dif_found.set(difid,1);
      if (bc>5E6) continue;

      if (bc<tmin) tmin=bc;
      if (bc>tmax) tmax=bc;
		
      uint32_t chid = _geo->difGeo(difid)["chamber"].asUInt();
      
      uint32_t key=(chid<<16)|(difid<<8)|asicid;
      std::map<uint32_t,uint64_t>::iterator it=theAsicCount_.find(key);
      if (theAsicCount_.find(key)!=theAsicCount_.end()) 
	it->second=it->second+1;
      else
	{
	  uint32_t n=1;
	  std::pair<uint32_t,uint64_t> p(key,n);
	  theAsicCount_.insert(p);
	}
    }
    //printf("DIF summary \n");
    //for (int i=0;i<256;i++)
    //    if (dif_found[i]!=0) printf("DIF %d Found \n",i);
  //	printf("%d %s\n",__LINE__,__PRETTY_FUNCTION__);
  theIntegratedTime_+=(tmax-tmin);
  theEventIntegratedTime_=(tmax-tmin);
  //std::cout<<tmin<<" "<<tmax<<" => Event "<<theEventIntegratedTime_<<" total " <<theIntegratedTime_<<std::endl;
  for (std::map<uint32_t,uint64_t>::iterator it=theAsicCount_.begin();it!=theAsicCount_.end();it++)
    {
      uint32_t chid =(it->first>>16)&0xFF;
      uint32_t difid =(it->first>>8)&0xFF;
      uint32_t asicid =(it->first)&0xFF;
		
      std::stringstream namec("");
      namec<<"/Noise/Chamber"<<chid<<"/DIF"<<difid;
		

      TH1* hocc= (TProfile*) rootHandler_->GetTH1(namec.str()+"/AsicOccupancy");	   
      TH1* hoccn= (TH1*) rootHandler_->GetTH1(namec.str()+"/AsicOccupancyNumber");	   
      if (hocc==0)
	{
			
	  hocc =rootHandler_->BookTH1( namec.str()+"/AsicOccupancy",48,0.,48.);
	  hoccn =rootHandler_->BookTH1( namec.str()+"/AsicOccupancyNumber",48,0.,48.);
	}

		
      hoccn->SetBinContent(asicid,it->second);
      hoccall->SetBinContent(difid*48+asicid,it->second/(theIntegratedTime_*2E-7));
		
      hocc->SetBinContent(asicid,it->second/(theIntegratedTime_*2E-7));
      float focc=it->second/(theIntegratedTime_*2E-7);
      if (focc>hoccallchamber->GetBinContent(chid)) hoccallchamber->SetBinContent(chid,focc);
      if (focc>hoccalldif->GetBinContent(difid)) hoccalldif->SetBinContent(difid,focc);


    }
  //	printf("%d %s\n",__LINE__,__PRETTY_FUNCTION__);
  TH1* htdiff= rootHandler_->GetTH1("TimeDif");

  if (htdiff==NULL)
    {
      htdiff =rootHandler_->BookTH1( "TimeDif",2000,0.,4000000.);

    }
  htdiff->Fill((tmax-tmin)*1.);
}

void hitMonitor::DIFStudy( IMPL::LCCollectionVec* rhcol,bool external)
{
  std::vector<RecoHit> _hits;
  for (int i=0;i<rhcol->getNumberOfElements();i++)
    {
      IMPL::RawCalorimeterHitImpl* hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(i);
      if (hit==0) continue;
      RecoHit h(_geo,hit);
      _hits.push_back(h);
    }
  printf("DIFSTUDY %d hits\n",_hits.size());
  for (std::vector<RecoHit>::iterator it=_hits.begin();it!=_hits.end();it++)
    {
      RecoHit& h=(*it);
      unsigned int bc = h.raw()->getTimeStamp();
      std::stringstream namec("");
      namec<<"/Noise/Chamber"<<h.chamber()<<"/DIF"<<h.dif();
      //std::cout<<"DIF "<<h.dif()<<" CH "<<h.chamber()<<" "<<namec.str()<<std::endl;
      TH1* hhits0 = rootHandler_->GetTH1(namec.str()+"/Hits0");	   
      TH1* hhits1 = rootHandler_->GetTH1(namec.str()+"/Hits1");
      TH1* hhits1it = rootHandler_->GetTH1(namec.str()+"/Hits1IT");	   
      TH1* hhits2 = rootHandler_->GetTH1(namec.str()+"/Hits2");
      TH1* hetd = rootHandler_->GetTH1(namec.str()+"/EventTime");
      TH1* hetdz = rootHandler_->GetTH1(namec.str()+"/EventTimeZoom");
		
      if (hhits0==0)
	{
	  //		    printf("%s %d %s\n",namec.str().c_str(),__LINE__,__PRETTY_FUNCTION__);
	  //::sleep(1);
	  hhits0 =rootHandler_->BookTH1( namec.str()+"/Hits0",48*64,0.1,48*64+0.1);
	  hhits1 =rootHandler_->BookTH1( namec.str()+"/Hits1",48*64,0.1,48*64+0.1);
	  hhits1it =rootHandler_->BookTH1( namec.str()+"/Hits1IT",48*64,0.1,48*64+0.1);
	  hhits2 =rootHandler_->BookTH1( namec.str()+"/Hits2",48*64,0.1,48*64+0.1);
	  hetd =rootHandler_->BookTH1(namec.str()+"/EventTime",10000,0.,15E6);
	  hetdz =rootHandler_->BookTH1(namec.str()+"/EventTimeZoom",10000,0.,10000);
	  //		    printf("%d %s\n",__LINE__,__PRETTY_FUNCTION__);

	}
      
      if (h.isTagged(RecoHit::THR0)||h.isTagged(RecoHit::THR2)) hhits0->SetBinContent((h.asic()-1)*64+h.channel()+1,hhits0->GetBinContent((h.asic()-1)*64+h.channel()+1)+1);
      if (h.isTagged(RecoHit::THR1)||h.isTagged(RecoHit::THR0)||h.isTagged(RecoHit::THR2))
	{
	  hhits1->SetBinContent((h.asic()-1)*64+h.channel()+1,hhits1->GetBinContent((h.asic()-1)*64+h.channel()+1)+1);
	  if (bc>8 && bc<20)
	    hhits1it->SetBinContent((h.asic()-1)*64+h.channel()+1,hhits1->GetBinContent((h.asic()-1)*64+h.channel()+1)+1);
	}
      if (h.isTagged(RecoHit::THR2)) hhits2->SetBinContent((h.asic()-1)*64+h.channel()+1,hhits2->GetBinContent((h.asic()-1)*64+h.channel()+1)+1);
      hetd->Fill(bc*1.);

      hetdz->Fill(bc*1.);

      std::stringstream namech("");
      namech<<"/Noise/Chamber"<<h.chamber();

      TH2* hthr0 = rootHandler_->GetTH2(namech.str()+"/Seuil0");
      TH2* hthr1 = rootHandler_->GetTH2(namech.str()+"/Seuil1");
      TH2* hthr1it = rootHandler_->GetTH2(namech.str()+"/Seuil1IT");

      TH2* hthr2 = rootHandler_->GetTH2(namech.str()+"/Seuil2");
      if (hthr0==NULL)
	{
	  hthr0 =rootHandler_->BookTH2( namech.str()+"/Seuil0",96,0.,96.,96,0.,96.);
	  hthr1 =rootHandler_->BookTH2( namech.str()+"/Seuil1",96,0.,96.,96,0.,96.);
	  hthr1it =rootHandler_->BookTH2( namech.str()+"/Seuil1IT",96,0.,96.,96,0.,96.);
	  hthr2 =rootHandler_->BookTH2( namech.str()+"/Seuil2",96,0.,96.,96,0.,96.);
	}
      if (h.isTagged(RecoHit::THR0)||h.isTagged(RecoHit::THR2)) hthr0->Fill(h.I()*1.,h.J()*1.);
      if (h.isTagged(RecoHit::THR1)||h.isTagged(RecoHit::THR2)||h.isTagged(RecoHit::THR0))
	{
	  hthr1->Fill(h.I()*1.,h.J()*1.);
	  if (bc<20) hthr1it->Fill(h.I()*1.,h.J()*1.);
	}
      if (h.isTagged(RecoHit::THR2)) hthr2->Fill(h.I()*1.,h.J()*1.);

    }

}

void hitMonitor::trackHistos(std::vector<recoTrack*> &tracks,std::vector<recoPoint*> &clusters,std::string tkdir)
{
  uint32_t minTkExtPoint=_geo->cuts()["tkExtMinPoint"].asUInt();
  float tkChi2=_geo->cuts()["tkChi2"].asFloat();
  float edge=_geo->cuts()["edge"].asFloat();
  float zedge=_geo->cuts()["zedge"].asFloat();
  //std::cout << minTkExtPoint<<" "<<tkChi2<<" "<<edge<<std::endl;
  // book all chamber
  uint32_t nch=0;
  //STEP;
  for (int ich=1;ich<60;ich++)
    {
      Json::Value jch=_geo->chamberGeo(ich);
      if (jch.empty()) continue;
      nch++;
      std::stringstream namec("");
      namec<<tkdir+"/Plan"<<ich;
      TH1* hposx = rootHandler_->GetTH1(namec.str()+"/XPos");	   
      TH1* hposy = rootHandler_->GetTH1(namec.str()+"/YPos");	   
      TH1* hpullx = rootHandler_->GetTH1(namec.str()+"/XDist");	   
      TH1* hpully = rootHandler_->GetTH1(namec.str()+"/YDist");	   
      TH1* hmult = rootHandler_->GetTH1(namec.str()+"/Multiplicity");	   
      TH2* hext= rootHandler_->GetTH2(namec.str()+"/ext");
      TH2* hfound= rootHandler_->GetTH2(namec.str()+"/found");
      TH2* hnear= rootHandler_->GetTH2(namec.str()+"/near");
      TH2* hfound1= rootHandler_->GetTH2(namec.str()+"/found1");
      TH2* hfound2= rootHandler_->GetTH2(namec.str()+"/found2");
      TH2* hmul= rootHandler_->GetTH2(namec.str()+"/mul");
      TH1* hdx= rootHandler_->GetTH1(namec.str()+"/dx");
      TH2* hdx2= rootHandler_->GetTH2(namec.str()+"/dx2");
      TH1* hdy= rootHandler_->GetTH1(namec.str()+"/dy");
      TH2* hdy2= rootHandler_->GetTH2(namec.str()+"/dy2");
      TH2* hderr= rootHandler_->GetTH2(namec.str()+"/derr");
      TH2* hmiss= rootHandler_->GetTH2(namec.str()+"/missing");

      if (hposx==0)
	{
	  float xi=jch["x0"].asFloat();
	  float xa=jch["x1"].asFloat();
	  float yi=jch["y0"].asFloat();	  
	  float ya=jch["y1"].asFloat();
	  int nx=int(xa-xi)+1;
	  int ny=int(ya-yi)+1;
	  hposx =rootHandler_->BookTH1( namec.str()+"/XPos",115,-10.,110.);
	  hposy =rootHandler_->BookTH1( namec.str()+"/YPos",115,-10.,110.);
	  hpullx =rootHandler_->BookTH1( namec.str()+"/XDist",200,-5.,5.);
	  hpully =rootHandler_->BookTH1( namec.str()+"/YDist",200,-5.,5.);
	  hmult =rootHandler_->BookTH1( namec.str()+"/Multiplicity",50,0.,50.);
	  hext= rootHandler_->BookTH2(namec.str()+"/ext",nx,xi,xa,ny,yi,ya);
	  hfound= rootHandler_->BookTH2(namec.str()+"/found",nx,xi,xa,ny,yi,ya);
	  hnear= rootHandler_->BookTH2(namec.str()+"/near",nx,xi,xa,ny,yi,ya);
	  hfound1= rootHandler_->BookTH2(namec.str()+"/found1",nx,xi,xa,ny,yi,ya);
	  hfound2= rootHandler_->BookTH2(namec.str()+"/found2",nx,xi,xa,ny,yi,ya);
	  hmiss= rootHandler_->BookTH2(namec.str()+"/missing",nx,xi,xa,ny,yi,ya);
	  hmul= rootHandler_->BookTH2(namec.str()+"/mul",nx,xi,xa,ny,yi,ya);
	  hdx=  rootHandler_->BookTH1(namec.str()+"/dx",400,-4.,4.);
	  hdx2=  rootHandler_->BookTH2(namec.str()+"/dx2",11,-0.1,10.9,300,-3.,3.);
	  hdy=  rootHandler_->BookTH1(namec.str()+"/dy",400,-4.,4.);
	  hderr=  rootHandler_->BookTH2(namec.str()+"/derr",400,-4.,4.,400,-4.,4.);
	  hdy2=  rootHandler_->BookTH2(namec.str()+"/dy2",11,-0.1,10.9,300,-3.,3.);
	  
	}

    }
  //STEP;
  TH1* hngood = rootHandler_->GetTH1(tkdir+"/NumberOfTracks");
  if (hngood==0)
    {
      hngood = rootHandler_->BookTH1(tkdir+"/NumberOfTracks",21,-0.1,20.9);
    }
  hngood->Fill(tracks.size()*1.);
  if (tracks.size()!=1) return;
  for (std::vector<recoTrack*>::iterator it=tracks.begin();it!=tracks.end();it++)
    {
      // STEP;    
      recoTrack* ptk = (*it);

      //DEBUG_PRINT("Tk=%d Time=%d %f %f %f %f %f \n",theTrackIndex_,currentTime_,tk.ax_,tk.bx_,tk.ay_,tk.by_,tk.chi2_);
      TH1* htchi2 = rootHandler_->GetTH1(tkdir+"/Chi2");
      TH1* htpchi2 = rootHandler_->GetTH1(tkdir+"/ProbChi2");
      TH1* htnpoint = rootHandler_->GetTH1(tkdir+"/NumberOfPoints");
      TH1* htax = rootHandler_->GetTH1(tkdir+"/Ax");
      TH1* htay = rootHandler_->GetTH1(tkdir+"/Ay");
      TH1* htxh = rootHandler_->GetTH1(tkdir+"/xh");
      TH1* htyh = rootHandler_->GetTH1(tkdir+"/yh");
      TH1* hnpl= rootHandler_->GetTH1(tkdir+"/Nplanes");
      TH1* hang= rootHandler_->GetTH1(tkdir+"/angle");
      TH1* hd3= rootHandler_->GetTH1(tkdir+"/d3");

      if (htchi2==0)
	{
	  htchi2 = rootHandler_->BookTH1(tkdir+"/Chi2",500,0.,100.);
	  htpchi2 = rootHandler_->BookTH1(tkdir+"/ProbChi2",1000,0.,1.);
	  htnpoint = rootHandler_->BookTH1(tkdir+"/NumberOfPoints",60,0.,60.);
	  htax = rootHandler_->BookTH1(tkdir+"/Ax",200,-10.,10.);
	  htay = rootHandler_->BookTH1(tkdir+"/Ay",200,-10.,10.);
	  hnpl=  rootHandler_->BookTH1(tkdir+"/Nplanes",51,-0.1,50.9);
	  hang=  rootHandler_->BookTH1(tkdir+"/angle",100,0.,50.);
	  hd3=  rootHandler_->BookTH1(tkdir+"/d3",100,0.,10.);
	}      


      htchi2->Fill(ptk->chi2());
      htpchi2->Fill(ptk->pchi2());
      //DEBUG_PRINT("track %f %d %f %f \n",tk.chi2_,2*tk.getList().size()-4,TMath::Prob(tk.chi2_,2*tk.getList().size()-4),tkChi2Cut_);
      //getchar();
      htnpoint->Fill(ptk->points().size());

      htax->Fill(ptk->dir().X());

      htay->Fill(ptk->dir().Y());

      if (ptk->size()>=4)
	{
	  recoTrack p23;
	  recoTrack p789;
	   for (std::vector<ROOT::Math::XYZPoint*>::iterator ip=ptk->points().begin();ip!=ptk->points().end();ip++)
	     {
	       std::vector<recoPoint*>::iterator ic=std::find(clusters.begin(),clusters.end(),(recoPoint*)(*ip));
	       if (ic==clusters.end()) continue;
	       if ((*ic)->plan()==2 || (*ic)->plan()==3) p23.addPoint((*ic));
	       if ((*ic)->plan()==7 || (*ic)->plan()==8 || (*ic)->plan()==9  ) p789.addPoint((*ic));
	     }
	   if (p23.size()>=2 && p789.size()>=2 &&ptk->pchi2() <1E-3 )
	     {
	       p23.regression();
	       p789.regression();
	       double s=p789.dir().Dot(p23.dir())/sqrt(p23.dir().Mag2()*p789.dir().Mag2());
	       double angle=acos(abs(s))*180./M_PI;
	       hang->Fill(angle);
	       /*
	       std::cout<<" Track angle:"<<std::endl;
	       std::cout<<ptk->dir().X()<<"/"<<ptk->dir().Y()<<"/"<<ptk->dir().Z()<<std::endl;
	       std::cout<<p23.dir().X()<<"/"<<p23.dir().Y()<<"/"<<p23.dir().Z()<<std::endl;
	       std::cout<<p789.dir().X()<<"/"<<p789.dir().Y()<<"/"<<p789.dir().Z()<<std::endl;
	       std::cout<<s<<"-> angle : "<<angle<<" Prob Chi2 "<<ptk->pchi2()<<std::endl;
	       */
	       double dist;
	       ROOT::Math::XYZPoint p1,p2;
	       p789.cap(p23,dist,p1,p2);
	       Json::Value jc3=_geo->chamberGeo(3);
	       Json::Value jc7=_geo->chamberGeo(7);
	       ROOT::Math::XYZPoint po3=p789.extrapolate(jc3["z0"].asFloat());
	       ROOT::Math::XYZPoint pi3=p23.extrapolate(jc3["z0"].asFloat());
	       ROOT::Math::XYZVector d3=po3-pi3;
	       
	       //std::cout<<p1.X()<<" "<<p1.Y()<<" "<<p1.Z()<<std::endl;
	       double rd3=sqrt(d3.Mag2());
	       hd3->Fill(rd3);
	       if (rd3>8) break;
	       double prob=1.-erf(rd3/sqrt(2.)/1.);
	       if (prob<1E-20) prob=1E-20;
	       std::cout<<" distance "<<rd3<<" @ "<<p1.Z()<<" "<<prob<<" "<<abs(log(prob))<<std::endl;
	       if (p1.Z()>jc3["z0"].asFloat() && p1.Z()<jc7["z0"].asFloat() )
		 { Json::Value jc5=_geo->chamberGeo(5);
		   
		   ROOT::Math::XYZPoint pex=p789.extrapolate(jc5["z0"].asFloat());
		 
		   std::stringstream namec("");
		   namec<<tkdir+"/Plan5";
		   TH2* hfound2= rootHandler_->GetTH2(namec.str()+"/found2");
		   hfound2->Fill(p1.X(),p1.Y(),rd3);
		 }
	     }
	}

      //STEP;
      //       if (tracks.size()>1)
      //DEBUG_PRINT("\t %d good hits found \n",tk.getList().size());
      for (std::vector<ROOT::Math::XYZPoint*>::iterator ip=ptk->points().begin();ip!=ptk->points().end();ip++)
	{
	  std::vector<recoPoint*>::iterator ic=std::find(clusters.begin(),clusters.end(),(recoPoint*)(*ip));
	  if (ic==clusters.end()) continue;
	  // if (tracks.size()>1)
	  //   DEBUG_PRINT("\t \t %f %f %f \n",tk.getList()[i].X(),tk.getList()[i].Y(),tk.getList()[i].Z());
	  //(tk.getList())[i].Print();
	  hnpl->Fill(1.*(*ic)->plan());
	  std::stringstream namec("");
	  namec<<tkdir+"/Plan"<<(*ic)->plan();
	  TH1* hposx = rootHandler_->GetTH1(namec.str()+"/XPos");	   
	  TH1* hposy = rootHandler_->GetTH1(namec.str()+"/YPos");	   
	  TH1* hpullx = rootHandler_->GetTH1(namec.str()+"/XDist");	   
	  TH1* hpully = rootHandler_->GetTH1(namec.str()+"/YDist");	   
	  TH1* hmult = rootHandler_->GetTH1(namec.str()+"/Multiplicity");	   
	  if (hposx==NULL)
	    {
	      std::cout <<"Opps "<<namec.str()<<std::endl;
	      std::cout<<"Cluster "<<(*ic)->Z()<<" "<<(*ic)->X()<<std::endl;
	      //(*ic)->Print();
	      //exit(0);
	      continue;
	    }

	  hposx->Fill((*ip)->X());
	  hposy->Fill((*ip)->Y());
	  ROOT::Math::XYZPoint pex=(*it)->extrapolate((*ip)->Z());
	  ROOT::Math::XYZVector dex=pex-(*(*ip));
	  hpullx->Fill(dex.X());
	  hpully->Fill(dex.Y());
	  /// A revoir hmult->Fill((*ic)->size());
	}
      //STEP;
      ///A revoir if ((*it)->pchi2()<tkChi2) continue;
      for (int ich=1;ich<60;ich++)
	{
	  Json::Value jch=_geo->chamberGeo(ich);
	  if (jch.empty()) continue;
	  if ((*it)->zmin()>jch["z0"].asFloat()+zedge) continue;
	  if ((*it)->zmax()<jch["z0"].asFloat()-zedge) continue;


	  recoTrack tkext;
	  tkext.clear();
	  for (std::vector<ROOT::Math::XYZPoint*>::iterator ip=(*it)->points().begin();ip!=(*it)->points().end();ip++)
	    {
	      if (abs((*ip)->Z()-jch["z0"].asFloat())<1E-3)
            continue;
	      else
            tkext.addPoint((*ip));
	    }
	    /// A revoir tkext.getChi2(clusters);
	  if (tkext.size()<minTkExtPoint) continue;
      /// A revoir if (tkext.pchi2()<tkChi2) continue;
	  tkext.regression();
	  std::stringstream namec("");
	  namec<<tkdir+"/Plan"<<ich;
	  TH2* hext= rootHandler_->GetTH2(namec.str()+"/ext");
	  TH2* hfound= rootHandler_->GetTH2(namec.str()+"/found");
	  TH2* hnear= rootHandler_->GetTH2(namec.str()+"/near");
	  TH2* hfound1= rootHandler_->GetTH2(namec.str()+"/found1");	 
	  TH2* hfound2= rootHandler_->GetTH2(namec.str()+"/found2");
	  TH2* hmul= rootHandler_->GetTH2(namec.str()+"/mul");
	  TH1* hdx= rootHandler_->GetTH1(namec.str()+"/dx");
	  TH2* hdx2= rootHandler_->GetTH2(namec.str()+"/dx2");
	  TH1* hdy= rootHandler_->GetTH1(namec.str()+"/dy");
	  TH2* hdy2= rootHandler_->GetTH2(namec.str()+"/dy2");
	  TH2* hderr= rootHandler_->GetTH2(namec.str()+"/derr");
	  TH2* hmiss= rootHandler_->GetTH2(namec.str()+"/missing");
	  
	  ROOT::Math::XYZPoint pex=(*it)->extrapolate(jch["z0"].asFloat());

	  if ((*it)->size()>=4)
	    {
	      hfound1->Fill(pex.X(),pex.Y());
	    }

	  
	  if (pex.X()<jch["x0"].asFloat()+edge) continue;
	  if (pex.X()>jch["x1"].asFloat()-edge) continue;
	  if (pex.Y()<jch["y0"].asFloat()+edge) continue;
	  if (pex.Y()>jch["y1"].asFloat()-edge) continue;
	  hext->Fill(pex.X(),pex.Y());
	  std::vector<recoPoint*>::iterator icf=clusters.end();
	  double dist=999999.;
	  for (std::vector<recoPoint*>::iterator ic=clusters.begin();ic!=clusters.end();ic++)
	    {
	      if (abs((*ic)->Z()-jch["z0"].asFloat())>1E-3) continue;
	      ROOT::Math::XYZVector dex=pex-(*(*ic));
	      hderr->Fill(dex.X(),dex.Y());
	      if (dex.X()<3 && dex.Y()<3.)
		{
		  
		  
          /// A revoir float nx=(*ic)->hits().size();
          float nx=1.;
		  double errx=100./96./sqrt(12.)/sqrt(nx);
		  double erry=100./96./sqrt(12.)/sqrt(nx);
		  errx=1;erry=1.;
		  hdx->Fill(dex.X()/errx);
		  hdx2->Fill(nx*1.,dex.X()/errx);
		  hdy->Fill(dex.Y()/erry);
		  hdy2->Fill(nx*1.,dex.Y()/erry);
		  
		  if (dex.Mag2()<dist)
		    {
		      dist=dex.Mag2();
		      icf=ic;
		    }
		}
	    }
	  if (icf==clusters.end())
	    hmiss->Fill(pex.X(),pex.Y());
	  else
      {
	    hnear->Fill((*icf)->X(),(*icf)->Y());
        hfound->Fill(pex.X(),pex.Y());
      }
	}
      //STEP;
    }
}
void hitMonitor::clusterHistos(std::vector<TricotCluster> &tcl,std::vector<planeCluster*> &clusters,std::string tkdir)
{
  uint32_t minTkExtPoint=_geo->cuts()["tkExtMinPoint"].asUInt();
  float tkChi2=_geo->cuts()["tkChi2"].asFloat();
  float edge=_geo->cuts()["edge"].asFloat();
  //std::cout << minTkExtPoint<<" "<<tkChi2<<" "<<edge<<std::endl;
  // book all chamber
  uint32_t nch=0;
  //STEP;
  for (int ich=1;ich<60;ich++)
  {
    Json::Value jch=_geo->chamberGeo(ich);
    if (jch.empty()) continue;
    nch++;
    std::stringstream namec("");
    namec<<tkdir+"/Plan"<<ich;
    TH1* hposx = rootHandler_->GetTH1(namec.str()+"/XPos");      
    TH1* hposy = rootHandler_->GetTH1(namec.str()+"/YPos");      
    TH2* hfound= rootHandler_->GetTH2(namec.str()+"/found");
    if (hposx==0)
    {
      float xi=jch["x0"].asFloat()-20;
      float xa=jch["x1"].asFloat()+20;
      float yi=jch["y0"].asFloat()-20;   
      float ya=jch["y1"].asFloat()+20;
      int nx=int(xa-xi)+1+20;
      int ny=int(ya-yi)+1+20;
      hposx =rootHandler_->BookTH1( namec.str()+"/XPos",nx,xi,xa);
      hposy =rootHandler_->BookTH1( namec.str()+"/YPos",ny,yi,ya);
      hfound= rootHandler_->BookTH2(namec.str()+"/found",nx,xi,xa,ny,yi,ya);
    }
    
  }
  //STEP;
 
  for (std::vector<TricotCluster>::iterator it=tcl.begin();it!=tcl.end();it++)
  {
    //std::cout<<"tricot " <<(int) it->dif()<<" "<<it->X()<<std::endl;
    Json::Value dif=_geo->difGeo(it->dif());
    Json::Value ch=_geo->chamberGeo(dif["chamber"].asUInt());
    std::stringstream namec("");
    namec<<tkdir+"/Plan"<<ch["plan"].asUInt();
    TH1* hposx = rootHandler_->GetTH1(namec.str()+"/XPos");      
    TH1* hposy = rootHandler_->GetTH1(namec.str()+"/YPos");      
    TH2* hfound= rootHandler_->GetTH2(namec.str()+"/found");
    if (hposx==NULL) continue;
    
    hfound->Fill(it->X(),it->Y(),1.);
    hposx->Fill(it->X());
    hposy->Fill(it->Y());
  }
  
  //std::cout<<" fin des strips  aaaa " <<clusters.size()<<std::endl;
  for (std::vector<planeCluster*>::iterator it=clusters.begin();it!=clusters.end();it++)
  {
    //std::cout<<"pads " <<(*it)->plan()<<" "<<(*it)->X()<<std::endl;
    std::stringstream namec("");
    namec<<tkdir+"/Plan"<<(*it)->plan();
    TH1* hposx = rootHandler_->GetTH1(namec.str()+"/XPos");      
    TH1* hposy = rootHandler_->GetTH1(namec.str()+"/YPos");      
    TH2* hfound= rootHandler_->GetTH2(namec.str()+"/found");
    if (hposx==NULL) continue;
    
    hfound->Fill((*it)->X(),(*it)->Y(),1.);
    hposx->Fill((*it)->X());
    hposy->Fill((*it)->Y());
  }
}
