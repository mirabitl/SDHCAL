#include <yami4-cpp/yami.h>

#include <cstdlib>
#include <iostream>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include "difserver.h"
int main(int argc, char * argv[])
{
#ifdef ARGGIVEN
  if (argc != 2)
    {
      std::cout
	<< "expecting one parameter: server destination\n";
      return EXIT_FAILURE;
    }

  const std::string server_address = argv[1];

#else
  char* dns=getenv("YAMI_DNS_NODE");
  if (dns==NULL)
    {
      std::cout
	<< "expecting one parameter: server destination\n";
      return EXIT_FAILURE;
    }
  const std::string server_address(dns);
#endif
  Difhw::StatemachineServerImpl s;
  s.Open(server_address);

}
