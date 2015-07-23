
#include "DimBMP183Server.h"

DimBMP183Server::DimBMP183Server() :thePeriod_(5),storeDb_(false),my_(NULL)
{

  theBMP183_= new BMP183(); 

  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0<<"/BMP183/"<<hname<<"/STATUS";

  s0.str(std::string());
  s0<<"/BMP183/"<<hname<<"/TEMPERATUREREADVALUES";
  TemperatureReadService_ = new DimService(s0.str().c_str(),(float &)TemperatureReadValues_);
  TemperatureReadService_->updateService();

  s0.str(std::string());
  s0<<"/BMP183/"<<hname<<"/PRESSIONREADVALUES";
  PressionReadService_ = new DimService(s0.str().c_str(),(float &)PressionReadValues_);
  PressionReadService_->updateService();

  
  s0.str(std::string());
  s0<<"/BMP183/"<<hname<<"/SETPERIOD";
  periodCommand_=new DimCommand(s0.str().c_str(),"I:1",this);

  s0.str(std::string());
  s0<<"/BMP183/"<<hname<<"/STORE";
  storeCommand_=new DimCommand(s0.str().c_str(),"C",this);

  s0.str(std::string());
  gethostname(hname,80);
  s0<<"DimBMP183Server-"<<hname;
  DimServer::start(s0.str().c_str()); 
  Loop();
}

DimBMP183Server::~DimBMP183Server()
{
  delete theBMP183_;
}

void DimBMP183Server::Loop()
{
  g_d.create_thread(boost::bind(&DimBMP183Server::readout, this));
}

void DimBMP183Server::store()
{
  if (!storeDb_) return;
  if (my_==NULL) return;
  my_->connect();
  std::stringstream ss;
#ifdef SLOW2015
  ss<<"INSERT INTO PT (P, T) VALUES ("<<TemperatureReadValues_<<","<<PressionReadValues_<<")";
#else
  ss<<"INSERT INTO PTMON (TEMPERATURE, PRESSURE) VALUES ("<<TemperatureReadValues_<<","<<PressionReadValues_<<")";
#endif
  my_->executeQuery(ss.str());
  my_->disconnect();
}

void DimBMP183Server::readout()
{
  while (true)
    {
      this->getTemperature();
      this->getPression();
      this->store();
      sleep((unsigned int) thePeriod_);
    }
}
void DimBMP183Server::commandHandler()
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
void DimBMP183Server::getTemperature()
{
  TemperatureReadValues_=theBMP183_->BMP183TemperatureRead();
  TemperatureReadService_->updateService();

}
void DimBMP183Server::getPression()
{
  PressionReadValues_=theBMP183_->BMP183PressionRead();
  PressionReadService_->updateService();

}
