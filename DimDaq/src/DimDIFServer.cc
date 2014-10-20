
#include "DimDIFServer.h"
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

DimDIFServer::DimDIFServer()  
{
  
  running_=false;
  readoutStarted_=false;

   std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0<<"/DDS/"<<hname<<"/STATUS";
  aliveService_ = new DimService(s0.str().c_str(),processStatus_);
  s0.str(std::string());
  memset(devicesStatus_,0,255*sizeof(int32_t));
  s0<<"/DDS/"<<hname<<"/DEVICES";
  devicesService_= new DimService(s0.str().c_str(),"I:255",devicesStatus_,255*sizeof(int32_t));
  processStatus_=DimDIFServer::ALIVED;
  aliveService_->updateService();
  allocateCommands();
  s0.str(std::string());
  gethostname(hname,80);
  s0<<"DimDIFServer-"<<hname;
  DimServer::start(s0.str().c_str()); 
  memset(infoServicesMap_,0,255*sizeof(DimService*));
  memset(dataServicesMap_,0,255*sizeof(DimService*));
  memset(theDBDimInfo_,0,255*sizeof(DimInfo*));
}

void DimDIFServer::allocateCommands()
{
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0<<"/DDS/"<<hname<<"/SCANDEVICES";
  scanCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DDS/"<<hname<<"/INITIALISE";
  initialiseCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DDS/"<<hname<<"/PRECONFIGURE";
  preconfigureCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DDS/"<<hname<<"/REGISTERSTATE";
  registerstateCommand_=new DimCommand(s0.str().c_str(),"C",this);
  s0.str(std::string());
  s0<<"/DDS/"<<hname<<"/CONFIGURECHIPS";
  configurechipsCommand_=new DimCommand(s0.str().c_str(),"I",this);

  s0.str(std::string());
  s0<<"/DDS/"<<hname<<"/START";
  startCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DDS/"<<hname<<"/STOP";
  stopCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DDS/"<<hname<<"/DESTROY";
  destroyCommand_=new DimCommand(s0.str().c_str(),"I:1",this);

  s0.str(std::string());
  s0<<"/DDS/"<<hname<<"/SETGAIN";
  gainCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DDS/"<<hname<<"/SETTHRESHOLD";
  thresholdCommand_=new DimCommand(s0.str().c_str(),"I:3",this);
}

void DimDIFServer::clearServices()
{
  for (int i=0;i<255;i++)
    {
      if (infoServicesMap_[i]!=NULL) {delete infoServicesMap_[i]; infoServicesMap_[i]=NULL;} 
      if (dataServicesMap_[i]!=NULL) {delete dataServicesMap_[i]; dataServicesMap_[i]=NULL;} 
      if (stateServicesMap_[i]!=NULL) {delete stateServicesMap_[i]; stateServicesMap_[i]=NULL;} 

    }
  memset(difStatus_,0,255*sizeof(DIFStatus));
}
void DimDIFServer::allocateServices(int32_t dif)
{

  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);

  s0.str(std::string());
  s0<<"/DDS/DIF"<<dif<<"/INFO";
  // id,Status,GTC,BCID,Bytes
  memcpy(difStatus_[dif].host,hname,80);
  infoServicesMap_[dif] = new DimService(s0.str().c_str(),"I:4;L:2;C:80",&difStatus_[dif],sizeof(DIFStatus));
  s0.str(std::string());
  s0<<"/DDS/DIF"<<dif<<"/DATA";
  // DIF buffer
  dataServicesMap_[dif] = new DimService(s0.str().c_str(),"I",&difData_[dif*32*1024],sizeof(uint32_t)*32*1024);

  s0.str(std::string());
  s0<<"/DDS/DIF"<<dif<<"/STATE";

  stateServicesMap_[dif]= new DimService(s0.str().c_str(),(char*) difState_[dif].c_str());

}


