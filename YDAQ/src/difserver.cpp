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
#include "browser.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
#include "ftdi.hpp"
#include <iomanip>
#include <cstring>
#include "ShmProxy.h"
#include <string.h>

using namespace Ftdi;

using namespace Difhw;
void StatemachineServerImpl::Open(std::string theAddress)
{
  name_server_address=theAddress;

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
  theFtdiDeviceInfoMap_.clear();
  int rc=system("rm /var/log/pi/ftdi_devices");
  rc=system("/opt/dhcal/bin/ListDevices.py");
  std::string line;
  std::ifstream myfile ("/var/log/pi/ftdi_devices");
  std::stringstream diflist;



  if (myfile.is_open())
    {
      while ( myfile.good() )
	{
	  getline (myfile,line);
	  FtdiDeviceInfo difi;
	  memset(&difi,0,sizeof(FtdiDeviceInfo));
	  sscanf(line.c_str(),"%x %x %s",&difi.vendorid,&difi.productid,difi.name);
	  if (strncmp(difi.name,"FT101",5)==0)
	    {
	      sscanf(difi.name,"FT101%d",&difi.id); 
	      difi.type=0;
	      std::pair<uint32_t,FtdiDeviceInfo> p(difi.id,difi);
	      theFtdiDeviceInfoMap_.insert(p);
	      Res.Diflist.push_back(difi.id);
	    }
	  if (strncmp(difi.name,"DCCCCC",6)==0)
	    {sscanf(difi.name,"DCCCCC%d",&difi.id);difi.type=0x10;}
	}
      myfile.close();
    }
  else std::cout << "Unable to open file"<<std::endl; 

  
}
void StatemachineServerImpl::Initialise(const Scanstatus & Conf,Difstatus & Res)
{
  Res.GtcValid=false;
  Res.BcidValid=false;
  Res.Status.clear();
  Res.Debug.clear();
  for( std::vector<int>::const_iterator itd=Conf.Diflist.begin();itd!=Conf.Diflist.end();itd++)
    {

      uint32_t difid=(uint32_t) (*itd);
      std::map<uint32_t,FtdiDeviceInfo>::iterator itf=theFtdiDeviceInfoMap_.find(difid);
      if (itf==theFtdiDeviceInfoMap_.end())
	{
	  Res.Status.push_back((difid<<8)|0x0);
	  std::stringstream s("");
	  s<<"No Ftdi device info  found for id "<<difid;
	  Res.Debug.push_back(s.str());
	  continue;
	}
      // Initialisation is done at instatiation
      DIFReadout* dif=NULL;
      try {
	//printf("initialising %s %x \n",itf->second.name,itf->second.productid);
	std::string s(itf->second.name);
	dif = new DIFReadout(s,itf->second.productid);
	//printf("Done \n");
      }
      catch (...)
	{
	  Res.Status.push_back((difid<<8)|0x1);
	  std::stringstream s("");
	  s<<"Cannot create a DIFReadout for id "<<difid;
	  Res.Debug.push_back(s.str());

	  continue;
	}
      try
	{
	  dif->checkReadWrite(0x1234,100);
	}
      catch (LocalHardwareException& e)
	{
#ifdef DEBUG_READ
	  std::cout<<e.message()<<std::endl;
#endif
	  Res.Status.push_back((difid<<8)|0x2);
	  std::stringstream s("");
	  s<<"Cannot check USB access for id "<<difid<<" -> "<<e.message();
	  Res.Debug.push_back(s.str());
	  continue;
	}
      try
	{
	  dif->checkReadWrite(0x1234,100);
	}
      catch (LocalHardwareException& e)
	{
#ifdef DEBUG_READ
	  std::cout<<e.message()<<std::endl;
#endif
	  Res.Status.push_back((difid<<8)|0x2);
	  std::stringstream s("");
	  s<<"Cannot check USB access for id "<<difid<<" -> "<<e.message();
	  Res.Debug.push_back(s.str());
	  continue;

	}
      /*
	if (dif!=NULL)
	delete dif;
	std::cout<<"On re essaie"<<std::endl;

	dif = new DIFReadout(cmd);
      */
      std::pair<uint32_t,DIFReadout*> p(difid,dif);
      theDIFMap_.insert(p);
     

      std::cout<<" The DIF "<<difid<<" is initialized  -> Service "<<std::hex<<dif<<std::dec<<std::endl;


      std::map<uint32_t,Difhw::Data*>::iterator itdf=databuf.find((*itd));
      if (itdf==databuf.end())
	{
	  Difhw::Data* d=new Difhw::Data;
	  d->Gtc=0;
	  d->Bcid=0;
	  std::pair<uint32_t,Difhw::Data*> p((*itd),d);
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
void StatemachineServerImpl::Registerdb(const Config & Conf,Difstatus & Res)
{
  theConf_=Conf;
  this->Subscribe();
  for (std::map<uint32_t,Difhw::Data*>::iterator itd=databuf.begin();itd!=databuf.end();itd++)
    {
      std::stringstream s("");
      s<<"/DIFSERVER/DIF"<<itd->first<<"/REGISTERED/"<<theConf_.Dbstate<<"/"<<theConf_.Trigger;
      Res.Status.push_back((itd->first<<8)|0x15);
      Res.Debug.push_back(s.str());

    }
}

void StatemachineServerImpl::Loadslowcontrol(Difstatus & Res)
{
  for ( std::map<uint32_t,Odb::Dbbuffer>::iterator itd=slowbufmap.begin();itd!=slowbufmap.end();itd++)
    {
      uint32_t difid=itd->first;
      std::map<uint32_t,DIFReadout*>::iterator itr=theDIFMap_.find(difid);      
      if (itr==theDIFMap_.end())
	{
	  Res.Status.push_back((difid<<8)|0x0);
	  std::stringstream s("");
	  s<<"No DIFReadout object for id "<<difid;
	  Res.Debug.push_back(s.str());
	  continue;
	}
      itr->second->setControlRegister(theConf_.Trigger);
      itr->second->setNumberOfAsics(itd->second.Nasic);
      itr->second->configureRegisters();

      uint32_t slc=itr->second->configureChips((SingleHardrocV2ConfigurationFrame*)itd->second.Payload.data() );
      std::stringstream s0;
      s0.str(std::string());
      s0<<"CONFIGURED => ";
      if ((slc&0x0003)==0x01) s0<<"SLC CRC OK       - ";
      else if ((slc&0x0003)==0x02) s0<<"SLC CRC Failed   - ";
      else s0<<"SLC CRC forb  - ";
      if ((slc&0x000C)==0x04) s0<<"All OK      - ";
      else if ((slc&0x000C)==0x08) s0<<"All Failed  - ";
      else  s0<<"All forb - ";
      if ((slc&0x0030)==0x10) s0<<"L1 OK     - ";
      else if ((slc&0x0030)==0x20) s0<<"L1 Failed - ";
      else s0<<"L1 forb   - ";
      std::stringstream s("");
      s<<"/DIFSERVER/DIF"<<itd->first<<"/CONFIGURED/"<<theConf_.Dbstate<<"/"<<s0.str();
      Res.Status.push_back((itd->first<<8)|0x15);
      Res.Debug.push_back(s.str());

    }
}


void StatemachineServerImpl:: Start(Difstatus & Res)
{
  running_=true;
  if (readoutStarted_)
    {
      for (std::map<uint32_t,Difhw::Data*>::iterator itd=databuf.begin();itd!=databuf.end();itd++)
	{
	  

	  std::stringstream s("");
	  s<<"/DIFSERVER/DIF"<<itd->first<<"/ALRSTARTED";
	  Res.Status.push_back((itd->first<<8)|0x15);
	  Res.Debug.push_back(s.str());
	}
      return;
    }
  readoutStarted_=true; 

  for (std::map<uint32_t,Difhw::Data*>::iterator itd=databuf.begin();itd!=databuf.end();itd++)
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
  std::map<uint32_t,Difhw::Data*>::iterator itd=databuf.find(difid);
  std::map<uint32_t,yami::value_publisher*>::iterator itvp=datapublisher.find(difid);
  if (itd==databuf.end()) return;
  if (itvp==datapublisher.end()) return;
  
  std::map<uint32_t,DIFReadout*>::iterator itr=theDIFMap_.find(difid);
  if (itr==theDIFMap_.end()) return;

  unsigned char cbuf[256*1024];
  uint32_t* ibuf=(uint32_t*) cbuf;
  
  while (readoutStarted_)
    {
      if (!running_) {usleep((uint32_t) 100000);continue;}
      usleep((uint32_t) 100);
      try 
	{
				
	  uint32_t nread=itr->second->DoHardrocV2ReadoutDigitalData(cbuf);
	  //printf(" Je lis %d %d \n",difid,nread);
	  if (nread==0) continue;
#ifdef DEBUG_SHM	  
	  ShmProxy::transferToFile(cbuf,
				   nread,
				   ShmProxy::getBufferABCID(cbuf),
				   ShmProxy::getBufferDTC(cbuf),
				   ShmProxy::getBufferGTC(cbuf),
				   ShmProxy::getBufferDIF(cbuf));
#endif

	  itd->second->Difid=difid;
	  itd->second->Gtc=ShmProxy::getBufferDTC(cbuf);
	  itd->second->Bcid=ShmProxy::getBufferABCID(cbuf);
	  itd->second->Payload.clear();
	  itd->second->Payload.assign(cbuf,cbuf+nread);
	  
	}
      catch (LocalHardwareException e)
	{
	  std::cout<<itd->first<<" is not started "<<e.what()<<std::endl;
	}

      

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
{
readoutStarted_=false;
 g_d.join_all();
 for (std::map<uint32_t,DIFReadout*>::iterator itd=theDIFMap_.begin();itd!=theDIFMap_.end();itd++)
   {
     
     
     //m_Thread_d[itd->first].join();
     if (itd->second!=NULL)
       {
	 try 
	   {
	     delete itd->second;
	   }
	 catch (LocalHardwareException& e)
	   {
	     
	     std::cout<<e.message()<<std::endl;

	     
	   }
       }
 
      std::stringstream s("");
      s<<"/DIFSERVER/DIF"<<itd->first<<"/DESTROYED";
      Res.Status.push_back(0XDEAD0);
      Res.Debug.push_back(s.str());
   }
}
void Difhw::StatemachineServerImpl::Subscribe()
{
  Difhw::browser b(name_server_address,&server_agent);
  b.QueryList();
  std::vector<std::string>& vnames= b.getNames();
  std::vector<std::string>& vlocs=b.getLocation();
  std::size_t size_ = vnames.size();
  theDBMsgHandler = boost::bind(&Difhw::StatemachineServerImpl::Processslowcontrolmsg, this, _1);   
  for (std::size_t i_ = 0; i_ != size_; ++i_)
    {
      if(vnames[i_].substr(0,5).compare("#DB#")!=0) continue;
      
      yami::parameters params;
      const std::string update_object_name =
	"update_handler";
      params.set_string("destination_object", update_object_name);

      server_agent.register_object(update_object_name,(theDBMsgHandler));
     
      for (std::map<uint32_t,Difhw::Data*>::iterator itd=databuf.begin();itd!=databuf.end();itd++)
	{
	  std::cout<<"Subscribing Slow control info of " <<itd->first<<std::endl;
	  std::stringstream ss;
	  ss<<"/DBSERVER/"<<theConf_.Dbstate<<"/DIF"<<itd->first;
	  server_agent.send_one_way(vlocs[i_],
				    ss.str(), "subscribe", params);
	}
    }


  
}

void Difhw::StatemachineServerImpl::Processslowcontrolmsg(yami::incoming_message & im)
{

  //std::cout<<im.get_object_name()<<std::endl;
  Odb::Dbbuffer Buf;
  Buf.read(im.get_parameters());
  
  Processslowcontrol(Buf);

}
void Difhw::StatemachineServerImpl::Processslowcontrol(const Odb::Dbbuffer & Buf)
{
  std::map<uint32_t,Odb::Dbbuffer>::iterator itd=slowbufmap.find(Buf.Difid);
  if (itd!=slowbufmap.end())
    itd->second=Buf;
  else
    {
      std::pair<uint32_t,Odb::Dbbuffer> p(Buf.Difid,Buf);
      slowbufmap.insert(p);
    }
}
