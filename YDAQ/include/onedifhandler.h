#ifndef ONE_DIF_HANDLER_INCLUDE
#define ONE_DIF_HANDLER_INCLUDE
#include <yami4-cpp/yami.h>

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "dif.h"

namespace Dif {
  class onedifhandler
  {
  public:
    onedifhandler( std::string objectname,
		   std::string serveraddress,
		   yami::agent* clientagent);

  

    void operator()(yami::incoming_message & message);
    void Stop();
    void Start();
    void Scan();
    void Initialise();
    void Configure(int32_t reg,std::string dbs);
    void Print();
    void Subscribe();
  private:
    Dif::Statemachine *s;
    Dif::Scanstatus Res;
    Dif::Config cf;
    Dif::Difstatus dst;
    Dif::Data d;

    std::string object_name;
    std::string server_address;
    yami::agent *client_agent;
  };
}
#endif
