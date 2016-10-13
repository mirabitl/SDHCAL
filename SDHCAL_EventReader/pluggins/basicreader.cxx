#include "basicreader.hh"
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
using namespace levbdim;
basicreader::basicreader(std::string dire) : _directory(dire),_run(0),_started(false),_fdIn(-1),_totalSize(0),_event(0),_geo(NULL),_t0(2E50),_t(0),_tspill(0)
,_readoutTotalTime(0),_numberOfMuon(0),_numberOfShower(0)
{_rh=DCHistogramHandler::instance();}

void basicreader::geometry(std::string name)
{
  _geo=new jsonGeo(name);
}
void basicreader::open(std::string filename)
{
  if (_geo==NULL)
    {
      std::cout<<"Please speicfy a geometry"<<std::endl;
      exit(0);
    }
  _run=0; 
  _fdIn= ::open(filename.c_str(), O_RDWR | O_NONBLOCK,S_IRWXU);
  if (_fdIn<0)
    {
      perror("No way to store to file :");
      //std::cout<<" No way to store to file"<<std::endl;
      return;
    }  
  _event=0;
  _started=true;
}
void basicreader::close()
{
  _started=false;
  ::sleep(1);
  if (_fdIn>0)
    {
      ::close(_fdIn);
      _fdIn=-1;
    }


}
uint32_t basicreader::totalSize(){return _totalSize;}
uint32_t basicreader::eventNumber(){return _event;}
uint32_t basicreader::runNumber(){return _run;}
void basicreader::read()
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
      // Clear previous event
      for (uint32_t i=0;i<theDIFPtrList_.size();i++) 
	{   
	  //theDIFPtrList_[i]->dumpDIFInfo();
	  delete theDIFPtrList_[i];
	}
      theDIFPtrList_.clear();
 
      int ier=::read(_fdIn,&_event,sizeof(uint32_t));
      if (ier<=0)
	{
	  printf("Cannot read anymore %d \n ",ier);return;
	}
      else
	printf("Event read %d \n",_event);
      
      ier=::read(_fdIn,&theNumberOfDIF,sizeof(uint32_t));
      if (ier<=0)
	{
	  printf("Cannot read anymore number of DIF %d \n ",ier);return;
	}
      else
	printf("Number of DIF found %d \n",theNumberOfDIF);

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
	  if (b.detectorId()!=100) continue;
	  if (idif==0)
	    {
	      
	      if (_t0>1E50)
		_t0=b.bxId()*2E-7;
	      double ct=b.bxId()*2E-7-_t0;
	      if ((ct-_t)>5.)
		{
		  _tspill=ct;
		  std::cout<<" New Spill====>"<<_tspill<<std::endl;
		}
	      _t=ct;
	    }
	  DIFPtr* d= new DIFPtr(&_buf[_idx],b.payloadSize());
	  _idx+=b.payloadSize();
	  uint8_t* buf=(uint8_t*) b.payload();
	  theDIFPtrList_.push_back(d);
#ifdef BIFDUMP
	  if (b.dataSourceId()==3)
	    {
	  for (int i=0;i<b.payloadSize();i++)
	    {
	      printf("%.2x ",buf[i]);
	   }
	  printf("\n %d \n",b.payloadSize());

	    }
