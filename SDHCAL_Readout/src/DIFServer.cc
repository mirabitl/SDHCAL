
#include "DIFServer.h"
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
#include "ftdi.hpp"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include "ShmProxy.h"
#include <string.h>

using namespace Ftdi;

DIFServer::DIFServer(std::string host,uint32_t port) : NMServer(host,port)
{
  this->registerCommand("SCANDEVICES",boost::bind(&DIFServer::commandHandler,this,_1));
  this->registerCommand("INITIALISE",boost::bind(&DIFServer::commandHandler,this,_1));
  this->registerCommand("PRECONFIGURE",boost::bind(&DIFServer::commandHandler,this,_1));
  this->registerCommand("CONFIGURECHIPS",boost::bind(&DIFServer::commandHandler,this,_1));
  this->registerCommand("CONFIGUREDIF",boost::bind(&DIFServer::commandHandler,this,_1));

  this->registerCommand("SETDIFPARAM",boost::bind(&DIFServer::commandHandler,this,_1));

  this->registerCommand("START",boost::bind(&DIFServer::commandHandler,this,_1));
  this->registerCommand("STOP",boost::bind(&DIFServer::commandHandler,this,_1));
  this->registerCommand("DESTROY",boost::bind(&DIFServer::commandHandler,this,_1));

  this->registerService("UNESSAI");
  this->start();
  //this->startServices();
  running_=false;
  readoutStarted_=false;
}

void DIFServer::UsbPrepare()
{
  int vid = 0x0403, pid = 0x6001;
  List* list = List::find_all(vid, pid);
  for (List::iterator it = list->begin(); it != list->end(); it++)
    {
      std::cout << "FTDI (" << &*it << "): "
		<< it->vendor() << ", "
		<< it->description() << ", "
		<< it->serial();
      
      // Open test
      if(it->open() == 0)
	{
	  std::cout << " (Open OK)";
	  it->reset();
	}       
      else
	std::cout << " (Open FAILED)";
      
      it->close();
      
      std::cout << std::endl;
      
    }
  
  delete list;
  pid=0x6014; // Usb2 version
  list = List::find_all(vid, pid);
  for (List::iterator it = list->begin(); it != list->end(); it++)
    {
      std::cout << "FTDI (" << &*it << "): "
		<< it->vendor() << ", "
		<< it->description() << ", "
		<< it->serial();
      
      // Open test
      if(it->open() == 0)
	{
	  std::cout << " (Open OK)";
	  it->reset();
	}       
      else
	std::cout << " (Open FAILED)";
      
      it->close();
      
      std::cout << std::endl;
      
    }
  
  delete list;
  return;
}




