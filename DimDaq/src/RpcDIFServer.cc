
#include "RpcDIFServer.h"
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
#include <string.h>

using namespace Ftdi;
RpcDIFRegisterDB::RpcDIFRegisterDB(RpcDIFServer* r,std::string name) : DimRpc(name.c_str(),"C","I:1"),_server(r) {}

void RpcDIFRegisterDB::rpcHandler()
{
  LOG4CXX_INFO(_logDDIF," CMD: REGISTERDB called"<<getData()<<" "<<getSize());
  char name[80];
  memset(name,0,80);
  memcpy(name,getData(),getSize());
   int32_t rc=0;
   std::string dbsta;dbsta.assign(name);
   _server->registerDB(dbsta);
   setData(rc);
}


RpcDIFScan::RpcDIFScan(RpcDIFServer* r,std::string name) : DimRpc(name.c_str(),"I:1","I"),_server(r) {}

void RpcDIFScan::rpcHandler()
{
   LOG4CXX_INFO(_logDDIF," CMD: SCANDEVICE called");
   _server->prepareDevices();
   std::map<uint32_t,FtdiDeviceInfo*>& fm=_server->getFtdiMap();
   std::map<uint32_t,DimDIF*> dm=_server->getDIFMap();
   LOG4CXX_INFO(_logDDIF," CMD: SCANDEVICE clear Maps");
   for ( std::map<uint32_t,DimDIF*>::iterator it=dm.begin();it!=dm.end();it++)
     { if (it->second!=NULL) delete it->second;}
   dm.clear();
   _ndif=0;
   for ( std::map<uint32_t,FtdiDeviceInfo*>::iterator it=fm.begin();it!=fm.end();it++)
     {
       _buf[_ndif]=it->first;
       _ndif++;

       DimDIF* d= new DimDIF(it->second);
       _server->getDIFMap().insert(std::make_pair(it->first,d));
       LOG4CXX_INFO(_logDDIF," CMD: SCANDEVICE created DimDIF @ "<<std::hex<<d<<std::dec);
     }
   setData(_buf,_ndif*sizeof(int32_t));
}

RpcDIFInitialise::RpcDIFInitialise(RpcDIFServer* r,std::string name) : DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}

void RpcDIFInitialise::rpcHandler()
{
  uint32_t difid=getInt();
  int32_t rc=1;
  std::map<uint32_t,DimDIF*> dm=_server->getDIFMap();
  if (difid>0 )
    {
      std::map<uint32_t,DimDIF*>::iterator itd=dm.find(difid);
      if (itd==dm.end())
	{
	  LOG4CXX_ERROR(_logDDIF," please do Scan devices first the dif  "<<difid<<"is not registered");
	  rc=-1;
	  rc=-1;setData(rc);
	  return;
	}
      
      itd->second->initialise();
      setData(rc);
      return;
    }
  else
    {

      for ( std::map<uint32_t,DimDIF*>::iterator it=dm.begin();it!=dm.end();it++)
	{
	  LOG4CXX_INFO(_logDDIF," calling initialise DimDIF @ "<<std::hex<<it->second<<std::dec);
	  it->second->initialise();
	}
      setData(rc);
      return;
    }
}

RpcDIFConfigure::RpcDIFConfigure(RpcDIFServer* r,std::string name) : DimRpc(name.c_str(),"I:2","I:1"),_server(r) {}
void RpcDIFConfigure::rpcHandler()
{
  uint32_t *data=(uint32_t*) getData();
  uint32_t difid=data[0];
  uint32_t ctrlreg=data[1];
  int32_t rc=1;
   std::map<uint32_t,DimDIF*> dm=_server->getDIFMap();
  if (difid>0 )
    {
      std::map<uint32_t,DimDIF*>::iterator itd=dm.find(difid);
      if (itd==dm.end())
	{
	  LOG4CXX_ERROR(_logDDIF," please do Scan devices first the dif  "<<difid<<"is not registered");
	  rc=-1;setData(rc);
	  return;
	}
      
      itd->second->configure(ctrlreg);
      rc=itd->second->status()->slc;setData(rc);
      return;
    }
  else
    {

      for ( std::map<uint32_t,DimDIF*>::iterator it=dm.begin();it!=dm.end();it++)
	it->second->configure(ctrlreg);
      setData(rc);
      return;
    }
}

