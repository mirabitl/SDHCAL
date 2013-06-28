#include "LMAnalyzer.h"

class LMFR
{
public:
  int time;
  std::map<int,IMPL::RawCalorimeterHitImpl*> mhit;
  int countHits(int level)
  {
    int nh=0;
    for (std::map<int,IMPL::RawCalorimeterHitImpl*>::iterator kt=mhit.begin(); kt!=mhit.end();kt++)
      {
       int ithr= (*kt).second->getAmplitude();
       if (ithr & (1<<level)) nh++;
      }
    return nh;
  }


};

class LMHR
{
public:
  int asicid;
  std::map<int,LMFR> mfr;
  int getNumberOfFrames(int lowtime=-1,int hightime=999999)
  {
    int nf=0;
    for (std::map<int,LMFR>::iterator kt=mfr.begin(); kt!=mfr.end();kt++)
      {
	  LMFR thefr =(*kt).second;
	  if (thefr.time<lowtime || thefr.time>hightime) continue;
	  nf++;
	}
    return nf;
  }
  int getNumberOfHits(int level,int lowtime=-1,int hightime=999999)
  {
    int nh=0;
    for (std::map<int,LMFR>::iterator kt=mfr.begin(); kt!=mfr.end();kt++)
      {
	  LMFR thefr =(*kt).second;
	  if (thefr.time<lowtime || thefr.time>hightime) continue;
	  
	  nh+= thefr.countHits(level);
	}
    return nh;
  }

};
class LMEDIF
{
public:
  int event_time;
  std::map<int,LMHR> mhr;
};

class LMDIF
{
public:
  int difid;
  std::map<int,LMHR> mhr;
  std::map<int,LMEDIF> mtim;
  int getNumberOfFrames(int lowtime=-1,int hightime=99999)
  {
    int nf=0;
    for (std::map<int,LMHR>::iterator kt=mhr.begin(); kt!=mhr.end();kt++)
      {
	  LMHR thehr =(*kt).second;

	  nf+=thehr.getNumberOfFrames(lowtime,hightime);
	}
    return nf;
  }
  int getNumberOfHits(int level,int lowtime=-1,int hightime=999999)
  {
    int nh=0;
     for (std::map<int,LMHR>::iterator kt=mhr.begin(); kt!=mhr.end();kt++)
      {
	  LMHR thehr =(*kt).second;
	  nh+=thehr.getNumberOfHits(level,lowtime,hightime);
	}
    return nh;
  }

};


class LMHCAL
{
public:
  std::map<int,LMDIF> mdif;
};

LMAnalyzer::LMAnalyzer(DHCalEventReader* r,DCHistogramHandler* h) 
{
  reader_=r;
  handler_ =h;
  
  nAnalyzed_=0; nInSynch_=0;
  lasttime_=0;
  collectionName_="DHCALRawHits";


  noisesum_.clear();


}

void LMAnalyzer::registerDIF(int id)
{
  pair<unsigned int,unsigned int> p2(0,0);
  pair< unsigned int,pair<unsigned int,unsigned int> > ps2(id,p2);
  noisesum_.insert(ps2);
}


bool LMAnalyzer::decodeTrigger(LCCollection* rhcol, float tcut)
{
  if (rhcol->getNumberOfElements()==0) return true;

  // Find Trigger information

  IntVec vtrig21;IMPL::RawCalorimeterHitImpl* hit;
  try {
    hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(0);
  }
  catch (exception e)
    {
      std::cout<<"No hits "<<endl;
      return false;
    }
  if (hit==0) return false;
  unsigned int difid = hit->getCellID0()&0xFF;


  std::stringstream pname("");
  pname <<"DIF"<<difid<<"_Triggers";

  rhcol->getParameters().getIntVals(pname.str(),vtrig21);
  //  cout<<pname.str()<<" "<<vtrig21.size()<<endl;
 if (vtrig21.size()==0) return false; 
  


  unsigned long long  lbc=0;
  unsigned long long  lb5=vtrig21[5] & 0xFFFFFFFF ;
  unsigned long long  lb4=vtrig21[4]&0xFFFFFFFF ;
  
  lbc = (lb5<<32) | lb4;
 //  if (lb5!=0) 
//     {
//       std::cout<<lb4<<endl;
//       std::cout<<lb5<<endl;
//       std::cout<<lbc<<endl;
//       getchar();
//     }
  // Fill Trigger info
  TH1* htspill= handler_->GetTH1("SpillDif");
  if (htspill==NULL)
    {
      htspill =handler_->BookTH1( "SpillDif",500,0.,100.);
    }



  float tdif = lbc*200E-9 - lasttime_;
  if (tdif>50 || tdif <-1E-3)
    {
      cout<<tdif << "  "<<lasttime_/200E-9<<endl;
      std::cout<<lb4<<endl;
       std::cout<<lb5<<endl;
       std::cout<<lbc<<endl;
       // getchar();

    }
  if (tdif>tcut) {std::cout<<tdif<<std::endl; htspill->Fill(tdif);}
  lasttime_=lbc*200E-9;
  //  for (unsigned int i=0;i<vtrig21.size();i++) std::cout<<i<<" "<<vtrig21[i]<<std::endl;
  if (tdif>tcut) return false;


  TH1* htdiff= handler_->GetTH1("TimeDif");
  if (htdiff==NULL)
    {
      htdiff =handler_->BookTH1( "TimeDif",500,0.,10.);
    }
  htdiff->Fill(tdif*1000.);

  return true;
}

