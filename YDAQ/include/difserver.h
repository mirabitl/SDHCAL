#ifndef YAMI_DIFSERVER_INCLUDE
#define YAMI_DIFSERVER_INCLUDE
#include "dif.h"
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <stdint.h>

namespace Dif {

  class StatemachineServerImpl : public StatemachineServer
  {

  public:
    void Open(std::string ad);
    void readout(uint32_t did);
    virtual void Scan(Scanstatus & Res);
    virtual void Configure(const Config & Conf, Difstatus & Res);
    virtual void Initialise(const Scanstatus & Conf, Difstatus & Res);
    virtual void Start(Difstatus & Res);
    virtual void Stop(Difstatus & Res);
    virtual void Destroy(Difstatus & Res);
    virtual void Processsc(const Odb::Dbbuffer & Buf)
    {std::cout<<"not yet done "<<std::endl;}

  private:
    void Loop();
    yami::agent server_agent;
    std::map<uint32_t,Dif::Data*> databuf;
    std::map<uint32_t,yami::value_publisher*> datapublisher;
    bool running_,readoutStarted_;
    boost::thread    m_Thread_s;
    boost::thread  m_Thread_d[255];
    boost::thread_group g_d;
  };
}
#endif