RpcDIFStart::RpcDIFStart(RpcDIFServer* r,std::string name) : DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}
void RpcDIFStart::rpcHandler()
{

  uint32_t difid=getInt();
  int32_t rc=1;
  std::map<uint32_t,DimDIF*> dm=_server->getDIFMap();
  if (difid>0 )
    {
      std::map<uint32_t,DimDIF*>::iterator itd=dm.find(difid);
      if (itd==dm.end())
	{
	  LOG4CXX_ERROR(_logDDIF," please do Scan devices first the dif  "<<difid<<"is not registered");
	  rc=-1;setData(rc);
	  return;
	}
      _server->startDIFThread(itd->second);
      itd->second->start();
      rc=itd->second->status()->slc;setData(rc);
      return;
    }
  else
    {

      for ( std::map<uint32_t,DimDIF*>::iterator it=dm.begin();it!=dm.end();it++)
	{
	  _server->startDIFThread(it->second);
	  it->second->start();
	}
      setData(rc);
      return;
    }
}
RpcDIFStop::RpcDIFStop(RpcDIFServer* r,std::string name) : DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}
void RpcDIFStop::rpcHandler()
{

  uint32_t difid=getInt();
  int32_t rc=1;
  std::map<uint32_t,DimDIF*> dm=_server->getDIFMap();
  if (difid>0 )
    {
      std::map<uint32_t,DimDIF*>::iterator itd=dm.find(difid);
      if (itd==dm.end())
	{
	  LOG4CXX_ERROR(_logDDIF," please do Scan devices first the dif  "<<difid<<"is not registered");
	  rc=-1;setData(rc);
	  return;
	}

      itd->second->stop();
      rc=itd->second->status()->slc;setData(rc);
      return;
    }
  else
    {

      for ( std::map<uint32_t,DimDIF*>::iterator it=dm.begin();it!=dm.end();it++)
	{
	  it->second->stop();
	}
      setData(rc);
      return;
    }
}
RpcDIFDestroy::RpcDIFDestroy(RpcDIFServer* r,std::string name) : DimRpc(name.c_str(),"I:1","I:1"),_server(r) {}
void RpcDIFDestroy::rpcHandler()
{

  uint32_t difid=getInt();
  int32_t rc=1;
  std::map<uint32_t,DimDIF*> dm=_server->getDIFMap();
  if (difid>0 )
    {
      std::map<uint32_t,DimDIF*>::iterator itd=dm.find(difid);
      if (itd==dm.end())
	{
	  LOG4CXX_ERROR(_logDDIF," please do Scan devices first the dif  "<<difid<<"is not registered");
	  rc=-1;setData(rc);
	  return;
	}

      itd->second->destroy();
      rc=itd->second->status()->slc;setData(rc);
      return;
    }
  else
    {
      bool running=false;
      for ( std::map<uint32_t,DimDIF*>::iterator it=dm.begin();it!=dm.end();it++)
	{
	  running=running ||it->second->readoutStarted();
	}
      if (running) _server->joinThreads();
      for ( std::map<uint32_t,DimDIF*>::iterator it=dm.begin();it!=dm.end();it++)
	{
	  it->second->destroy();
	}


      setData(rc);
      return;
    }
}






RpcDIFServer::RpcDIFServer()  : _state("CREATED")
{
  
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0<<"/DDS/"<<hname<<"/STATE";
  _dsState = new DimService(s0.str().c_str(),(char*) _state.c_str());
  this->publishState("CREATED");
  allocateCommands();
  s0.str(std::string());
  gethostname(hname,80);
  s0<<"RpcDIFServer-"<<hname;
  DimServer::start(s0.str().c_str()); 
  
  memset(theDBDimInfo_,0,255*sizeof(DimInfo*));
}

void RpcDIFServer::allocateCommands()
{
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0<<"/DDS/"<<hname<<"/SCANDEVICES";
  scanCommand_=new RpcDIFScan(this,s0.str());
  s0.str(std::string());
  s0<<"/DDS/"<<hname<<"/INITIALISE";
  initialiseCommand_=new RpcDIFInitialise(this,s0.str());
  s0.str(std::string());
  s0<<"/DDS/"<<hname<<"/CONFIGURE";
  configureCommand_=new RpcDIFConfigure(this,s0.str());
  s0.str(std::string());
  s0<<"/DDS/"<<hname<<"/REGISTERDB";
  registerdbCommand_=new RpcDIFRegisterDB(this,s0.str().c_str());
  s0.str(std::string());
  s0<<"/DDS/"<<hname<<"/START";
  startCommand_=new RpcDIFStart(this,s0.str());
  s0.str(std::string());
  s0<<"/DDS/"<<hname<<"/STOP";
  stopCommand_=new RpcDIFStop(this,s0.str());
  s0.str(std::string());
  s0<<"/DDS/"<<hname<<"/DESTROY";
  destroyCommand_=new RpcDIFDestroy(this,s0.str());
  /*
  s0.str(std::string());
  s0<<"/DDS/"<<hname<<"/SETGAIN";
  gainCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DDS/"<<hname<<"/SETTHRESHOLD";
  thresholdCommand_=new DimCommand(s0.str().c_str(),"I:3",this);
  s0.str(std::string());
  s0<<"/DDS/"<<hname<<"/LOOPCONFIGURE";
  loopConfigureCommand_=new DimCommand(s0.str().c_str(),"I:2",this);
  s0.str(std::string());
  s0<<"/DDS/"<<hname<<"/REGISTERREAD";
  registerreadCommand_=new DimCommand(s0.str().c_str(),"I:2",this);
  s0.str(std::string());
  s0<<"/DDS/"<<hname<<"/REGISTERWRITE";
  registerwriteCommand_=new DimCommand(s0.str().c_str(),"I:3",this);
  */
}






