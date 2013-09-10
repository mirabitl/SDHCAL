
#include "DIFClient.h"
#include "ShmProxy.h"
using namespace std;
DIFClient::DIFClient(std::string host,uint32_t port) : NMClient(host,port),theDBManager_(NULL),theHost_(host),thePort_(port)
{
	
	//this->start();
	nessai_=0;
	nbytes_=0;
	bsem_.lock();
}
DIFClient::~DIFClient()
{
  std::cout<<"destroying DIF Client \n";
}
NetMessage* DIFClient::serviceHandler(NetMessage* m)
{
	if (m->getName().compare(0,3,"DIF") == 0)
	{
		uint32_t nb=m->getPayloadSize();
		uint32_t difid=ShmProxy::getBufferDIF(m->getPayload());
		uint32_t GTC=ShmProxy::getBufferGTC(m->getPayload());
		uint64_t BCID=ShmProxy::getBufferABCID(m->getPayload());
		
		std::map<uint32_t,DIFInfo>::iterator itd=theDIFMapStatus_.find(difid);
		if (itd!=theDIFMapStatus_.end())
			{
				itd->second.bytesReceived+= (uint64_t) nb;
				itd->second.lastReceivedBCID=BCID;
				itd->second.lastReceivedGTC= GTC;
			}
		ShmProxy::transferToFile(m->getPayload(),
			m->getPayloadSize(),
			ShmProxy::getBufferABCID(m->getPayload()),
			ShmProxy::getBufferDTC(m->getPayload()),
			ShmProxy::getBufferGTC(m->getPayload()),
			ShmProxy::getBufferDIF(m->getPayload()));
		nbytes_+=nb*1.;
		if (ShmProxy::getBufferDTC(m->getPayload())%10000 == 0)
		  printf("%s receieve %d (%f) bytes, BCID %lld DTC %d GTC %d DIF %d \n",__PRETTY_FUNCTION__,
			 m->getPayloadSize(),nbytes_,
			ShmProxy::getBufferABCID(m->getPayload()),
			ShmProxy::getBufferDTC(m->getPayload()),
			ShmProxy::getBufferGTC(m->getPayload()),
			ShmProxy::getBufferDIF(m->getPayload()));
		return NULL;	
	}
	nessai_++;
	nbytes_+=m->getPayloadSize()*1.;
	if (nessai_%1000 ==0)
	{
		uint32_t* ibuf=(uint32_t*) m->getPayload();
		printf(" J'ai recu un service %s %d %d-%d %f \n",m->getName().c_str(),m->getPayloadSize(),nessai_,ibuf[0],nbytes_/1024./1024.);
	}
	return NULL;
}
void DIFClient::sendSecureCommand(std::string s,NetMessage* m)
{
	// printf("LOCKING %s\n",s.c_str());
	//bsem_.lock();
	this->sendCommand(s,m);
	// printf("LOCKING %s\n",s.c_str());
	bsem_.lock();
	//bsem_.unlock();

}

NetMessage* DIFClient::answerHandler(NetMessage* m)
{
	printf("I got answer for command %s \n",m->getName().c_str());
	aSize_=m->getPayloadSize();
	memcpy(aBuf_,m->getPayload(),aSize_);
	//sleep((unsigned int) 5);
	bsem_.unlock();
	//global_stream_lock.unlock();
	return NULL;
}


void DIFClient::doRegistration()
{
	this->registerHandler("SCANDEVICES",boost::bind(&DIFClient::answerHandler,this,_1));
	this->registerHandler("INITIALISE",boost::bind(&DIFClient::answerHandler,this,_1));
	this->registerHandler("PRECONFIGURE",boost::bind(&DIFClient::answerHandler,this,_1));
	this->registerHandler("START",boost::bind(&DIFClient::answerHandler,this,_1));
	this->registerHandler("STOP",boost::bind(&DIFClient::answerHandler,this,_1));
	this->registerHandler("CONFIGURECHIPS",boost::bind(&DIFClient::answerHandler,this,_1));
	this->registerHandler("DESTROY",boost::bind(&DIFClient::answerHandler,this,_1));
	this->registerHandler("SETDIFPARAM",boost::bind(&DIFClient::answerHandler,this,_1));

	this->subscribeService("UNESSAI",boost::bind(&DIFClient::serviceHandler,this,_1));
	this->start();
}

void DIFClient::doScanDevices()
{
 NetMessage m("SCANDEVICES",NetMessage::COMMAND,4);
	this->sendSecureCommand("SCANDEVICES",&m);
	uint32_t ndif=aSize_/sizeof(uint32_t);
	uint32_t* vdif =(uint32_t*) aBuf_;
	theDIFList_.clear();
	for (uint32_t i=0;i<ndif;i++)
	  {
	    theDIFList_.push_back(vdif[i]);
	    printf("%d \n",vdif[i]);
			DIFInfo di;
			memset(&di,0,sizeof(DIFInfo));
			std::pair<uint32_t,DIFInfo> p(vdif[i],di);
			theDIFMapStatus_.insert(p);
	  }
}
void DIFClient::doDestroy()
{
	NetMessage m("DESTROY",NetMessage::COMMAND,4);
	this->sendSecureCommand("DESTROY",&m);

	theDIFList_.clear();
	theDIFMapStatus_.clear();

}


