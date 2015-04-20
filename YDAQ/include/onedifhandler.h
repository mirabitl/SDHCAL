#ifndef ONE_DIF_HANDLER_INCLUDE
#define ONE_DIF_HANDLER_INCLUDE
#include <yami4-cpp/yami.h>

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "difhw.h"

namespace Difhw {
  class onedifhandler
  {
  public:
    onedifhandler( std::string objectname,
		   std::string serveraddress,
		   yami::agent* clientagent);

  

    void operator()(yami::incoming_message & message);
    void Stop();
    void Destroy();
    void Start();
    void Scan();
    void Initialise();
    void Configure(int32_t reg,std::string dbs);
    void LoadSlowControl();
    void Print();
    std::string JSONStatus();
    Difhw::Difstatus& getStatus(){return dst;}
    void Subscribe();
  private:
    Difhw::Statemachine *s;
    Difhw::Scanstatus Res;
    Difhw::Config cf;
    Difhw::Difstatus dst;
    Difhw::Data d;

    std::string object_name;
    std::string server_address;
    yami::agent *client_agent;
  };
}
#endif
