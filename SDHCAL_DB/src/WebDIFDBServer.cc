
#include "WebDIFDBServer.h"

WebDIFDBServer::WebDIFDBServer(std::string host,uint32_t port) : NMServer(host,port)
{
	
  this->registerCommand("DOWNLOAD",boost::bind(&WebDIFDBServer::commandHandler,this,_1));
  this->registerCommand("DIFSETTINGS",boost::bind(&WebDIFDBServer::commandHandler,this,_1));
  this->start();
  //this->startServices();
  running_=false;
  theManager_=NULL;
}
NetMessage* WebDIFDBServer::commandHandler(NetMessage* m)
{
  printf(" J'ai recu %s COMMAND  \n",m->getName().c_str());
  if (m->getName().compare("DOWNLOAD")==0)
    {
      std::string state((const char*) m->getPayload());
      state.erase(m->getPayloadSize(),-1);
      if (theManager_!=NULL) delete theManager_;
      theManager_= new OracleDIFDBManager("74",state);
      theManager_->initialize();
      theManager_->download();
      NetMessage* mrep = new NetMessage("DOWNLOAD",NetMessage::COMMAND_ACKNOWLEDGE,4);
      return mrep;
    }
  if (m->getName().compare("DIFSETTINGS")==0)
    {
      if (theManager_!=NULL)
	{
	  uint32_t difid=0;
	  memcpy(&difid,m->getPayload(),sizeof(uint32_t));
	  std::map<uint32_t,unsigned char*> dbm=theManager_->getAsicKeyMap();
	  SingleHardrocV2ConfigurationFrame* slow = new SingleHardrocV2ConfigurationFrame[48];
	  uint32_t nbasic=0;
	  for (uint32_t iasic=1;iasic<=48;iasic++)
	    {
	      uint32_t key=(difid<<8)|iasic;
	      std::map<uint32_t,unsigned char*>::iterator it=dbm.find(key);
	      if (it==dbm.end()) continue;
	      unsigned char* bframe=it->second;

	      uint32_t       framesize=bframe[0];
	      memcpy(&slow[nbasic],&bframe[1],framesize);
	      nbasic++;
	    }
	  printf("Getting DIF settings for %d asics on DIF %d \n",nbasic,difid);
	  NetMessage* m= new NetMessage("DIFSETTINGS",NetMessage::COMMAND_ACKNOWLEDGE,sizeof(uint32_t)+nbasic*sizeof(SingleHardrocV2ConfigurationFrame));
	  uint32_t* ipay=(uint32_t*) m->getPayload();
	  ipay[0]=difid;
	  memcpy(&ipay[1],slow,nbasic*sizeof(SingleHardrocV2ConfigurationFrame));
	  delete slow;
	  return m;
		    
	  //theManager_->configure();
	}
      else
	{
	  NetMessage* mrep = new NetMessage("DIFSETTINGS",NetMessage::COMMAND_ACKNOWLEDGE,4);
	  return mrep;
	}
    }
  return NULL;
}
