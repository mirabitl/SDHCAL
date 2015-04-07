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
  if (theManager_!=NULL)
    {
      theManager_->configure();
      Res.Cccstatus="CONFIGURED";
    }
  else
    Res.Cccstatus="NOCCCFOUND";

  Res.CccregisterValid=false;
}
void  StatemachineServerImpl::Start(Status & Res)
{
  if (theManager_!=NULL)
    {
      theManager_->start();
      Res.Cccstatus="STARTED";
    }
  else
    Res.Cccstatus="NOCCCFOUND";

  Res.CccregisterValid=false;
}
void  StatemachineServerImpl::Stop(Status & Res)
{
  if (theManager_!=NULL)
    {
      theManager_->stop();
      Res.Cccstatus="STOPPED";
    }
  else
    Res.Cccstatus="NOCCCFOUND";

  Res.CccregisterValid=false;
}

void StatemachineServerImpl::Difreset(Status & Res)
{
  if (theManager_!=NULL)
    theManager_->getCCCReadout()->DoSendDIFReset();
  Res.Cccstatus="Difreset";
  Res.CccregisterValid=false;
}
void StatemachineServerImpl::Bcidreset(Status & Res)
{
  if (theManager_!=NULL)
    theManager_->getCCCReadout()->DoSendBCIDReset();
  Res.Cccstatus="Bcidreset";
  Res.CccregisterValid=false;
}
void StatemachineServerImpl::Startacquisitionauto(Status & Res)
{
  if (theManager_!=NULL)
    theManager_->getCCCReadout()->DoSendStartAcquisitionAuto();
  Res.Cccstatus="Startacquisitionauto";
  Res.CccregisterValid=false;
}
void StatemachineServerImpl::Ramfullext(Status & Res)
{
  if (theManager_!=NULL)
    theManager_->getCCCReadout()->DoSendRamfullExt();
  Res.Cccstatus="Ramfullext";
  Res.CccregisterValid=false;
}
void StatemachineServerImpl::Trigext(Status & Res)
{
  if (theManager_!=NULL)
    theManager_->getCCCReadout()->DoSendTrigExt();
  Res.Cccstatus="Trigext";
  Res.CccregisterValid=false;
}
void StatemachineServerImpl::Stopacquisition(Status & Res)
{
  if (theManager_!=NULL)
    theManager_->getCCCReadout()->DoSendStopAcquisition();
  Res.Cccstatus="Stopacquisition";
  Res.CccregisterValid=false;
}
void StatemachineServerImpl::Startsingleacquisition(Status & Res)
{
  if (theManager_!=NULL)
    theManager_->getCCCReadout()->DoSendStartSingleAcquisition();
  Res.Cccstatus="Startsingleacquisition";
  Res.CccregisterValid=false;
}
void StatemachineServerImpl::Pulselemo(Status & Res)
{
  if (theManager_!=NULL)
    theManager_->getCCCReadout()->DoSendPulseLemo();
  Res.Cccstatus="Pulselemo";
  Res.CccregisterValid=false;
}
void StatemachineServerImpl::Trigger(Status & Res)
{
  if (theManager_!=NULL)
    theManager_->getCCCReadout()->DoSendTrigger();
  Res.Cccstatus="Trigger";
  Res.CccregisterValid=false;
}
void StatemachineServerImpl::Cccreset(Status & Res)
{
  if (theManager_!=NULL)
    theManager_->getCCCReadout()->DoSendCCCReset();
  Res.Cccstatus="Cccreset";
  Res.CccregisterValid=false;
}
void StatemachineServerImpl::Spillon(Status & Res)
{
  if (theManager_!=NULL)
    theManager_->getCCCReadout()->DoSendSpillOn();
  Res.Cccstatus="Spillon";
  Res.CccregisterValid=false;
}
void StatemachineServerImpl::Spilloff(Status & Res)
{
  if (theManager_!=NULL)
    theManager_->getCCCReadout()->DoSendSpillOff();
  Res.Cccstatus="Spilloff";
  Res.CccregisterValid=false;
}
void StatemachineServerImpl::Pausetrigger(Status & Res)
{
  if (theManager_!=NULL)
    theManager_->getCCCReadout()->DoSendPauseTrigger();
  Res.Cccstatus="Pausetrigger";
  Res.CccregisterValid=false;
}
void StatemachineServerImpl::Resumetrigger(Status & Res)
{
  if (theManager_!=NULL)
    theManager_->getCCCReadout()->DoSendResumeTrigger();
  Res.Cccstatus="Resumetrigger";
  Res.CccregisterValid=false;
}
void StatemachineServerImpl::Initialise(const Config & Conf, Status & Res)
{
  theConf_=Conf;
  theManager_= new CCCManager(Conf.Serial);
  theManager_->initialise();
  
  Res.Cccstatus="INITIALISED";
  Res.CccregisterValid=false;
}
  void StatemachineServerImpl::Writeregister(const Registeraccess & Ra, Status & Res)
{
  if (theManager_!=NULL)
    theManager_->getCCCReadout()->DoWriteRegister(Ra.Address,Ra.Cccregister);

  Res.Cccstatus="Writeregister";
  Res.CccregisterValid=false;
}
     void StatemachineServerImpl::Readregister(const Registeraccess & Ra, Status & Res)
{
  Res.Cccregister=0;
  if (theManager_!=NULL)
    Res.Cccregister=theManager_->getCCCReadout()->DoReadRegister(Ra.Address);

  Res.Cccstatus="Readregister";
  Res.CccregisterValid=true;

}
