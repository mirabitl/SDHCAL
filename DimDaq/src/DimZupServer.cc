
#include "DimZupServer.h"

DimZupServer::DimZupServer()
{

  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0<<"/DZUP/"<<hname<<"/STATUS";
  aliveService_ = new DimService(s0.str().c_str(),processStatus_);
  s0.str(std::string());
  s0<<"/DZUP/"<<hname<<"/READVALUES";
  memset(readValues_,0,3*sizeof(float));
  readService_ = new DimService(s0.str().c_str(),"F:3",readValues_,3*sizeof(float));

  processStatus_=DimZupServer::ALIVED;
  aliveService_->updateService();
  allocateCommands();
  s0.str(std::string());
  gethostname(hname,80);
  s0<<"DimZupServer-"<<hname;
  DimServer::start(s0.str().c_str()); 
  theZup_=NULL;

	

}
void DimZupServer::allocateCommands()
{
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0<<"/DZUP/"<<hname<<"/OPEN";
  openCommand_=new DimCommand(s0.str().c_str(),"C",this);
  s0.str(std::string());
  s0<<"/DZUP/"<<hname<<"/ON";
  onCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DZUP/"<<hname<<"/OFF";
  offCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DZUP/"<<hname<<"/DOREAD";
  readCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());

  s0<<"/DZUP/"<<hname<<"/CLOSE";
  closeCommand_=new DimCommand(s0.str().c_str(),"I:1",this);

}

void DimZupServer::commandHandler()
{
  DimCommand *currCmd = getCommand();
  printf(" J'ai recu %s COMMAND  \n",currCmd->getName());
  if (currCmd==openCommand_)
    {
      std::string device((const char*) currCmd->getString());
      //device.erase(m->getPayloadSize(),-1);
      theZup_= new Zup(device,1); //Oops a changer

      processStatus_=DimZupServer::OPENED;
      aliveService_->updateService();
    }
  if (currCmd==onCommand_)
    {
      if (theZup_!=NULL)
	theZup_->ON();
      processStatus_=DimZupServer::ON;
      aliveService_->updateService();

    }
  if (currCmd==offCommand_)
    {
      if (theZup_!=NULL)
	theZup_->OFF();
      processStatus_=DimZupServer::OFF;
      aliveService_->updateService();

    }

  if (currCmd==readCommand_)
    {
      if (theZup_!=NULL)
	{
	  readValues_[0]=theZup_->ReadVoltageSet();
	  readValues_[1]=theZup_->ReadVoltageUsed();
	  readValues_[2]=theZup_->ReadCurrentUsed();
	  readService_->updateService();
	}
    }

  if (currCmd==closeCommand_)
    {
      
      if (theZup_!=NULL)
	{
	  delete theZup_;
	  theZup_=NULL;
	}

      processStatus_=DimZupServer::CLOSED;
      aliveService_->updateService();

    }
  return;
}
