#include "RpcDIFClient.h"
#include "RpcCCCClient.h"
#include "RpcDbClient.h"
#include "RpcZupClient.h"
#include "RpcShmClient.h"
#include <unistd.h>
#include <stdint.h>
#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;

std::string theDBPrefix_,theCCCPrefix_,theWriterPrefix_,theProxyPrefix_,theZupPrefix_;
std::vector<std::string> theDIFPrefix_;
std::vector<RpcDIFClient::rpiClient*> theDIFs_;

void processStatus(const std::string &jsonString,Json::Value &m_processInfo)
{
        Json::Reader reader;
        bool parsingSuccessful = reader.parse(jsonString, m_processInfo);

        if (parsingSuccessful)
        {
                Json::StyledWriter styledWriter;
                std::cout << styledWriter.write(m_processInfo) << std::endl;
        }

}



void scandns()
{
  // Look for DB server
  DimBrowser* dbr=new DimBrowser(); 
  char *service, *format; 
  int type;
  // Get DB service
  cout<<"On rentre dans scandns "<<endl;

 char *server,*node;
  dbr->getServers( ); 
  cout<<"On sort \n";
  while(dbr->getNextServer(server, node)) 
    { 
      cout << server << " @ " << node << endl; 
    }
  cout<<"0"<<endl;
  dbr->getServices("/DB/*/DOWNLOAD" ); 
  cout<<"1\n";
  while(type = dbr->getNextService(service, format)) 
    { 
      cout << service << " -  " << format << endl; 
      std::string ss;
      ss.assign(service);
      size_t n=ss.find("/DOWNLOAD");
      cout<<ss.substr(0,n)<<endl;
      theDBPrefix_=ss.substr(0,n);
    } 
  // Get the CCC prefix
  cout<<"2\n";
  dbr->getServices("/DCS/*/STATE" ); 
  while(type = dbr->getNextService(service, format)) 
    { 
      cout << service << " -  " << format << endl; 
      std::string ss;
      ss.assign(service);
      size_t n=ss.find("/STATE");
      theCCCPrefix_=ss.substr(0,n);
    } 

  theWriterPrefix_="";
  dbr->getServices("/DSP/*/STATE" ); 
  while(type = dbr->getNextService(service, format)) 
    { 
      cout << service << " -  " << format << endl; 
      std::string ss;
      ss.assign(service);
      size_t n=ss.find("/STATE");
      theWriterPrefix_=ss.substr(0,n);

 
    } 

  theZupPrefix_="";
  dbr->getServices("/DZUP/*/STATE" ); 
  while(type = dbr->getNextService(service, format)) 
    { 
      cout << service << " -  " << format << endl; 
      std::string ss;
      ss.assign(service);
      size_t n=ss.find("/STATE");
      theZupPrefix_=ss.substr(0,n);
    } 
  std::string theProxyPrefix_="";
  dbr->getServices("/DSP/*/STATE" ); 
  while(type = dbr->getNextService(service, format)) 
    { 
      cout << service << " -  " << format << endl; 
      std::string ss;
      ss.assign(service);
      size_t n=ss.find("/STATE");
      theProxyPrefix_=ss.substr(0,n);;
    }
  theDIFPrefix_.clear();
  dbr->getServices("/DDS/*/STATE" ); 
  while(type = dbr->getNextService(service, format)) 
    { 
      cout << service << " -  " << format << endl; 
      std::string ss;
      ss.assign(service);
      size_t n=ss.find("/STATE");
      std::cout<<"DIF on "<<ss.substr(0,n)<<std::endl;
      theDIFPrefix_.push_back(ss.substr(0,n));
    } 

}

