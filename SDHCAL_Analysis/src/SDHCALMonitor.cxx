#include "SDHCALMonitor.h"
SDHCALMonitor::SDHCALMonitor(DHCalEventReader* r,DCHistogramHandler* h) : reader_(r),rootHandler_(h),theTrackIndex_(0),theFirstChamber_(1),theLastChamber_(50),
									  theExtrapolationMinimumPoint_(6),theExtrapolationMinimumChi2_(0.001),theExtrapolationDistanceCut_(8),theTrackAngularCut_(0.1),theChamberEdgeCut_(5.),useTk4_(true),theIntegratedTime_(0)
{
  theAsicCount_.clear();
}
void SDHCALMonitor::clear()
{
  theAsicCount_.clear();
  theTrackIndex_=0;
  theIntegratedTime_=0;
}
void SDHCALMonitor::setFirstChamber(uint32_t i) 
{theFirstChamber_=i;}

void SDHCALMonitor::setLastChamber(uint32_t i)
{theLastChamber_=i;}
void SDHCALMonitor::setExtrapolationMinimumPoint(uint32_t i) 
{theExtrapolationMinimumPoint_=i;}
void SDHCALMonitor::setExtrapolationMinimumChi2(float i) 
{theExtrapolationMinimumChi2_=i;}
void SDHCALMonitor::setChamberEdgeCut( float i) 
{theChamberEdgeCut_=i;} 
void SDHCALMonitor::setUseTk4(bool t)
{useTk4_=t;};
	
void SDHCALMonitor::FillTimeAsic(IMPL::LCCollectionVec* rhcol)
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
  this->DIFStudy(rhcol);
  //	printf("%d %s\n",__LINE__,__PRETTY_FUNCTION__);
  //IMPL::LCCollectionVec* rhcol=(IMPL::LCCollectionVec*) evt_->getCollection(collectionName_);
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
      if (bc>5E6) continue;

      if (bc<tmin) tmin=bc;
      if (bc>tmax) tmax=bc;
		
      std::map<unsigned int,DifGeom>::iterator idg = reader_->getDifMap().find(difid);
      DifGeom& difgeom = idg->second;
      uint32_t chid = idg->second.getChamberId();
      uint32_t key=(chid<<16)|(difid<<8)|asicid;
      std::map<uint32_t,uint32_t>::iterator it=theAsicCount_.find(key);
      if (theAsicCount_.find(key)!=theAsicCount_.end()) 
	it->second=it->second+1;
      else
	{
	  uint32_t n=1;
	  std::pair<uint32_t,uint32_t> p(key,n);
	  theAsicCount_.insert(p);
	}
    }
  //	printf("%d %s\n",__LINE__,__PRETTY_FUNCTION__);
  theIntegratedTime_+=(tmax-tmin);
  theEventIntegratedTime_=(tmax-tmin);
  //std::cout<<tmin<<" "<<tmax<<" => Event "<<theEventIntegratedTime_<<" total " <<theIntegratedTime_<<std::endl;
  for (std::map<uint32_t,uint32_t>::iterator it=theAsicCount_.begin();it!=theAsicCount_.end();it++)
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
      hoccall->SetBinContent(difid*48+asicid,it->second/(theIntegratedTime_*DCBufferReader::getDAQ_BC_Period()*1.E-6));
		
      hocc->SetBinContent(asicid,it->second/(theIntegratedTime_*DCBufferReader::getDAQ_BC_Period()*1.E-6));
      float focc=it->second/(theIntegratedTime_*DCBufferReader::getDAQ_BC_Period()*1.E-6);
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

