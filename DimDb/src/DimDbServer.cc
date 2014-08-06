
#include "DimDbServer.h"

DimDbServer::DimDbServer()
{

  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);

  s0<<"/DB/"<<hname<<"/STATUS";
  aliveService_ = new DimService(s0.str().c_str(),processStatus_);
  s0.str(std::string());
  processStatus_=DimDbServer::ALIVED;
  aliveService_->updateService();
  allocateCommands();
  DimServer::start("TheDBServer"); 
  memset(difInfos_,0,255*sizeof(DIFDbInfo));
  memset(difServices_,0,255*sizeof(DimService*));
	

}
void DimDbServer::allocateCommands()
{
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0<<"/DB/"<<hname<<"/DOWNLOAD";
  downloadCommand_=new DimCommand(s0.str().c_str(),"C",this);
  s0.str(std::string());
  s0<<"/DB/"<<hname<<"/DELETE";
  deleteCommand_=new DimCommand(s0.str().c_str(),"C",this);

}
void DimDbServer::doDelete()
{
  for (int i=0;i<255;i++)
    if (difInfos_[i].id!=0) delete difServices_[i];
  memset(difInfos_,0,255*sizeof(DIFDbInfo));
  memset(difServices_,0,255*sizeof(DimService*));
}
void DimDbServer::doDownload(std::string state)
{
  state_=state;
  if (theDBManager_!=NULL)
    {
      this->doDelete(); //delete exiting service
      delete theDBManager_;
    }
  theDBManager_= new OracleDIFDBManager("74",state_);
  theDBManager_->initialize();
  theDBManager_->download();
  std::map<uint32_t,unsigned char*> dbm=theDBManager_->getAsicKeyMap();

  for (std::map<uint32_t,unsigned char*>::iterator idb=dbm.begin();idb!=dbm.end();idb++)
    {
      uint32_t id = (idb->first>>8)&0xFF;
      difInfos_[id].id=id;
    }
  for (uint32_t i=0;i<255;i++)
    {

      if (difInfos_[i].id==0) continue;
      uint32_t id=i;
      difInfos_[id].nbasic=0;
      for (uint32_t iasic=1;iasic<=48;iasic++)
	{
	  uint32_t key=(id<<8)|iasic;
	  std::map<uint32_t,unsigned char*>::iterator it=dbm.find(key);
	  if (it==dbm.end()) continue;
	  unsigned char* bframe=it->second;

	  uint32_t       framesize=bframe[0];
	  memcpy(&difInfos_[id].slow[difInfos_[id].nbasic],&bframe[1],framesize);
	  difInfos_[id].nbasic++;
	}
      std::stringstream s0;
      
      s0<<"/DB/"<<state_<<"/DIF"<<id;
      difServices_[id] = new DimService(s0.str().c_str(),"I:2;C",&difInfos_[id],2*sizeof(uint32_t)+difInfos_[id].nbasic*sizeof(SingleHardrocV2ConfigurationFrame));
      s0.str(std::string());
      difServices_[id]->updateService();
    }
}
void DimDbServer::commandHandler()
{
  DimCommand *currCmd = getCommand();
  printf(" J'ai recu %s COMMAND  \n",currCmd->getName());
  if (currCmd==downloadCommand_)
    {
      std::string device((const char*) currCmd->getString());
      this->doDownload(device);
      processStatus_=DimDbServer::DOWNLOADED;
      aliveService_->updateService();
    }
  if (currCmd==deleteCommand_)
    {
      this->doDelete();
      processStatus_=DimDbServer::DELETED;
      aliveService_->updateService();

    }
  return;
}