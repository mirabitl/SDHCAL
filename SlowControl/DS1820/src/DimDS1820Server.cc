
#include "DimDS1820Server.h"

DimDS1820Server::DimDS1820Server() : thePeriod_(10)
{

  theDS1820_= new DS1820(); 

  std::stringstream s0,s1;
  char hname[80];
  gethostname(hname,80);
  s0<<"/DTEMP/"<<hname<<"/STATUS";

  s0.str(std::string());
  s0<<"/DTEMP/"<<hname<<"/READVALUES";
	
	s1<<"F:"<<theDS1820_->DS1820GetNbSensors();
printf ("%s\n",s1.str().c_str());
  readService_ = new DimService(s0.str().c_str(),s1.str().c_str(),(float *)&readValues_, sizeof(readValues_ ));
	readService_->updateService(); 
s0.str(std::string());
  s0<<"/DTEMP/"<<hname<<"/SETPERIOD";
  periodCommand_=new DimCommand(s0.str().c_str(),"I:1",this);

  s0.str(std::string());
  s0<<"/DTEMP/"<<hname<<"/STORE";
  storeCommand_=new DimCommand(s0.str().c_str(),"C",this);

  
  s0.str(std::string());
  gethostname(hname,80);
  s0<<"DimDS1820Server-"<<hname;
  DimServer::start(s0.str().c_str());
  Loop();

}
void DimDS1820Server::Loop()
{
  g_d.create_thread(boost::bind(&DimDS1820Server::readout, this));
}

void DimDS1820Server::store()
{
  if (!storeDb_) return;
  if (my_==NULL) return;
  my_->connect();
  std::stringstream ss;
  ss<<"INSERT INTO DS1820MON (TIN, TOUT) VALUES ("<<readValues_[0]<<","<<readValues_[1]<<")";

  my_->executeQuery(ss.str());
  my_->disconnect();
}

void DimDS1820Server::readout()
{
  while (true)
    {
      this->getTemperature();
      this->store();
      sleep((unsigned int) thePeriod_);
    }
}

DimDS1820Server::~DimDS1820Server()
{
	delete theDS1820_;
}

void DimDS1820Server::getTemperature()
{
	theDS1820_->DS1820Read();
	theDS1820_->DS1820GetTemps(readValues_);
//	readValues_[0]=theDS1820_->tempC[0];
//	readValues_[1]=theDS1820_->tempC[1];
	printf ("%f %f\n", readValues_[0], readValues_[1]);
	readService_->updateService();

}
void DimDS1820Server::commandHandler()
{
  DimCommand *currCmd = getCommand();
  printf(" J'ai recu %s COMMAND  \n",currCmd->getName());
  if (currCmd==periodCommand_)
    {
      thePeriod_=currCmd->getInt();
    }
  if (currCmd==storeCommand_)
    {
      theAccount_.assign(currCmd->getString());
      my_=new MyInterface(theAccount_);
      storeDb_=true;
    }

}
