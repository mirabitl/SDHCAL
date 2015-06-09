
#include "DimDS1820Server.h"

DimDS1820Server::DimDS1820Server()
{

  theDS1820_= new DS1820(); 

  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0<<"/DTEMP/"<<hname<<"/STATUS";

  s0.str(std::string());
  s0<<"/DTEMP/"<<hname<<"/READVALUES";
  readService_ = new DimService(s0.str().c_str(),(float &)readValues_);
	readService_->updateService();

  s0.str(std::string());
  gethostname(hname,80);
  s0<<"DimDS1820Server-"<<hname;
  DimServer::start(s0.str().c_str()); 
}

DimDS1820Server::~DimDS1820Server()
{
	delete theDS1820_;
}

void DimDS1820Server::getTemperature()
{
	readValues_=theDS1820_->DS1820Read();
	readService_->updateService();

}