void SDHCALMonitor::DIFStudy( IMPL::LCCollectionVec* rhcol,bool external)
{
  for (int i=0;i<rhcol->getNumberOfElements();i++)
    {
      IMPL::RawCalorimeterHitImpl* hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(i);
      if (hit==0) continue;
		
      unsigned int difid = hit->getCellID0()&0xFF;
      std::map<unsigned int,DifGeom>::iterator idg = reader_->getDifMap().find(difid);
      DifGeom& difgeom = idg->second;
      uint32_t chid = idg->second.getChamberId();
      int asicid = (hit->getCellID0()&0xFF00)>>8;
      int channel= (hit->getCellID0()&0x3F0000)>>16;
      //streamlog_out(MESSAGE)<<"ch-"<<channel<<std::endl;
      unsigned int bc = hit->getTimeStamp();
      //if (bc>20) continue;
      bool thr[3];
      //      DEBUG_PRINT("%x \n",hit->getCellID0());
      int ithr= hit->getAmplitude()&0x3;
      if (ithr<=0 || ithr>3)
	{
	  std::cout<<difid<<" had:"<<asicid<<":"<<channel<<":"<<bc<<":"<<ithr<<std::endl;
	  return;
	}
      thr[0] = (ithr == 1);
      thr[1] = (ithr == 2);
      thr[2] = (ithr == 3);
      int asic=asicid;int x,y;
      if (difid>1000) asic=(asic-1)%4+1; // Small chamber
      if (chid<49)
	DifGeom::PadConvert(asic,channel,x,y,2);
      else
	DifGeom::PadConvert(asic,channel,x,y,11);
      int difLocalI=int(x);
      int difLocalJ=int(y);
      int chamberLocalI=difgeom.toGlobalX(difLocalI);
      int chamberLocalJ=difgeom.toGlobalY(difLocalJ);

      //INFO_PRINT("%d %d %d %d %d %d \n",x,y,difLocalI,difLocalJ,chamberLocalI,chamberLocalJ);
      std::stringstream namec("");
      namec<<"/Noise/Chamber"<<chid<<"/DIF"<<difid;

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
      if (thr[0]||thr[2]) hhits0->SetBinContent((asic-1)*64+channel+1,hhits0->GetBinContent((asic-1)*64+channel+1)+1);
      if (thr[1]||thr[0]||thr[2])
	{
	  hhits1->SetBinContent((asic-1)*64+channel+1,hhits1->GetBinContent((asic-1)*64+channel+1)+1);
	  if (bc>8 && bc<20)
	    hhits1it->SetBinContent((asic-1)*64+channel+1,hhits1->GetBinContent((asic-1)*64+channel+1)+1);
	}
      if (thr[2]) hhits2->SetBinContent((asic-1)*64+channel+1,hhits2->GetBinContent((asic-1)*64+channel+1)+1);
      hetd->Fill(bc*1.);

      hetdz->Fill(bc*1.);

      std::stringstream namech("");
      namech<<"/Noise/Chamber"<<chid;

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
      if (thr[0]||thr[2]) hthr0->Fill(chamberLocalI*1.,chamberLocalJ*1.);
      if (thr[1]||thr[2]||thr[0])
	{
	  hthr1->Fill(chamberLocalI*1.,chamberLocalJ*1.);
	  if (bc<20) hthr1it->Fill(chamberLocalI*1.,chamberLocalJ*1.);
	}
      if (thr[2]) hthr2->Fill(chamberLocalI*1.,chamberLocalJ*1.);

    }

  //printf("Fill Hit \n");
  if (!external) return;
  std::vector<RecoHit*> theHitVector_;
  theHitVector_.clear();


  for (int i=0;i<rhcol->getNumberOfElements();i++)
    {
      IMPL::RawCalorimeterHitImpl* hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(i);
      if (hit==0) continue;




      unsigned int bc = hit->getTimeStamp();
      if (bc<8 || bc>20) continue;
      unsigned int difid = hit->getCellID0()&0xFF;
      if (difid<1 || difid>255) continue;
      int asicid = (hit->getCellID0()&0xFF00)>>8;
      int channel= (hit->getCellID0()&0x3F0000)>>16;
      bool thr[3];
      //      DEBUG_PRINT("%x \n",hit->getCellID0());
      int ithr= hit->getAmplitude()&0x3;
      if (ithr==0)
	{
	  std::cout<<difid<<" had:"<<asicid<<":"<<channel<<":"<<ithr<<std::endl;
	  continue;
	}
      std::map<unsigned int,DifGeom>::iterator idg = reader_->getDifMap().find(difid);
      DifGeom& difgeom = idg->second;
      int x=0,y=0;
      uint32_t chid = idg->second.getChamberId();

      uint32_t hrtype=2;
      DifGeom::PadConvert(asicid,channel,x,y,hrtype);
      uint32_t I=difgeom.toGlobalX(x);
      if (I<1 || I>96) continue;
      uint32_t J=difgeom.toGlobalY(y);
      if (J<1 || J>96) continue;
      if (chid<1 || chid>60) continue;

      RecoHit* h=  new RecoHit();
      //planes.set(chid-1,true);
      std::map<unsigned int,ChamberGeom>::iterator icg = reader_->getChamberMap().find( chid);
      ChamberGeom& chgeom = icg->second;
      //printf("Hit beeing filled %d %d %d\n",chid-1,I-1,J-1);
      chgeom.setZ(reader_->getPosition(chid).getZ0());

      h->initialise(difgeom,chgeom,hit,hrtype);
      theHitVector_.push_back(h);


    }

  // Now build clusters
  //printf("Build cluster \n");
  std::vector<RECOCluster> vCluster;
  vCluster.clear();
  for (std::vector<RecoHit*>::iterator ih=theHitVector_.begin();ih!=theHitVector_.end();ih++)
    {
      bool append= false;
      //std::cout<<"Clusters "<<vCluster.size()<<std::endl;
      RecoHit* hit=(*ih);
      for (std::vector<RECOCluster>::iterator icl=vCluster.begin();icl!=vCluster.end();icl++)
	if (icl->Append(*hit))
	  {
	    append=true;
	    break;
	  }
      if (append) continue;
      RECOCluster cl(*hit);
		
      vCluster.push_back(cl);
      // std::cout<<"Apres push Clusters "<<vCluster.size()<<std::endl;
    }




  // Merge adjacent clusters
  //std::cout<<"Avant merged Clusters "<<vCluster.size()<<std::endl;
  bool merged=false;
  do
    {
      merged=false;
      std::vector<RECOCluster> vNew;
      vNew.clear();
      for (uint32_t i=0;i<vCluster.size();i++)
	{
	  if (!vCluster[i].isValid()) continue;
	  for (uint32_t j=i+1;j<vCluster.size();j++)
	    {
	      if (!vCluster[j].isValid()) continue;
	      if (vCluster[i].isAdjacent(vCluster[j]))
		{
		  RECOCluster c;
		  for (std::vector<RecoHit>::iterator iht=vCluster[i].getHits()->begin();iht!=vCluster[i].getHits()->end();iht++)
		    c.getHits()->push_back((*iht));
		  for (std::vector<RecoHit>::iterator jht=vCluster[j].getHits()->begin();jht!=vCluster[j].getHits()->end();jht++)
		    c.getHits()->push_back((*jht));
		  vCluster[i].setValidity(false);
		  vCluster[j].setValidity(false);
					
					
		  //DEBUG_PRINT("Merged cluster %d %d \n",i,j);
		  vNew.push_back(c);
		  merged=true;
		  break;
		}
				
	    }
	}
      if (merged)
	{
	  for (std::vector<RECOCluster>::iterator jc=vCluster.begin();jc!=vCluster.end();)
	    {
				
	      if (!jc->isValid())
		vCluster.erase(jc);
	      else
		{
					
		  ++jc;
		}
	    }
	  //DEBUG_PRINT(" vCluster Size %d\n",vCluster.size());
	  //DEBUG_PRINT(" New clusters found %d\n",vNew.size());
	  for (std::vector<RECOCluster>::iterator ic=vNew.begin();ic!=vNew.end();ic++)
	    vCluster.push_back((*ic));
	  //DEBUG_PRINT(" New clusters found %d\n",vCluster.size());
	}
    } while (merged);
  //printf("Fill Histo \n");
  TH1* hnc= (TH1F*) rootHandler_->GetTH1("/Cluster/Ncluster");
  if (hnc==0)
    hnc=rootHandler_->BookTH1("/Cluster/Ncluster",65,-0.9,64.1);
  hnc->Fill(vCluster.size()*1.);
  if (vCluster.size()<=4)
  for ( std::vector<RECOCluster>::iterator ic=vCluster.begin();ic!=vCluster.end();ic++)
    {
      std::stringstream namec("");
      namec<<"/Cluster/Chamber"<<ic->chamber();


      TH1* hnp= (TH1F*) rootHandler_->GetTH1(namec.str()+"/Npad");
      TH1* hnp0= (TH1F*) rootHandler_->GetTH1(namec.str()+"/Npad0");
      TH1* hnp1= (TH1F*) rootHandler_->GetTH1(namec.str()+"/Npad1");
      TH1* hnp2= (TH1F*) rootHandler_->GetTH1(namec.str()+"/Npad2");

      
      TH2* hpos= (TH2F*) rootHandler_->GetTH2(namec.str()+"/ClusterPos");
      TH2* hpos0= (TH2F*) rootHandler_->GetTH2(namec.str()+"/ClusterPos0");
      TH2* hpos1= (TH2F*) rootHandler_->GetTH2(namec.str()+"/ClusterPos1");
      TH2* hpos2= (TH2F*) rootHandler_->GetTH2(namec.str()+"/ClusterPos2");
	
      if (hnp==0)
	{
	  hnp=rootHandler_->BookTH1(namec.str()+"/Npad",32,0.,32.);
	  hnp0=rootHandler_->BookTH1(namec.str()+"/Npad0",32,0.,32.);
	  hnp1=rootHandler_->BookTH1(namec.str()+"/Npad1",32,0.,32.);
	  hnp2=rootHandler_->BookTH1(namec.str()+"/Npad2",32,0.,32.);
	  hpos=rootHandler_->BookTH2(namec.str()+"/ClusterPos",100,0.,100.,100,0.,100.);
	  hpos0=rootHandler_->BookTH2(namec.str()+"/ClusterPos0",100,0.,100.,100,0.,100.);
	  hpos1=rootHandler_->BookTH2(namec.str()+"/ClusterPos1",100,0.,100.,100,0.,100.);
	  hpos2=rootHandler_->BookTH2(namec.str()+"/ClusterPos2",100,0.,100.,100,0.,100.);
	}
      hnp->Fill(ic->size()*1.);
      
      hpos->Fill(ic->X(),ic->Y());
      bool th0=false;
      bool th1=false;
      bool th2=false;
      uint32_t np0=0,np1=0,np2=0;
      for (int j=0;j<ic->size();j++)
	{
	  RecoHit& h=ic->getHits()->at(j);
	  if (h.getFlag(RecoHit::THR0)!=0) np0++;
	  if (h.getFlag(RecoHit::THR1)!=0) np1++;
	  if (h.getFlag(RecoHit::THR2)!=0) np2++;
	}
      if (np0>0) 
	{
	  hpos0->Fill(ic->X(),ic->Y());
	  hnp0->Fill(np0*1.);
	}
      if (np1>0) 
	{
	  hpos1->Fill(ic->X(),ic->Y());
	  hnp1->Fill(np1*1.);
	}
      if (np2>0) 
	{
	  hpos2->Fill(ic->X(),ic->Y());
	  hnp2->Fill(np2*1.);
	}

    }
  if (theHitVector_.size()!=0)
    for (std::vector<RecoHit*>::iterator ih=theHitVector_.begin();ih!=theHitVector_.end();ih++) delete (*ih);

}

