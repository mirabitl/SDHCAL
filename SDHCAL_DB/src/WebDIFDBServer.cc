
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
		theManager_= new OracleDBManager("74",state);
		theManager_->initialise();

		NetMessage* mrep = new NetMessage("DOWNLOAD",NetMessage::COMMAND_ACKNOWLEDGE,4);
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
void WebDIFDBServer::startServices()
{
	m_Thread_s = boost::thread(&WebDIFDBServer::services, this);  
}
void WebDIFDBServer::joinServices()
{
	m_Thread_s.join();  
}
void WebDIFDBServer::services()
{
	while (true)
	{
		usleep((uint32_t) 100);
		if (!running_) continue;
		
	}
}