void RpcDIFServer::prepareDevices()
{
  for ( std::map<uint32_t,FtdiDeviceInfo*>::iterator it=theFtdiDeviceInfoMap_.begin();it!=theFtdiDeviceInfoMap_.end();it++)
    if (it->second!=NULL) delete it->second;
  theFtdiDeviceInfoMap_.clear();
  for ( std::map<uint32_t,DimDIF*>::iterator it=theDIFMap_.begin();it!=theDIFMap_.end();it++)
    if (it->second!=NULL) delete it->second;
  theDIFMap_.clear();
  system("/opt/dhcal/bin/ListDevices.py");
  std::string line;
  std::ifstream myfile ("/var/log/pi/ftdi_devices");
  std::stringstream diflist;



  if (myfile.is_open())
    {
      while ( myfile.good() )
	{
	  getline (myfile,line);
	  FtdiDeviceInfo* difi=new FtdiDeviceInfo();
	  memset(difi,0,sizeof(FtdiDeviceInfo));
	  sscanf(line.c_str(),"%x %x %s",&difi->vendorid,&difi->productid,difi->name);
	  if (strncmp(difi->name,"FT101",5)==0)
	    {
	      sscanf(difi->name,"FT101%d",&difi->id); 
	      difi->type=0;
	      std::pair<uint32_t,FtdiDeviceInfo*> p(difi->id,difi);
	      theFtdiDeviceInfoMap_.insert(p);
	    }
	  if (strncmp(difi->name,"DCCCCC",6)==0)
	    {sscanf(difi->name,"DCCCCC%d",&difi->id);difi->type=0x10;}


	}
      myfile.close();
    }
  else 
    {
      std::cout << "Unable to open file"<<std::endl; 
      LOG4CXX_FATAL(_logDDIF," Unable to open /var/log/pi/ftdi_devices");
    }

  for (std::map<uint32_t,FtdiDeviceInfo*>::iterator it=theFtdiDeviceInfoMap_.begin();it!=theFtdiDeviceInfoMap_.end();it++)
    printf("Device found and register: %d with info %d %d %s type %d \n", it->first,it->second->vendorid,it->second->productid,it->second->name,it->second->type);
}


void RpcDIFServer::commandHandler()
{
  DimCommand *currCmd = getCommand();
  //printf(" J'ai recu %s COMMAND  \n",currCmd->getName());
  LOG4CXX_INFO(_logDDIF," CMD: "<<currCmd->getName());
  /*
  if (currCmd==registerwriteCommand_)
    {
      uint32_t* dd=(uint32_t*) currCmd->getData();
      uint32_t difid=dd[0];
      uint32_t adr=dd[1];
      uint32_t reg=dd[2];
      for (std::map<uint32_t,DIFReadout*>::iterator itd=theDIFMap_.begin();itd!=theDIFMap_.end();itd++)
	{
	  if (itd->first!=difid) continue;
	  try 
	    {
	      itd->second->UsbRegisterWrite(adr,reg);
	    }
	  catch (LocalHardwareException e)
	    {
	
	      LOG4CXX_ERROR(_logDDIF,"Cannot write register "<<itd->first<<" ["<<std::hex<<adr<<"]<-"<<reg<<std::dec);
	    }
	

	}

    }

  if (currCmd==registerreadCommand_)
    {
      
      uint32_t* dd=(uint32_t*) currCmd->getData();
      uint32_t difid=dd[0];
      uint32_t adr=dd[1];
      uint32_t reg;
      for (std::map<uint32_t,DIFReadout*>::iterator itd=theDIFMap_.begin();itd!=theDIFMap_.end();itd++)
	{
	  if (itd->first!=difid) continue;
	  LOG4CXX_INFO(_logDDIF,"Reading "<<itd->first<<" ["<<std::hex<<adr<<"]<-");
	  try 
	    {
	      itd->second->UsbRegisterRead(adr,&reg);
	    }
	  catch (LocalHardwareException e)
	    {
	
	      LOG4CXX_ERROR(_logDDIF,"Cannot read register "<<itd->first<<" ["<<std::hex<<adr<<"]<-"<<reg<<std::dec);
	    }
	  LOG4CXX_INFO(_logDDIF,"Got "<<itd->first<<" ["<<std::hex<<adr<<"]<-"<<reg<<std::dec);

	}
      register_=reg;
      registerService_->updateService();
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
  */ 
  return ;
}


