
#include "CCCServer.h"

CCCServer::CCCServer(std::string host,uint32_t port) : NMServer(host,port)
{
	
	this->registerCommand("INITIALISE",boost::bind(&CCCServer::commandHandler,this,_1));
	this->registerCommand("CONFIGURE",boost::bind(&CCCServer::commandHandler,this,_1));
	this->registerCommand("DIFRESET",boost::bind(&CCCServer::commandHandler,this,_1));
	this->registerCommand("CCCRESET",boost::bind(&CCCServer::commandHandler,this,_1));
	this->registerCommand("START",boost::bind(&CCCServer::commandHandler,this,_1));
	this->registerCommand("STOP",boost::bind(&CCCServer::commandHandler,this,_1));
	this->registerCommand("PAUSE",boost::bind(&CCCServer::commandHandler,this,_1));
	this->registerCommand("RESUME",boost::bind(&CCCServer::commandHandler,this,_1));
	this->registerCommand("TESTREGISTERWRITE",boost::bind(&CCCServer::commandHandler,this,_1));
	this->registerCommand("TESTREGISTERREAD",boost::bind(&CCCServer::commandHandler,this,_1));
	this->registerService("UNESSAI");
	this->start();
	//this->startServices();
	nessai_=0;
	running_=false;
}
void CCCServer::doInitialise(std::string device)
{
		theManager_= new CCCManager(device);
		theManager_->initialise();
}
NetMessage* CCCServer::commandHandler(NetMessage* m)
{
	printf(" J'ai recu %s COMMAND  \n",m->getName().c_str());
	if (m->getName().compare("INITIALISE")==0)
	{
		std::string device((const char*) m->getPayload());
		device.erase(m->getPayloadSize(),-1);
		theManager_= new CCCManager(device);
		theManager_->initialise();
		NetMessage* mrep = new NetMessage("INITIALISE",NetMessage::COMMAND_ACKNOWLEDGE,4);
		return mrep;
	}
	if (m->getName().compare("CONFIGURE")==0)
	{
		if (theManager_!=NULL)
		theManager_->configure();
		NetMessage* mrep = new NetMessage("CONFIGURE",NetMessage::COMMAND_ACKNOWLEDGE,4);
		return mrep;
	}

	if (m->getName().compare("START")==0)
	{
		running_=true;
		if (theManager_!=NULL)
		theManager_->start();
		NetMessage* mrep = new NetMessage("START",NetMessage::COMMAND_ACKNOWLEDGE,4);
		return mrep;
	}
	if (m->getName().compare("STOP")==0)
	{
		running_=false;
		if (theManager_!=NULL)
		theManager_->stop();
		NetMessage* mrep = new NetMessage("STOP",NetMessage::COMMAND_ACKNOWLEDGE,4);
		return mrep;
	}
	if (m->getName().compare("PAUSE")==0)
	{
		if (theManager_!=NULL)
		theManager_->getCCCReadout()->DoSendPauseTrigger();
		NetMessage* mrep = new NetMessage("PAUSE",NetMessage::COMMAND_ACKNOWLEDGE,4);
		return mrep;
	}
	if (m->getName().compare("RESUME")==0)
	{
		if (theManager_!=NULL)
		theManager_->getCCCReadout()->DoSendResumeTrigger();
		NetMessage* mrep = new NetMessage("RESUME",NetMessage::COMMAND_ACKNOWLEDGE,4);
		return mrep;
	}

	if (m->getName().compare("DIFRESET")==0)
	{
		if (theManager_!=NULL)
		theManager_->getCCCReadout()->DoSendDIFReset();
		NetMessage* mrep = new NetMessage("DIFRESET",NetMessage::COMMAND_ACKNOWLEDGE,4);
		return mrep;
	}
	if (m->getName().compare("CCCRESET")==0)
	{
		if (theManager_!=NULL)
		theManager_->getCCCReadout()->DoSendCCCReset();
		NetMessage* mrep = new NetMessage("CCCRESET",NetMessage::COMMAND_ACKNOWLEDGE,4);
		return mrep;
	}
	if (m->getName().compare("TESTREGISTERWRITE")==0)
	{
		uint32_t regctrl;
		memcpy(&regctrl,m->getPayload(),sizeof(uint32_t));
		if (theManager_!=NULL)
		theManager_->getCCCReadout()->DoWriteRegister(2,regctrl);
		NetMessage* mrep = new NetMessage("TESTREGISTERWRITE",NetMessage::COMMAND_ACKNOWLEDGE,4);
		return mrep;
	}
	if (m->getName().compare("TESTREGISTERREAD")==0)
	{
		int32_t regctrl;
		if (theManager_!=NULL)
		regctrl=theManager_->getCCCReadout()->DoReadRegister(2);
		NetMessage* mrep = new NetMessage("TESTREGISTERREAD",NetMessage::COMMAND_ACKNOWLEDGE,4);
		mrep->setPayload((unsigned char*) &regctrl,4);
		return mrep;
	}
	return NULL;
}
void CCCServer::startServices()
{
	m_Thread_s = boost::thread(&CCCServer::services, this);  
}
void CCCServer::joinServices()
{
	m_Thread_s.join();  
}
void CCCServer::services()
{
	while (true)
	{
		usleep((uint32_t) 100);
		if (!running_) continue;
		DEBUG(" J'update UNESSAI \n");
		uint32_t len=rand()%(0x10000-0x100) +0x100;
		NetMessage m("UNESSAI",NetMessage::SERVICE,len);
		uint32_t* ibuf=(uint32_t*) m.getPayload();
		ibuf[0]=nessai_++;
		this->updateService("UNESSAI",&m);
		
	}
}