#undef DUMMYTEST
uint32_t DIFServer::configureChips(std::string path,uint32_t difid) throw (LocalHardwareException)
{
#ifndef DUMMYTEST
  std::map<uint32_t,DIFReadout*>::iterator itd=theDIFMap_.find(difid);
  if (itd==theDIFMap_.end())
    {
      std::stringstream errorMessage;
      errorMessage << "DIF   id ("<< difid << ") is not initialised"<< std::ends;
      throw (LocalHardwareException( "Chip Configuration" ,errorMessage.str(), __FILE__, __LINE__, __FUNCTION__ ) );	  
    }
  else
    {
#endif	
      SingleHardrocV2ConfigurationFrame* slow = new  SingleHardrocV2ConfigurationFrame[48];
      uint32_t nasic=0;
      std::stringstream sd;
      sd<<path<<"/"<<difid<<"/";
      unsigned char cbuf[4096];
      for (uint32_t tasic=48;tasic>0;tasic--)
	{		
	  std::stringstream sf;
	  sf<<sd.str()<<tasic;
	  int fd=::open(sf.str().c_str(),O_RDONLY);
	  if (fd<0) 
	    {
	      printf("%s  Cannot open file %s : return code %d \n",__PRETTY_FUNCTION__,sf.str().c_str(),fd);
	      continue;
	    }	
	  int size_buf=::read(fd,cbuf,4096);
	  printf("# %d ASIC=> %d Read size %d  buffer size %d %x \n",nasic,tasic,size_buf,cbuf[0],cbuf[1]);
	  ::close(fd);
	  memcpy(&slow[nasic],&cbuf[1],cbuf[0]);
	  nasic++;
			
	}
	
	
	
	
	
	
#ifndef DUMMYTEST		
      if (nasic!=48)
	{
	  itd->second->setNumberOfAsics(nasic);
	  itd->second->configureRegisters();
	}
      return itd->second->configureChips(slow);
    }
#else
			 
			
  return 0;
#endif
		
}
uint32_t DIFServer::configureChips(uint32_t difid,SingleHardrocV2ConfigurationFrame* slow,uint32_t nasic) throw (LocalHardwareException)
{
  std::map<uint32_t,DIFReadout*>::iterator itd=theDIFMap_.find(difid);
  if (itd==theDIFMap_.end())
    {
      std::stringstream errorMessage;
      errorMessage << "DIF   id ("<< difid << ") is not initialised"<< std::ends;
      throw (LocalHardwareException( "Chip Configuration" ,errorMessage.str(), __FILE__, __LINE__, __FUNCTION__ ) );	  
    }
  else
    {
      if (nasic!=48)
	{
	  itd->second->setNumberOfAsics(nasic);
	  itd->second->configureRegisters();
	}
      return itd->second->configureChips(slow);
		
    }
}
void DIFServer::preConfigure(uint32_t difid,uint32_t ctrlreg) throw (LocalHardwareException)
{
  std::map<uint32_t,DIFReadout*>::iterator itd=theDIFMap_.find(difid);
  if (itd==theDIFMap_.end())
    {
      std::stringstream errorMessage;
      errorMessage << "DIF   id ("<< difid << ") is not initialised"<< std::ends;
      throw (LocalHardwareException( "PreConfiguration" ,errorMessage.str(), __FILE__, __LINE__, __FUNCTION__ ) );	  
    }
  else
    {
      itd->second->setControlRegister(ctrlreg);
      itd->second->configureRegisters();
		
    }
}
void DIFServer::initialise(uint32_t difid) throw (LocalHardwareException)
{
  std::map<uint32_t,DIFReadout*>::iterator itd=theDIFMap_.find(difid);
  if (itd!=theDIFMap_.end())
    {
      std::stringstream errorMessage;
      errorMessage << "DIF  already registered id ("<< difid << ")"<< std::ends;
      throw (LocalHardwareException( "DIFRegistration" ,errorMessage.str(), __FILE__, __LINE__, __FUNCTION__ ) );	  
    }
  else
    {
      std::map<uint32_t,FtdiDeviceInfo>::iterator itf=theFtdiDeviceInfoMap_.find(difid);
      if (itf==theFtdiDeviceInfoMap_.end())
	{
	  std::cout << " No device found \n";
	  getchar();
	  std::stringstream errorMessage;
	  errorMessage << "DIF not found in Device map ("<< difid << ")"<< std::ends;
	  throw (LocalHardwareException( "DIFRegistration" ,errorMessage.str(), __FILE__, __LINE__, __FUNCTION__ ) );
	}
      char cmd[16];
      memset(cmd,0,16);
      sprintf(cmd,"FT101%03d",difid);
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
	  std::cout<<"cannot initialize "<<difid<<std::endl;
				
	}
      
      dif->checkReadWrite(0x1234,100);

      dif->checkReadWrite(0x1234,100);
      /*
      if (dif!=NULL)
	delete dif;
      std::cout<<"On re essaie"<<std::endl;

      dif = new DIFReadout(cmd);
      */
      std::pair<uint32_t,DIFReadout*> p(difid,dif);
      theDIFMap_.insert(p);
      std::stringstream s;
      s<<"DIF"<<difid;
      this->registerService(s.str());

      std::cout<<" The DIF "<<difid<<" is initialized  -> Service "<<s.str()<<std::endl;

		
    }
}
std::vector<uint32_t>& DIFServer::scanDevices()
{
  system("/opt/dhcal/bin/scan_devices.sh");
  std::string line;
  std::ifstream myfile ("/tmp/ftdi_devices");
  std::stringstream diflist;

  theListOfDIFFounds_.clear();
  uint32_t ndif=0;
  if (myfile.is_open())
    {
      while ( myfile.good() )
	{
	  getline (myfile,line);
	  uint32_t difid=atoi(line.c_str());
	  if (difid<=0) continue;
	  theListOfDIFFounds_.push_back(difid);
	  std::cout << " DIF found "<<difid << std::endl;
	  ndif++;
	}
      myfile.close();
    }
  else std::cout << "Unable to open file"<<std::endl; 
  std::sort(theListOfDIFFounds_.begin(),theListOfDIFFounds_.end());
  if (theListOfDIFFounds_.size()>0)
    {
      for (uint8_t i=0;i<theListOfDIFFounds_.size()-1;i++)
	diflist<<theListOfDIFFounds_[i]<<",";
      diflist<<theListOfDIFFounds_[theListOfDIFFounds_.size()-1];
    }
  std::cout<<ndif<<"List of DIFs is: "<<diflist.str()<<std::endl;
  this->UsbPrepare();

  this->prepareDevices();
  return theListOfDIFFounds_;
}



