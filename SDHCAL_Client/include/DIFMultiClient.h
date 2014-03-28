#ifndef _DIFMultiClient_h
#define _DIFMultiClient_h
#include <iostream>
#include <string.h>
#include<stdio.h>
#include "DIFClient.h"
#include "CCCClient.h"
#include "ShmProxy.h"

#include <stdint.h>

class DIFMultiClient
{
public:
  DIFMultiClient(){theProxy_=NULL;theCCCClient_=NULL;theDIFClients_.clear();}
  void addClient(std::string host,uint32_t port);
  void addCCC(std::string host,uint32_t port);
  void ScanDevices();
	void Print(std::ostream& os=std::cout) const;
  void doScanDevices(DIFClient* d);
  void Initialise();
  void purge();
  void doInitialise(DIFClient* d);
  uint32_t Configure(DIFDBManager* db,uint32_t ctrlreg=0x89580000);
  void doConfigure(DIFClient* d);
  uint32_t Start(uint32_t run=0,std::string dir="/tmp");
  uint32_t Stop();
  uint32_t Destroy();
  uint32_t createProxy(uint32_t nd,bool onDisk);
  inline void setProxy( ShmProxy* s){ theProxy_=s;}
  inline CCCClient* getCCCClient() {return theCCCClient_;}
  inline DIFClient* getDIFClient(uint32_t i){return theDIFClients_[i];}
  friend std::ostream& operator<<(std::ostream& os, const DIFMultiClient& dt);
  inline uint32_t getRunNumber(){return theProxy_->getRunNumber();}
  inline uint32_t getEventNumber(){return theProxy_->getEventNumber();}

 private:
  std::vector<DIFClient*> theDIFClients_;
  CCCClient* theCCCClient_;
  ShmProxy* theProxy_;
};
#endif