#endif	  
	}
  // Minimal histos
  
  TH1* hacqtim= _rh->GetTH1("/BR/AcquistionTime");	
  if (hacqtim==NULL)
  {
    hacqtim=_rh->BookTH1("/BR/AcquistionTime",1000.,0.,5.);
  }
  // Filling frame and selecting events
  _tframe.clear();
  _tcount.clear();
  std::map<uint32_t,std::vector<std::pair<DIFPtr*,uint32_t> > >::iterator ifm=_tframe.end();
  std::map<uint32_t,std::bitset<256> >::iterator im=_tcount.end();
  for (std::vector<DIFPtr*>::iterator it = theDIFPtrList_.begin();it!=theDIFPtrList_.end();it++) // Loop on DIF
  {
    DIFPtr* d = (*it);
    //uint32_t chid= getChamber(d->getID());
    // LMTest      uint32_t bc = rint(f->getBunchCrossingTime()/DCBufferReader::getDAQ_BC_Period());
    uint32_t chid = _geo->difGeo(d->getID())["chamber"].asUInt();
    uint32_t window=_geo->cuts()["timeWindow"].asUInt();
    // Loop on Frames
    for (uint32_t ifra=0;ifra<d->getNumberOfFrames();ifra++)
    {
      uint32_t bc=d->getFrameTimeToTrigger(ifra);
      bool found=false;
      for (int dt=-1*window;dt<=window;dt++)
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
	  std::bitset<256> v(0);
	  std::pair<uint32_t,std::bitset<256> > p(bc,v);
	  _tcount.insert(p);
	  std::vector<std::pair<DIFPtr*,uint32_t> > vf;
	  vf.push_back(std::pair<DIFPtr*,uint32_t>(d,ifra));
	  std::pair<uint32_t,std::vector<std::pair<DIFPtr*,uint32_t> > > pf(bc,vf);
	  _tframe.insert(pf);
       
      }
    }
    
  std::vector<uint32_t> seeds;seeds.clear();
 uint32_t npmin=_geo->cuts()["minPlans"].asUInt();
 uint32_t itmin=0xFFFFFFFF;
 uint32_t itmax=0;
 
    for (std::map<uint32_t,std::bitset<256> >::iterator im=_tcount.begin();im!=_tcount.end();im++)
    {
      //
      if (im->first<itmin) itmin=im->first;
      if (im->first>itmax) itmax=im->first;
      if (im->second.count()>=npmin)
	{
	this->processEvent(im->first);
	seeds.push_back(im->first);
	}
    }
    
    _readoutTime=(itmax-itmin)*2E-7;
    hacqtim->Fill(_readoutTime);
    if (_readoutTime>0 && _readoutTime<5.)
      _readoutTotalTime+=_readoutTime;
    std::cout<<"seeds : "<<seeds.size()<<" readout time :"<<_readoutTime<<" absolute "<<_t<<" in spill "<<_t-_tspill<<" Sh (n:rate) "<<_numberOfShower<<":"<<_numberOfShower/_readoutTotalTime<<" Mu(n:rate) "<<_numberOfMuon<<":"<<_numberOfMuon/_readoutTotalTime<<" total time "<<_readoutTotalTime<<std::endl;
  if (_event%100==0 &&_event>0)
    _rh->writeSQL();
     
    }
    }

}
void basicreader::processEvent(uint32_t iseed)
{
  double bestX1=0.049985, bestY1 =0.049985,bestZ1 =0.049985,bestX2 =2.63409e-05, bestY2=3.00341e-05,bestZ2=7.91877e-05,bestX3=-1.75883e-08, bestY3=-9.99385e-09,bestZ3 =-9.99385e-09;
  
  TH1* hnhit= _rh->GetTH1("/BR/nhit");
  TH1* hhitparasic= _rh->GetTH1("/BR/hitparasic");
  TH1* hnhitm= _rh->GetTH1("/BR/nhitmuon");
  TH1* hnhits= _rh->GetTH1("/BR/nhitshower");
  TH2* hhitvsratio=_rh->GetTH2("/BR/hitvsratio");
  TH1* hen= _rh->GetTH1("/BR/energy");
  TProfile* hr0 = (TProfile*) _rh->GetTH1("/BR/r0");
  TProfile* hr1 = (TProfile*) _rh->GetTH1("/BR/r1");
  TProfile* hr2 = (TProfile*) _rh->GetTH1("/BR/r2");
	  // if (hhtx==NULL)
	  //   {
	  //     hhtx = _rh->BookProfile("HoughTransformX",100,0.,100,-50.,50.);
  if (hnhit==NULL)
  {
    hnhit=_rh->BookTH1("/BR/nhit",1000.,0.,2000.);
    hnhitm=_rh->BookTH1("/BR/nhitmuon",1000.,0.,2000.);
    hnhits=_rh->BookTH1("/BR/nhitshower",1000.,0.,2000.);
    hhitparasic=_rh->BookTH1("/BR/hitparasic",512.,0.,64.);
    hhitvsratio=_rh->BookTH2("/BR/hitvsratio",1000,0.,2000,512,0.,20.);
    hen=_rh->BookTH1("/BR/energy",500.,0.,150.);
    hr0 = _rh->BookProfile("/BR/r0",100,0.,5.,0.,5000.);
    hr1 = _rh->BookProfile("/BR/r1",100,0.,5.,0.,5000.);
    hr2 = _rh->BookProfile("/BR/r2",100,0.,5.,0.,5000.);

  }
  std::map<uint32_t,std::vector<std::pair<DIFPtr*,uint32_t> > >::iterator ifm=_tframe.find(iseed);
  if (ifm==_tframe.end())
    return;
  std::map<uint32_t,std::bitset<256> >::iterator im=_tcount.find(iseed);
  if (im==_tcount.end())
      return;
  uint32_t nshower=0,nmuon=0;	
  uint32_t nhit=0,nh0=0,nh1=0,nh2=0;
  for (std::vector<std::pair<DIFPtr*,uint32_t> >::iterator itf=ifm->second.begin();itf!=ifm->second.end();itf++)
  {
    DIFPtr* d=itf->first;
    uint32_t ifra=itf->second;
    //std::cout<<hex<<(uint64_t) d<<" "<<dec<<ifra<<std::endl;
  
    for (uint32_t j=0;j<64;j++)
    {
      if (!(d->getFrameLevel(ifra,j,0) || d->getFrameLevel(ifra,j,1))) continue;
      nhit++;
      if (d->getFrameLevel(ifra,j,0)) nh1++;
      if (d->getFrameLevel(ifra,j,1)) nh0++;
      if (d->getFrameLevel(ifra,j,0) && d->getFrameLevel(ifra,j,1)) nh2++;
    }
    //std::cout<<ifra<<" "<<nhit<<std::endl;
  }
 
  hnhit->Fill(nhit*1.);
  double hitsparasic=(nh1+nh2+nh0)*1./ifm->second.size();
  hhitparasic->Fill(hitsparasic);
  hhitvsratio->Fill(nhit,hitsparasic);
  if (hitsparasic>2.5 && nhit>150)
  {nshower++;_numberOfShower++;
    hr0->Fill(_t-_tspill-iseed*2E-7,nh0);
    hr1->Fill(_t-_tspill-iseed*2E-7,nh1);
    hr2->Fill(_t-_tspill-iseed*2E-7,nh2);

  nh0=nh0+int((_t-_tspill-iseed*2E-7)*57.);
  nh1=nh1+int((_t-_tspill-iseed*2E-7)*9.8);
  nh2=nh2+int((_t-_tspill-iseed*2E-7)*3.4);
    
     double en=(bestX1+bestX2*nhit+bestX3*nhit*nhit)*nh0;
 
  en=en+(bestY1+bestY2*nhit+bestY3*nhit*nhit)*nh1;
  en=en+(bestZ1+bestZ2*nhit+bestZ3*nhit*nhit)*nh2;
  en=en*0.755;
    std::cout<<" CANDIDATE seed :"<<iseed<<" -> chambers: "<<im->second.count()<<" -> frames: "<<ifm->second.size()<<" -> Hits: "<<nhit<<" -> ratio:"<<hitsparasic<<" "<<nh0<<" "<<nh1<<" "<<nh2<<" "<<en<<std::endl;
    hnhits->Fill(nhit);
    hen->Fill(en);
  }
  else
  {nmuon++;_numberOfMuon++;
    hnhitm->Fill(nhit);
  }
  //std::cout<<" CANDIDATE seed :"<<im->first<<" -> chambers: "<<im->second.count()<<" -> frames: "<<ifm->second.size()<<" -> Hits: "<<nhit<<" -> ratio:"<<hitsparasic<<std::endl;
}
#ifdef TESTMAINEXAMPLE
int main()
{
  levbdim::basicreader bs("/tmp");
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