void RpcDIFServer::startDIFThread(DimDIF* d)
{
  if (d->readoutStarted()) return;
  d->setReadoutStarted(true);	

  g_d.create_thread(boost::bind(&DimDIF::readout,d));
  
}


void RpcDIFServer::registerDB(std::string state)
{

  for (std::map<uint32_t,DimDIF*>::iterator itd=theDIFMap_.begin();itd!=theDIFMap_.end();itd++)
    {
      if (theDBDimInfo_[itd->first]!=NULL) delete theDBDimInfo_[itd->first];
      std::stringstream s;
      s<<"/DB/"<<state<<"/DIF"<<itd->first;
      theDBDimInfo_[itd->first] = new DimInfo(s.str().c_str(),itd->second->dbdif(),sizeof(DIFDbInfo),this);

      itd->second->publishState("DB_REGISTERED");


    }
}

void  RpcDIFServer::infoHandler( ) 
{
  
  DimInfo *curr = (DimInfo*) getInfo(); // get current DimStampedInfo address
  std::cout<<curr->getName()<<std::endl;
  for (int i=0;i<255;i++)
    {
      if (curr!=theDBDimInfo_[i]) continue;
      if (theDIFMap_.find(i)== theDIFMap_.end()) continue;
      //memcpy(&theDIFDbInfo_[i],curr->getData(),sizeof(DIFDbInfo));
      memcpy(theDIFMap_[i]->dbdif(),curr->getData(),curr->getSize());
      //printf("Dim info read %d %d \n",theDIFMap_[i]->dbdif()->id,theDIFMap_[i]->dbdif()->nbasic);
      LOG4CXX_INFO(_logDDIF,"DIF "<<theDIFMap_[i]->dbdif()->id<<" is read from DB with nasic="<<theDIFMap_[i]->dbdif()->nbasic);
    }
}
/*
void RpcDIFServer::setThreshold(uint32_t B0,uint32_t B1,uint32_t B2,SingleHardrocV2ConfigurationFrame& ConfigHR2)
{
  printf(" Seuil %d %d %d %x %x %x %x %x\n",B0,B1,B2,ConfigHR2[3],ConfigHR2[4],ConfigHR2[5],ConfigHR2[6],ConfigHR2[7]);
  ConfigHR2[3]= ((B2>>2)&0xFF);
  ConfigHR2[4]= 0;
  ConfigHR2[4]|=((B2&0x03)<<6);
  ConfigHR2[4]|=((B1>>4)&0x3F);
  ConfigHR2[5]= 0;
  ConfigHR2[5]|=((B1&0x0F)<<4);
  ConfigHR2[5]|=((B0>>6)&0x0F);
  ConfigHR2[6]&=0x3;
  ConfigHR2[6]|=((B0&0x3F)<<2);
  printf(" Apres %d %d %d %x %x %x %x %x \n",B0,B1,B2,ConfigHR2[3],ConfigHR2[4],ConfigHR2[5],ConfigHR2[6],ConfigHR2[7]);
}
void RpcDIFServer::setGain(uint32_t gain,SingleHardrocV2ConfigurationFrame& ConfigHR2)
{
 
  for (uint32_t ip=0;ip<64;ip++)
    ConfigHR2[100-ip]=(gain&0xFF); // Pas |=
}

void RpcDIFServer::setThreshold(uint32_t B0,uint32_t B1,uint32_t B2,DIFDbInfo& s)
{
  printf(" DIF %d \n",s.id);
  for (int i=0;i<s.nbasic;i++)
    {
      printf("ASIC %d \n",i);
      setThreshold(B0,B1,B2,s.slow[i]);
    }
}
void RpcDIFServer::setGain(uint32_t gain,DIFDbInfo& s)
{
  for (int i=0;i<s.nbasic;i++)
    setGain(gain,s.slow[i]);
}
void RpcDIFServer::setThreshold(uint32_t B0,uint32_t B1,uint32_t B2)
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
void RpcDIFServer::setGain(uint32_t gain)
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
*/
