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
#include "LCIOWritterInterface.h"
#include "BasicWritterInterface.h"

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
      while (true)
	{
	  sleep((unsigned int) 1);
	}
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
  theConf_=Conf;
  std::cout<<"On ecrirai dans "<<theConf_.Shmpath<<std::endl;
  DIFWritterInterface* lc= NULL;
  bool ondisk=(theConf_.Outputmode.compare("LCIO")==0)||(theConf_.Outputmode.compare("BINARY")==0);
  if (theConf_.Outputmode.compare("LCIO")==0)
    lc= new LCIOWritterInterface();
  if (theConf_.Outputmode.compare("BINARY")==0)
     lc= new BasicWritterInterface();
  theProxy_=new ShmProxy(theConf_.Numberoffragment,ondisk,lc);
  theProxy_->setDirectoryName(theConf_.Outputpath);
  theProxy_->Initialise();
  theProxy_->Configure();
  Res.Evbstatus="INITIALISED";

}
void Evb::StatemachineServerImpl::Subscribe()
{
  Difhw::browser b(name_server_address,&server_agent);
  b.QueryList();
  std::vector<std::string>& vnames= b.getNames();
  std::vector<std::string>& vlocs=b.getLocation();
  std::size_t size_ = vnames.size();
  theDifMsgHandler = boost::bind(&Evb::StatemachineServerImpl::Processdifmsg, this, _1);   
  for (std::size_t i_ = 0; i_ != size_; ++i_)
    {
      if(vnames[i_].substr(0,5).compare("#DIF#")!=0) continue;
      
      yami::parameters params;
      const std::string update_object_name =
	"update_handler";
      params.set_string("destination_object", update_object_name);

      server_agent.register_object(update_object_name,theDifMsgHandler);
      for (int i=0;i<255;i++)
	{

	  std::stringstream ss;
	  ss<<"/DIFSERVER/DIF"<<i<<"/DATA";
	  std::cout<<"Subscirbing " <<ss.str()<<"@"<<vlocs[i_]<<std::endl;
	  server_agent.send_one_way(vlocs[i_],
				    ss.str(), "subscribe", params);
	}
    }


  
}

void Evb::StatemachineServerImpl::updateStatus(int run,int event,int difid,int gtc,int dtc,int bcid)
{
  theCurrentStatus_.RunValid=(run!=0);
  if (run!=0)
    theCurrentStatus_.Run=run;
  theCurrentStatus_.StarttimeValid=(run!=0) && event==0;

  if (run!=0 && event==0)
    {
      time_t rawtime;
      struct tm * timeinfo;
      char buffer[80];

      time (&rawtime);
      timeinfo = localtime(&rawtime);

      strftime(buffer,80,"%d-%m-%Y %I:%M:%S",timeinfo);
      theCurrentStatus_.Starttime.assign(buffer);

    }
  theCurrentStatus_.CompletedValid=(event!=0);
  theCurrentStatus_.Completed=event;
  theCurrentStatus_.EventValid=(event!=0);
  theCurrentStatus_.Event=event;
  theCurrentStatus_.DifidValid=(difid!=0 || theCurrentStatus_.Difid.size()!=0);
  
  theCurrentStatus_.GtcValid=(difid!=0 || theCurrentStatus_.Difid.size()!=0);
  theCurrentStatus_.DtcValid=(difid!=0 || theCurrentStatus_.Difid.size()!=0);
  theCurrentStatus_.BcidValid=(difid!=0 || theCurrentStatus_.Difid.size()!=0);
  if (difid==0) return;
  std::vector<int>::iterator itd = std::find(theCurrentStatus_.Difid.begin(),theCurrentStatus_.Difid.end(),difid);
  if (itd==theCurrentStatus_.Difid.end())
    {
      theCurrentStatus_.Difid.push_back(difid);
      theCurrentStatus_.Gtc.push_back(gtc);
      theCurrentStatus_.Dtc.push_back(dtc);
      theCurrentStatus_.Bcid.push_back(bcid);

    }
  else
    {
      size_t index = std::distance(theCurrentStatus_.Difid.begin(), itd);
      theCurrentStatus_.Gtc[index]=gtc;
      theCurrentStatus_.Dtc[index]=dtc;
      theCurrentStatus_.Bcid[index]=bcid;
    }
}

void Evb::StatemachineServerImpl:: Start(const Runconfig & Runconf,Evb::Status & Res)
{

  this->Subscribe();
  theRunconf_=Runconf;
  if (theProxy_ != NULL)
    {
      std::cout <<"Starting the run "<<Runconf.Dbstate<<std::endl;
      theProxy_->setSetupName(Runconf.Dbstate);
      std::cout <<"Starting the run "<<Runconf.Numberoffragment<<std::endl;
      theProxy_->setNumberOfDIF(Runconf.Numberoffragment);

      theProxy_->purgeShm(); // remove old data not written
      theProxy_->Start(Runconf.Run,theConf_.Outputpath);
      running_=true;
    }
  updateStatus(Runconf.Run);
  Res=theCurrentStatus_;
  Res.Evbstatus="STARTED";

}
void Evb::StatemachineServerImpl::Processdifmsg(yami::incoming_message & im)
{
  if (!running_) return; 
  //std::cout<<im.get_object_name()<<std::endl;
  Difhw::Data Buf;
  Buf.read(im.get_parameters());
  
  Processdif(Buf);

}

void Evb::StatemachineServerImpl::Processdif(const Difhw::Data & Buf)
{

  unsigned char* buf= (unsigned char*) Buf.Payload.data();int* ibuf=(int*) buf;
  //std::cout << "received update: " << Buf.Difid<<" "<<Buf.Gtc <<" "<<Buf.Payload.size()<<"->"<<ibuf[0]<<" "<<ibuf[1]<<" "<<ibuf[2]<< std::endl;
 ShmProxy::transferToFile(buf,
			  Buf.Payload.size(),
			  ShmProxy::getBufferABCID(buf),
			  ShmProxy::getBufferDTC(buf),
			  ShmProxy::getBufferGTC(buf),
			  ShmProxy::getBufferDIF(buf));
 updateStatus(0,0,ShmProxy::getBufferDIF(buf),ShmProxy::getBufferDTC(buf),ShmProxy::getBufferGTC(buf),ShmProxy::getBufferABCID(buf));
}
void Evb::StatemachineServerImpl:: Stop(Evb::Status & Res)
{
  running_=false;
  sleep((unsigned int) 1);
  std::cout<<"Stopping"<<std::endl;
  theProxy_->Stop();
  Res.Evbstatus="STOPPED";
  std::cout<<"Stopped"<<std::endl;

}
void Evb::StatemachineServerImpl:: Currentstatus(Evb::Status & Res)
{
  if (theProxy_!=NULL)
    {
      updateStatus(theProxy_->getRunNumber(),theProxy_->getEventNumber());
    }
  Res=theCurrentStatus_;
  Res.Evbstatus="UPDATED";

}
