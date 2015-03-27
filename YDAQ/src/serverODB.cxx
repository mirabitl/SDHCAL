#include <yami4-cpp/yami.h>

#include <cstdlib>
#include <iostream>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include "odbserver.h"
int main(int argc, char * argv[])
{
  if (argc != 2)
    {
      std::cout
	<< "expecting one parameter: server destination\n";
      return EXIT_FAILURE;
    }

  const std::string server_address = argv[1];
  Odb::StatemachineServerImpl s;
  s.Open(server_address);


}