int main()
{
  printf("parsing the config file \n");
  DOMConfigurator::configure("/etc/Log4cxxConfig.xml");
  //_logger->setLevel(log4cxx::Level::getInfo());
  LOG4CXX_INFO (_logWriter, "this is a info message, after parsing configuration file");
  theDIFPrefix_.clear();
  scandns();
  std::cout<<theDBPrefix_<<" "<<theCCCPrefix_<<" "<<theWriterPrefix_<<" "<<theProxyPrefix_<<" "<<theZupPrefix_<<std::endl;
  getchar();

  RpcShmClient::rpiClient* sh= new RpcShmClient::rpiClient(theWriterPrefix_);

  //sh->destroy();
  getchar();
  sh->initialise();
  getchar();
  sh->directory("/tmp");
  getchar();
  
  RpcZupClient::rpiClient* z=new RpcZupClient::rpiClient(theZupPrefix_);

  z->open(1,"/dev/ttyUSB0");
  /*
  getchar();
  z->lvswitch(0);
  sleep((unsigned int) 2);
  z->lvswitch(1);
  sleep((unsigned int) 20);
  z->lvswitch(0);
  sleep((unsigned int) 2);
  z->lvswitch(1);
  sleep((unsigned int) 20);
  std::cout<<" LV is ON"<<std::endl;
  getchar();
  */
  RpcDbClient::rpiClient* db=new RpcDbClient::rpiClient(theDBPrefix_);
  db->download("dome_tricot_46");
  getchar();
  
  RpcCCCClient::rpiClient* c=new RpcCCCClient::rpiClient(theCCCPrefix_);

  c->open("DCCCCC01");
  getchar();
  c->initialise();
  getchar();
  for (std::vector<std::string>::iterator it=theDIFPrefix_.begin();it!=theDIFPrefix_.end();it++)
    {
      RpcDIFClient::rpiClient* s=new RpcDIFClient::rpiClient((*it));
      theDIFs_.push_back(s);
    }
  
      for (std::vector<RpcDIFClient::rpiClient*>::iterator it=theDIFs_.begin();it!=theDIFs_.end();it++)
    {
      (*it)->scan();
      std::cout<<(*it)->status()<<std::endl;
    }
  
  getchar();
  int n=0;
  while (n<3)
    {
      n++;
      //getchar();
      for (std::vector<RpcDIFClient::rpiClient*>::iterator it=theDIFs_.begin();it!=theDIFs_.end();it++)
    {
      (*it)->initialise();
      std::cout<<(*it)->status()<<std::endl;
    }

      for (std::vector<RpcDIFClient::rpiClient*>::iterator it=theDIFs_.begin();it!=theDIFs_.end();it++)
    {
      (*it)->registerDB("dome_tricot_46");
      std::cout<<(*it)->status()<<std::endl;
    }
      /*
      s->initialise();
      std::cout<<s->status()<<std::endl;
      //getchar();
      s->registerDB("dome_tricot_46");
      */
      c->initialise();
      c->configure();
      
      //std::cout<<s->status()<<std::endl;
      //getchar();
      for (std::vector<RpcDIFClient::rpiClient*>::iterator it=theDIFs_.begin();it!=theDIFs_.end();it++)
    {
      (*it)->configure(0x815A1B00);
      std::cout<<(*it)->status()<<std::endl;
    }
      uint32_t run=db->newrun();
      std::cout<<" New run " <<run<<std::endl;


      //s->configure(0x815A1B00);
      //std::cout<<s->status()<<std::endl;
      //getchar();
      sleep((unsigned int) 1);
      uint32_t ndif=0;
      for (std::vector<RpcDIFClient::rpiClient*>::iterator it=theDIFs_.begin();it!=theDIFs_.end();it++)
    {
      (*it)->start();
      std::cout<<(*it)->status()<<std::endl;
      Json::Value _jsroot;
      processStatus((*it)->status(),_jsroot);
      std::cout<<" Number of DIF "<<_jsroot["difs"].size()<<std::endl;
      ndif+=_jsroot["difs"].size();
    }

      sh->start(ndif);
      sleep((unsigned int) 1);
      c->start();
      for (int i=0;i<20;i++)
	{
	  //std::cout<<s->status()<<std::endl;
	  for (std::vector<RpcDIFClient::rpiClient*>::iterator it=theDIFs_.begin();it!=theDIFs_.end();it++)
	    {
	      std::cout<<(*it)->status()<<std::endl;
	    }

	  sleep((unsigned int) 1);
	}
      c->stop();
      //getchar();
      sleep((unsigned int) 1);
      for (std::vector<RpcDIFClient::rpiClient*>::iterator it=theDIFs_.begin();it!=theDIFs_.end();it++)
    {
      (*it)->stop();
      std::cout<<(*it)->status()<<std::endl;
    }


      sh->stop();
      //std::cout<<s->status()<<std::endl;
      sleep((unsigned int) 1);
      //getchar();
      for (std::vector<RpcDIFClient::rpiClient*>::iterator it=theDIFs_.begin();it!=theDIFs_.end();it++)
    {
      (*it)->destroy();
      std::cout<<(*it)->status()<<std::endl;
    }

      //s->destroy();
      //std::cout<<s->status()<<std::endl;
      sleep((unsigned int) 1);
      //getchar();
    }
 
 
  while (true)
    {
      //      std::cout<<s->status()<<std::endl;
      sleep((unsigned int) 3);
    }
}


