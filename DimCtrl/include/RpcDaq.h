#ifndef _RpcDaq_h
#define _RpcDaq_h
#include "RpcDIFClient.h"
#include "RpcCCCClient.h"
#include "RpcDbClient.h"
#include "RpcZupClient.h"
#include "RpcShmClient.h"
#include <unistd.h>
#include <stdint.h>


class RpcDaq
{
public:
  RpcDaq();
  ~RpcDaq();
  //void allocateCommands();
  void processStatus(const std::string &jsonString,Json::Value &m_processInfo);
  void scandns();
  void doScandns();
  void allocateClients();

  void initialiseWriter(std::string directory);
  void initialiseZup(int port,std::string device);
  void doubleSwicthZup(unsigned int pause);
  void LVON();
  void LVOFF();
  void downloadDB(std::string s);
  void downloadDB();
  void openCCC(std::string device);
  void configureCCC();
  void scanFtdi1(RpcDIFClient::rpiClient* d);
  void scanFtdi();

  void setParameters(std::string jsonString);
  void setDBState(std::string dbstate);
  void setControlRegister(uint32_t reg);
  

  void prepareServices();
  void initialise();

  void configure();
  void initialiseDIF();
  void initialise1DIF(RpcDIFClient::rpiClient* d);
  void registerDB1(RpcDIFClient::rpiClient* d,std::string s);
  void registerDB(std::string s);
  void configure1DIF(RpcDIFClient::rpiClient* d,uint32_t reg);
  void configureDIF(uint32_t reg=0x815A1B00);
  void start1(RpcDIFClient::rpiClient* d);
  void start(uint32_t tempo=5);
  void stop(uint32_t tempo=1);
  void destroy1(RpcDIFClient::rpiClient* d);
  void destroy();
  std::string msg(){return _msg;}
  std::string status();
  std::string shmStatus();
  void setState(std::string s);
  std::string state();
  // Publish DIM services
  void publishState(std::string s);
  
private:
  std::string _state,_msg;
  DimService* _shmState;



  std::string theDBPrefix_,theCCCPrefix_,theWriterPrefix_,theProxyPrefix_,theZupPrefix_;
  std::vector<std::string> theDIFPrefix_;
  std::vector<RpcDIFClient::rpiClient*> _DIFClients;
  
  RpcShmClient::rpiClient* _shClient;
  RpcZupClient::rpiClient* _zupClient;
  RpcDbClient::rpiClient* _dbClient;
  RpcCCCClient::rpiClient* _cccClient;


  Json::Value _jparam;
  std::string _dbstate;
  std::string _dccname;
  std::string _zupdevice;
  std::string _writerdir;
  uint32_t _zupport;
  uint32_t _ctrlreg;
  uint32_t _run;

};
#endif