void LMAnalyzer::processEvent()
{
  
  if (reader_->getEvent()==0) return;
  try {
    if (reader_->getEvent()->getEventNumber()==1)
      {
	nAnalyzed_=0; nInSynch_=0;}
    
  } catch (std::string s)
    {
      std::cout<<" error "<<s<<std::endl;
    }
  int hrtype = reader_->getRunHeader()->parameters().getIntVal("HardRoc");


  // LMHCAL thehcal;


  // Loop on all frames
  std::stringstream name("");int x,y;




  recevents_.clear();
  //  std::cout<<"Collection " <<collectionName_<<std::endl;
  LCCollection* rhcol=0;
  try 
    {
      rhcol = reader_->getEvent()->getCollection(collectionName_);
    }
  catch (...)
    {
      std::cout<<" No collection try to build it" <<std::endl;
      reader_->buildEvent();
      rhcol = reader_->getEvent()->getCollection(collectionName_);
    }
  if (rhcol == 0) return;


  if (!decodeTrigger(rhcol,1.)) return;

  // Event is accepted
  nAnalyzed_++;

  bool inSynch=false;


  //Loop on hits and fill recevents_
  for (int i=0;i<rhcol->getNumberOfElements();i++)
    {
      IMPL::RawCalorimeterHitImpl* hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(i);
      if (hit==0) continue;
      unsigned int difid = hit->getCellID0()&0xFF;
      int asicid = (hit->getCellID0()&0xFF00)>>8;
      int channel= (hit->getCellID0()&0x3F0000)>>16;
      //std::cout<<"ch-"<<channel<<std::endl;
      unsigned int bc = hit->getTimeStamp();

      //      std::cout<<i<<" "<<bc<<" "<<hit->getCellID1()<<std::endl;

      // Find or build the event     
      if (recevents_.find(bc) == recevents_.end())
	{
	  std::map<unsigned int,RecoDIF> m;
	  std::pair<unsigned int, std::map<unsigned int,RecoDIF> > pm(bc,m);
	  recevents_.insert(pm);

	}
      std::map<unsigned int, std::map<unsigned int,RecoDIF> >::iterator ktc= recevents_.find(bc);
      std::map<unsigned int,RecoDIF>& mrecobc = ktc->second;

      // Find or build the recodif
      if (ktc->second.find(difid) == ktc->second.end())
	{

	  RecoDIF a(difid);
	  std::pair<unsigned int,RecoDIF> p(difid,a);
	  ktc->second.insert(p);

	}


      //      std::cout<<bc<<std::endl;
      bool thr[2];
      //      printf("%x \n",hit->getCellID0());
      int ithr= hit->getAmplitude();
      //std::cout<<difid<<":"<<asicid<<":"<<channel<<":"<<bc<<":"<<ithr<<std::endl;
      thr[0] = ithr &1;
      thr[1] = ithr &2;

      if (thr[0]) {
	// Threshold unused until further notice	
	int asic=asicid;
	if (difid>1000) asic=(asic-1)%4+1;
	DCBufferReader::RKPadConvert(asic,channel,x,y,hrtype);
      }

      if (thr[1]|| thr[0]) 
	{
	  
	  int asic=asicid;
	  if (difid>1000) asic=(asic-1)%4+1;
	  //	DCBufferReader::PadConvert(asic,channel,x,y);
	  DCBufferReader::RKPadConvert(asic,channel,x,y,hrtype);
	  // Add the hit to the recodif
	  RecoHitAddress b((unsigned char)x,(unsigned char)y);
	  std::map<unsigned int,RecoDIF>::iterator itc = mrecobc.find(difid);
	      // std::cout<<"Ädding a hit to "<<std::endl;
	  if (itc!=mrecobc.end())
	    {

	      itc->second.AddHit(b);
	    }
	  
	}
    }
  // Now Fill thehistograms
  fillHistos(10);

}
void LMAnalyzer::fillHistos(unsigned int synchcut)
{
  //  cout<<" new event "<<recevents_.size()<<std::endl;
  int nbinx = 96;
  int nbiny= 32;
  int tfirst=0,tlast=0;
  map<unsigned int,unsigned int> noisefreq;
  noisefreq.clear();
  for (map<unsigned int,pair<unsigned int,unsigned int> >::iterator isum=noisesum_.begin();isum!=noisesum_.end();isum++)
    {
      pair<unsigned int,unsigned int> p2(isum->first,0);
      noisefreq.insert(p2);
    }

  std::map <unsigned int, std::map<unsigned int,RecoDIF> >::reverse_iterator ilarger = recevents_.rbegin();



  bool inSynch=false;
  TH1* hetime= handler_->GetTH1("EventTime");
  if (hetime==NULL)
    {
      hetime =handler_->BookTH1( "EventTime",5000,0.,5000.);
    }


  for (std::map <unsigned int, std::map<unsigned int,RecoDIF> >::iterator jt = recevents_.begin();jt!=recevents_.end();jt++)
    {
      std::map<unsigned int,RecoDIF> recos=jt->second;
      hetime->Fill(jt->first*1.);
      // Drop the last bx if not alone
      if (jt->first == ilarger->first && jt!=recevents_.begin()) 
	{
	  //	  cout<<"Last one"<<endl;
	  continue;
	}
      if (jt->first<synchcut) 
	inSynch=true;
      //  std::cout<<"Bx "<<jt->first<<" # dif "<<recos.size()<<std::endl;

      for (std::map<unsigned int,RecoDIF>::iterator it = recos.begin();it!=recos.end();it++)
	{
	  //  std:: cout<<"\t DIF " <<it->first<<" #clusters "<<it->second.getClusters().size()<<std::endl;
	  std::stringstream name("");
	  RecoDIF& r= it->second;
	  if (jt->first<synchcut)
	    {
	      name<<"/DIF"<<r.getDifId()<<"/Synchronised";
	      TH1* htmin= handler_->GetTH1("SynchronizedMinimum");
	      if (htmin==NULL)
		{
		  htmin =handler_->BookTH1( "SynchronizedMinimum",10,0.,10.);
		}
	      htmin->Fill(jt->first*1.);
	      tfirst=jt->first;
	    }
	  else
	    {
	      name<<"/DIF"<<r.getDifId()<<"/OffTime";
	      map<unsigned int,unsigned int>::iterator ifq=noisefreq.find(r.getDifId());
	      ifq->second=ifq->second+r.getClusters()->size();
	    }
	  tlast =jt->first;
	  	  
	  
	  TH2* hclpos = handler_->GetTH2(name.str()+"/ClusterMap");
	  TH2* hclhit = handler_->GetTH2(name.str()+"/HitMap");
	  TH1* hclposx = handler_->GetTH1(name.str()+"/ClusterMapX");
	  TH1* hclposy = handler_->GetTH1(name.str()+"/ClusterMapY");
	  if (hclpos==NULL)
	    {
	      hclpos =handler_->BookTH2( name.str()+"/ClusterMap",nbinx,0.1,nbinx+0.1,nbiny,0.1,nbiny+0.1);
	      hclhit =handler_->BookTH2( name.str()+"/HitMap",nbinx,0.1,nbinx+0.1,nbiny,0.1,nbiny+0.1);
	      hclposx =handler_->BookTH1( name.str()+"/ClusterMapX",nbinx,0.1,nbinx+0.1);
	      hclposy =handler_->BookTH1( name.str()+"/ClusterMapY",nbiny,0.1,nbiny+0.1);
	    }
	      
	  TH1* hclsize = handler_->GetTH1(name.str()+"/ClusterSize");
	  if (hclsize==NULL)
	    {
	      hclsize =handler_->BookTH1( name.str()+"/ClusterSize",20,0.,20.);
	    }
	      
	  TH1* hclmul = handler_->GetTH1(name.str()+"/ClusterMultiplicity");
	  if (hclmul==NULL)
	    {
	      hclmul =handler_->BookTH1( name.str()+"/ClusterMultiplicity",20,0.,20.);
	    }
	      
	  hclmul->Fill(r.getClusters()->size()*1.);

	  // std::cout<<r.getDifId()<<":"<<r.getClusters().size()<<" ";
	  for (std::vector<RecoCluster>::iterator ic=r.getClusters()->begin();ic!=r.getClusters()->end();ic++)
	    {
	      hclpos->Fill(ic->X(),ic->Y());
	      hclposx->Fill(ic->X());
	      hclposy->Fill(ic->Y());
		  
	      hclsize->Fill(ic->getHits()->size()*1.);
	      for (std::vector< RecoHitAddress >::iterator iht=ic->getHits()->begin();iht!=ic->getHits()->end();iht++)
		{
		  hclhit->Fill(iht->first*1.,iht->second*1.);
		}
	    }
	    
	}
    }

    
  // std::cout<<std::endl;
  //      getchar();
  if (tlast>tfirst)
    for(map<unsigned int,unsigned int>::iterator ifq=noisefreq.begin();ifq!=noisefreq.end();ifq++)
      {
	//	std::cout<<ifq->first<<" "<<ifq->second<<std::endl;
	std::stringstream name("");
	name<<"/DIF"<<ifq->first<<"/NoiseFrequency";
	TH1* h = handler_->GetTH1(name.str());
	if (h==NULL)
	  {
	    h =handler_->BookTH1( name.str(),2000,0.,50.);
	}
	float fq=ifq->second/2E-7/(tlast-tfirst)/48./64.;
	h->Fill(fq);
	map<unsigned int,pair<unsigned int,unsigned int> >::iterator isum=noisesum_.find(ifq->first);
	isum->second.first=isum->second.first+ifq->second;
	isum->second.second=isum->second.second+(tlast-tfirst);

	//	std::cout<<isum->first<<" "<<isum->second.first<<" "<<isum->second.second<<std::endl;
      }   



    
  if (inSynch) nInSynch_++;
  
 

}



