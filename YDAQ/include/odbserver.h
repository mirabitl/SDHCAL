#ifndef YAMI_ODBSERVER_INCLUDE
#define YAMI_ODBSERVER_INCLUDE
#include "odb.h"
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <stdint.h>
#include "OracleDIFDBManager.h"
#include "DIFReadoutConstant.h"

using namespace std;
namespace Odb {

  class StatemachineServerImpl : public StatemachineServer
  {

  public:
    void Open(std::string ad);
    virtual void Initialise(Status & Res);
    virtual void Download(const Config & Conf, Status & Res);
    virtual void Dispatch(Status & Res);
  private:
    Config theConf_;
    std::string name_server_address;
    yami::agent server_agent;
    std::map<uint32_t,Odb::Dbbuffer*> databuf;
    std::map<uint32_t,yami::value_publisher*> datapublisher;

    RunInfo* theRunInfo_;
    OracleDIFDBManager* theDBManager_;
  };
}
#endif