void DimDIFServer::UsbPrepare()
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
uint32_t DimDIFServer::configureChips(std::string path,uint32_t difid) throw (LocalHardwareException)
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
uint32_t DimDIFServer::configureChips(uint32_t difid,SingleHardrocV2ConfigurationFrame* slow,uint32_t nasic) throw (LocalHardwareException)
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
void DimDIFServer::preConfigure(uint32_t difid,uint32_t ctrlreg) throw (LocalHardwareException)
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
void DimDIFServer::initialise(uint32_t difid) throw (LocalHardwareException)
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
     

      std::cout<<" The DIF "<<difid<<" is initialized  -> Service "<<std::hex<<dif<<std::dec<<std::endl;

		
    }
}
std::vector<uint32_t>& DimDIFServer::scanDevices()
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



void DimDIFServer::prepareDevices()
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


void DimDIFServer::commandHandler()
{
  DimCommand *currCmd = getCommand();
  printf(" J'ai recu %s COMMAND  \n",currCmd->getName());
  if (currCmd==scanCommand_)
    {
      std::vector<uint32_t> v=scanDevices();
      memset(devicesStatus_,0,255*sizeof(int32_t));
      for (std::vector<uint32_t>::iterator i=v.begin();i!=v.end();i++)
	{
	  devicesStatus_[(*i)]=(*i);
	  uint32_t difid=(*i);
	  difStatus_[difid].id=difid;
	  difState_[difid]="FOUND";
	  this->allocateServices(difid);

	}
      devicesService_->updateService(devicesStatus_,255*sizeof(uint32_t));
      processStatus_=DimDIFServer::SCANNED;
      aliveService_->updateService();

    }
  if (currCmd==initialiseCommand_)
    {
      uint32_t difid=currCmd->getInt();
      int rc=1;
      try 
	{
	  this->initialise(difid);
	  rc=difid;
	  difState_[difid]="INITIALISED";
	}
      catch (LocalHardwareException e)
	{
	  std::cout<<e.what()<<std::endl;
	  rc=-1;
	  difStatus_[difid].status=DimDIFServer::FAILED;
	  difState_[difid]="INIT_FAILED";
	}
      difStatus_[difid].status=DimDIFServer::INITIALISED;
      cout <<" host of dif " <<difStatus_[difid].host<<" "<<sizeof(DIFStatus)<<endl;
      infoServicesMap_[difid]->updateService(&difStatus_[difid],sizeof(DIFStatus));
      stateServicesMap_[difid]->updateService((char*) difState_[difid].c_str());
      processStatus_=DimDIFServer::INITIALISED;
      aliveService_->updateService();

    }

  /*	
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
*/
    if (currCmd==registerstateCommand_)
    {

      // First allocate services
      this->registerDBService(currCmd->getString());
      aliveService_->updateService();
    }
  if (currCmd==preconfigureCommand_)
    {

      // First allocate services
      
      uint32_t ctrlreg=currCmd->getInt();
      
      int rc=1;
      for (std::map<uint32_t,DIFReadout*>::iterator itd=theDIFMap_.begin();itd!=theDIFMap_.end();itd++)
	{
	  try 
	    {
	      this->preConfigure(itd->first,ctrlreg);
	      difState_[itd->first]="PRECONFIGURED";

	    }
	  catch (LocalHardwareException e)
	    {
	      rc=-1;
	      std::cout<<itd->first<<" is not preconfigured "<<e.what()<<std::endl;
	      difState_[itd->first]="PRECONFIGURE_FAILED";
	    }

	  stateServicesMap_[itd->first]->updateService((char*) difState_[itd->first].c_str());
	  difStatus_[itd->first].slc=rc*DimDIFServer::PRECONFIGURED;
	  infoServicesMap_[itd->first]->updateService(&difStatus_[itd->first],sizeof(DIFStatus));

	  uint32_t difid=itd->first;
	  if (theDIFDbInfo_[difid].id==difid)
	    {
	      uint32_t slc=this->configureChips(difid,theDIFDbInfo_[difid].slow,theDIFDbInfo_[difid].nbasic);

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





	      difState_[itd->first]=s0.str();
	      stateServicesMap_[itd->first]->updateService((char*) difState_[itd->first].c_str());

	      difStatus_[difid].slc=slc;
	      infoServicesMap_[difid]->updateService(&difStatus_[difid],sizeof(DIFStatus));
	    }
	}

      processStatus_=DimDIFServer::PRECONFIGURED;
      aliveService_->updateService();
    }
	
  if (currCmd==destroyCommand_)
    {
      readoutStarted_=false;
      g_d.join_all();
      for (std::map<uint32_t,DIFReadout*>::iterator itd=theDIFMap_.begin();itd!=theDIFMap_.end();itd++)
	{


	  //m_Thread_d[itd->first].join();
	  if (itd->second!=NULL)
	    delete itd->second;
	  std::stringstream s;
	  s<<"DIF"<<itd->first;
	  std::cout<<s.str()<<" beiing destroyed"<<std::endl;
	  //this->destroyService(s.str());
	  difState_[itd->first]="DESTROYED";
	  stateServicesMap_[itd->first]->updateService((char*) difState_[itd->first].c_str());
			
	}
      this->clearServices();
      theDIFMap_.clear();
      processStatus_=DimDIFServer::DESTROYED;
      aliveService_->updateService();
    }
	
  if (currCmd==startCommand_)
    {
      running_=true;
      this->startReadout();
      for (std::map<uint32_t,DIFReadout*>::iterator itd=theDIFMap_.begin();itd!=theDIFMap_.end();itd++)
	{
	  try 
	    {
	      itd->second->start();
	      difState_[itd->first]="STARTED";
	    }
	  catch (LocalHardwareException e)
	    {
	      difState_[itd->first]="START_FAILED";
	      std::cout<<itd->first<<" is not started "<<e.what()<<std::endl;
	    }
	  stateServicesMap_[itd->first]->updateService((char*) difState_[itd->first].c_str());
	}

      processStatus_=DimDIFServer::RUNNING;
      aliveService_->updateService();
    }
  if (currCmd==stopCommand_)
    {
      running_=false;
      for (std::map<uint32_t,DIFReadout*>::iterator itd=theDIFMap_.begin();itd!=theDIFMap_.end();itd++)
	{
	  try 
	    {
	      itd->second->stop();
	      difState_[itd->first]="STOPPED";
	    }
	  catch (LocalHardwareException e)
	    {
	      difState_[itd->first]="STOP_FAILED";
	      std::cout<<itd->first<<" is not started "<<e.what()<<std::endl;
	    }

	  stateServicesMap_[itd->first]->updateService((char*) difState_[itd->first].c_str());

	}

      processStatus_=DimDIFServer::STOPPED;
      aliveService_->updateService();
    
    }

  if (currCmd==configurechipsCommand_)
    {
      uint32_t difid;
			memcpy(theSlowBuffer_,currCmd->getData(),currCmd->getSize());
      memcpy(&difid,theSlowBuffer_,sizeof(uint32_t));
      uint32_t nasic=(currCmd->getSize()-sizeof(uint32_t))/sizeof(SingleHardrocV2ConfigurationFrame);
      printf("I found DIF %d asics %d \n",difid,nasic);
      SingleHardrocV2ConfigurationFrame* slow =(SingleHardrocV2ConfigurationFrame*) &theSlowBuffer_[4];
      uint32_t slc=this->configureChips(difid,slow,nasic);
      difStatus_[difid].slc=slc;
      infoServicesMap_[difid]->updateService(&difStatus_[difid],sizeof(DIFStatus));
    }

  if (currCmd==gainCommand_)
    {
      uint32_t gain=currCmd->getInt();
      this->setGain(gain);
       aliveService_->updateService();
    }
  if (currCmd==thresholdCommand_)
    {
      uint32_t* thr=(uint32_t*) currCmd->getData();
      this->setThreshold(thr[0],thr[1],thr[2]);
      aliveService_->updateService();
    }
 
  return ;
}

