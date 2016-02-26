#include "DimBMP183Server.h"
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
LOG4CXX_INFO (_logger, "this is a info message, ignore.")
DimBMP183Server* s=new DimBMP183Server();
  while (true)
	{	
	  //s->getTemperature();
	  //	s->getPression();
		
    sleep((unsigned int) 1);
	}	
}
