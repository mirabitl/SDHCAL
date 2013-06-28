#include "LMNewAnalyzer.h"



LMNewAnalyzer::LMNewAnalyzer(DHCalEventReader* r,DCHistogramHandler* h) : dropFirstSpillEvent_(true), dropFirstPowerPulsed_(false), nAnalyzed_(0), nInSynch_(0),
									  externalTriggerTime_(0), spillSize_(10.), clockSynchCut_(10), collectionName_("DHCALRawHits")
{
  reader_=r;
  handler_ =h;
  


  noiseSummaryMap_.clear();
  padMask_.clear();

  lastPowerPulsedTime_=0;
}

bool LMNewAnalyzer::isFirstPowerPulsed(int cutms)
{
  bool t=false;
  t= (getExternalTriggerTime()-lastPowerPulsedTime_)*400E-6>cutms && dropFirstPowerPulsed_;

  std::cout << getExternalTriggerTime()-lastPowerPulsedTime_<<" "<<(getExternalTriggerTime()-lastPowerPulsedTime_)*DAQ_BC_Period*1.E-6<<std::endl;
  lastPowerPulsedTime_ = getExternalTriggerTime();
  return t;

}


void LMNewAnalyzer::mask(unsigned int difid,unsigned int ix,unsigned int iy)
{
  unsigned int m= ((difid&0xFFFF)<<16) | ((ix&0XFF)<<8) | (iy&0XFF);
  if (std::find(padMask_.begin(),padMask_.end(),m)==padMask_.end()) padMask_.push_back(m);
}

bool LMNewAnalyzer::isMasked(unsigned int difid,unsigned int ix,unsigned int iy)
{
  unsigned int m= ((difid&0xFFFF)<<16) | ((ix&0XFF)<<8) | (iy&0XFF);
  return (std::find(padMask_.begin(),padMask_.end(),m)!=padMask_.end());
}


void LMNewAnalyzer::registerDIF(int id)
{
  pair<unsigned int,unsigned int> p2(0,0);
  pair< unsigned int,pair<unsigned int,unsigned int> > ps2(id,p2);
  noiseSummaryMap_.insert(ps2);
}


bool LMNewAnalyzer::decodeTrigger(LCCollection* rhcol, float tcut)
{
  // if (rhcol->getNumberOfElements()==0) return true;

  // Find Trigger information
  IntVec vTrigger;IMPL::RawCalorimeterHitImpl* hit;
  unsigned int difid=0;
  // Find the first read DIF id for this trigger

  if (rhcol->getNumberOfElements()!=0)
    {
      
      try {
	hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(0);
      }
      catch (exception e)
	{
	  std::cout<<"No hits "<<endl;
	  return false;
	}
      if (hit!=0) 
	difid = hit->getCellID0()&0xFF;
    }
  
  if (difid==0) return false;

  //Find the parameters
  std::stringstream pname("");
  pname <<"DIF"<<difid<<"_Triggers";
  
  rhcol->getParameters().getIntVals(pname.str(),vTrigger);

 if (vTrigger.size()==0) return false; 
  

 // Decode Large Bunch Crossing
  unsigned long long  lbc=0;
  unsigned long long  lb5=vTrigger[5] & 0xFFFFFFFF ;
  unsigned long long  lb4=vTrigger[4]&0xFFFFFFFF ;
  
  lbc = (lb5<<32) | lb4;
  // Fill Trigger info
  TH1* htspill= handler_->GetTH1("SpillDif");
  if (htspill==NULL)
    {
      htspill =handler_->BookTH1( "SpillDif",500,0.,100.);
    }


  // Calculate tiem differences since the last trigger
  double tdif = (lbc-externalTriggerTime_)*(DAQ_BC_Period*1E-6);
#ifdef DEBUG
  if (tdif>50 || tdif <-1E-3)
    {
      cout<<tdif << "  "<<externalTriggerTime_<<endl;
      std::cout<<lb4<<endl;
       std::cout<<lb5<<endl;
       std::cout<<lbc<<endl;
       
    }
  std::cout<<lbc <<" "<<tdif<<" # hits "<<rhcol->getNumberOfElements()<<std::endl;
#endif

  if (tdif>tcut) {lastSpill_=lbc;std::cout<<"time since Last Splll "<<tdif<<std::endl; htspill->Fill(tdif);}
  externalTriggerTime_=lbc;
  //  for (unsigned int i=0;i<vTrigger.size();i++) std::cout<<i<<" "<<vTrigger[i]<<std::endl;




  // Drop the first event of the Spill
  if (tdif>tcut && dropFirstSpillEvent_) return false;


  TH1* htdiff= handler_->GetTH1("TimeDif");
  if (htdiff==NULL)
    {
      htdiff =handler_->BookTH1( "TimeDif",2000,0.,200.);
    }
  htdiff->Fill(tdif*1000.);




  return true;
}