void DimDIFServer::startReadout()
{
  if (readoutStarted_) return;
  readoutStarted_=true;	

  for (std::map<uint32_t,DIFReadout*>::iterator itd=theDIFMap_.begin();itd!=theDIFMap_.end();itd++)
    {
      //m_Thread_d[itd->first]= boost::thread(&DimDIFServer::readout, this,itd->first); 
      g_d.create_thread(boost::bind(&DimDIFServer::readout, this,itd->first));
    }
  
}

void DimDIFServer::readout(uint32_t difid)
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

	 
	  memcpy(&difData_[itd->first*32*1024],cbuf,nread);
		dataServicesMap_[itd->first]->updateService(&difData_[itd->first*32*1024],nread);
	  difStatus_[itd->first].gtc=ShmProxy::getBufferDTC(cbuf);
	  difStatus_[itd->first].bcid=ShmProxy::getBufferABCID(cbuf);
	  difStatus_[itd->first].bytes+=nread;
	  infoServicesMap_[itd->first]->updateService(&difStatus_[itd->first],sizeof(DIFStatus));

	}
      catch (LocalHardwareException e)
	{
	  std::cout<<itd->first<<" is not started "<<e.what()<<std::endl;
	}
		
    }
  std::cout<<"Thread of dif "<<difid<<" is stopped"<<readoutStarted_<<std::endl;
}

