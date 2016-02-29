#include "DimShmProxy.h"
#include <unistd.h>
#include <stdint.h>
#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;

int main()
{
printf("parsing the config file \n");
DOMConfigurator::configure("/etc/Log4cxxConfig.xml");
//_logger->setLevel(log4cxx::Level::getInfo());
LOG4CXX_INFO (_logWriter, "this is a info message, after parsing configuration file")

  DimShmProxy* c=new DimShmProxy();

  //s->scandns();
  //getchar();
  //s->scan();
  //getchar();
  //s->initialise();
  while (true)
    sleep((unsigned int) 3);
}
