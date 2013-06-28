#include "LMBasicAnalyzer.h"
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

LMBasicAnalyzer::LMBasicAnalyzer(DHCalEventReader* r,DCHistogramHandler* h) 
{
  reader_=r;
  handler_ =h;
}

void LMBasicAnalyzer::initHistograms()
{
  std::cout<<"LMBasicAnalyzer:: creating histograns"<<std::endl;
  std::string timeName[2]={"Synchronised","OffTime"};
  std::stringstream name("");
  std::stringstream difname("");
  std::stringstream timename("");
  std::stringstream levelname("");
  std::stringstream basename("");
  LCCollection* rhcol=0;
  try {
    rhcol = reader_->getEvent()->getCollection("DHCALRawHits");
  }
  catch (...)
    {
      std::string s="No Hits ";s+=__PRETTY_FUNCTION__;
      throw s;
    }
  if (rhcol <= 0) return;

  std::cout<<"LMBasicAnalyzer:; Number of Raw Hits "<<rhcol->getNumberOfElements()<<std::endl;

  for (int i=0;i<rhcol->getNumberOfElements();i++)
    {
      IMPL::RawCalorimeterHitImpl* hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(i);
      if (hit==0) continue;
      int difid = hit->getCellID0()&0xFF;
      difname.str("");
      difname<<"/DIF"<<difid;
      name.str("");
      name <<difname.str()<<"/EventTime";
      if (handler_->GetTH1(name.str())!=NULL) continue;
      handler_->BookTH1(name.str(),500,-0.5,499.5);

      name.str("");
      name <<difname.str()<<"/FrameTime";
      handler_->BookTH1(name.str(),500,-0.5,50000.5);

      for (int it=0;it<2;it++)
	{
	  timename.str("");
	  timename<<"/"<<timeName[it];

	  name.str("");
	  name <<difname.str()<<timename.str()<<"/NumberOfFrame_vs_Asic";

	  handler_->BookTH2(name.str(),48,0.1,48.1,128,0.,128.);

	  name.str("");
	  name <<difname.str()<<timename.str()<<"/NumberOfAsic";
	  handler_->BookTH1(name.str(),48,0.,48.);

	  int nbinx = 96;
	  int nbiny= 32;
	  if (difid>1000) nbinx=8;

	  for (int il=0;il<2;il++)
	    {
	      levelname.str("");
	      levelname<<"/Level"<<il;


	      basename.str("");
	      basename<<difname.str()<<timename.str()<<levelname.str();
	      // Last Hit Map
	      name.str("");
	      name<<basename.str()<<"/Current_Hit_Map";
	      handler_->BookTH2(name.str(),nbinx,0.1,nbinx+0.1,nbiny,0.1,nbiny+0.1);
	      // Hit Map
	      name.str("");
	      name<<basename.str()<<"/Integrated_Hit_Map";
	      handler_->BookTH2(name.str(),nbinx,0.1,nbinx+0.1,nbiny,0.1,nbiny+0.1);

	      name.str("");
	      name <<basename.str()<<"/NumberOfHit_vs_Asic";
	      handler_->BookProfile(name.str(),48,0.1,48.1,0.,1000.);
	      name.str("");
	      name <<basename.str()<<"/NumberOfHit_per_Frame";
	      handler_->BookTH1(name.str(),64,0.1,64.1);

	    }
	}
    }

}