void LMNewAnalyzer::processEvent()
{
  
  if (reader_->getEvent()==0) return; // No event exit


  run_=reader_->getEvent()->getRunNumber();
  int hrtype = reader_->getRunHeader()->parameters().getIntVal("HardRoc");



  // Loop on all framesFind the hit collection
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

  // Find the trigger time and drop the first event of the spill
  //  if (!decodeTrigger(rhcol,spillSize_)) return;

  // Drop the first event of the power pulsing train
  //if (isFirstPowerPulsed(7)) return;
  // Event is accepted
  nAnalyzed_++;

  bool inSynch=false;


  //Loop on hits and fill recevents_
  for (int i=0;i<rhcol->getNumberOfElements();i++)
    {
      IMPL::RawCalorimeterHitImpl* hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(i);
      if (hit==0) continue;
      // Decode
      unsigned int difid = hit->getCellID0()&0xFF;
      int asicid = (hit->getCellID0()&0xFF00)>>8;
      int channel= (hit->getCellID0()&0x3F0000)>>16;
      //std::cout<<"ch-"<<channel<<std::endl;
      unsigned int bc = hit->getTimeStamp();

      // Recevents is a map < time stamp , map < difid, RECODIF> >

      // Find or build the event     
      if (recevents_.find(bc) == recevents_.end())
	{
	  std::map<unsigned int,RecoDIF> m;
	  std::pair<unsigned int, std::map<unsigned int,RecoDIF> > pm(bc,m);
	  recevents_.insert(pm);

	}
      
      std::map<unsigned int, std::map<unsigned int,RecoDIF> >::iterator ktc= recevents_.find(bc);
      std::map<unsigned int,RecoDIF>& mrecobc = ktc->second;

      // Find or build the recodif for this time stamp
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
//	std::cout<<"Seuil 0!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<std::endl;
      }

      if (thr[1]|| thr[0]) 
	{
	  
	  int asic=asicid;
	  if (difid>1000) asic=(asic-1)%4+1;
	  //	DCBufferReader::PadConvert(asic,channel,x,y);
	  DCBufferReader::RKPadConvert(asic,channel,x,y,hrtype);
	  if (isMasked(difid,x,y)) continue;
	  // Add the hit to the recodif
	  RecoHitAddress b((unsigned char)x,(unsigned char)y);
	  std::map<unsigned int,RecoDIF>::iterator itc = mrecobc.find(difid); // Find the RecoDIF
	      // std::cout<<"Ädding a hit to "<<std::endl;
	  if (itc!=mrecobc.end())
	    {

	      itc->second.AddHit(b);
	    }
	  
	}
    }
  // Now Fill thehistograms
  fillHistos(clockSynchCut_);

}
void LMNewAnalyzer::fillHistos(unsigned int synchcut)
{
  //  cout<<" new event "<<recevents_.size()<<std::endl;
  int nbinx = 96;
  int nbiny= 32;
  int timeFirstFrame=0,timeLastFrame=0;
  map<unsigned int,unsigned int> noisefreq;
  noisefreq.clear();
  for (map<unsigned int,pair<unsigned int,unsigned int> >::iterator isum=noiseSummaryMap_.begin();isum!=noiseSummaryMap_.end();isum++)
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


  // Hit times 




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





 





      unsigned long long lasthittime=0;

      for (std::map<unsigned int,RecoDIF>::iterator it = recos.begin();it!=recos.end();it++)
	{
	  //  std:: cout<<"\t DIF " <<it->first<<" #clusters "<<it->second.getClusters()->size()<<std::endl;
	  std::stringstream name("");
	  RecoDIF& r= it->second;
	  if (r.getClusters()->size()==0) continue; // drop frame where all clusters get masked
	  if (jt->first<synchcut)
	    {
	      name<<"/DIF"<<r.getDifId()<<"/Synchronised";
	      TH1* htmin= handler_->GetTH1("SynchronizedMinimum");
	      if (htmin==NULL)
		{
		  htmin =handler_->BookTH1( "SynchronizedMinimum",10,0.,10.);
		}
	      htmin->Fill(jt->first*1.);
	      timeFirstFrame=jt->first;
	    }
	  else
	    {
	      name<<"/DIF"<<r.getDifId()<<"/OffTime";
	      map<unsigned int,unsigned int>::iterator ifq=noisefreq.find(r.getDifId());
	      ifq->second=ifq->second+r.getClusters()->size();
	    }
	  timeLastFrame =jt->first;



	  TH1* hthit= handler_->GetTH1(name.str()+"/TimeHit");
	  TH1* htdist= handler_->GetTH1(name.str()+"/TimeDist");
	  if (hthit==NULL)
	    {
	      hthit =handler_->BookTH1( name.str()+"/TimeHit",5000,0.,5000.);
	      htdist =handler_->BookTH1( name.str()+"/TimeDist",5000,0.,500.);

	    }


	  hthit->Fill(((getExternalTriggerTime()-lastSpill_)-timeLastFrame)*400E-6);
	  htdist->Fill((((getExternalTriggerTime()-lastSpill_)-timeLastFrame)-lasthittime)*400E-6);
	  lasthittime = ((getExternalTriggerTime()-lastSpill_)-timeLastFrame);
    
	  	  
	  
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
  if (timeLastFrame>timeFirstFrame)
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
	float fq=ifq->second/2E-7/(timeLastFrame-timeFirstFrame)/48./64.;
	h->Fill(fq);
	map<unsigned int,pair<unsigned int,unsigned int> >::iterator isum=noiseSummaryMap_.find(ifq->first);
	isum->second.first=isum->second.first+ifq->second;
	isum->second.second=isum->second.second+(timeLastFrame-timeFirstFrame);

	//	std::cout<<isum->first<<" "<<isum->second.first<<" "<<isum->second.second<<std::endl;
      }   



    
  if (inSynch) nInSynch_++;
  
 

}



void LMNewAnalyzer::endJob()
{ 


  std::cout<<"Efficiency "<<run_<<" "<<nAnalyzed_<<" "<<nInSynch_<<" "<<nInSynch_*100./nAnalyzed_<<" %"<<std::endl;
  for (map<unsigned int,pair<unsigned int,unsigned int> >::iterator isum=noiseSummaryMap_.begin();isum!=noiseSummaryMap_.end();isum++)
    {
      std::cout<<"NoiseOccupancy  "<<run_<<" "<<isum->first<<" "<<isum->second.first<<" "<<isum->second.second<<std::endl;
    }
}







