#ifndef YAMI_ZUPSERVER_INCLUDE
#define YAMI_ZUPSERVER_INCLUDE
#include "zuplv.h"
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <stdint.h>
#include "Zup.h"
using namespace std;
#include <sstream>
namespace Zuplv {

  class StatemachineServerImpl : public StatemachineServer
  {

  public:
    void Open(std::string ad);
    
    virtual void Check(Status & Res);
    virtual void On(Status & Res);
    virtual void Off(Status & Res);
    virtual void Initialise(const Config & Conf);
    virtual void Close();
  private:
    Config theConf_;
    std::string name_server_address;
    yami::agent server_agent;
    Zup* theZup_;
  };
}
#endif