void DIFServer::prepareDevices()
{
  theFtdiDeviceInfoMap_.clear();
  system("/opt/dhcal/bin/ListDevices.py");
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
	    }
	  if (strncmp(difi.name,"DCCCCC",6)==0)
	    {sscanf(difi.name,"DCCCCC%d",&difi.id);difi.type=0x10;}


	}
      myfile.close();
    }
  else std::cout << "Unable to open file"<<std::endl; 

  for (std::map<uint32_t,FtdiDeviceInfo>::iterator it=theFtdiDeviceInfoMap_.begin();it!=theFtdiDeviceInfoMap_.end();it++)
    printf("Device found and register: %d with info %d %d %s type %d \n", it->first,it->second.vendorid,it->second.productid,it->second.name,it->second.type);
}


NetMessage* DIFServer::commandHandler(NetMessage* m)
{
  printf(" J'ai recu %s COMMAND  \n",m->getName().c_str());
  if (m->getName().compare("SCANDEVICES")==0)
    {
      std::vector<uint32_t> v=scanDevices();
		
      NetMessage* mrep = new NetMessage("SCANDEVICES",NetMessage::COMMAND_ACKNOWLEDGE,v.size()*sizeof(uint32_t));
      uint32_t* vi=(uint32_t*) mrep->getPayload();
      for (uint32_t i=0;i<v.size();i++)
	vi[i]=v[i];
      return mrep;
    }
  if (m->getName().compare("INITIALISE")==0)
    {
      uint32_t difid=0;
      memcpy(&difid,m->getPayload(),sizeof(uint32_t));
      try 
	{
	  this->initialise(difid);
	}
      catch (LocalHardwareException e)
	{
	  std::cout<<e.what()<<std::endl;
	}
      NetMessage* mrep = new NetMessage("INITIALISE",NetMessage::COMMAND_ACKNOWLEDGE,4);
      std::cout << "Answer prepared"<<std::endl;
      return mrep;
    }

	
  if (m->getName().compare("SETDIFPARAM")==0)
    {
      uint32_t *ipay=(uint32_t*) m->getPayload();
      //uint32_t difid=0,nbasic=0,asictype=0,ctrlreg=0;
      //memcpy(&difid,(m->getPayload())[0],sizeof(uint32_t));
      //memcpy(&nbasic,(m->getPayload())[4],sizeof(uint32_t));
      //memcpy(&asictype,(m->getPayload())[8],sizeof(uint32_t));
      //memcpy(&ctrlreg,(m->getPayload())[8],sizeof(uint32_t));
      std::map<uint32_t,DIFReadout*>::iterator itd=theDIFMap_.find(ipay[0]);
      if (itd!=theDIFMap_.end())
	{
	  itd->second->setNumberOfAsics(ipay[1]);
	  itd->second->setAsicType(ipay[2]);
	  itd->second->setControlRegister(ipay[3]);
	}	
		
      NetMessage* mrep = new NetMessage("SETDIFPARAM",NetMessage::COMMAND_ACKNOWLEDGE,4);
      std::cout << "Answer prepared"<<std::endl;
      return mrep;
    }
  if (m->getName().compare("PRECONFIGURE")==0)
    {
      uint32_t ctrlreg=0;
      memcpy(&ctrlreg,m->getPayload(),sizeof(uint32_t));
      for (std::map<uint32_t,DIFReadout*>::iterator itd=theDIFMap_.begin();itd!=theDIFMap_.end();itd++)
	{
	  try 
	    {
	      this->preConfigure(itd->first,ctrlreg);
	    }
	  catch (LocalHardwareException e)
	    {
	      std::cout<<itd->first<<" is not preconfigured "<<e.what()<<std::endl;
	    }
	}
      NetMessage* mrep = new NetMessage("PRECONFIGURE",NetMessage::COMMAND_ACKNOWLEDGE,4);
      return mrep;
    }
	
  if (m->getName().compare("DESTROY")==0)
    {
      readoutStarted_=false;
      g_d.join_all();
      for (std::map<uint32_t,DIFReadout*>::iterator itd=theDIFMap_.begin();itd!=theDIFMap_.end();itd++)
	{


	  //m_Thread_d[itd->first].join();

 
	  delete itd->second;
	  std::stringstream s;
	  s<<"DIF"<<itd->first;
	  std::cout<<s.str()<<" beiing destroyed"<<std::endl;
	  this->destroyService(s.str());
			
	}
      theDIFMap_.clear();
      NetMessage* mrep = new NetMessage("DESTROY",NetMessage::COMMAND_ACKNOWLEDGE,4);
      return mrep;
    }
	
  if (m->getName().compare("START")==0)
    {
      running_=true;
      this->startReadout();
      for (std::map<uint32_t,DIFReadout*>::iterator itd=theDIFMap_.begin();itd!=theDIFMap_.end();itd++)
	{
	  try 
	    {
	      itd->second->start();
	    }
	  catch (LocalHardwareException e)
	    {
	      std::cout<<itd->first<<" is not started "<<e.what()<<std::endl;
	    }
	}
      NetMessage* mrep = new NetMessage("START",NetMessage::COMMAND_ACKNOWLEDGE,4);
      return mrep;
    }
  if (m->getName().compare("STOP")==0)
    {
      running_=false;
      for (std::map<uint32_t,DIFReadout*>::iterator itd=theDIFMap_.begin();itd!=theDIFMap_.end();itd++)
	{
	  try 
	    {
	      itd->second->stop();
	    }
	  catch (LocalHardwareException e)
	    {
	      std::cout<<itd->first<<" is not started "<<e.what()<<std::endl;
	    }
	}

      NetMessage* mrep = new NetMessage("STOP",NetMessage::COMMAND_ACKNOWLEDGE,4);
      return mrep;
    }

  if (m->getName().compare("CONFIGURECHIPS")==0)
    {
      uint32_t difid;
      memcpy(&difid,m->getPayload(),sizeof(uint32_t));
      uint32_t nasic=(m->getPayloadSize()-sizeof(uint32_t))/sizeof(SingleHardrocV2ConfigurationFrame);
      printf("I found DIF %d asics %d \n",difid,nasic);
      SingleHardrocV2ConfigurationFrame* slow =(SingleHardrocV2ConfigurationFrame*) &(m->getPayload())[4];
      uint32_t slc=this->configureChips(difid,slow,nasic);
      NetMessage* mrep = new NetMessage("CONFIGURECHIPS",NetMessage::COMMAND_ACKNOWLEDGE,4);
      uint32_t* ipay=(uint32_t*) mrep->getPayload();
      ipay[0]=slc;
      return mrep;
    }
  if (m->getName().compare("CONFIGUREDIF")==0)
    {
      uint32_t difid;
      memcpy(&difid,m->getPayload(),sizeof(uint32_t));
      std::string setupname( (const char*) &(m->getPayload())[4],(m->getPayloadSize()-4));
		
		
      printf("I found DIF %d Setup %s \n",difid,setupname.c_str());
      uint32_t slc=this->configureChips(setupname,difid);
      NetMessage* mrep = new NetMessage("CONFIGUREDIF",NetMessage::COMMAND_ACKNOWLEDGE,4);
      uint32_t* ipay=(uint32_t*) mrep->getPayload();
      ipay[0]=slc;
      return mrep;
    }
  return NULL;
}
void DIFServer::startServices()
{
  m_Thread_s = boost::thread(&DIFServer::services, this);  
}

