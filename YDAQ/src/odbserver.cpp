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
  Res.RunValid=false;
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
  if (theDBManager_!=NULL)
    {
      delete theDBManager_;
      
    }
  
  theDBManager_= new OracleDIFDBManager("74",Conf.Dbstate);
  theDBManager_->initialize();
  theDBManager_->download();
  std::map<uint32_t,unsigned char*> dbm=theDBManager_->getAsicKeyMap();
  for (std::map<uint32_t,unsigned char*>::iterator idb=dbm.begin();idb!=dbm.end();idb++)
    {
      uint32_t id = (idb->first>>8)&0xFF;
      std::map<uint32_t,Odb::Dbbuffer*>::iterator itdf=databuf.find(id);
      if (itdf==databuf.end())
	{
	  Odb::Dbbuffer* d=new Odb::Dbbuffer;
	  d->Difid=id;
	  std::pair<uint32_t,Odb::Dbbuffer*> p(id,d);
	  databuf.insert(p);
	  yami::value_publisher* vp=new yami::value_publisher;
	  std::stringstream s("");
	  s<<"/DBSERVER/"<<theConf_.Dbstate<<"/DIF"<<(id);
	  server_agent.register_value_publisher(s.str(),*vp);
	  std::pair<uint32_t,yami::value_publisher*> pp((id),vp);
	  datapublisher.insert(pp);
	
	}
    }
  
  

  for( uint32_t id=1;id<256;id++)
    {
      std::map<uint32_t,Odb::Dbbuffer*>::iterator itdf=databuf.find(id);

      if (itdf==databuf.end()) continue;
      itdf->second->Nasic=0;
      for (uint32_t iasic=1;iasic<=48;iasic++)
	{
	  uint32_t key=(id<<8)|iasic;
	  std::map<uint32_t,unsigned char*>::iterator it=dbm.find(key);
	  if (it==dbm.end()) continue;
	  unsigned char* bframe=it->second;
	  uint32_t       framesize=bframe[0];
	  itdf->second->Payload.assign(&bframe[1],&bframe[1]+framesize);
	  itdf->second->Nasic++;

	}
    }
    Res.Oraclestatus="DOWNLOAD";
    Res.RunValid=false;

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

    Res.Oraclestatus="DISPATCH";
    Res.RunValid=false;

}
void StatemachineServerImpl::Newrun(Odb::Status & Res)
{
   if (theRunInfo_==NULL)
    {
      try {


	std::stringstream daqname("");    
	char dateStr [64];
            
	time_t tm= time(NULL);
	strftime(dateStr,50,"LaDaqAToto_%d%m%y_%H%M%S",localtime(&tm));
	daqname<<dateStr;
	Daq* me=new Daq(daqname.str());

	printf("la daq est creee %s\n",daqname.str().c_str());
	me->setStatus(0);
	printf("la daq a change de statut\n");
	me->setXML("/opt/dhcal/include/dummy.xml");
	me->uploadToDatabase();
	printf("Upload DOne");
  
	theRunInfo_=new RunInfo(0,"LaDaqAToto");
	printf("le run est creee\n");
	theRunInfo_->setStatus(1);
	Res.Run=theRunInfo_->getRunNumber();
      } catch (ILCException::Exception e)
	{
	  theRunInfo_=NULL;
	  std::cout<<e.getMessage()<<std::endl;
	}
    }
  else
    {
      theRunInfo_->setStatus(4);
      delete theRunInfo_;
      theRunInfo_=new RunInfo(0,"LaDaqAToto");
      theRunInfo_->setStatus(1);
     Res.Run=theRunInfo_->getRunNumber();
      
    }
    Res.Oraclestatus="NEWRUN";
    Res.RunValid=true;

}
void StatemachineServerImpl::Currentrun(Odb::Status & Res)
{
   if (theRunInfo_==NULL)
    {
      this->Newrun(Res);
    }
  else
    {
      Res.Run=theRunInfo_->getRunNumber();
      
    }
    Res.Oraclestatus="CURRENTRUN";
    Res.RunValid=true;

}
