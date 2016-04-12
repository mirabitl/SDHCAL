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
	
void hitMonitor::FillTimeAsic(IMPL::LCCollectionVec* rhcol)
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
