#ifndef _DIFServer_h

#define _DIFServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "NMServer.h"
#include "DIFReadout.h"
using namespace std;
#include <sstream>
#include <map>
#include <vector>
class DIFServer: public NMServer
{
public:
	DIFServer(std::string host,uint32_t port);
	NetMessage* commandHandler(NetMessage* m);
	uint32_t configureChips(uint32_t difid,SingleHardrocV2ConfigurationFrame* slow,uint32_t nasic=48) throw (LocalHardwareException);
	uint32_t configureChips(std::string path,uint32_t difid) throw (LocalHardwareException);

	void preConfigure(uint32_t difid,uint32_t ctrlreg) throw (LocalHardwareException);
	void initialise(uint32_t difid) throw (LocalHardwareException);
	std::vector<uint32_t>& scanDevices();
	  void UsbPrepare();
	void startServices();
	void startReadout();
	void readout(uint32_t difid);

	void joinServices();

	void services();

private:
	bool running_,readoutStarted_;
	boost::thread    m_Thread_s;
	std::map<uint32_t,DIFReadout*> theDIFMap_;
	std::vector<uint32_t> theListOfDIFFounds_;
	boost::thread  m_Thread_d[255];
};
#endif

