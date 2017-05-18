#include "tdcrb.hh"
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <map>
#include <bitset>
#include "RecoHit.hh"
#include "recoPoint.hh"
#include "recoTrack.hh"
#include "rCluster.hh"
#include "TCanvas.h"


using namespace levbdim;
tdcrb::tdcrb(std::string dire) : _directory(dire),_run(0),_started(false),_fdIn(-1),_totalSize(0),_event(0),_geo(NULL),_t0(2E50),_t(0),_tspill(0)
			       ,_readoutTotalTime(0),_numberOfMuon(0),_numberOfShower(0),_runType(0),_dacSet(0)
{_rh=DCHistogramHandler::instance();}

void tdcrb::geometry(std::string name)
{
  _geo=new jsonGeo(name);
}
void tdcrb::open(std::string filename)
{
  if (_geo==NULL)
    {
      std::cout<<"Please speicfy a geometry"<<std::endl;
      exit(0);
    }
  _fdIn= ::open(filename.c_str(), O_RDONLY | O_NONBLOCK,S_IRWXU);
  if (_fdIn<0)
    {
      perror("Ici No way to store to file :");
      //std::cout<<" No way to store to file"<<std::endl;
      return;
    }  
  _event=0;
  _started=true;
}
void tdcrb::close()
{
  _started=false;
  ::sleep(1);
  if (_fdIn>0)
    {
      ::close(_fdIn);
      _fdIn=-1;
    }


}
uint32_t tdcrb::totalSize(){return _totalSize;}
uint32_t tdcrb::eventNumber(){return _event;}
uint32_t tdcrb::runNumber(){return _run;}
void tdcrb::Read()
{
  for (std::vector<std::pair<uint32_t,std::string> >::iterator it=_files.begin();it!=_files.end();it++)
    {
      std::cout<<"NEW File "<<it->first<<" "<<it->second<<std::endl;
      _run=it->first;
      std::stringstream sff;
      sff<<"sudo chmod o+r "<<it->second;
      system(sff.str().c_str());
      this->open(it->second);
      this->read();
      this->close();
    }
}
void tdcrb::read()
{
  
  levbdim::buffer b(0x100000);
  while (_started)
    {
      if (!_started) return;
      uint32_t theNumberOfDIF=0;
      // To be implemented
      if (_fdIn>0)
	{
	  _idx=0;

 
	  int ier=::read(_fdIn,&_event,sizeof(uint32_t));
	  if (ier<=0)
	    {
	      printf("Cannot read anymore %d \n ",ier);return;
	    }
	  //else
	  //	printf("Event read %d \n",_event);
      
	  ier=::read(_fdIn,&theNumberOfDIF,sizeof(uint32_t));
	  if (ier<=0)
	    {
	      printf("Cannot read anymore number of DIF %d \n ",ier);return;
	    }
	  // else
	  //   printf("Number of DIF found %d \n",theNumberOfDIF);

	  for (uint32_t idif=0;idif<theNumberOfDIF;idif++) 
	    {
	      //printf("\t writing %d bytes",idata[SHM_BUFFER_SIZE]);
	      //(*iv)->compress();
	      uint32_t bsize=0;
	      // _totalSize+=bsize;
	      ier=::read(_fdIn,&bsize,sizeof(uint32_t));
	      if (ier<=0)
		{
		  printf("Cannot read anymore  DIF Size %d \n ",ier);return;
		}
	      //else
	      //  printf("\t DIF size %d \n",bsize);
	  
	      ier=::read(_fdIn,b.ptr(),bsize);
	      if (ier<=0)
		{
		  printf("Cannot read anymore Read data %d \n ",ier);return;
		}
	      b.setPayloadSize(bsize-(3*sizeof(uint32_t)+sizeof(uint64_t)));
	      b.uncompress();
	      memcpy(&_buf[_idx], b.payload(),b.payloadSize());
	      //printf("\t \t %d %d %d %x %d %d %d\n",b.detectorId(),b.dataSourceId(),b.eventId(),b.bxId(),b.payloadSize(),bsize,_idx);
	      if (b.detectorId()==255)
		{
		  uint32_t* buf=(uint32_t*) b.payload();
		  printf("NEW RUN %d \n",_event);
		  _run=_event;
		  for (int i=0;i<b.payloadSize()/4;i++)
		    {
		      printf("%d ",buf[i]);
		    }
		  _difId=b.dataSourceId();
		  _runType=buf[0];
		  if (_runType==1)
		    _dacSet=buf[1];
		  if (_runType==2)
		    _vthSet=buf[1];
		  printf("\n Run type %d DAC set %d VTH set %d \n",_runType,_dacSet,_vthSet);

		}
	      if (b.detectorId()==110)
		{
		  uint32_t* ibuf=(uint32_t*) b.payload();
	       
		  // for (int i=0;i<7;i++)
		  //   {
		  //     printf("%d ",ibuf[i]);
		  //  }
		  uint32_t nch=ibuf[6];
		  //printf("\n channels -> %d \n",nch);
		  _channels.clear();	  
		  if (ibuf[6]>0)
		    {

		      uint8_t* cbuf=( uint8_t*)&ibuf[7];
		      for (int i=0;i<nch;i++)
			{
			  /*
			    for (int j=0;j<8;j++)
			    printf("\t %.2x ",cbuf[i*8+j]);
			    printf("\n");
			  */
			  TdcChannel c(&cbuf[8*i]);
			  _channels.push_back(c);
			}
		    }
		  _mezzanine=ibuf[4];
		  _difId=(ibuf[5]>>24)&0xFF;
		  _gtc=ibuf[1];

		  //printf("Type %d Mez %d DIF %d %x  channels %d Event %d \n",_runType,_mezzanine,_difId,ibuf[5],_channels.size(),_gtc);
		  if (_runType==1) this->pedestalAnalysis();
		  if (_runType==2) this->scurveAnalysis();
		  if (_runType==0) this->normalAnalysis();
		
		}

     
	    }
	}

    }
}

