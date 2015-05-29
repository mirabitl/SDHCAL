
#include "DimCCCServer.h"

DimCCCServer::DimCCCServer()
{

  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0<<"/DCS/"<<hname<<"/STATUS";
  aliveService_ = new DimService(s0.str().c_str(),processStatus_);
  s0.str(std::string());
  s0<<"/DCS/"<<hname<<"/REGISTER";
  registerService_ = new DimService(s0.str().c_str(),register_);
  register_=0;
  processStatus_=DimCCCServer::ALIVED;
  aliveService_->updateService();
  allocateCommands();
  s0.str(std::string());
  gethostname(hname,80);
  s0<<"DimCCCServer-"<<hname;
  DimServer::start(s0.str().c_str()); 


	

}
void DimCCCServer::allocateCommands()
{
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0<<"/DCS/"<<hname<<"/CONFIGURE";
  configureCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DCS/"<<hname<<"/INITIALISE";
  initialiseCommand_=new DimCommand(s0.str().c_str(),"C",this);
  s0.str(std::string());
  s0<<"/DCS/"<<hname<<"/DIFRESET";
  difresetCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DCS/"<<hname<<"/CCCRESET";
  cccresetCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DCS/"<<hname<<"/START";
  startCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DCS/"<<hname<<"/STOP";
  stopCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DCS/"<<hname<<"/PAUSE";
  pauseCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DCS/"<<hname<<"/RESUME";
  resumeCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DCS/"<<hname<<"/TESTREGISTERREAD";
  testregisterreadCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DCS/"<<hname<<"/TESTREGISTERWRITE";
  testregisterwriteCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DCS/"<<hname<<"/REGISTERREAD";
  registerreadCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DCS/"<<hname<<"/REGISTERWRITE";
  registerwriteCommand_=new DimCommand(s0.str().c_str(),"I:2",this);

}
void DimCCCServer::doInitialise(std::string device)
{
  theManager_= new CCCManager(device);
  theManager_->initialise();
}
void DimCCCServer::commandHandler()
{
  DimCommand *currCmd = getCommand();
  printf(" J'ai recu %s COMMAND  \n",currCmd->getName());
  if (currCmd==initialiseCommand_)
    {
      std::string device((const char*) currCmd->getString());
      //device.erase(m->getPayloadSize(),-1);
      theManager_= new CCCManager(device);
      theManager_->initialise();
      processStatus_=DimCCCServer::INITIALISED;
      aliveService_->updateService();
    }
  if (currCmd==configureCommand_)
    {
      if (theManager_!=NULL)
	theManager_->configure();
      processStatus_=DimCCCServer::CONFIGURED;
      aliveService_->updateService();

    }

  if (currCmd==startCommand_)
    {
      if (theManager_!=NULL)
	theManager_->start();
      processStatus_=DimCCCServer::RUNNING;
      aliveService_->updateService();

    }

  if (currCmd==stopCommand_)
    {

      if (theManager_!=NULL)
	theManager_->stop();
      processStatus_=DimCCCServer::STOPPED;
      aliveService_->updateService();

    }

  if (currCmd==pauseCommand_)
    {
      if (theManager_!=NULL)
	theManager_->getCCCReadout()->DoSendPauseTrigger();
      processStatus_=DimCCCServer::PAUSED;
      aliveService_->updateService();

    }

  if (currCmd==resumeCommand_)
    {
      if (theManager_!=NULL)
	theManager_->getCCCReadout()->DoSendResumeTrigger();
      processStatus_=DimCCCServer::RUNNING;
      aliveService_->updateService();

    }

  if (currCmd==difresetCommand_)
    {
      if (theManager_!=NULL)
	theManager_->getCCCReadout()->DoSendDIFReset();
    }

  if (currCmd==cccresetCommand_)
    {
      if (theManager_!=NULL)
	theManager_->getCCCReadout()->DoSendCCCReset();
    }

  if (currCmd==testregisterwriteCommand_)
    {
      int32_t regctrl=currCmd->getInt();
      if (theManager_!=NULL)
	theManager_->getCCCReadout()->DoWriteRegister(2,regctrl);
    }

  if (currCmd==testregisterreadCommand_)
    {
      register_=0;
      if (theManager_!=NULL)
	register_=theManager_->getCCCReadout()->DoReadRegister(2);
      registerService_->updateService();
	}

  if (currCmd==registerwriteCommand_)
    {
      int32_t* regctrl=(int32_t*) currCmd->getData();
      if (theManager_!=NULL)
	theManager_->getCCCReadout()->DoWriteRegister(regctrl[0],regctrl[1]);
    }

  if (currCmd==registerreadCommand_)
    {
      
      int32_t regaddr=currCmd->getInt();
      register_=0;
      if (theManager_!=NULL)
	register_=theManager_->getCCCReadout()->DoReadRegister(regaddr);
      registerService_->updateService();
    }
  return;
}
