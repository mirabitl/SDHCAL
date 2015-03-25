#ifndef DIF_BROWSER_INCLUDE
#define DIF_BROWSER_INCLUDE
#include <yami4-cpp/yami.h>

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <stdint.h>


namespace Dif {
  class browser
  {
  public:
    browser( std::string nameserver_address,
		   yami::agent* clientagent);
    void QueryList();
    std::vector<std::string>& getNames(){return vnames;}
    std::vector<std::string>& getLocation(){return vlocs;}
  private:
    std::string name_server_address;
    yami::agent *client_agent;
    std::vector<std::string> vnames;
    std::vector<std::string> vlocs;
  };
}
#endif