void tdcrb::pedestalAnalysis()
{

  //if (_gtc[_mezzanine-1]
  std::cout<<"Mezzanine "<<_mezzanine<<"Event "<<_event<<" GTC"<<_gtc<<" hits"<<_channels.size()<<std::endl;

  // Analyze
  std::stringstream sr;
  sr<<"/run"<<_run<<"/TDC"<<_mezzanine<<"/";

  uint32_t dac =_dacSet;
  for (int ich=0;ich<28;ich++)
    {
 
      std::stringstream src;
      src<<sr.str()<<"dac"<<ich;
      TH1* hdac=_rh->GetTH1(src.str());
      if (hdac==NULL)
	{
	 
	  hdac=_rh->BookTH1(src.str(),64,0.,64.);
	}
      bool found=false;
      double lastf=0;
      for (auto x :_channels)
	{
	  if (x.channel()==ich) {
	    //printf("%d %d %f \n",x.channel(),x.bcid(),x.tdcTime());
	    double dt=x.tdcTime()-lastf;
	    lastf=x.tdcTime();
	    if (dt>25 || dt<0)
	      hdac->Fill(dac*1.);
	  }//break;}
	}
    }

}
void tdcrb::scurveAnalysis()
{

  //if (_gtc[_mezzanine-1]
  std::cout<<"Mezzanine "<<_difId<<"Event "<<_event<<" GTC"<<_gtc<<" hits"<<_channels.size()<<std::endl;

  // Analyze
  std::stringstream sr;
  sr<<"/run"<<_run<<"/TDC"<<_difId<<"/";

  uint32_t vth =_vthSet;
  for (int ich=0;ich<28;ich++)
    {
 
      std::stringstream src;
      src<<sr.str()<<"vth"<<ich;
      TH1* hvth=_rh->GetTH1(src.str());
      if (hvth==NULL)
	{
	 
	  hvth=_rh->BookTH1(src.str(),900,0.,900.);
	}
      bool found=false;
      double lastf=0;
      for (auto x :_channels)
	{
	  if (x.channel()==ich) {
	    //printf("%d %d %f \n",x.channel(),x.bcid(),x.tdcTime());
	    double dt=x.tdcTime()-lastf;
	    lastf=x.tdcTime();
	    //  if (dt>25 || dt<0)
	    hvth->Fill(vth*1.);
	    break;
	  }
	}
    }

}
void tdcrb::normalAnalysis()
{
  this->LmAnalysis();
}

