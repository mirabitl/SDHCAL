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
#include "cccserver.h"
#include "browser.h"
using namespace Ccc;
void StatemachineServerImpl::Open(std::string theAddress)
{
  name_server_address=theAddress;

  try
    {
      struct utsname uts;
      uname(&uts);
      std::stringstream s;
      s<<"#CCC#"<<uts.nodename;
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

void  StatemachineServerImpl::Configure(Status & Res)
{
  Res.Cccstatus="CONFIGURED";
  Res.CccregisterValid=false;
}

void StatemachineServerImpl::Difreset(Status & Res)
{
  Res.Cccstatus="Difreset";
  Res.CccregisterValid=false;
}
void StatemachineServerImpl::Bcidreset(Status & Res)
{
  Res.Cccstatus="Bcidreset";
  Res.CccregisterValid=false;
}
void StatemachineServerImpl::Startacquisitionauto(Status & Res)
{
  Res.Cccstatus="Startacquisitionauto";
  Res.CccregisterValid=false;
}
void StatemachineServerImpl::Ramfullext(Status & Res)
{
  Res.Cccstatus="Ramfullext";
  Res.CccregisterValid=false;
}
void StatemachineServerImpl::Trigext(Status & Res)
{
  Res.Cccstatus="Trigext";
  Res.CccregisterValid=false;
}
void StatemachineServerImpl::Stopacquisition(Status & Res)
{
  Res.Cccstatus="Stopacquisition";
  Res.CccregisterValid=false;
}
void StatemachineServerImpl::Startsingleacquisition(Status & Res)
{
  Res.Cccstatus="Startsingleacquisition";
  Res.CccregisterValid=false;
}
void StatemachineServerImpl::Pulselemo(Status & Res)
{
  Res.Cccstatus="Pulselemo";
  Res.CccregisterValid=false;
}
void StatemachineServerImpl::Trigger(Status & Res)
{
  Res.Cccstatus="Trigger";
  Res.CccregisterValid=false;
}
void StatemachineServerImpl::Cccreset(Status & Res)
{
  Res.Cccstatus="Cccreset";
  Res.CccregisterValid=false;
}
void StatemachineServerImpl::Spillon(Status & Res)
{
  Res.Cccstatus="Spillon";
  Res.CccregisterValid=false;
}
void StatemachineServerImpl::Spilloff(Status & Res)
{
  Res.Cccstatus="Spilloff";
  Res.CccregisterValid=false;
}
void StatemachineServerImpl::Pausetrigger(Status & Res)
{
  Res.Cccstatus="Pausetrigger";
  Res.CccregisterValid=false;
}
void StatemachineServerImpl::Resumetrigger(Status & Res)
{
  Res.Cccstatus="Resumetrigger";
  Res.CccregisterValid=false;
}
void StatemachineServerImpl::Initialise(const Config & Conf, Status & Res)
{
  theConf_=Conf;
  Res.Cccstatus="INITIALISED";
  Res.CccregisterValid=false;
}
  void StatemachineServerImpl::Writeregister(const Registeraccess & Ra, Status & Res)
{

  Res.Cccstatus="Writeregister";
  Res.CccregisterValid=false;
}
     void StatemachineServerImpl::Readregister(const Registeraccess & Ra, Status & Res)
{

  Res.Cccstatus="Readregister";
  Res.CccregisterValid=true;
  Res.Cccregister=0;
}
