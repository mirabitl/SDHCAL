#ifndef YAMI_EVBSERVER_INCLUDE
#define YAMI_EVBSERVER_INCLUDE
#include "evb.h"
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
    void Subscribe();
    virtual void Processdif(const Dif::Data & Buf);
    void Processdifmsg(yami::incoming_message & im);
  private:
    bool running_;
    Config theConf_;
    std::string name_server_address;
    yami::agent server_agent;
    boost::function<void(yami::incoming_message &)> theDifMsgHandler;
  };
}
#endif
