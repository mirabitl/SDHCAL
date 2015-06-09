
#include "DimGPIOServer.h"

DimGPIOServer::DimGPIOServer()
{
printf ("entering constructor\n");
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0<<"/DPWR/"<<hname<<"/STATUS";
  aliveService_ = new DimService(s0.str().c_str(),processStatus_);
  s0.str(std::string());
  s0<<"/DPWR/"<<hname<<"/READVALUES";
  memset(readValues_,0,3*sizeof(float));
  readService_ = new DimService(s0.str().c_str(),"I:2",readValues_,2*sizeof(int));

  processStatus_=DimGPIOServer::ALIVED;
  aliveService_->updateService();
  allocateCommands();
  s0.str(std::string());
  gethostname(hname,80);
  s0<<"DimGPIOServer-"<<hname;
  DimServer::start(s0.str().c_str()); 
  theGPIO_=NULL;
}

void DimGPIOServer::allocateCommands()
{
printf ("entering allocate command\n");
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
	printf ("hostname : %s\n",hname);
  s0<<"/DGPIO/"<<hname<<"/OPEN";
  openCommand_=new DimCommand(s0.str().c_str(),"C",this);
  
	s0.str(std::string());
  s0<<"/DGPIO/"<<hname<<"/VME_ON";
  VMEonCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DGPIO/"<<hname<<"/VME_OFF";
  VMEoffCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
	
	s0.str(std::string());
  s0<<"/DGPIO/"<<hname<<"/DIF_ON";
  DIFonCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DGPIO/"<<hname<<"/DIF_OFF";
  DIFoffCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
	
  s0.str(std::string());
  s0<<"/DGPIO/"<<hname<<"/DOREAD";
  readCommand_=new DimCommand(s0.str().c_str(),"I:2",this);
  s0.str(std::string());

  s0<<"/DGPIO/"<<hname<<"/CLOSE";
  closeCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
}

void DimGPIOServer::commandHandler()
{

  DimCommand *currCmd = getCommand();
  printf(" J'ai recu %s COMMAND  \n",currCmd->getName());
  if (currCmd==openCommand_)
  {
    std::string device((const char*) currCmd->getString());
    theGPIO_= new GPIO(); 

    processStatus_=DimGPIOServer::OPENED;
    aliveService_->updateService();
  }
  if (currCmd==DIFonCommand_)
  {
    if (theGPIO_!=NULL)
		{
			theGPIO_->DIFON();
			sleep((unsigned int) 1);
		  readValues_[0]=theGPIO_->getDIFPower();
		  readService_->updateService();
		}
		if (theGPIO_->getVMEPower()==0)
	    processStatus_=DimGPIOServer::DIF_ON;
		else	
	    processStatus_=DimGPIOServer::BOTH_ON;
    aliveService_->updateService();
  }
  if (currCmd==DIFoffCommand_)
  {
    if (theGPIO_!=NULL)
		{
			theGPIO_->DIFOFF();
			sleep((unsigned int) 1);
		  readValues_[0]=theGPIO_->getDIFPower();
		  readService_->updateService();
		}
		if (theGPIO_->getVMEPower()==1)
    	processStatus_=DimGPIOServer::VME_ON;
		else
    	processStatus_=DimGPIOServer::BOTH_OFF;
    aliveService_->updateService();
  }
  if (currCmd==VMEonCommand_)
  {
    if (theGPIO_!=NULL)
		{
			theGPIO_->VMEON();
			sleep((unsigned int) 1);
		  readValues_[0]=theGPIO_->getVMEPower();
		  readService_->updateService();
		}
		if (theGPIO_->getDIFPower()==0)
	    processStatus_=DimGPIOServer::VME_ON;
		else
	    processStatus_=DimGPIOServer::BOTH_ON;
    aliveService_->updateService();
  }
  if (currCmd==VMEoffCommand_)
  {
    if (theGPIO_!=NULL)
		{
			theGPIO_->VMEOFF();
			sleep((unsigned int) 1);
		  readValues_[0]=theGPIO_->getVMEPower();
		  readService_->updateService();
		}
		if (theGPIO_->getDIFPower()==1)
    	processStatus_=DimGPIOServer::DIF_ON;
		else	
    	processStatus_=DimGPIOServer::BOTH_OFF;
    aliveService_->updateService();
  }

  if (currCmd==readCommand_)
  {
    if (theGPIO_!=NULL)
		{
	  	readValues_[0]=theGPIO_->getVMEPower();
		  readValues_[1]=theGPIO_->getDIFPower();
	  	readService_->updateService();
		}
  }

  if (currCmd==closeCommand_)
  {
    if (theGPIO_!=NULL)
		{
		  delete theGPIO_;
	  	theGPIO_=NULL;
		}
    processStatus_=DimGPIOServer::CLOSED;
    aliveService_->updateService();
  }
  return;
}