void tdcrb::end()
{

  if (_runType==1)
    {
      for (int mez=1;mez<=2;mez++)
	{
	  std::stringstream sr;
	  
	  sr<<"/run"<<_run<<"/TDC"<<mez<<"/";

	  int ipr=0;
	  for (int ich=0;ich<32;ich++)
	    {

	      if (ich%2==0)
		ipr=ich/2;
	      else
		ipr=31-ich/2;
	      std::stringstream src;
	      src<<sr.str()<<"dac"<<ich;
	      TH1* hdac=_rh->GetTH1(src.str());
	      int ped=31;
	      if (hdac!=NULL)
		{
		  printf("Mezzanine %d Channel %d Mean %f RMS %f \n",mez,ich,hdac->GetMean(),hdac->GetRMS());
		  ped=int(hdac->GetMean());
		  if (hdac->GetRMS()>6)
		    {
		      printf("\t \t ======================>ILL %d \n",ipr);
		      ped-=int(hdac->GetRMS());
		    }
		  // if (mez==1)
		  //   _s1.set6bDac(ipr,ped&0xFF);
		  // else
		  //   _s2.set6bDac(ipr,ped&0xFF);
		}
	      // else
	      // 	if (mez==1)
	      // 	  _s1.set6bDac(ipr,31);
	      // 	else
	      // 	  _s2.set6bDac(ipr,31);

	      if (ped==0)
		{printf("\t \t ======================>DEAD %d \n",ipr);
		  ped=31;
		}
	      printf("\t %d %d \n",ipr,ped);
	    }
	}
      // _s1.toJson();
      // _s1.dumpJson();
      // _s2.toJson();
      // _s2.dumpJson();

    }
  



  std::stringstream sr;
  sr<<"/tmp/tdcb"<<_run<<".root";
  
  _rh->writeHistograms(sr.str());


  
}
void tdcrb::LmAnalysis()
{

  if (_channels.size()>4096) return;
  uint32_t run=_run;
  // Analyze
  uint16_t chtrg;
  if (_mezzanine==1)
    chtrg=0x10;
  else
    chtrg=0x1c;
  chtrg=0x18; //24
  chtrg=16; //16 canaux
  std::stringstream sr;
  sr<<"/run"<<run<<"/TDC"<<_difId<<"/LmAnalysis/";
  TH2* hpos=_rh->GetTH2(sr.str()+"Position");
  TH1* hdt=_rh->GetTH1(sr.str()+"DeltaT");

  TH2* hdtr=_rh->GetTH2(sr.str()+"DeltaTr");
  TH2* hdtra=_rh->GetTH2(sr.str()+"DeltaTrAll");
  TH1* hns=_rh->GetTH1(sr.str()+"NChannel");
  TH1* hnst=_rh->GetTH1(sr.str()+"NChannelTrigger");
  TH1* hfin=_rh->GetTH1(sr.str()+"Fine");
  TH1* heff=_rh->GetTH1(sr.str()+"Efficiency");
  TH1* hstrip=_rh->GetTH1(sr.str()+"Strips");
  TH1* hstript=_rh->GetTH1(sr.str()+"Stript");
  TH1* hnstrip=_rh->GetTH1(sr.str()+"NStrips");
  TH1* hxp=_rh->GetTH1(sr.str()+"XP");
  TH1* hti=_rh->GetTH1(sr.str()+"time");
  TH1* hra=_rh->GetTH1(sr.str()+"rate");
  if (hpos==NULL)
    {
      hpos=_rh->BookTH2(sr.str()+"Position",100,0.,20.,300,-300.,300.);
      hdt=_rh->BookTH1(sr.str()+"DeltaT",500,-10.,10.);

      hdtr=_rh->BookTH2(sr.str()+"DeltaTr",32,0,32.,400,-1000.,-800.);
      hdtra=_rh->BookTH2(sr.str()+"DeltaTrAll",32,0,32.,500,-20.,20.);
      hns=_rh->BookTH1(sr.str()+"NChannel",1024,0.,1024.);
      hnst=_rh->BookTH1(sr.str()+"NChannelTrigger",1024,0.,1024.);
      hfin=_rh->BookTH1(sr.str()+"Fine",257,0.,257.);
      heff=_rh->BookTH1(sr.str()+"Efficiency",32,0.,32.);
      hstrip=_rh->BookTH1(sr.str()+"Strips",32,0.,32.);
      hstript=_rh->BookTH1(sr.str()+"Stript",32,0.,32.);
      hnstrip=_rh->BookTH1(sr.str()+"NStrips",32,0.,32.);
      hxp=_rh->BookTH1(sr.str()+"XP",400,0.,10.);
      hti=_rh->BookTH1(sr.str()+"time",400,0.,0.2);
      hra=_rh->BookTH1(sr.str()+"rate",750,0.,200000.);

    }
  hns->Fill(_channels.size()*1.);

  double shift1[32]={32*0};
		    
  double shift2[32]={32*0};


  _nevt++;
  heff->Fill(0.1);
  uint32_t ndec=0;
  
  float ti=0,tmax=0;
  uint32_t lbcid=0,bcidshift=0,bcidmax=0;
  int32_t tbcid=0;
  for (std::vector<TdcChannel>::iterator it=_channels.begin();it!=_channels.end();it++)
    {
      hstrip->Fill(it->channel()*1.+0.5);
      if (it->bcid()>bcidmax) bcidmax=it->bcid();
      // if (it->bcid()<lbcid) {
      //   printf("lbcid %d bcid %d  Shift %d \n",lbcid,it->bcid(),b
      //   bcidshift+=65535;
      // }
      lbcid=it->bcid();
      float t=((int) it->bcid()*2E-7)+(bcidshift*2.E-7)-ti;
      if (t>tmax) tmax=t;
      if (it->channel()==chtrg) {ndec++; tbcid=it->bcid();}
      //printf("%d %d %x %x %x \n",_gtc,it->channel(),it->coarse(),it->fine(),it->bcid());
    }
  //printf("BCID max %d Bcidshift %d Tmax %f \n",bcidmax,bcidshift,tmax);
  // if (tmax==0 && _channels.size()>0)
  //   {
  //     for (std::vector<TdcChannel>::iterator it=_channels.begin();it!=_channels.end();it++)
  // 	std::cout<<(int) it->channel()<<" "<<it->coarse()*2.5E-9<<" "<<it->bcid()*2E-7<<endl;
  //     //getchar();
  //   }
  
  if (ndec==0) hti->Fill(tmax);
  if (tmax>0 && ndec==0) hra->Fill(_channels.size()/tmax);
  // Accept events with only one trigger
  if (ndec!=1) return;
  // Find the trigger


  std::map<uint32_t,std::vector<TdcChannel> > _trmap;
  _trmap.clear();

  for (std::vector<TdcChannel>::iterator it=_channels.begin();it!=_channels.end();it++)
    {
      it->setUsed(false);
      if (it->channel()!=chtrg) hdtra->Fill(it->channel(),(it->bcid()-tbcid)*1.);
    }
  
  for (std::vector<TdcChannel>::iterator it=_channels.begin();it!=_channels.end();it++)
    {
      if (it->channel()==chtrg)
	{
	  it->setUsed(true);
	  std::vector<TdcChannel> vc;
	  vc.push_back(*it);

	
	  for (auto x:_channels)
	    {
	      if (x.used()) continue;
	      if (x.channel() == chtrg) continue;
	      if (x.bcid()>(it->bcid()-3) || x.bcid()<(it->bcid()-6)) continue;
	      //if ((x.bcid()-it->bcid())!=-4) continue;
	      vc.push_back(x);
	      x.setUsed(true);
	    }
	  std::pair<uint32_t,std::vector<TdcChannel> > p(it->bcid(),vc);
	  _trmap.insert(p);
	}

    }//break;}


  // Now loop on all channel of the event
  for (std::vector<TdcChannel>::iterator it=_channels.begin();it!=_channels.end();it++)
    {
      hstrip->Fill(it->channel()*1.);
    }

  if (_trmap.size()>0) printf("TDC %d  GTC %d   Number %d \n",_difId,_gtc,_trmap.size());
  bool found=false;
  bool bside=false;
 
  for (auto t:_trmap)
    {
      //printf("Trigger %d \n",t.first);
      double trigtime=0,trigbcid=0;
      bool chused[32]={32*0};
      bool sused[32]={32*0};
      for (int i=0;i<32;i++) {chused[i]=false;sused[i]=false;}
      std::sort(t.second.begin(),t.second.end());
      for (auto x:t.second)
	if (x.channel()==chtrg) {chused[chtrg]=1;trigtime=x.tdcTime();trigbcid=x.bcid()*200;}
      hnst->Fill(t.second.size()*1.);
      if (t.second.size()>1)  heff->Fill(4.1);
      printf(" Effective TDC %d  GTC %d   Number %d \n",_difId,_gtc,t.second.size());
      if (t.second.size()>2000) continue; // Use trigger with less than  20 strip
      for (auto x:t.second)
	{
	  printf("Chan %d bcid %d Time %f %f \n",x.channel(),x.bcid(),x.tdcTime(),trigtime);
	  if (x.channel()==chtrg) continue;

	  if (x.tdcTime()-trigtime>-861) continue;
	  if (x.tdcTime()-trigtime<-900) continue;
	  found=true;

	  
	  if (chused[x.channel()]) continue;
	  if (x.channel()%2!=0) continue;
	 
	  hdtr->Fill(x.channel()+1.,x.tdcTime()-trigtime);
	  //hdtr->Fill(x.channel(),x.bcid()*200-trigbcid);

	  int str=x.channel()/2;
	  if (sused[str]) continue;
	  for (auto y:t.second)
	    {
	      if (chused[y.channel()]) continue;
	      if (y.channel()!=(x.channel()+1)) continue;
	      double t0=x.tdcTime();
	      double t1=y.tdcTime();
	      ///if (it->coarse()!=jt->coarse()-1) continue;
	      //if (abs(t1-t0)>100) continue;
	      chused[x.channel()]=true;
	      chused[y.channel()]=true;
	      sused[str]=true;
	      double tsh=0;
	      hdt->Fill((t0-t1)-tsh);
	      std::stringstream s;
	      s<<"hdts"<<str;
	      TH1* hdts=_rh->GetTH1(sr.str()+s.str());
	      if (hdts==NULL)
		{
		  hdts=_rh->BookTH1(sr.str()+s.str(),500,-20.,20.);
		}
	      std::stringstream sx;
	      sx<<"hdtr"<<(int) x.channel();
	      TH1* hdtrx=_rh->GetTH1(sr.str()+sx.str());
	      if (hdtrx==NULL)
		{
		  hdtrx=_rh->BookTH1(sr.str()+sx.str(),400,-900.,-860.);
		}
	      hdts->Fill(t0-t1-tsh);
	      hdtrx->Fill(t0-trigtime);
	      std::stringstream sry;
	      sry<<"hdtr"<<(int) y.channel();
	      TH1* hdtry=_rh->GetTH1(sr.str()+sry.str());
	      if (hdtry==NULL)
		{
		  hdtry=_rh->BookTH1(sr.str()+sry.str(),400,-900.,-860.);
		}
	      hdtry->Fill(t1-trigtime);
	      double x=str*0.4+0.2;
	      double yp=((t0-t1)-tsh)*100./7.;
	      //std::cout<<x<<" "<<y<<std::endl;
	      bside=true;
	      hpos->Fill(x,yp);
	      break;
	    }     
	}
    }
  //if (_difId==15 ) getchar();

  _ntrigger++;
  heff->Fill(1.1);
  if (!found) return;
  _nfound++;
  // update efficency
  heff->Fill(2.1);
  if (bside) {_nbside++;heff->Fill(3.1);}
  printf("%d-%d %d  #evt %d #trig %d #found %d  #time %d \n",_run,_event,_gtc,_nevt,_ntrigger,_nfound,_nbside); 
}


