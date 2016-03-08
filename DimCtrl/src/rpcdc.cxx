#include "RpcDIFClient.h"
#include "RpcCCCClient.h"
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
  LOG4CXX_INFO (_logWriter, "this is a info message, after parsing configuration file");


  RpcCCCClient::rpiClient* c=new RpcCCCClient::rpiClient("/DCS/lyoilcrpi18");

  c->open("DCCCCC01");
  getchar();
  c->initialise();
  getchar();
  RpcDIFClient::rpiClient* s=new RpcDIFClient::rpiClient("/DDS/lyoilcrpi18");
  
  
  getchar();
  while (true)
    {
      s->scan();
      std::cout<<s->status()<<std::endl;
      //getchar();
      s->initialise();
      std::cout<<s->status()<<std::endl;
      //getchar();
      s->registerDB("dome_tricot_46");
      c->initialise();
      c->configure();
      
      std::cout<<s->status()<<std::endl;
      //getchar();
      s->configure(0x815A1B00);
      std::cout<<s->status()<<std::endl;
      //getchar();
      s->start();
      c->start();
      for (int i=0;i<10;i++)
	{
	  std::cout<<s->status()<<std::endl;
	  sleep((unsigned int) 1);
	}
      c->stop();
      //getchar();
      s->stop();
      std::cout<<s->status()<<std::endl;
      //getchar();
      s->destroy();
      std::cout<<s->status()<<std::endl;
      //getchar();
    }
 
 
  while (true)
    {
      std::cout<<s->status()<<std::endl;
      sleep((unsigned int) 3);
    }
}