std::string DIFClient::getDIFString()
{
	  std::stringstream diflist;
		for (uint8_t i=0;i<theDIFList_.size()-1;i++)
			diflist<<theDIFList_[i]<<",";
    diflist<<theDIFList_[theDIFList_.size()-1];
		return diflist.str();
}
uint32_t DIFClient::getNumberOfDIDF()
{
return theDIFList_.size();
}
void DIFClient::doInitialise()
{
	for (uint32_t i=0;i<theDIFList_.size();i++)
		{
			this->doInitialise(theDIFList_[i]);
			//printf("%d %d \n",theDIFList_[i],ier);
		}

	return ;
}
void DIFClient::doInitialise(uint32_t difid)
{
	NetMessage m("INITIALISE",NetMessage::COMMAND,4);
	uint32_t* ipay=(uint32_t*) m.getPayload();
	ipay[0]=difid;
	this->sendSecureCommand("INITIALISE",&m);
	std::stringstream s;
	s<<"DIF"<<difid;
	this->subscribeService(s.str(),boost::bind(&DIFClient::serviceHandler,this,_1));
}

void DIFClient::doConfigure()
{
if (theDBManager_==NULL) return ;
this->doPreConfigure(theControlRegister_);
this->doConfigureChips();
}


void DIFClient::doPreConfigure(uint32_t ctrlreg)
{
	NetMessage m("PRECONFIGURE",NetMessage::COMMAND,4);
	uint32_t* ipay=(uint32_t*) m.getPayload();
	ipay[0]=ctrlreg;
	this->sendSecureCommand("PRECONFIGURE",&m);
	
}


void DIFClient::Print(std::ostream& os) const
{
	os<<"== "<<theHost_<<":"<<thePort_<<std::endl;
	for (uint32_t i=0;i<theDIFList_.size();i++)
		{
			
			std::map<uint32_t,DIFInfo>::const_iterator itd=theDIFMapStatus_.find(theDIFList_[i]);
		if (itd!=theDIFMapStatus_.end())
			{
				os<<theDIFList_[i]<<"|"<<itd->second.status<<"|"<<itd->second.lastReceivedBCID<<"|"<<itd->second.lastReceivedGTC<<"|"<<itd->second.bytesReceived<<std::endl;
				}
		}
}


std::ostream& operator<<(std::ostream& os, const DIFClient& dt)
{
		dt.Print(os);
    return os;
}
void DIFClient::doConfigureChips()
{
	if (theDBManager_==NULL) return ;
	for (uint32_t i=0;i<theDIFList_.size();i++)
		{
			uint32_t ier=this->doConfigureChips(theDIFList_[i]);
			printf("DIF %d  Status %d \n",theDIFList_[i],ier);
			std::map<uint32_t,DIFInfo>::iterator itd=theDIFMapStatus_.find(theDIFList_[i]);
		if (itd!=theDIFMapStatus_.end())
			{
				itd->second.status=ier;
				}
		}

	return ;
}
uint32_t DIFClient::doConfigureChips(uint32_t difid)
{
	if (theDBManager_==NULL) return 0;
	std::map<uint32_t,unsigned char*> dbm=theDBManager_->getAsicKeyMap();
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
		printf("Configuring %d asic on DIF %d \n",nbasic,difid);
		uint32_t ier=doConfigureChips(difid,nbasic,slow);
		delete slow;
	return ier;
}
uint32_t DIFClient::doConfigureChips(uint32_t difid,uint32_t nAsic,SingleHardrocV2ConfigurationFrame* slow)
{
	NetMessage m("CONFIGURECHIPS",NetMessage::COMMAND,sizeof(uint32_t)+nAsic*sizeof(SingleHardrocV2ConfigurationFrame));
	uint32_t* ipay=(uint32_t*) m.getPayload();
	ipay[0]=difid;
	memcpy(&ipay[1],slow,nAsic*sizeof(SingleHardrocV2ConfigurationFrame));
	this->sendSecureCommand("CONFIGURECHIPS",&m);
	uint32_t* irep =(uint32_t*) aBuf_;
	return irep[0];
}
void DIFClient::doStart()
{
	NetMessage m("START",NetMessage::COMMAND,4);
	this->sendSecureCommand("START",&m);
}
void DIFClient::doStop()
{
	NetMessage m("STOP",NetMessage::COMMAND,4);
	this->sendSecureCommand("STOP",&m);
}
