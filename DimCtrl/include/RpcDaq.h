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
  void openCCC(std::string device);
  void configureCCC();
  void scanFtdi();

  void setParameters(std::string jsonString);


  void prepareServices();
  void initialise();

  void configure();
  void initialiseDIF();
  void registerDB(std::string s);
  void configureDIF(uint32_t reg=0x815A1B00);
  void start(uint32_t tempo=1);
  void stop(uint32_t tempo=1);
  void destroy();
  
  std::string status();
  void setState(std::string s);
  std::string state();
  // Publish DIM services
  void publishState(std::string s);
  
private:
  std::string _state;
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