#ifdef TESTMAINEXAMPLE
int main()
{
  levbdim::tdcrb bs("/tmp");
  bs.geometry("/home/acqilc/SDHCAL/SDHCAL_EventReader/pluggins/m3_avril2015.json");
  //bs.open("/data/NAS/June2016/SMM_160616_163121_732786.dat");
  // bs.open("/data/NAS/June2016/SMM_160616_110612_732783.dat");
  //bs.open("/data/NAS/June2016/SMM_170616_052256_732795.dat");
  //bs.open("/data/NAS/June2016/SMM_170616_092331_732799.dat");
  // bs.open("/data/NAS/June2016/SMM_160616_110612_732783.dat");
  /*
    /data/NAS/Oct2016/SMM_071016_123856_733633.dat
    /data/NAS/Oct2016/SMM_071016_124907_733636.dat
    /data/NAS/Oct2016/SMM_071016_125306_733637.dat
    /data/NAS/Oct2016/SMM_071016_153619_733641.dat
    /data/NAS/Oct2016/SMM_071016_154539_733642.dat
    /data/NAS/Oct2016/SMM_071016_155358_733643.dat
    /data/NAS/Oct2016/SMM_071016_155937_733644.dat
    /data/NAS/Oct2016/SMM_071016_165755_733645.dat
    /data/NAS/Oct2016/SMM_071016_170520_733646.dat
    /data/NAS/Oct2016/SMM_071016_173728_733647.dat
    /data/NAS/Oct2016/SMM_071016_193047_733650.dat
    /data/NAS/Oct2016/SMM_071016_205435_733653.dat
    /data/NAS/Oct2016/SMM_071016_210657_733654.dat
    /data/NAS/Oct2016/SMM_071016_232430_733655.dat
    /data/NAS/Oct2016/SMM_071016_233612_733655.dat
    /data/NAS/Oct2016/SMM_081016_012606_733656.dat
    /data/NAS/Oct2016/SMM_081016_015222_733656.dat
    /data/NAS/Oct2016/SMM_081016_033908_733658.dat
    /data/NAS/Oct2016/SMM_081016_035422_733659.dat
    /data/NAS/Oct2016/SMM_081016_035811_733660.dat
    /data/NAS/Oct2016/SMM_081016_054542_733660.dat
    /data/NAS/Oct2016/SMM_081016_082718_733665.dat
    /data/NAS/Oct2016/SMM_081016_092637_733665.dat
    /data/NAS/Oct2016/SMM_081016_110948_733665.dat
    /data/NAS/Oct2016/SMM_081016_160612_733675.dat
    /data/NAS/Oct2016/SMM_081016_171614_733678.dat
    /data/NAS/Oct2016/SMM_081016_173543_733679.dat
    /data/NAS/Oct2016/SMM_091016_010348_733680.dat
    /data/NAS/Oct2016/SMM_091016_041603_733680.dat
    /data/NAS/Oct2016/SMM_091016_065059_733680.dat
    /data/NAS/Oct2016/SMM_091016_072800_733683.dat
    /data/NAS/Oct2016/SMM_091016_104731_733686.dat
    /data/NAS/Oct2016/SMM_091016_122843_733688.dat
    /data/NAS/Oct2016/SMM_091016_124430_733688.dat
    /data/NAS/Oct2016/SMM_091016_140032_733689.dat
    /data/NAS/Oct2016/SMM_091016_154423_733689.dat
    /data/NAS/Oct2016/SMM_091016_163928_733692.dat
    /data/NAS/Oct2016/SMM_091016_164335_733693.dat
    /data/NAS/Oct2016/SMM_091016_182544_733693.dat
    /data/NAS/Oct2016/SMM_091016_184359_733696.dat
    /data/NAS/Oct2016/SMM_091016_202828_733696.dat
    /data/NAS/Oct2016/SMM_091016_211028_733698.dat
    /data/NAS/Oct2016/SMM_091016_223807_733698.dat
    /data/NAS/Oct2016/SMM_091016_232004_733699.dat
    /data/NAS/Oct2016/SMM_101016_001144_733700.dat
    /data/NAS/Oct2016/SMM_101016_001759_733701.dat
    /data/NAS/Oct2016/SMM_101016_004241_733702.dat
    /data/NAS/Oct2016/SMM_101016_012948_733705.dat
    /data/NAS/Oct2016/SMM_101016_013700_733707.dat
    /data/NAS/Oct2016/SMM_101016_014138_733708.dat
    /data/NAS/Oct2016/SMM_101016_015433_733710.dat
    /data/NAS/Oct2016/SMM_101016_020437_733711.dat
    /data/NAS/Oct2016/SMM_101016_033112_733711.dat
    /data/NAS/Oct2016/SMM_101016_045829_733711.dat
    /data/NAS/Oct2016/SMM_101016_054606_733718.dat
    /data/NAS/Oct2016/SMM_101016_063034_733718.dat
    /data/NAS/Oct2016/SMM_101016_075846_733718.dat
    /data/NAS/Oct2016/SMM_101016_092544_733719.dat
    /data/NAS/Oct2016/SMM_101016_103528_733720.dat
    /data/NAS/Oct2016/SMM_101016_111905_733722.dat
    /data/NAS/Oct2016/SMM_101016_113444_733723.dat
    /data/NAS/Oct2016/SMM_101016_120745_733724.dat
    /data/NAS/Oct2016/SMM_101016_132143_733724.dat
    /data/NAS/Oct2016/SMM_101016_144929_733725.dat
    /data/NAS/Oct2016/SMM_101016_151024_733728.dat
    /data/NAS/Oct2016/SMM_101016_175002_733738.dat
    /data/NAS/Oct2016/SMM_101016_175744_733740.dat
    /data/NAS/Oct2016/SMM_101016_181441_733741.dat
    /data/NAS/Oct2016/SMM_101016_181943_733742.dat
    /data/NAS/Oct2016/SMM_101016_224057_733743.dat
    /data/NAS/Oct2016/SMM_111016_004226_733743.dat
    /data/NAS/Oct2016/SMM_111016_005214_733748.dat
    /data/NAS/Oct2016/SMM_111016_022420_733750.dat
    /data/NAS/Oct2016/SMM_111016_024904_733750.dat
    /data/NAS/Oct2016/SMM_111016_044205_733750.dat
    /data/NAS/Oct2016/SMM_111016_063534_733750.dat
    /data/NAS/Oct2016/SMM_111016_072721_733754.dat
    /data/NAS/Oct2016/SMM_111016_084258_733754.dat
    /data/NAS/Oct2016/SMM_111016_105343_733756.dat
    /data/NAS/Oct2016/SMM_111016_113540_733756.dat
    /data/NAS/Oct2016/SMM_111016_135741_733757.dat
    /data/NAS/Oct2016/SMM_111016_140953_733758.dat
    /data/NAS/Oct2016/SMM_111016_151355_733759.dat
    acqilc@lyosdhcal9:~$ 

  */
  bs.open("/data/srv02/RAID6/Oct2016/SMM_101016_224057_733743.dat");
  bs.read();
}
#endif
