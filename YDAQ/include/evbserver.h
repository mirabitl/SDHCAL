#ifndef YAMI_EVBSERVER_INCLUDE
#define YAMI_EVBSERVER_INCLUDE
#include "dif.h"
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <stdint.h>

namespace Evb {

  class StatemachineServerImpl : public StatemachineServer
  {

  public:
    void Open(std::string ad);
    virtual void Initialise(const Config & Conf, Status & Res);
    virtual void Start(Status & Res);
    virtual void Stop(Status & Res);
    
    
  private:
    void Loop();
    yami::agent server_agent;
    std::map<uint32_t,Dif::Data*> databuf;
    std::map<uint32_t,yami::value_publisher*> datapublisher;
  };
}
#endif
