#include <yami4-cpp/yami.h>
#include <yami4-cpp/agent.h>
#include <yami4-cpp/errors.h>
#include <yami4-cpp/incoming_message.h>
#include <yami4-cpp/outgoing_message.h>
#include <memory>
#include <map>

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <sys/utsname.h>
#include "odbserver.h"
#include "browser.h"
using namespace Odb;
void StatemachineServerImpl::Open(std::string theAddress)
{
  name_server_address=theAddress;

  try
    {
      struct utsname uts;
      uname(&uts);
      std::stringstream s;
      s<<"#ODB#"<<uts.nodename;
      std::cout<<"Objectr Name "<<s.str();
#ifdef NONAMESRV
      const std::string resolved_address =server_agent.add_listener(theAddress);

      std::cout << "The server is listening on "<< resolved_address << std::endl;
#else
      const std::string resolved_address =
	server_agent.add_listener("tcp://*:*");

      std::cout << "The server is listening on "
		<< resolved_address << std::endl;

      yami::parameters bind_params;
      bind_params.set_string("object", s.str());
      bind_params.set_string("location", resolved_address);

      std::auto_ptr<yami::outgoing_message> ns_bind(
            server_agent.send(theAddress,
                "names", "bind", bind_params));

      ns_bind->wait_for_completion();
      if (ns_bind->get_state() != yami::replied)
        {
	  std::cout << "error: "
		    << ns_bind->get_exception_msg() << std::endl;
	  
	  return ;
        }



#endif
      server_agent.register_object(s.str(),*this);

      std::cout<<"Object "<<s.str()<<" is registered"<<std::endl;

      databuf.clear();
      datapublisher.clear();
	  
      // block
      // block
      std::string dummy;
      std::cin >> dummy;
    }
  catch (const std::exception & e)
    {
      std::cout << "error: " << e.what() << std::endl;
    }
}

void StatemachineServerImpl:: Initialise(Odb::Status & Res)
{
  Res.Oraclestatus="INITIALISED";
}
void StatemachineServerImpl::Download(const Config & Conf,Odb::Status & Res)
{
  theConf_=Conf;
  for (std::map<uint32_t,Odb::Dbbuffer*>::iterator itd=databuf.begin();itd!=databuf.end();itd++)
    delete itd->second;
  databuf.clear();
  for (std::map<uint32_t,yami::value_publisher*>::iterator itd=datapublisher.begin();itd!=datapublisher.end();itd++)
    delete itd->second;

  datapublisher.clear();

  for( int itd=0;itd<255;itd++)
    {
      std::map<uint32_t,Odb::Dbbuffer*>::iterator itdf=databuf.find((itd));
      if (itdf==databuf.end())
	{
	  Odb::Dbbuffer* d=new Odb::Dbbuffer;
	  d->Difid=itd;
	  std::pair<uint32_t,Odb::Dbbuffer*> p((itd),d);
	  databuf.insert(p);
	  yami::value_publisher* vp=new yami::value_publisher;
	  std::stringstream s("");
	  s<<"/DBSERVER/"<<theConf_.Dbstate<<"/DIF"<<(itd);
	  server_agent.register_value_publisher(s.str(),*vp);
	  std::pair<uint32_t,yami::value_publisher*> pp((itd),vp);
	  datapublisher.insert(pp);
	
	}
      
    }
}
void StatemachineServerImpl::Dispatch(Odb::Status & Res)
{
  for (std::map<uint32_t,yami::value_publisher*>::iterator itvp=datapublisher.begin();itvp!=datapublisher.end();itvp++)
    {
      yami::parameters Conf_;
      std::map<uint32_t,Odb::Dbbuffer*>::iterator itd=databuf.find(itvp->first);
      itd->second->write(Conf_); 
      itvp->second->publish(Conf_);

    }
}

