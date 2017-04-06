#ifndef _levbdim_tdcreadbinary_h
#define _levbdim_tdcreadbinary_h

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
  class tdcreadbinary
  {
  public:
    tdcreadbinary(std::string dire="/tmp");
    void Read();
    void end();
    void open(std::string name);
    void close();
    void read();
    void draw(std::vector<recoPoint*> vp);

    void geometry(std::string name);
    void timeAnalysis();
    void processEvent(uint32_t seed);
    uint32_t totalSize();
    uint32_t eventNumber();
    uint32_t runNumber();
    void pedestalAnalysis();
    void addRun(uint32_t r,std::string name) { _files.push_back(std::pair<uint32_t,std::string>(r,name));}
  private:
    std::vector<std::pair<uint32_t,std::string> > _files;
    uint64_t _bxId;
    uint32_t _gtc;
    double _t,_t0,_tspill;
    std::string _directory;
    uint32_t _run,_event,_totalSize;
    int32_t _fdIn;
    bool _started;
    unsigned char _buf[32*1024*1024];
    uint32_t _idx;
    std::vector<DIFPtr*> theDIFPtrList_;
    jsonGeo* _geo;
    std::map<uint32_t,std::bitset<64> > _tcount;
    std::map<uint32_t,std::vector<std::pair<DIFPtr*,uint32_t> > > _tframe;
    double _readoutTime,_readoutTotalTime;
    uint32_t _numberOfShower,_numberOfMuon;
    DCHistogramHandler* _rh;
    std::vector<recoTrack*> _vtk;
    uint32_t _runType,_dacSet,_mezzanine;
    std::vector<TdcChannel> _channels;
  };
#endif
