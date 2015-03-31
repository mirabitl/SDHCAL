#ifndef YAMI_EVBSERVER_INCLUDE
#define YAMI_EVBSERVER_INCLUDE
#include "evb.h"
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <stdint.h>
#include "ShmProxy.h"
#include "DIFReadoutConstant.h"
using namespace std;
namespace Evb {

  class StatemachineServerImpl : public StatemachineServer
  {

  public:
    void Open(std::string ad);
    virtual void Initialise(const Config & Conf, Status & Res);

    void Subscribe();
    void Processdifmsg(yami::incoming_message & im);
    virtual void Start(const Runconfig & Runconf, Status & Res);
    virtual void Stop(Status & Res);
    virtual void Currentstatus(Status & Res);
    virtual void Processdif(const Difhw::Data & Buf);


  private:
    void updateStatus(int run=0,int event=0,int difid=0,int gtc=0,int dtc=0,int bcid=0);
    bool running_;
    Config theConf_;
    Runconfig theRunconf_;
    Status theCurrentStatus_;
    ShmProxy* theProxy_;

    std::string name_server_address;
    yami::agent server_agent;
    boost::function<void(yami::incoming_message &)> theDifMsgHandler;
  };
}
#endif
