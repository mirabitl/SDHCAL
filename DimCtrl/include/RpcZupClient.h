#ifndef _RpcZupClient_h
#define _RpcZupClient_h
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "dis.hxx"
#include "dic.hxx"

#include "DIFReadoutConstant.h"
using namespace std;
#include <sstream>
#include <map>
#include <vector>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <boost/interprocess/sync/interprocess_mutex.hpp>
// -- json headers
#include "json/json.h"

#include <string>
#include "CtrlLogger.h"
namespace RpcZupClient
{

  class open : public DimRpcInfo
  {
  public:
    open(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),-1){_sem.lock();}
    inline void doIt(uint32_t p,std::string s){
      int ibuf[32]; ibuf[0]=p;memcpy(&ibuf[1],s.c_str(),s.length());
      setData(ibuf,s.length()+sizeof(uint32_t));_sem.lock();}
    void rpcInfoHandler(){_rc=getInt();_sem.unlock();}
    inline int32_t value(){return _rc;}
  private:
    boost::interprocess::interprocess_mutex _sem;
    int32_t _rc;
  };
  class lvread : public DimRpcInfo
  {
  public:
    lvread(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),-1){_sem.lock();}
    inline void doIt(int32_t i){int d=i;setData(d);_sem.lock();}
    void rpcInfoHandler(){memcpy(_rc,getData(),getSize());_sem.unlock();}
    inline float vset(){return _rc[0];}
    inline float vout(){return _rc[1];}
    inline float iout(){return _rc[2];}
  private:
    boost::interprocess::interprocess_mutex _sem;
    float _rc[3];
  };

  class lvswitch : public DimRpcInfo
  {
  public:
    lvswitch(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),-1){_sem.lock();}
    inline void doIt(int32_t i){int d=i;setData(d);_sem.lock();}
    void rpcInfoHandler(){_rc=getInt();_sem.unlock();}
    inline int32_t value(){return _rc;}
  private:
    boost::interprocess::interprocess_mutex _sem;
    int32_t _rc;
  };

  class rpiClient : public DimClient
  {
  public:
    rpiClient(std::string prefix) : _prefix(prefix)
    {
      std::stringstream s0;
      s0.str(std::string());s0<<_prefix<<"/OPEN";_open=new RpcZupClient::open(s0.str());
      s0.str(std::string());s0<<_prefix<<"/READ";_lvread=new RpcZupClient::lvread(s0.str());
      s0.str(std::string());s0<<_prefix<<"/SWITCH";_lvswitch=new RpcZupClient::lvswitch(s0.str());



      cout<<"Building rpiClient"<<endl;
      s0.str(std::string());
      char hname[80];
      gethostname(hname,80);
      _state="CREATED";
      s0<<"/RPIZUP/"<<hname<<"/STATE";
      _rdcState = new DimService(s0.str().c_str(),(char*) _state.c_str());
      _rdcState->updateService();
      s0.str(std::string());
      s0<<"rpiClient-"<<hname;
      
      DimServer::start(s0.str().c_str()); 
    }

    void open(uint32_t p,std::string s)
    {
      std::cout<<"Send Open command"<<std::endl;
      _open->doIt(p,s);
      this->publishState("OPENED");

    }

   void lvread()
    {
      _lvread->doIt(0);

    }
    void lvswitch(uint32_t status)
    {
      _lvswitch->doIt(status);
      if (status==0)
	this->publishState("OFF");
      else
	this->publishState("ON");
    }

    inline float vset(){return _lvread->vset();}
    inline float vout(){return _lvread->vout();}
    inline float iout(){return _lvread->iout();}
    inline void setState(std::string s){_state.assign(s);}
    inline std::string state() const {return _state;}
    // Publish DIM services
    inline void publishState(std::string s){setState(s);_rdcState->updateService((char*) _state.c_str());}
  private:
    std::string _prefix;
    std::string _state;

    DimService* _rdcState;
    // Command
    RpcZupClient::open* _open;
    RpcZupClient::lvswitch* _lvswitch;
    RpcZupClient::lvread* _lvread;
    // Info
  };
   


};
#endif