void DIFServer::startReadout()
{
  if (readoutStarted_) return;
  readoutStarted_=true;	

  for (std::map<uint32_t,DIFReadout*>::iterator itd=theDIFMap_.begin();itd!=theDIFMap_.end();itd++)
    {
      //m_Thread_d[itd->first]= boost::thread(&DIFServer::readout, this,itd->first); 
      g_d.create_thread(boost::bind(&DIFServer::readout, this,itd->first));
    }
  
}

void DIFServer::readout(uint32_t difid)
{
  std::cout<<"Thread of dif "<<difid<<" is started"<<std::endl;
  std::map<uint32_t,DIFReadout*>::iterator itd=theDIFMap_.find(difid);
  if (itd==theDIFMap_.end()) return;
  unsigned char cbuf[MAX_EVENT_SIZE];

  while (readoutStarted_)
    {
      if (!running_) {usleep((uint32_t) 100000);continue;}
      usleep((uint32_t) 100);
		
		
      try 
	{
				
	  uint32_t nread=itd->second->DoHardrocV2ReadoutDigitalData(cbuf);
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

	  std::stringstream s;
	  s<<"DIF"<<itd->first;
	  NetMessage m(s.str(),NetMessage::SERVICE,nread);
	  memcpy(m.getPayload(),cbuf,nread);
	  this->updateService(s.str(),&m);
	  DEBUG(" J'update %s \n",s.str().c_str());
	
	}
      catch (LocalHardwareException e)
	{
	  std::cout<<itd->first<<" is not started "<<e.what()<<std::endl;
	}
		
    }
  std::cout<<"Thread of dif "<<difid<<" is stopped"<<readoutStarted_<<std::endl;
}


void DIFServer::joinServices()
{
  m_Thread_s.join();  
}
void DIFServer::services()
{
  unsigned char cbuf[MAX_EVENT_SIZE];
  while (true)
    {
      if (!running_) {usleep((uint32_t) 100000);continue;}
      usleep((uint32_t) 100);
		
      for (std::map<uint32_t,DIFReadout*>::iterator itd=theDIFMap_.begin();itd!=theDIFMap_.end();itd++)
	{
	  try 
	    {
				
	      uint32_t nread=itd->second->DoHardrocV2ReadoutDigitalData(cbuf);
	      if (nread==0) continue;
	      std::stringstream s;
	      s<<"DIF"<<itd->first;
	      NetMessage m(s.str(),NetMessage::SERVICE,nread);
	      memcpy(m.getPayload(),cbuf,nread);
	      this->updateService(s.str(),&m);
	      printf(" J'update %s \n",s.str().c_str());
	
	    }
	  catch (LocalHardwareException e)
	    {
	      std::cout<<itd->first<<" is not started "<<e.what()<<std::endl;
	    }
	}
		
		
		
		
		
		
    }

}