void DimDIFServer::registerDBService(const char* state)
{
  memset(theDIFDbInfo_,0,255*sizeof(DIFDbInfo));
  for (std::map<uint32_t,DIFReadout*>::iterator itd=theDIFMap_.begin();itd!=theDIFMap_.end();itd++)
    {
      if (theDBDimInfo_[itd->first]!=NULL) delete theDBDimInfo_[itd->first];
      std::stringstream s;
      s<<"/DB/"<<state<<"/DIF"<<itd->first;
      theDBDimInfo_[itd->first] = new DimInfo(s.str().c_str(),&theDIFDbInfo_[itd->first],sizeof(DIFDbInfo),this);
    }
}

void  DimDIFServer::infoHandler( ) 
{
  
    DimInfo *curr = (DimInfo*) getInfo(); // get current DimStampedInfo address
    std::cout<<curr->getName()<<std::endl;
    for (int i=0;i<255;i++)
      {
	if (curr!=theDBDimInfo_[i]) continue;
	memcpy(&theDIFDbInfo_[i],curr->getData(),sizeof(DIFDbInfo));
	printf("Dim info read %d %d \n",theDIFDbInfo_[i].id,theDIFDbInfo_[i].nbasic);
      }
}
void DimDIFServer::setThreshold(uint32_t B0,uint32_t B1,uint32_t B2,unsigned char* ConfigHR2)
{
  ConfigHR2[3]= ((B2>>2)&0xFF);
  ConfigHR2[4]= 0;
  ConfigHR2[4]|=((B2&0x03)<<6);
  ConfigHR2[4]|=((B1>>4)&0x3F);
  ConfigHR2[5]= 0;
  ConfigHR2[5]|=((B1&0x0F)<<4);
  ConfigHR2[5]|=((B0>>6)&0x0F);
  ConfigHR2[6]&=0x3;
  ConfigHR2[6]|=((B0&0x3F)<<2);
}
void DimDIFServer::setGain(uint32_t gain,unsigned char* ConfigHR2)
{
 for (uint32_t ip=0;ip<64;ip++)
    ConfigHR2[100-ip]|=(gain&0xFF);
}

void DimDIFServer::setThreshold(uint32_t B0,uint32_t B1,uint32_t B2,DIFDbInfo s)
{
  for (int i=0;i<s.nbasic;i++)
    setThreshold(B0,B1,B2,s.slow[i]);
}
void DimDIFServer::setGain(uint32_t gain,DIFDbInfo s)
{
  for (int i=0;i<s.nbasic;i++)
    setGain(gain,s.slow[i]);
}
void DimDIFServer::setThreshold(uint32_t B0,uint32_t B1,uint32_t B2)
{

 for (std::map<uint32_t,DIFReadout*>::iterator itd=theDIFMap_.begin();itd!=theDIFMap_.end();itd++)
   {
     uint32_t difid=itd->first;
     if (theDIFDbInfo_[difid].id==difid)
       {
	 setThreshold(B0,B1,B2,theDIFDbInfo_[difid]);
       }
   }



}
void DimDIFServer::setGain(uint32_t gain)
{
 for (std::map<uint32_t,DIFReadout*>::iterator itd=theDIFMap_.begin();itd!=theDIFMap_.end();itd++)
   {
     uint32_t difid=itd->first;
     if (theDIFDbInfo_[difid].id==difid)
       
       {
	 setGain(gain,theDIFDbInfo_[difid]);
       }
   }


}