void LMAnalyzer::endJob()
{ 
  std::cout<<"Efficiency "<<nAnalyzed_<<" "<<nInSynch_<<" "<<nInSynch_*100./nAnalyzed_<<" %"<<std::endl;
  for (map<unsigned int,pair<unsigned int,unsigned int> >::iterator isum=noisesum_.begin();isum!=noisesum_.end();isum++)
    {
      std::cout<<"NoiseOccupancy  "<<isum->first<<" "<<isum->second.first<<" "<<isum->second.second<<std::endl;
    }
}







#ifdef OLD_DUMP
      //getchar();
      //      LMFR fr;
//       thehcal.mdif[difid].mhr[asicid].mfr[bc].mhit[channel]=hit;
//       thehcal.mdif[difid].mtim[bc].mhr[asicid].mfr[bc].mhit[channel]=hit;
//       thehcal.mdif[difid].difid=difid;
//       thehcal.mdif[difid].mhr[asicid].asicid=asicid;
//       thehcal.mdif[difid].mhr[asicid].mfr[bc].time=bc;
//       thehcal.mdif[difid].mtim[bc].event_time=bc;
//       thehcal.mdif[difid].mtim[bc].mhr[asicid].asicid=asicid;
//       thehcal.mdif[difid].mtim[bc].mhr[asicid].mfr[bc].time=bc;
	//.mhr[asicid].mfr[bc] = hit;

  for (std::map<int,LMDIF>::iterator it=thehcal.mdif.begin();it!=thehcal.mdif.end();it++)
    {
      std::cout<<"New DIF found "<<(*it).first<<" "<<(*it).second.difid<<std::endl;
      LMDIF thedif=(*it).second;
      for (std::map<int,LMEDIF>::iterator lt=thedif.mtim.begin();lt!=thedif.mtim.end();lt++)
	{
	  std::cout<<"New Event DIF found "<<(*lt).first<<" "<<(*lt).second.event_time<<std::endl;
      LMEDIF theedif=(*lt).second;
      for (std::map<int,LMHR>::iterator jt=theedif.mhr.begin(); jt!=theedif.mhr.end();jt++)
	{
	  LMHR thehr =(*jt).second;
	  std::cout<<"\t HR "<<(*jt).first<<" "<<thehr.asicid<<"  # frames "<<thehr.mfr.size()<<std::endl;

	  for (std::map<int,LMFR>::iterator kt=thehr.mfr.begin(); kt!=thehr.mfr.end();kt++)
	{
	  LMFR thefr =(*kt).second;
	  std::cout<<"\t \t Frame "<<(*kt).first<<" "<<thefr.time<<"  # hit "<<thefr.mhit.size()<<std::endl;
	}
	}

    }
    }
  //  getchar();
#endif
