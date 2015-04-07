#ifndef YAMI_CCCSERVER_INCLUDE
#define YAMI_CCCSERVER_INCLUDE
#include "ccc.h"
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <stdint.h>
#include "CCCManager.h"
namespace Ccc {

  class StatemachineServerImpl : public StatemachineServer
  {

  public:
    void Open(std::string ad);
    
    virtual void Initialise(const Config & Conf, Status & Res);
    virtual void Configure(Status & Res);
    virtual void Start(Status & Res);
    virtual void Stop(Status & Res);
    virtual void Difreset(Status & Res);
    virtual void Bcidreset(Status & Res);
    virtual void Startacquisitionauto(Status & Res);
    virtual void Ramfullext(Status & Res);
    virtual void Trigext(Status & Res);
    virtual void Stopacquisition(Status & Res);
    virtual void Startsingleacquisition(Status & Res);
    virtual void Pulselemo(Status & Res);
    virtual void Trigger(Status & Res);
    virtual void Cccreset(Status & Res);
    virtual void Spillon(Status & Res);
    virtual void Spilloff(Status & Res);
    virtual void Pausetrigger(Status & Res);
    virtual void Resumetrigger(Status & Res);
    virtual void Writeregister(const Registeraccess & Ra, Status & Res);
    virtual void Readregister(const Registeraccess & Ra, Status & Res);

  private:
    Config theConf_;
    std::string name_server_address;
    yami::agent server_agent;
    CCCManager* theManager_;

  };
}
#endif
