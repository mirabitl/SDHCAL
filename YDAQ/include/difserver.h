#ifndef YAMI_DIFSERVER_INCLUDE
#define YAMI_DIFSERVER_INCLUDE
#include "difhw.h"
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <stdint.h>
#include "DIFReadout.h"
#include "DIFReadoutConstant.h"
using namespace std;
#include <sstream>
#include <map>
#include <vector>
#include "odb.h"

typedef struct 
{
  uint32_t vendorid;
  uint32_t productid;
  char name[12];
  uint32_t id;
  uint32_t type;
} FtdiDeviceInfo;



namespace Difhw {

  class StatemachineServerImpl : public StatemachineServer
  {

  public:
    void Open(std::string ad);
    void readout(uint32_t did);
    virtual void Scan(Scanstatus & Res);
    virtual void Registerdb(const Config & Conf, Difstatus & Res);
    virtual void Loadslowcontrol(Difstatus & Res);
    virtual void Initialise(const Scanstatus & Conf, Difstatus & Res);
    virtual void Start(Difstatus & Res);
    virtual void Stop(Difstatus & Res);
    virtual void Destroy(Difstatus & Res);
    virtual void Processslowcontrol(const Odb::Dbbuffer & Buf);

  private:
    void Loop();
    void Subscribe();
    void Processslowcontrolmsg(yami::incoming_message & im);
    std::string name_server_address;
    yami::agent server_agent;
    std::map<uint32_t,Difhw::Data*> databuf;
    std::map<uint32_t,yami::value_publisher*> datapublisher;
    std::map<uint32_t,DIFReadout*> theDIFMap_;
    std::map<uint32_t,FtdiDeviceInfo> theFtdiDeviceInfoMap_;	
    std::map<uint32_t,Odb::Dbbuffer> slowbufmap;	

    bool running_,readoutStarted_;
    boost::thread    m_Thread_s;
    boost::thread  m_Thread_d[255];
    boost::thread_group g_d;
    boost::function<void(yami::incoming_message &)> theDBMsgHandler;
    Config theConf_;
  };
}
#endif
