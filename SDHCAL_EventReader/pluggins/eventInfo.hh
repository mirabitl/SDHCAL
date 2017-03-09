#ifndef _EVENT_INFO_HH
#define _EVENT_INFO_HH
class eventInfo
{
public:
  eventInfo() : _t0(1E30) {}
  void processDIFs( std::vector<DIFPtr*> vd)
  {
    std::vector<DIFPtr*>::iterator itb =vd.begin();
  if (itb==vd.end()) return;
				      
  DIFPtr* dbase= (*itb);
  
  _bxId=dbase->getAbsoluteBCID();
  _gtc=dbase->getGTC(); 

   if (_t0>1E20)
     _t0=_bxId*2E-7;
   double ct=_bxId*2E-7-_t0;
   if ((ct-_t)>5.)
     {
       _tspill=ct;
       std::cout<<" New Spill====>"<<_tspill<<std::endl;
     }
   _t=ct;


   uint32_t itmin=0xFFFFFFFF;
   uint32_t itmax=0;
   for (std::vector<DIFPtr*>::iterator it = vd.begin();it!=vd.end();it++)
     {
       DIFPtr* d = (*it);
       // Loop on Frames
       for (uint32_t ifra=0;ifra<d->getNumberOfFrames();ifra++)
    {
      uint32_t bc=d->getFrameTimeToTrigger(ifra);
      if (bc<itmin) itmin=bc;
      if (bc>itmax) itmax=bc;
       
      }
    }
    
    _readoutTime=(itmax-itmin)*2E-7;
    if (_readoutTime>0 && _readoutTime<5.)
      _readoutTotalTime+=_readoutTime;

    printf("Event %d BX %ld time %f  in Spill %f  from start %f Length %f  Full acq time %f Efficiency %f \n",_gtc,_bxId,_t,_tspill,_t0,_readoutTime,_readoutTotalTime,100.*_readoutTotalTime/_t);
   
  }  
  inline uint64_t getBxId(){return _bxId;}
  inline uint32_t getGtc(){return _gtc;}
  inline double   getTime(){ return _t;}
  inline double   getT0(){return _t0;}
  inline double   getTSpill(){return _tspill;}
  inline double   getReadoutTime(){ return _readoutTime;}
  inline double   getReadoutTotalTime(){ return _readoutTotalTime;}
private:
  uint64_t _bxId;
  uint32_t _gtc;
  double _t,_t0,_tspill;
  double _readoutTime,_readoutTotalTime;

};
#endif
