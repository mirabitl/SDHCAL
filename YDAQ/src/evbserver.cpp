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
#include "evbserver.h"
#include "browser.h"

void Evb::StatemachineServerImpl::Open(std::string theAddress)
{
  name_server_address=theAddress;
  running_=false;
  try
    {
      struct utsname uts;
      uname(&uts);
      std::stringstream s;
      s<<"#EVB#"<<uts.nodename;
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

void Evb::StatemachineServerImpl::Initialise(const Evb::Config & Conf,Evb::Status & Res)
{

  std::cout<<"On ecrirai dans "<<Conf.Shmpath<<std::endl;
  Res.Evbstatus="INITIALISED";
}
void Evb::StatemachineServerImpl::Subscribe()
{
  Dif::browser b(name_server_address,&server_agent);
  b.QueryList();
  std::vector<std::string>& vnames= b.getNames();
  std::vector<std::string>& vlocs=b.getLocation();
  std::size_t size_ = vnames.size();
  
  for (std::size_t i_ = 0; i_ != size_; ++i_)
    {
      if(vnames[i_].substr(0,5).compare("#DIF#")!=0) continue;
      
      yami::parameters params;
      const std::string update_object_name =
	"update_handler";
      params.set_string("destination_object", update_object_name);
      server_agent.register_object(update_object_name, *this);
      for (int i=0;i<255;i++)
	{
	  std::cout<<"Subscirbing " <<i<<std::endl;
	  std::stringstream ss;
	  ss<<"/DIFSERVER/DIF"<<i<<"/DATA";
	  server_agent.send_one_way(vlocs[i_],
				    ss.str(), "subscribe", params);
	}
    }


  
}
void Evb::StatemachineServerImpl:: Start(Evb::Status & Res)
{
  this->Subscribe();
  Res.Evbstatus="STARTED";
  running_=true;
}
void Evb::StatemachineServerImpl::Processdif(const Dif::Data & Buf)
{
  if (!running_) return; 
 const char* buf=Buf.Payload.data();int* ibuf=(int*) buf;
 std::cout << "received update: " << Buf.Difid<<" "<<Buf.Gtc <<" "<<Buf.Payload.size()<<"->"<<ibuf[0]<<" "<<ibuf[1]<<" "<<ibuf[2]<< std::endl;
}
void Evb::StatemachineServerImpl:: Stop(Evb::Status & Res)
{
  this->Subscribe();
  Res.Evbstatus="STOPPED";
  running_=false;
}
