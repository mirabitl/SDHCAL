#ifndef _DIFClient_h
#define _DIFClient_h
#include <iostream>
#include <string.h>
#include<stdio.h>
#include "NMClient.h"
using namespace std;
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include <netlink/socket.h>
#include <netlink/socket_group.h>
#include <string>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <sstream>
#include "DIFReadoutConstant.h"
#include "DIFDBManager.h"

#include <stdint.h>
#include <iostream>

typedef struct
{
	uint32_t status;
	uint64_t lastReceivedBCID;
	uint32_t lastReceivedGTC;
	uint64_t bytesReceived;
} DIFInfo;


class DIFClient: public NMClient
{
public:
	DIFClient(std::string host,uint32_t port);

	NetMessage* serviceHandler(NetMessage* m);

	void sendSecureCommand(std::string s,NetMessage* m);

	NetMessage* answerHandler(NetMessage* m);
	void doRegistration();
	void doScanDevices();
	void doDestroy();
	std::string getDIFString();
	uint32_t getNumberOfDIDF();
	void Print(std::ostream& os=std::cout) const;
	void doInitialise();
	void doInitialise(uint32_t difid);
	void doPreConfigure(uint32_t ctrlreg);
	uint32_t doConfigureChips(uint32_t difid,uint32_t nAsic,SingleHardrocV2ConfigurationFrame* slow);
  uint32_t doConfigureChips(uint32_t difid);
	void doConfigureChips();

	void doConfigure();
	void doStart();
	void doStop();

	inline std::vector<uint32_t>& getDIFList(){return theDIFList_;}
	inline void setDIFDBManager(DIFDBManager* db){theDBManager_=db;}
	inline void setControlRegister(uint32_t t){theControlRegister_=t;}
	inline std::string const getHost() {return theHost_;}
	inline uint32_t const getPort(){return thePort_;}
	inline std::map<uint32_t,DIFInfo> const getDIFMapStatus(){ return theDIFMapStatus_;}
	friend std::ostream& operator<<(std::ostream& os, const DIFClient& dt);
private:
	std::string theHost_;
	uint32_t thePort_;
	DIFDBManager* theDBManager_;
	uint32_t nessai_;
	uint32_t theControlRegister_;
	double nbytes_;
	std::string prefix_;
	boost::interprocess::interprocess_mutex bsem_;
	uint32_t aSize_;
	unsigned char aBuf_[0x10000];
	std::vector<uint32_t> theDIFList_;
	std::map<uint32_t,DIFInfo> theDIFMapStatus_;
};
#endif
