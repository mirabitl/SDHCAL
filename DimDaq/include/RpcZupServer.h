#ifndef _RpcZupServer_h

#define _RpcZupServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "dis.hxx"
#include "Zup.h"
using namespace std;
#include <sstream>
#include "DaqLogger.h"
class RpcZupServer;

class RpcZupOpen : public DimRpc
{
 public:
  RpcZupOpen(RpcZupServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcZupServer* _server;
};
class RpcZupSwitch : public DimRpc
{
 public:
  RpcZupSwitch(RpcZupServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcZupServer* _server;
};
class RpcZupRead : public DimRpc
{
 public:
  RpcZupRead(RpcZupServer* serv,std::string name);
  void rpcHandler(); 
 private:
  RpcZupServer* _server;
};

class RpcZupServer: public DimServer
{
public:
  RpcZupServer();
  void allocateCommands();
  void setState(std::string s){_state.assign(s);}
  inline std::string state() const {return _state;}
  // Publish DIM services
  inline void publishState(std::string s){setState(s);_zsState->updateService((char*) _state.c_str());}
  inline void publishStatus(){_zsStatus->updateService(_status,3*sizeof(float));}
  // getters
  void setZup(Zup*z ){_zup =z;}
  Zup* getZup(){return _zup;}
  // action
  void Open(std::string,uint32_t m);
  void Read();
  void Switch(uint32_t m);
  float* readstatus(){return _status;}
private:
  std::string _state;
  float _status[3];
  DimService* _zsStatus; //State of the last register read
  DimService* _zsState; //State of the last register read 
  RpcZupOpen* _openCommand;
  RpcZupSwitch*  _switchCommand;
  RpcZupRead* _readCommand;

 
  Zup* _zup;
};
#endif

