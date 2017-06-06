#ifndef _levbdim_tdcrb_h
#define _levbdim_tdcrb_h

#include <stdint.h>
#include <stdlib.h>
#include "buffer.hh"
#include "shmdriver.hh"
#include <vector>
#include <map>
#include <string>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "DIFSlowControl.h"
#include "jsonGeo.hh"
#include "DCHistogramHandler.h"
#include "recoPoint.hh"
#include "recoTrack.hh"
#include "TdcFpga.hh"
class TdcStrip
{
public:
  TdcStrip(uint16_t dif,uint16_t st,double t0,double t1,double shift=0) :_dif(dif), _str(st),_t0(t0),_t1(t1),_shift(shift){;}
  inline uint8_t strip() const {return _str;}
  inline uint8_t dif() const {return _dif;}
  inline double t0() const {return _t0;}
  inline double t1() const {return _t1;}
  inline double ypos() const {return (_t0-_t1-_shift)/0.125;}
  inline double xpos() const {
    if (_dif%2==1) return -1*(_str*0.4+0.2);
    else return +1*(_str*0.4+0.2);
  }
  
private:
  uint16_t _dif,_str;
  double _t0,_t1,_shift;

};
class tdcrb
{
public:
  tdcrb(std::string dire="/tmp");
  void Read();
  void end();
  void open(std::string name);
  void close();
  void read();
   

  void geometry(std::string name);
    
  uint32_t totalSize();
  uint32_t eventNumber();
  uint32_t runNumber();
  void pedestalAnalysis();
  void scurveAnalysis();
  void normalAnalysis();
  void streamout(uint32_t ndifout);

  void LmAnalysis();
  void addRun(uint32_t r,std::string name) { _files.push_back(std::pair<uint32_t,std::string>(r,name));}
  void setRun(int r){_run=r;}
  void setOutFileId(int32_t fid){_fdOut=fid;}
private:
  std::vector<std::pair<uint32_t,std::string> > _files;
  uint64_t _bxId;
  uint32_t _gtc;
  double _t,_t0,_tspill;
  std::string _directory;
  uint32_t _run,_event,_totalSize;
  uint32_t _nevt,_ntrigger,_nfound,_nbside;
  int32_t _fdIn,_fdOut;
  bool _started;
  unsigned char _buf[32*1024*1024];
  uint32_t _idx;
  jsonGeo* _geo;
  std::map<uint32_t,std::bitset<64> > _tcount;

  double _readoutTime,_readoutTotalTime;
  uint32_t _numberOfShower,_numberOfMuon;
  DCHistogramHandler* _rh;
  std::vector<recoTrack*> _vtk;
  uint32_t _runType,_dacSet,_vthSet,_mezzanine,_difId;
  std::vector<TdcChannel> _channels;
  std::vector<TdcChannel>::iterator _trigger;
  std::vector<TdcStrip> _strips;
  std::map<int32_t,std::vector<levbdim::buffer*> > _bmap;
};
#endif
