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
#include "difserver.h"
using namespace Dif;
void StatemachineServerImpl::Open(std::string theAddress)
{

  try
    {
      struct utsname uts;
      uname(&uts);
      std::stringstream s;
      s<<"#DIF#"<<uts.nodename;
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
      readoutStarted_=false;
      running_=false;
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

void StatemachineServerImpl:: Scan(Scanstatus & Res)
{
  Res.Diflist.clear();
  for (int i=1;i<=53;i++)
    Res.Diflist.push_back(i);
  
}
void StatemachineServerImpl::Initialise(const Scanstatus & Conf,Difstatus & Res)
{
  Res.GtcValid=false;
  Res.BcidValid=false;
  Res.Status.clear();
  Res.Debug.clear();
  for( std::vector<int>::const_iterator itd=Conf.Diflist.begin();itd!=Conf.Diflist.end();itd++)
    {
      std::map<uint32_t,Dif::Data*>::iterator itdf=databuf.find((*itd));
      if (itdf==databuf.end())
	{
	  Dif::Data* d=new Dif::Data;
	  d->Gtc=0;
	  d->Bcid=0;
	  std::pair<uint32_t,Dif::Data*> p((*itd),d);
	  databuf.insert(p);
	  yami::value_publisher* vp=new yami::value_publisher;
	  std::stringstream s("");
	  s<<"/DIFSERVER/DIF"<<(*itd)<<"/DATA";
	  server_agent.register_value_publisher(s.str(),*vp);
	  std::pair<uint32_t,yami::value_publisher*> pp((*itd),vp);
	  datapublisher.insert(pp);
	  Res.Status.push_back(((*itd)<<8)|0x15);
	  s<<" is created";
	  Res.Debug.push_back(s.str());
	}
      
    }
}
void StatemachineServerImpl::Configure(const Config & Conf,Difstatus & Res)
{
  for (std::map<uint32_t,Dif::Data*>::iterator itd=databuf.begin();itd!=databuf.end();itd++)
    {
      std::stringstream s("");
      s<<"/DIFSERVER/DIF"<<itd->first<<"/CONFIGURED/"<<Conf.Dbstate<<"/"<<Conf.Trigger;
      Res.Status.push_back((itd->first<<8)|0x15);
      Res.Debug.push_back(s.str());

    }
}

void StatemachineServerImpl:: Start(Difstatus & Res)
{
  running_=true;
  if (readoutStarted_)
    {
      for (std::map<uint32_t,Dif::Data*>::iterator itd=databuf.begin();itd!=databuf.end();itd++)
	{

	  std::stringstream s("");
	  s<<"/DIFSERVER/DIF"<<itd->first<<"/ALRSTARTED";
	  Res.Status.push_back((itd->first<<8)|0x15);
	  Res.Debug.push_back(s.str());
	}
      return;
    }
  readoutStarted_=true; 

  for (std::map<uint32_t,Dif::Data*>::iterator itd=databuf.begin();itd!=databuf.end();itd++)
    {
      //m_Thread_d[itd->first]= boost::thread(&DimDIFServer::readout, this,itd->first); 
      g_d.create_thread(boost::bind(&StatemachineServerImpl::readout, this,itd->first));
    
      std::stringstream s("");
      s<<"/DIFSERVER/DIF"<<itd->first<<"/STARTED";
      Res.Status.push_back((itd->first<<8)|0x15);
      Res.Debug.push_back(s.str());
    }
 
}
void StatemachineServerImpl::readout(uint32_t difid)
{
  std::cout<<"Thread of dif "<<difid<<" is started"<<std::endl;
  std::map<uint32_t,Dif::Data*>::iterator itd=databuf.find(difid);
  std::map<uint32_t,yami::value_publisher*>::iterator itvp=datapublisher.find(difid);
  if (itd==databuf.end()) return;
  if (itvp==datapublisher.end()) return;
  unsigned char cbuf[256*1024];
  uint32_t* ibuf=(uint32_t*) cbuf;
  while (readoutStarted_)
    {
      if (!running_) {usleep((uint32_t) 100000);continue;}
      usleep((uint32_t) 100);
      // Modify itd
      itd->second->Difid=difid;
      itd->second->Gtc++;
      itd->second->Bcid=itd->second->Bcid+1000;
      itd->second->Payload.clear();
      size_t size_=rand() % (256*1024);
      if (size_<12) size_=12;
      ibuf[0]=difid;
      ibuf[1]=size_;
      ibuf[2]=itd->second->Bcid;
      itd->second->Payload.assign(cbuf,cbuf+size_);

      // Publish data
      //std::cout<<"Pushing "<<difid<<" "<<itd->second->Gtc<<std::endl;
      yami::parameters Conf_;
      itd->second->write(Conf_); 
      itvp->second->publish(Conf_);
      //std::cout<<"exitiing "<<difid<<" "<<itd->second->Gtc<<std::endl;
    }
}
void StatemachineServerImpl::Stop(Difstatus & Res)
{
  running_=false;
}
void StatemachineServerImpl::Destroy(Difstatus & Res)
{}
