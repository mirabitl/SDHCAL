#include "LDIF.hh"
#include "ShmProxy.h"
LDIF::LDIF(FtdiDeviceInfo* ftd) : _rd(NULL),_state("CREATED"),_dsStatus(NULL),_dsState(NULL),_dsData(NULL)
{
  // Creation of data structure
  memcpy(&_ftd,ftd,sizeof(FtdiDeviceInfo));
  _status = new DIFStatus();
  memset(_status,0,sizeof(DIFStatus));
  _status->id=_ftd.id;
  _dbdif = new DIFDbInfo();
  _readoutStarted=false;
  _readoutCompleted=true;
}
LDIF::~LDIF()
{
  delete _status;
  delete _dbdif;
  if (_dsStatus!=NULL)
    {
      delete _dsStatus;
      delete _dsData;
      delete _dsState;
      _dsStatus=NULL;
      _dsData=NULL;
      _dsState=NULL;

    }
  if (_rd!=NULL)
    {
      delete _rd;
      _rd=NULL;
    }
}
void LDIF::writeRegister(uint32_t adr,uint32_t reg)
{
  LOG4CXX_INFO(_logLdaq,"Writing "<<_status->id<<" ["<<std::hex<<adr<<"]<-"<<reg);
  try 
    {
      _rd->UsbRegisterWrite(adr,reg);
    }
  catch (LocalHardwareException e)
    {
	
      LOG4CXX_ERROR(_logLdaq,"Cannot write register "<<_status->id<<" ["<<std::hex<<adr<<"]<-"<<reg<<std::dec);
    }
  LOG4CXX_INFO(_logLdaq,"Wrote "<<_status->id<<" ["<<std::hex<<adr<<"]<-"<<reg<<std::dec);
}
void LDIF::readRegister(uint32_t adr,uint32_t &reg)
{

  LOG4CXX_INFO(_logLdaq,"Reading "<<_status->id<<" ["<<std::hex<<adr<<"]<-");
  try 
    {
      _rd->UsbRegisterRead(adr,&reg);
    }
  catch (LocalHardwareException e)
    {
	
      LOG4CXX_ERROR(_logLdaq,"Cannot read register "<<_status->id<<" ["<<std::hex<<adr<<"]<-"<<reg<<std::dec);
    }
  LOG4CXX_INFO(_logLdaq,"Got "<<_status->id<<" ["<<std::hex<<adr<<"]<-"<<reg<<std::dec);

}
void LDIF::start()
{

  if (_rd==NULL)
    {
      LOG4CXX_ERROR(_logLdaq, "DIF   id ("<<_status->id << ") is not initialised");
      this->publishState("START_FAILED");
      return;
    }
  try 
    {
      _rd->start();
      this->publishState("STARTED");
      LOG4CXX_INFO(_logLdaq,"DIF "<<_status->id<<" is started");
      _status->bytes=0;
      _running=true;
      
    }
  catch (LocalHardwareException e)
    {
      LOG4CXX_ERROR(_logLdaq,"Start failed "<<_status->id);
      this->publishState("START_FAILED");
    }
  
}
void LDIF::readout()
{
  LOG4CXX_INFO(_logLdaq,"Thread of dif "<<_status->id<<" is started");
  if (_rd==NULL)
    {
      LOG4CXX_ERROR(_logLdaq, "DIF   id ("<<_status->id << ") is not initialised");
      this->publishState("READOUT_FAILED");
      _readoutStarted=false;
      return;
    }


  unsigned char cbuf[MAX_EVENT_SIZE];
  _readoutCompleted=false;
  while (_readoutStarted)
    {
      if (!_running) {usleep((uint32_t) 100000);continue;}
      usleep((uint32_t) 100);
		
		
      try 
	{
				
	  uint32_t nread=_rd->DoHardrocV2ReadoutDigitalData((unsigned char*) _dsData->payload());
	  //printf(" Je lis %d %d \n",difid,nread);
	  if (nread==0) continue;
	 
	  //memcpy(_data,cbuf,nread);
	  //this->publishData(nread);
	 
	  _status->gtc=ShmProxy::getBufferDTC(cbuf);
	  _status->bcid=ShmProxy::getBufferABCID(cbuf);
	  _status->bytes+=nread;
	  _dsData->publish(_status->gtc,_status->bcid,nread);
	  this->publishStatus();


	}
      catch (LocalHardwareException e)
	{
	  LOG4CXX_ERROR(_logLdaq,"DIF "<<_status->id<<" cannot read events"<<e.what() );

	}
		
    }
  _readoutCompleted=true;
  LOG4CXX_INFO(_logLdaq,"Thread of dif "<<_status->id<<" is stopped"<<_readoutStarted);
  _status->status=0XFFFF;
}
void LDIF::stop()
{
  _running=false;
  if (_rd==NULL)
    {
      LOG4CXX_ERROR(_logLdaq, "DIF   id ("<<_status->id << ") is not initialised");
      this->publishState("STOP_FAILED");
      return;
    }
   try 
     {
       _rd->stop();
       this->publishState("STOPPED");
       this->publishStatus();
     }
   catch (LocalHardwareException e)
     {
       this->publishState("STOP_FAILED");
       
       LOG4CXX_ERROR(_logLdaq,"Stop failed "<<_status->id);
     }
}
void LDIF::destroy()
{
  if (_readoutStarted)
    {
      _readoutStarted=false;
      uint32_t ntry=0;
      while (!_readoutCompleted && ntry<100)
	{usleep((uint32_t) 200000);ntry++;}
    }
  if (_rd!=NULL)
    {
      try
	{
	  delete _rd;
	  _rd=NULL;
	}
      catch (LocalHardwareException& e)
	{
	  LOG4CXX_FATAL(_logLdaq,"Destroy failed for "<<_status->id<<" "<<e.what());

	  this->publishState("DESTROY_FAILED");
	  
	}
      this->publishState("CREATED");
    }
  if (_dsStatus!=NULL)
    {
      LOG4CXX_INFO(_logLdaq," Deleting dim services ");
      delete _dsStatus;
      delete _dsData;
      delete _dsState;
      _dsStatus=NULL;
      _dsData=NULL;
      _dsState=NULL;
      
    }

}
void LDIF::difConfigure(uint32_t ctrlreg)
{
  if (_rd==NULL)
    {
      LOG4CXX_ERROR(_logLdaq, "DIF   id ("<<_status->id << ") is not initialised");
      this->publishState("DIF_CONFIGURE_FAILED");
      return;
    }
  
  _rd->setPowerManagment(0x8c52, 0x3e6,0xd640,0x4e,0x4e);// Start decale de 36000 clock (8b68 a la place de 43 ECAL needs)
  _rd->setControlRegister(ctrlreg);
  try
    {
      _rd->configureRegisters();
    }
  catch(...)
    {
      LOG4CXX_ERROR(_logLdaq, "DIF   id ("<<_status->id << ") cannot write registers");
      this->publishState("DIF_CONFIGURE_FAILED");
      return;
    }
  this->publishState("DIF_CONFIGURED");
  LOG4CXX_INFO(_logLdaq, "DIF   id ("<<_status->id << ") has writen registers");

}
void LDIF::chipConfigure()
{
  if (_rd==NULL)
    {
      LOG4CXX_ERROR(_logLdaq, "DIF   id ("<<_status->id << ") is not initialised");
      this->publishState("CHIP_CONFIGURE_FAILED");
      return;
    }
  if (_dbdif->id !=_status->id)
    {
      LOG4CXX_ERROR(_logLdaq, "DB info for DIF   id ("<<_status->id << ") is not available");
      this->publishState("CHIP_CONFIGURE_FAILED");
      return;
    }
  try
    {
      if (_dbdif->nbasic!=48)
	{
	  _rd->setNumberOfAsics(_dbdif->nbasic);
	  _rd->configureRegisters();
	}
      _status->slc=_rd->configureChips(_dbdif->slow);
      this->publishStatus();
    }
  catch(...)
    {
       LOG4CXX_ERROR(_logLdaq, "DB info for DIF   id ("<<_status->id << ") cannot configure chips");
      this->publishState("CHIP_CONFIGURE_FAILED");
      return;
    }
  this->publishState("CHIP_CONFIGURED");
  LOG4CXX_INFO(_logLdaq, "DIF   id ("<<_status->id << ") has programmed ASICs");
		

}
void LDIF::configure(uint32_t ctrlreg)
{
  this->difConfigure(ctrlreg);
  if (_state.compare("DIF_CONFIGURED")!=0)
    {
      _status->slc=0;
      return;
    }
  this->chipConfigure();
  if (_state.compare("CHIP_CONFIGURED")!=0)
    {
      return;
    }
  bool bad=false;
  std::stringstream s0;
  s0.str(std::string());
  s0<<"CONFIGURED => ";
  if ((_status->slc&0x0003)==0x01) s0<<"SLC CRC OK       - ";
  else
    { 
      if ((_status->slc&0x0003)==0x02) 
	s0<<"SLC CRC Failed   - ";
      else 
	s0<<"SLC CRC forb  - ";
      bad=true;
    }
  if ((_status->slc&0x000C)==0x04) s0<<"All OK      - ";
  else 
    {
      if ((_status->slc&0x000C)==0x08) 
	s0<<"All Failed  - ";
      else  
	s0<<"All forb - ";
      bad=true;
    }
  if ((_status->slc&0x0030)==0x10) s0<<"L1 OK     - ";
  else 
    {
      if ((_status->slc&0x0030)==0x20) s0<<"L1 Failed - ";
      else s0<<"L1 forb   - ";
      bad=true;
    }
  //std::cout<<s0.str()<<std::endl;
  if (bad)
    LOG4CXX_ERROR(_logLdaq,"Configure failed on "<<_status->id<<s0.str()<<" SLC="<<_status->slc);

  this->publishState(s0.str());
  LOG4CXX_INFO(_logLdaq, "DIF   id ("<<_status->id << ") ="<<s0.str());
}
void LDIF::initialise()
{
  uint32_t difid=_ftd.id;
  //  create services
  
  this->registration();

  try
    {
      std::string s(_ftd.name);
      _rd = new DIFReadout(s,_ftd.productid);
      
      }
  catch (...)
    {
      if (_rd!=NULL)
	{
	  delete _rd;
	  _rd=NULL;
	}
      LOG4CXX_FATAL(_logLdaq,"cannot create DIFReadout for  "<<difid);
      this->publishState("INIT_RD_FAILED");
      return;
    }
  try
    {
      _rd->checkReadWrite(0x1234,100);
    }
  catch (LocalHardwareException& e)
    {
      LOG4CXX_FATAL(_logLdaq," Unable to read USB register (check clock) "<<e.message());
      this->publishState("INIT_FAILED");
      return;
    }
  try
    {
      _rd->checkReadWrite(0x1234,100);
    }
  catch (LocalHardwareException& e)
    {
      LOG4CXX_FATAL(_logLdaq," Second check read write failed "<<e.message());
      this->publishState("INIT_FAILED");
      return;
    }
  this->publishState("INITIALISED");
}
 
void LDIF::registration()
{
  // Creation of status, data and state services
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  if (_dsStatus!=NULL)
    {
      LOG4CXX_INFO(_logLdaq," Deleting dim services ");
      delete _dsStatus;
      delete _dsData;
      delete _dsState;
      _dsStatus=NULL;
      _dsData=NULL;
      _dsState=NULL;
      
    }
  LOG4CXX_INFO(_logLdaq," creating dim services ");

  // Services
  s0.str(std::string());
  s0<<"/DDS/DIF"<<_status->id<<"/INFO";
  _dsStatus = new DimService(s0.str().c_str(),"I:4;L:2;C:80",_status,sizeof(DIFStatus));

  s0.str(std::string());
  s0<<"/DDS/DIF"<<_status->id<<"/STATE";
  _dsState=new DimService(s0.str().c_str(),(char*) _state.c_str());

  _dsData = new levbdim::datasource(100,_status->id,MAX_EVENT_SIZE);
}


