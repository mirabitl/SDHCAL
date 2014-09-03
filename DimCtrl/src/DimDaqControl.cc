
#include "DimDaqControl.h"
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include "ShmProxy.h"
#include <string.h>



DimDaqControl::DimDaqControl(std::string dns) :theDns_(dns)  
{
  
  running_=false;
  readoutStarted_=false;
  
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0<<"/DDC/"<<hname<<"/STATUS";
  aliveService_ = new DimService(s0.str().c_str(),processStatus_);
  s0.str(std::string());
  processStatus_=DimDaqControl::ALIVED;
  aliveService_->updateService();
  allocateCommands();
  s0.str(std::string());
  s0<<"DimDaqControl-"<<hname;
  DimServer::start(); 

}
 DimDaqControl::~DimDaqControl()
 {
   delete aliveService_;
 }
void DimDaqControl::allocateCommands()
{
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0<<"/DDC/"<<hname<<"/SCANDNS";
  scanCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DDC/"<<hname<<"/INITIALISE";
  initialiseCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DDC/"<<hname<<"/CONFIGURE";
  configureCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DDC/"<<hname<<"/REGISTERSTATE";
  registerstateCommand_=new DimCommand(s0.str().c_str(),"I:1,C",this);
  s0.str(std::string());
  s0<<"/DDC/"<<hname<<"/START";
  startCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DDC/"<<hname<<"/STOP";
  stopCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DDC/"<<hname<<"/DESTROY";
  destroyCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  
}

void DimDaqControl::scandns()
{
  // Look for DB server
   DimBrowser dbr; 
  char *service, *format; 
  int type;
  // Get DB service
  dbr.getServices("/DB/*/DOWNLOAD" ); 
  while(type = dbr.getNextService(service, format)) 
    { 
      cout << service << " -  " << format << endl; 
      std::string ss;
      ss.assign(service);
      size_t n=ss.find("/DOWNLOAD");
      cout<<ss.substr(0,n)<<endl;
      theDBPrefix_=ss.substr(0,n);
    } 
  // Get the CCC prefix
  dbr.getServices("/DCS/*/STATUS" ); 
  while(type = dbr.getNextService(service, format)) 
    { 
      cout << service << " -  " << format << endl; 
      std::string ss;
      ss.assign(service);
      size_t n=ss.find("/STATUS");
      theCCCPrefix_=ss.substr(0,n);
    } 

  char *server,*node;
  dbr.getServers( ); 
  while(dbr.getNextServer(server, node)) 
    { 
      cout << server << " @ " << node << endl; 
      


      if (strncmp(server,"DimDIFServer",12)!=0) continue;
      std::string ss;
      ss.assign(node);
      size_t n=ss.find(".");
      std::string toto;
      toto=ss.substr(0,n);

      std::stringstream s0;
      s0<<"/DDS/"<<toto;
      DimDDSClient* d=new DimDDSClient(toto,s0.str());
      std::pair<std::string,DimDDSClient*> p(toto,d);
      theDDSCMap_.insert(p);
      
    }
      


}

void DimDaqControl::commandHandler()
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
	}
      devicesService_->updateService(devicesStatus_,255*sizeof(uint32_t));
      processStatus_=DimDaqControl::SCANNED;
      aliveService_->updateService();
    
    }
  if (currCmd==initialiseCommand_)
    {
      uint32_t difid=currCmd->getInt();
      difStatus_[difid].id=difid;
      this->allocateServices(difid);
      int rc=1;
      try 
	{
	  this->initialise(difid);
	  rc=difid;
	}
      catch (LocalHardwareException e)
	{
	  std::cout<<e.what()<<std::endl;
	  rc=-1;
	  difStatus_[difid].status=DimDaqControl::FAILED;
	}
      difStatus_[difid].status=DimDaqControl::INITIALISED;
      infoServicesMap_[difid]->updateService(&difStatus_[difid],sizeof(DIFStatus));
      processStatus_=DimDaqControl::INITIALISED;
      aliveService_->updateService();
    
    }
  
  /*	
   *  if (m->getName().compare("SETDIFPARAM")==0)
   *    {
   *      uint32_t *ipay=(uint32_t*) m->getPayload();
   *      //uint32_t difid=0,nbasic=0,asictype=0,ctrlreg=0;
   *      //memcpy(&difid,(m->getPayload())[0],sizeof(uint32_t));
   *      //memcpy(&nbasic,(m->getPayload())[4],sizeof(uint32_t));
   *      //memcpy(&asictype,(m->getPayload())[8],sizeof(uint32_t));
   *      //memcpy(&ctrlreg,(m->getPayload())[8],sizeof(uint32_t));
   *      std::map<uint32_t,DIFReadout*>::iterator itd=theDIFMap_.find(ipay[0]);
   *      if (itd!=theDIFMap_.end())
   *	{
   *	  itd->second->setNumberOfAsics(ipay[1]);
   *	  itd->second->setAsicType(ipay[2]);
   *	  itd->second->setControlRegister(ipay[3]);
   }	
   
   NetMessage* mrep = new NetMessage("SETDIFPARAM",NetMessage::COMMAND_ACKNOWLEDGE,4);
   std::cout << "Answer prepared"<<std::endl;
   return mrep;
   }
  */
  if (currCmd==registerstateCommand_)
    {
    
      // First allocate services
      this->registerstate(theCtrlReg_,theState_);
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
	    }
	  catch (LocalHardwareException e)
	    {
	      rc=-1;
	      std::cout<<itd->first<<" is not preconfigured "<<e.what()<<std::endl;
	    }
      
      
	  difStatus_[itd->first].slc=rc*DimDaqControl::PRECONFIGURED;
	  infoServicesMap_[itd->first]->updateService(&difStatus_[itd->first],sizeof(DIFStatus));
      
	  uint32_t difid=itd->first;
	  if (theDIFDbInfo_[difid].id==difid)
	    {
	      uint32_t slc=this->configureChips(difid,theDIFDbInfo_[difid].slow,theDIFDbInfo_[difid].nbasic);
	      difStatus_[difid].slc=slc;
	      infoServicesMap_[difid]->updateService(&difStatus_[difid],sizeof(DIFStatus));
	    }
	}
    
      processStatus_=DimDaqControl::PRECONFIGURED;
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
      
	}
      this->clearServices();
      theDIFMap_.clear();
      processStatus_=DimDaqControl::DESTROYED;
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
	    }
	  catch (LocalHardwareException e)
	    {
	      std::cout<<itd->first<<" is not started "<<e.what()<<std::endl;
	    }
	}
    
      processStatus_=DimDaqControl::RUNNING;
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
	    }
	  catch (LocalHardwareException e)
	    {
	      std::cout<<itd->first<<" is not started "<<e.what()<<std::endl;
	    }
	}
    
      processStatus_=DimDaqControl::STOPPED;
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
  
  return ;
}
   
void DimDaqControl::startReadout()
{
  if (readoutStarted_) return;
  readoutStarted_=true;	
     
  for (std::map<uint32_t,DIFReadout*>::iterator itd=theDIFMap_.begin();itd!=theDIFMap_.end();itd++)
    {
      //m_Thread_d[itd->first]= boost::thread(&DimDaqControl::readout, this,itd->first); 
      g_d.create_thread(boost::bind(&DimDaqControl::readout, this,itd->first));
    }
     
}
   
void DimDaqControl::readout(uint32_t difid)
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
   
void DimDaqControl::registerDBService(const char* state)
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
   
void  DimDaqControl::infoHandler( ) 
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
   