void LMBasicAnalyzer::processEvent()
{
  
  if (reader_->getEvent()==0) return;
  try {
    if (reader_->getEvent()->getEventNumber()==1)
      {
	this->initHistograms();
	nAnalyzed_=0; nInSynch_=0;}
    
  } catch (std::string s)
    {
      std::cout<<" error "<<s<<std::endl;
    }
  int hrtype = reader_->getRunHeader()->parameters().getIntVal("HardRoc");
  nAnalyzed_++;
 // LMCounter ctr;
  // LMHCAL thehcal;
//   std::vector<std::string> curMaps;
//   handler_->ListHisto("Current_Hit_Map",2,curMaps);
  
//    if (curMaps.size() == 0) this->initHistograms();
//    for (unsigned int i=0;i<curMaps.size();i++) handler_->GetTH2(curMaps[i])->Reset();
  
  // if (1>0) return;  

  // Loop on all frames
  std::stringstream name("");int x,y;
  std::stringstream countername("");
//    getchar();
//   std::cout<<fCurEvent->GetFrames()->size()<<"frames found "<<std::endl;

  LCCollection* rhcol=0;
  try 
    {
      rhcol = reader_->getEvent()->getCollection("DHCALRawHits");
    }
  catch (...)
    {
      std::cout<<" No collection try to build it" <<std::endl;
      reader_->buildEvent();
      rhcol = reader_->getEvent()->getCollection("DHCALRawHits");
    }
  // LCTOOLS::printParameters(rhcol->getParameters());
//   getchar();
  if (rhcol == 0) return;


//  getchar();
//  std::cout<<rhcol->getNumberOfElements() <<" frames found "<<std::endl;
  //    getchar();
  bool inSynch=false;
  for (int i=0;i<rhcol->getNumberOfElements();i++)
    {
      IMPL::RawCalorimeterHitImpl* hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(i);
      if (hit==0) continue;
      int difid = hit->getCellID0()&0xFF;
      int asicid = (hit->getCellID0()&0xFF00)>>8;
      int channel= (hit->getCellID0()&0x3F0000)>>16;
      int bc = hit->getTimeStamp();
      //      std::cout<<bc<<std::endl;
      bool thr[2];
      //      printf("%x \n",hit->getCellID0());
      int ithr= hit->getAmplitude();
      //     std::cout<<difid<<":"<<asicid<<":"<<channel<<":"<<bc<<":"<<ithr<<std::endl;
      thr[0] = ithr &1;
      thr[1] = ithr &2;
      name.str("");
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
      name<<"/DIF"<<difid;

      if (handler_->GetTH1(name.str()+"/FrameTime")==NULL)
 	{
	  // 	  handler_->BookTH1(name.str()+"/FrameTime");
	  handler_->BookTH1(name.str()+"/FrameTime",500,-0.5,499.5);

 	}
      handler_->GetTH1(name.str()+"/FrameTime")->Fill(bc);
      

//       // DEBUG(8001)
// 	std::string s = name.str()+"/NumberOfFrame_vs_Time";
//       if (ctr.getCounters(s) == NULL)
// 	{
// 	  //  DEBUG(8002)
// 	  ctr.addCounter(s,15000);
// 	}
//       // DEBUG(8003)
//       if (bc<500)
// 	ctr.increment(s,TMath::Nint(bc);

      // DEBUG(8004)
      //if (f->isSynchronised()) {std::cout<<f->getTimeToTrigger()<<std::endl;getchar();}

      if (bc<7)
	{
	  name<<"/Synchronised";
	  inSynch=true;
	}
      else
	name<<"/OffTime";

      //DEBUG(8005)
//       std::string s1 = name.str()+"/NumberOfFrame_vs_Asic";
//       if (ctr.getCounters(s1) == NULL)
// 	ctr.addCounter(s1,48);

//       //DEBUG(8006)
//       std::string s2 = name.str()+"/NumberOfAsic";
//       if (ctr.getCounters(s2) == NULL)
// 	ctr.addCounter(s2);


      //DEBUG(8007)

//       ctr.increment(s1,asicid-1);
//       // DEBUG(8008)
//       ctr.increment(s2);
      int nbinx = 96;
      int nbiny= 32;
      if (difid>1000) nbinx=8;

      TH2* hl0int = handler_->GetTH2(name.str()+"/Level0/Integrated_Hit_Map");
      if (hl0int==NULL)
	{
	  hl0int =handler_->BookTH2( name.str()+"/Level0/Integrated_Hit_Map",nbinx,0.1,nbinx+0.1,nbiny,0.1,nbiny+0.1);
	}

      TH2* hl1int = handler_->GetTH2(name.str()+"/Level1/Integrated_Hit_Map");
      if (hl1int==NULL)
	{
	  hl1int =handler_->BookTH2( name.str()+"/Level1/Integrated_Hit_Map",nbinx,0.1,nbinx+0.1,nbiny,0.1,nbiny+0.1);
	 }

      TH2* hl0cur= handler_->GetTH2(name.str()+"/Level0/Current_Hit_Map");
      if (hl0cur==NULL)
	{
	  hl0cur = handler_->BookTH2(name.str()+"/Level0/Current_Hit_Map",nbinx,0.1,nbinx+0.1,nbiny,0.1,nbiny+0.1);
	}

      TH2* hl1cur= handler_->GetTH2(name.str()+"/Level1/Current_Hit_Map");
      if (hl1cur==NULL)
	{
	  hl1cur = handler_->BookTH2(name.str()+"/Level1/Current_Hit_Map",nbinx,0.1,nbinx+0.1,nbiny,0.1,nbiny+0.1);
	}

      //DEBUG(8009)
//       if (handler_->GetTH1(name.str()+"/Level0/NumberOfHit_vs_Asic")==NULL)
// 	{
// 	  std::string s= name.str()+"/Level0/NumberOfHit_vs_Asic";
// 	  TProfile* h =  new TProfile(s.c_str(),s.c_str(),48,0.1,48.1,0.,1000.);
// 	  h->SetDirectory(0);
// 	  pair<std::string,TH1*> pr(s,h);
// 	  mapH1.insert( pr);
// 	}
//       handler_->GetTH1(name.str()+"/Level0/NumberOfHit_vs_Asic")->Fill(f->getAsicId()*1.,f->getNumberOfHit(0));

//       if (handler_->GetTH1(name.str()+"/Level1/NumberOfHit_vs_Asic")==NULL)
// 	{
// 	  std::string s= name.str()+"/Level1/NumberOfHit_vs_Asic";
// 	  TProfile* h =  new TProfile(s.c_str(),s.c_str(),48,0.1,48.1,0.,1000.);
// 	  h->SetDirectory(0);
// 	  pair<std::string,TH1*> pr(s,h);
// 	  mapH1.insert( pr);
// 	}
//       handler_->GetTH1(name.str()+"/Level1/NumberOfHit_vs_Asic")->Fill(f->getAsicId()*1.,f->getNumberOfHit(1));

      //      std::cout<<f->getNumberOfHit(0)<<" et "<<f->getNumberOfHit(1)<<" on" <<f->getAsicId()<< std::hex<<hl0int<<" "<<hl0cur<<" "<<hl1int<<" "<<hl1cur<<std::dec<<std::endl;
      //  if (f->isSynchronised()) std::cout <<f->getDifId()<<":"<<f->getAsicId()<<":"<<f->getNumberOfHit(0)<<":"<<f->getNumberOfHit(1)<<name.str()<<std::endl;

     
      if (thr[0]) {
	
	int asic=asicid;
	if (difid>1000) asic=(asic-1)%4+1;
	DCBufferReader::RKPadConvert(asic,channel,x,y,hrtype);

	if (hl0int) hl0int->Fill(x*1.,y*1.);
	if (hl0cur) hl0cur->Fill(x*1.,y*1.);
      }
      if (thr[1]) {
	
	int asic=asicid;
	if (difid>1000) asic=(asic-1)%4+1;
	//	DCBufferReader::PadConvert(asic,channel,x,y);
	DCBufferReader::RKPadConvert(asic,channel,x,y,hrtype);
	//	printf("%d %d %d %d %d \n",asic,channel,x,y,hrtype);
	//	getchar();
	if (hl1int) hl1int->Fill(x*1.,y*1.);
	if (hl1cur) hl1cur->Fill(x*1.,y*1.);
      }
    }
//   //DAEBUG(8012)
//   std::vector<std::string> vname= ctr.getNames();
//   for (unsigned int j=0;j<vname.size();j++)
//     {
//       //      std::cout<<vname[j]<<std::endl;
//       std::vector<unsigned int>*  v=ctr.getCounters(vname[j]);
//       if (handler_->GetTH1(vname[j]) !=0)
// 	{
// 	  if (v->size() == 1) handler_->GetTH1(vname[j])->Fill((*v)[0]*1.);
// 	  continue;
// 	}
//       if (handler_->GetTH2(vname[j]) !=0)
// 	{
// 	  for (unsigned int k=0;k<v->size();k++)
// 	    handler_->GetTH2(vname[j])->Fill(k+0.5,(*v)[k]*1.);
// 	}

//     }
//   //DEBUG(8013)
//   std::vector<unsigned int>*  v=ctr.getCounters("/DIF9/NumberOfFrame_vs_Time");
//   //std::cout<<v<<std::endl;
//   //  getchar();
//   if (v!=0)
//     for (unsigned int k=0;k<v->size();k++)
//       if((*v)[k]!=0) {if (handler_->GetTH1("/DIF9/EventTime")) handler_->GetTH1("/DIF9/EventTime")->Fill(k*1.);//handler_->GetTH1("/DIF9/FrameTime")->Fill(k*1.,(*v)[k]);
//       }
//   //DEBUG(8014)
//   v=ctr.getCounters("/DIF10/NumberOfFrame_vs_Time");
//   if (v!=0)
//    for (unsigned int k=0;k<v->size();k++)
//      if((*v)[k]!=0) {if (handler_->GetTH1("/DIF10/EventTime")) handler_->GetTH1("/DIF10/EventTime")->Fill(k*1.);
//        //handler_->GetTH1("/DIF10/FrameTime")->Fill(k*1.,(*v)[k]);
//      }
//   //DEBUG(8015)
//   v=ctr.getCounters("/DIF6/NumberOfFrame_vs_Time");
//   if (v!=0)
//    for (unsigned int k=0;k<v->size();k++)
//      if((*v)[k]!=0) { if (handler_->GetTH1("/DIF6/EventTime")) handler_->GetTH1("/DIF6/EventTime")->Fill(k*1.);
//        //handler_->GetTH1("/DIF6/FrameTime")->Fill(k*1.,(*v)[k]);
//      }

//   //  std::cout<<"fini"<<std::endl;

//   ////DEBUG(15 c est fini )
  //DEBUG(8016)

  //getchar();
#ifdef DUMP
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
  getchar();
#endif

  if (inSynch) nInSynch_++;

  std::cout<<nAnalyzed_<<" "<<nInSynch_<<" "<<nInSynch_*100./nAnalyzed_<<" %"<<std::endl;

}