void SDHCALMonitor::trackHistos(std::vector<RecoCandTk> &tracks,std::vector<RecoPoint> &points,std::string tkdir)
{
  /*
    TH1* htimedif = rootHandler_->GetTH1("TimeToTrigger");
    if (htimedif==NULL)
    {
    htimedif=rootHandler_->BookTH1( "TimeToTrigger",5000,0.,5000.);

    }
    htimedif->Fill(currentTime_*1.);
    //std::string tkdir="/Tracking";
    if (currentTime_>=(uint32_t)clockSynchCut_) tkdir="/OtherTracking";
  */
  for (unsigned int i=0;i<points.size();i++)
    {	 

      //if (points[i].getCluster().getHits()->size()>1) continue;
      std::stringstream namec("");


      namec<<tkdir+"/Plan"<<points[i].getChamberId();
      if (points[i].isUsed()) 
	namec<<"/OnTrack";
      else
	namec<<"/OffTrack";
      TH2* hpos = rootHandler_->GetTH2(namec.str()+"/XYPos");	   
      TH2* hcpos = rootHandler_->GetTH2(namec.str()+"/XYClusterPos");	   
      TH2* hposhit = rootHandler_->GetTH2(namec.str()+"/XYPosHit");	   
      TH1* hposmul = rootHandler_->GetTH1(namec.str()+"/Multiplicity");	   
      if (hpos==NULL)
	{
	  hpos=rootHandler_->BookTH2( namec.str()+"/XYPos",115,-10.1,110.1,115,-10.1,110.1);
	  hcpos=rootHandler_->BookTH2( namec.str()+"/XYClusterPos",100,0.,96.,100,0.,96.);
	  hposhit=rootHandler_->BookTH2( namec.str()+"/XYPosHit",96,0.,96.,96,0.,96.);
	  hposmul=rootHandler_->BookTH1( namec.str()+"/Multiplicity",50,0.,50.);
	}
      hpos->Fill(points[i].X(),points[i].Y());
      hcpos->Fill(points[i].getCluster().X(),points[i].getCluster().Y());
      hposmul->Fill(points[i].getCluster().getHits()->size()*1.);
      for (std::vector<RecoHit>::iterator ih=points[i].getCluster().getHits()->begin();ih!=points[i].getCluster().getHits()->end();ih++)
	hposhit->Fill(ih->X()*1.,ih->Y()*1.);	
    }

  TH1* hngood = rootHandler_->GetTH1(tkdir+"/NumberOfTracks");
  if (hngood==0)
    {
      hngood = rootHandler_->BookTH1(tkdir+"/NumberOfTracks",21,-0.1,20.9);
    }
  hngood->Fill(tracks.size()*1.);
  if (tracks.size()==0) return;
  for (unsigned int itk=0;itk<tracks.size();itk++)
    {
      theTrackIndex_++;
      RecoCandTk& tk = tracks[itk];
      //DEBUG_PRINT("Tk=%d Time=%d %f %f %f %f %f \n",theTrackIndex_,currentTime_,tk.ax_,tk.bx_,tk.ay_,tk.by_,tk.chi2_);
      TH1* htchi2 = rootHandler_->GetTH1(tkdir+"/Chi2");
      TH1* htpchi2 = rootHandler_->GetTH1(tkdir+"/ProbChi2");
      TH1* htnpoint = rootHandler_->GetTH1(tkdir+"/NumberOfPoints");
      TH1* htax = rootHandler_->GetTH1(tkdir+"/Ax");
      TH1* htay = rootHandler_->GetTH1(tkdir+"/Ay");
      TH1* htxh = rootHandler_->GetTH1(tkdir+"/xh");
      TH1* htyh = rootHandler_->GetTH1(tkdir+"/yh");

      if (htchi2==0)
	{
	  htchi2 = rootHandler_->BookTH1(tkdir+"/Chi2",500,0.,100.);
	  htpchi2 = rootHandler_->BookTH1(tkdir+"/ProbChi2",1000,0.,1.);
	  htnpoint = rootHandler_->BookTH1(tkdir+"/NumberOfPoints",60,0.,60.);
	  htax = rootHandler_->BookTH1(tkdir+"/Ax",200,-10.,10.);
	  htay = rootHandler_->BookTH1(tkdir+"/Ay",200,-10.,10.);
	  htxh = rootHandler_->BookTH1(tkdir+"/xh",6000,0.,6000.);
	  htyh = rootHandler_->BookTH1(tkdir+"/yh",6000,0.,6000.);

	}

      htchi2->Fill(tk.chi2_/(2*tk.getList().size()-4.));
      htpchi2->Fill(tk.prChi2_);
      //DEBUG_PRINT("track %f %d %f %f \n",tk.chi2_,2*tk.getList().size()-4,TMath::Prob(tk.chi2_,2*tk.getList().size()-4),tkChi2Cut_);
      //getchar();
      htnpoint->Fill(tk.getList().size()*1.);

      htax->Fill(tk.ax_);

      htay->Fill(tk.ay_);


      for (unsigned int ich=0;ich<61;ich++)
	{
	  uint32_t bintk=((theTrackIndex_-1)%100)*60+ich+1;
	  htxh->SetBinContent(bintk,0);
	  htyh->SetBinContent(bintk,0);
	}

      //       if (tracks.size()>1)
      //DEBUG_PRINT("\t %d good hits found \n",tk.getList().size());
      for (unsigned int i =0;i<tk.getList().size();i++)
	{
	  // if (tracks.size()>1)
	  //   DEBUG_PRINT("\t \t %f %f %f \n",tk.getList()[i].X(),tk.getList()[i].Y(),tk.getList()[i].Z());
	  //(tk.getList())[i].Print();
	  std::stringstream namec("");
	  namec<<tkdir+"/Plan"<<(tk.getList())[i]->getChamberId();
	  TH1* hposx = rootHandler_->GetTH1(namec.str()+"/XPos");	   
	  TH1* hposy = rootHandler_->GetTH1(namec.str()+"/YPos");	   
	  TH1* hpullx = rootHandler_->GetTH1(namec.str()+"/XDist");	   
	  TH1* hpully = rootHandler_->GetTH1(namec.str()+"/YDist");	   
	  TH1* hmult = rootHandler_->GetTH1(namec.str()+"/Multiplicity");	   

	  if (hposx==0)
	    {
				
	      hposx =rootHandler_->BookTH1( namec.str()+"/XPos",115,-10.,110.);
	      hposy =rootHandler_->BookTH1( namec.str()+"/YPos",115,-10.,110.);
	      hpullx =rootHandler_->BookTH1( namec.str()+"/XDist",200,-5.,5.);
	      hpully =rootHandler_->BookTH1( namec.str()+"/YDist",200,-5.,5.);
	      hmult =rootHandler_->BookTH1( namec.str()+"/Multiplicity",50,0.,50.);
				
	    }
	  uint32_t bintk=((theTrackIndex_-1)%100)*60+(tk.getList())[i]->getChamberId();
	  htxh->SetBinContent(bintk,(tk.getList())[i]->X());
	  htyh->SetBinContent(bintk,(tk.getList())[i]->Y());

	  hposx->Fill((tk.getList())[i]->X());
	  hposy->Fill((tk.getList())[i]->Y());
	  hpullx->Fill((tk.getList())[i]->X() -tk.getXext((tk.getList())[i]->Z()) );
	  hpully->Fill((tk.getList())[i]->Y() -tk.getYext((tk.getList())[i]->Z()) );
	  hmult->Fill(tk.getList()[i]->getCluster().getHits()->size()*1.);
	}


      if (fabs(tk.ax_)>theTrackAngularCut_ || fabs(tk.ay_)>theTrackAngularCut_) continue;
      if (tracks.size()!=1) continue;

      std::bitset<255> intrack;
      bool synch= false;
      unsigned int s_shift=0;
      if (synch) 
	intrack.set(s_shift,true);
      else
	{
	  s_shift =100;
	  intrack.set(s_shift,true);

	}
      for (std::map<unsigned int,ChamberGeom>::iterator ip=reader_->getChamberMap().begin();ip!=reader_->getChamberMap().end();ip++)
	{
	  // std::cout<<ip->first<<" "<<ip->second.getZ()<<std::endl;
	  //getchar();
	  int chid = ip->first;
	  int32_t interval=3;
	  if (chid<=theFirstChamber_+1) interval=5;
	  if (chid>=theLastChamber_-1) interval=5;

	  int32_t tkFirstEx=((chid-interval)>theFirstChamber_)?(chid-interval):theFirstChamber_;
	  int32_t tkLastEx=((chid+interval)<theLastChamber_)?(chid+interval):theLastChamber_;
	  //std::cout<<chid<<" "<<tkFirstEx<<" "<<tkLastEx<<std::endl;

	  RecoCandTk tk0;
	  for (unsigned int j =0;j<tk.getList().size();j++)
	    {
	      //std::cout<<fabs(tk.getList()[j].Z()-zplane[ip])<<std::endl;
	      if (tk.getList()[j]->getChamberId()==chid) continue;
				
	      tk0.addPoint(*tk.getList()[j]);
	    }
	  //std::cout<<" extra "<<chid<<" " <<tk0.getList().size()<<std::endl;
	  float xext=-999999,yext=-999999;
	  if (useTk4_ && chid>=theFirstChamber_ && chid<=theLastChamber_)
	    {
	      RecoCandTk tk4;
	      for (unsigned int j =0;j<tk.getList().size();j++)
		{
		  //std::cout<<fabs(tk.getList()[j].Z()-zplane[ip])<<std::endl;
		  if (tk.getList()[j]->getChamberId()==chid) continue;
		  if (tk.getList()[j]->getChamberId()<tkFirstEx) continue;
		  if (tk.getList()[j]->getChamberId()>tkLastEx) continue;
		  tk4.addPoint(*tk.getList()[j]);

		  // if (chid==theLastChamber_)
		  //   if (tk.getList()[j]->getChamberId()==(chid-3)) tk4.addPoint(*tk.getList()[j]);
		  // if (chid>=theFirstChamber_+2)
		  //   if (tk.getList()[j]->getChamberId()==(chid-2)) tk4.addPoint(*tk.getList()[j]);
		  // if (chid>=theFirstChamber_+1)
		  // if (tk.getList()[j]->getChamberId()==(chid-1)) tk4.addPoint(*tk.getList()[j]);

		  // if (chid<=theLastChamber_-1)
		  //   if (tk.getList()[j]->getChamberId()==(chid+1)) tk4.addPoint(*tk.getList()[j]);
		  // if (chid<=theLastChamber_-2)
		  //   if (tk.getList()[j]->getChamberId()==(chid+2)) tk4.addPoint(*tk.getList()[j]);
		  // if (chid==theFirstChamber_)
		  //   if (tk.getList()[j]->getChamberId()==(chid+3)) tk4.addPoint(*tk.getList()[j]);
					

		}
	      tk4.regression();
	      if (tk4.getList().size()>=3 && TMath::Prob(tk4.chi2_,2*tk4.getList().size()-4)>theExtrapolationMinimumChi2_)
		{
		  xext=tk4.getXext(ip->second.getZ());
		  yext=tk4.getYext(ip->second.getZ());
		}
	      else 
		continue;

	    }
	  if (xext==-999999 || yext==-999999)
	    {
	      tk0.regression();
	      if (tk0.getList().size()<(uint32_t) theExtrapolationMinimumPoint_ || TMath::Prob(tk0.chi2_,2*tk0.getList().size()-4)<theExtrapolationMinimumChi2_) continue;
	      xext=tk0.getXext(ip->second.getZ());
	      yext=tk0.getYext(ip->second.getZ());
	    }
	  double xchext = ip->second.toLocalX(xext);
	  double ychext = ip->second.toLocalY(yext);
	  double zch=ip->second.getZ();
	  //std::cout <<xext<<" " <<yext<<" "<<xchext<<" " <<ychext<< " "<<zch<<" "<<ip->second.getZ()<<std::endl;
	  ip->second.calculateLocal(xext,yext,0,xchext,ychext,zch);
	  //std::cout <<xext<<" " <<yext<<" "<<xchext<<" " <<ychext<< " "<<zch<<" "<<ip->second.getZ()<<std::endl;
	  //	  getchar();
	  if (xchext<theChamberEdgeCut_ || xchext >96-theChamberEdgeCut_) continue;
	  if (ychext<theChamberEdgeCut_ || ychext >96-theChamberEdgeCut_) continue;
	  if (s_shift+ip->first<255)
	    intrack.set(s_shift+ip->first,true);
	  std::stringstream namec("");
	  namec<<tkdir+"/Plan"<<ip->first;

	  TH2* hextpos = rootHandler_->GetTH2( namec.str()+"/LocalExtrapolationMap");
	  TH2* hfoundpos = rootHandler_->GetTH2( namec.str()+"/LocalFoundMap");
	  TH2* hnearpos = rootHandler_->GetTH2( namec.str()+"/LocalNearestMap");
	  TH2* hmispos = rootHandler_->GetTH2( namec.str()+"/LocalMissedMap");
	  if (hextpos == NULL)
	    {
	      hextpos =rootHandler_->BookTH2( namec.str()+"/LocalExtrapolationMap",96,0.,96.,96,0.,96.);
	      hfoundpos =rootHandler_->BookTH2( namec.str()+"/LocalFoundMap",96,0.,96.,96,0.,96.);
	      hnearpos =rootHandler_->BookTH2( namec.str()+"/LocalNearestMap",96,0.,96.,96,0.,96.);
	      hmispos =rootHandler_->BookTH2( namec.str()+"/LocalMissedMap",96,0.,96.,96,0.,96.);

	    }
	  hextpos->Fill(xchext,ychext);
	  //std::cout<<xext<<" "<<yext<<std::endl;
	  unsigned int imin=999999;double distmin=9999999;
	  for (unsigned int irp=0;irp<points.size();irp++)
	    {
	      if (points[irp].getChamberId()!=chid) continue;
	      double dist =sqrt((points[irp].X()-xext)*(points[irp].X()-xext)+(points[irp].Y()-yext)*(points[irp].Y()-yext));
	      //std::cout<<chid<<" "<<dist<<std::endl;
	      if (dist<distmin)
		{
		  distmin=dist;
		  imin=irp;
		}
	    }
	  if (imin == 999999)
	    {
	      hmispos->Fill(xchext,ychext);
	      //		std::cout<<" chamber "<<chid<<" not found ("<<xchext<<","<<ychext<<") "<<tk0.ax_<<":"<<tk0.ay_<<":"<<tk0.chi2_<<std::endl;
	    }
	  // else
	  //   std::cout<<chid<<" is found "<<std::endl;
	  if (distmin<theExtrapolationDistanceCut_*1000)
	    {
	      std::stringstream namec("");
	      namec<<tkdir+"/Plan"<<ip->first;
	      TH1* hresol = rootHandler_->GetTH1(namec.str()+"/Resolution");	  
	      TH1* hresx = rootHandler_->GetTH1(namec.str()+"/XPull");	  
	      TH1* hresy = rootHandler_->GetTH1(namec.str()+"/YPull");	  
	      if (hresol==0)
		{
		  hresol =rootHandler_->BookTH1(namec.str()+"/Resolution",200,0.,50.);
		  hresx =rootHandler_->BookTH1( namec.str()+"/XPull",2000,-150.,150.);
		  hresy =rootHandler_->BookTH1( namec.str()+"/YPull",2000,-150.,150.);
		}
	      hresol->Fill(distmin);
	      hresx->Fill(points[imin].X()-xext);
	      hresy->Fill(points[imin].Y()-yext);
				

	      if (distmin<theExtrapolationDistanceCut_)
		{
		  if (s_shift+ip->first+60<255)
		    intrack.set(s_shift+ip->first+60,true);
		  double xchnear = ip->second.toLocalX(points[imin].X());
		  double ychnear = ip->second.toLocalY(points[imin].Y());
		  double zch;
		  ip->second.calculateLocal(points[imin].X(),points[imin].Y(),0,xchnear,ychnear,zch);

		  hnearpos->Fill(xchnear,ychnear);
		  hfoundpos->Fill(xchext,ychext);
					
		}
	      else
		if (1<0)
		  DEBUG_PRINT("Dist = %f X =%f Y =%f \n",distmin,points[imin].X()-xext,points[imin].Y()-yext);
	    }
	}
      TH1* hintrack= rootHandler_->GetTH1("PlanInTrack");
      if (hintrack==NULL)
	{
	  hintrack =rootHandler_->BookTH1( "PlanInTrack",255,-0.1,254.9);
	}
      for (unsigned int ib=0;ib<255;ib++)
	if (intrack[ib]!=0) hintrack->Fill(ib*1.);
		
    }

}

