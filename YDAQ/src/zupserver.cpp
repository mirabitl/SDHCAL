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
#include "zupserver.h"
#include "browser.h"
using namespace Zuplv;
void StatemachineServerImpl::Open(std::string theAddress)
{
  name_server_address=theAddress;

  try
    {
      struct utsname uts;
      uname(&uts);
      std::stringstream s;
      s<<"#ZUP#"<<uts.nodename;
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

void StatemachineServerImpl:: Check(Status & Res)
{
  if (theZup_!=NULL)
    {

      Res.Voltage=theZup_->ReadVoltageSet();
      Res.Voltageread=theZup_->ReadVoltageUsed();
      Res.Current=theZup_->ReadCurrentUsed();
      Res.Zupstatus="CHECK";
    }
  else
    {
      Res.Zupstatus="NOZUP";
      Res.Voltage=0.1;
      Res.Voltageread=0.1;
      Res.Current=0.1;
    }

}
void StatemachineServerImpl:: On(Status & Res)
{
  if (theZup_!=NULL)
    {
      theZup_->ON();
      sleep((unsigned int) 1);
      Res.Voltage=theZup_->ReadVoltageSet();
      Res.Voltageread=theZup_->ReadVoltageUsed();
      Res.Current=theZup_->ReadCurrentUsed();
      Res.Zupstatus="ON";
    }
  else
    {
      Res.Zupstatus="NOZUP";
      Res.Voltage=0.1;
      Res.Voltageread=0.1;
      Res.Current=0.1;
    }
}
void StatemachineServerImpl:: Off(Status & Res)
{
  if (theZup_!=NULL)
    {
      theZup_->OFF();
      sleep((unsigned int) 1);
      Res.Voltage=theZup_->ReadVoltageSet();
      Res.Voltageread=theZup_->ReadVoltageUsed();
      Res.Current=theZup_->ReadCurrentUsed();
      Res.Zupstatus="OFF";
    }
  else
    {
      Res.Zupstatus="NOZUP";
      Res.Voltage=0.1;
      Res.Voltageread=0.1;
      Res.Current=0.1;
    }
}

void StatemachineServerImpl::Initialise(const Config & Conf)
{
  theConf_=Conf;
  //device.erase(m->getPayloadSize(),-1);
  theZup_= new Zup(Conf.Serial,Conf.Port);

}
void StatemachineServerImpl::Close()
{
  if (theZup_!=NULL)
    {
      delete theZup_;
      theZup_=NULL;
    }

}
