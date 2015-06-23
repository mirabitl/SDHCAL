
#include "DimBMP183Server.h"

DimBMP183Server::DimBMP183Server()
{

  theBMP183_= new BMP183(); 

  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0<<"/DTEMP/"<<hname<<"/STATUS";

  s0.str(std::string());
  s0<<"/DTEMP/"<<hname<<"/TEMPERATUREREADVALUES";
  TemperatureReadService_ = new DimService(s0.str().c_str(),(float &)TemperatureReadValues_);
  TemperatureReadService_->updateService();

  s0.str(std::string());
  s0<<"/DTEMP/"<<hname<<"/PRESSIONREADVALUES";
  PressionReadService_ = new DimService(s0.str().c_str(),(float &)PressionReadValues_);
  PressionReadService_->updateService();

  s0.str(std::string());
  gethostname(hname,80);
  s0<<"DimDS1820Server-"<<hname;
  DimServer::start(s0.str().c_str()); 
}

DimBMP183Server::~DimBMP183Server()
{
	delete theBMP183_;
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
