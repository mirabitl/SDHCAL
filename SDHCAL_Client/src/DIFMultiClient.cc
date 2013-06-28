#include "DIFMultiClient.h"

#include <stdint.h>
#include <iostream>

#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>

#include <typeinfo>
#include <string>
#include <sstream>

using namespace std;

void DIFMultiClient::addClient(std::string host,uint32_t port)
{
  DIFClient* dc= new DIFClient(host,port);
  dc->doRegistration();
  theDIFClients_.push_back(dc);
}

void DIFMultiClient::addCCC(std::string host,uint32_t port)
{
  theCCCClient_= new CCCClient(host,port);
  theCCCClient_->doRegistration();
}
void DIFMultiClient::ScanDevices()
{
  
  boost::thread_group g;
  uint32_t nserv=0;
  for (std::vector<DIFClient*>::iterator it=theDIFClients_.begin();it!=theDIFClients_.end();it++)
    {
      g.create_thread(boost::bind(&DIFMultiClient::doScanDevices, this,(*it)));
     
      std::cout << " New thread added " <<std::endl;
      nserv++;
    }
  std::cout << " Calling join_all " <<std::endl;
  
  g.join_all();
  
  
}
void DIFMultiClient::doScanDevices(DIFClient* d)
{
  d->doScanDevices();
  std::cout<<"Scan devices thread completed "<<std::endl;
}

void DIFMultiClient::Initialise()
{

  theCCCClient_->doInitialise("DCCCCC01");
  theCCCClient_->doConfigure();
  theCCCClient_->doDIFReset();
  theCCCClient_->doCCCReset();
  theCCCClient_->doStop();
  
  
  boost::thread_group g;

  for (std::vector<DIFClient*>::iterator it=theDIFClients_.begin();it!=theDIFClients_.end();it++)
    {
      g.create_thread(boost::bind(&DIFMultiClient::doInitialise, this,(*it)));
    }
  g.join_all();
	
  
}
void DIFMultiClient::doInitialise(DIFClient* d)
{
  d->doInitialise();
  std::cout<<"intialise thread completed "<<std::endl;

}

uint32_t DIFMultiClient::Configure(DIFDBManager* db,uint32_t ctrlreg)
{
  uint32_t ndif=0;
  //theCCCClient_->doDIFReset();
  //  theCCCClient_->doCCCReset();


	boost::thread_group g;
  for (std::vector<DIFClient*>::iterator it=theDIFClients_.begin();it!=theDIFClients_.end();it++)
    {
      (*it)->setDIFDBManager(db);
      (*it)->setControlRegister(ctrlreg);
      ndif=ndif+(*it)->getNumberOfDIDF();
      g.create_thread(boost::bind(&DIFMultiClient::doConfigure, this,(*it)));
    }
  g.join_all();

  

  return ndif;
}
void DIFMultiClient::doConfigure(DIFClient* d)
{
  d->doConfigure();
}


uint32_t DIFMultiClient::createProxy(uint32_t nd,bool onDisk)
{
if (theProxy_ == NULL)
  {
    theProxy_=new ShmProxy(nd,onDisk);
    theProxy_->Initialise();
    theProxy_->Configure();
  }
}
  uint32_t DIFMultiClient::Start(uint32_t run,std::string dir)
{
  if (theProxy_ != NULL)
    {
      theProxy_->Start(run,dir);
    }
 for (std::vector<DIFClient*>::iterator it=theDIFClients_.begin();it!=theDIFClients_.end();it++)
   (*it)->doStart();

 theCCCClient_->doStart();
}

uint32_t DIFMultiClient::Stop()
{
theCCCClient_->doStop();

 for (std::vector<DIFClient*>::iterator it=theDIFClients_.begin();it!=theDIFClients_.end();it++)
   (*it)->doStop();

 

if (theProxy_ != NULL)
    {
      theProxy_->Stop();
    }
}

void DIFMultiClient::Print(std::ostream& os) const
{
 for (std::vector<DIFClient*>::const_iterator it=theDIFClients_.begin();it!=theDIFClients_.end();it++)
   (*it)->Print(os);


}

std::ostream& operator<<(std::ostream& os, const DIFMultiClient& dt)
{
		dt.Print(os);
    return os;
}

uint32_t DIFMultiClient::Destroy()
{
 for (std::vector<DIFClient*>::iterator it=theDIFClients_.begin();it!=theDIFClients_.end();it++)
   (*it)->doDestroy();


}
