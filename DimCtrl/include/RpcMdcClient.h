#ifndef _RpcMdcClient_h
#define _RpcMdcClient_h
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
namespace RpcMdcClient
{

  class open : public DimRpcInfo
  {
  public:
    open(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),-1){_sem.lock();}
    inline void doIt(std::string s){setData((char*) s.c_str());_sem.lock();}
    void rpcInfoHandler(){_rc=getInt();_sem.unlock();}
    inline int32_t value(){return _rc;}
  private:
    boost::interprocess::interprocess_mutex _sem;
    int32_t _rc;
  };
  class mask : public DimRpcInfo
  {
  public:
    mask(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),-1){_sem.lock();}
    inline void doIt(int32_t i){int d=i;setData(d);_sem.lock();}
    void rpcInfoHandler(){_rc=getInt();_sem.unlock();}
    inline int32_t value(){return _rc;}
  private:
    boost::interprocess::interprocess_mutex _sem;
    int32_t _rc;
  };
  
  class status : public DimRpcInfo
  {
  public:
    status(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),-1){_sem.lock();}
    inline void doIt(int32_t i){int d=i;setData(d);_sem.lock();}
    void rpcInfoHandler(){memcpy(_rc,getData(),getSize());_sem.unlock();}
    inline int32_t value(int j){return _rc[j];}
  private:
    boost::interprocess::interprocess_mutex _sem;
    int32_t _rc[5];
  };
  
  class reset : public DimRpcInfo
  {
  public:
    reset(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),-1){_sem.lock();}
    inline void doIt(int32_t i){int d=i;setData(d);_sem.lock();}
    void rpcInfoHandler(){_rc=getInt();_sem.unlock();}
    inline int32_t value(){return _rc;}
  private:
    boost::interprocess::interprocess_mutex _sem;
    int32_t _rc;
  };

  class writereg : public DimRpcInfo
  {
  public:
    writereg(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),-1){_sem.lock();}
    inline void doIt(int32_t adr,int32_t val){int32_t d[2];d[0]=adr;d[1]=val;setData(d,2*sizeof(int32_t));_sem.lock();}
    void rpcInfoHandler(){_rc=getInt();_sem.unlock();}
    inline int32_t value(){return _rc;}
  private:
    boost::interprocess::interprocess_mutex _sem;
    int32_t _rc;
  };

  class readreg : public DimRpcInfo
  {
  public:
    readreg(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),-1){_sem.lock();}
    inline void doIt(int32_t adr){int32_t d=adr;setData(d);_sem.lock();}
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
      s0.str(std::string());s0<<_prefix<<"/OPEN";_open=new RpcMdcClient::open(s0.str());
      s0.str(std::string());s0<<_prefix<<"/STATUS";_status=new RpcMdcClient::status(s0.str());
      s0.str(std::string());s0<<_prefix<<"/MASK";_mask=new RpcMdcClient::mask(s0.str());
      s0.str(std::string());s0<<_prefix<<"/RESET";_reset=new RpcMdcClient::reset(s0.str());
      s0.str(std::string());s0<<_prefix<<"/WRITEREG";_writereg=new RpcMdcClient::writereg(s0.str());
      s0.str(std::string());s0<<_prefix<<"/READREG";_readreg=new RpcMdcClient::readreg(s0.str());


      cout<<"Building rpiClient"<<endl;
      s0.str(std::string());
      char hname[80];
      gethostname(hname,80);
      _state="CREATED";
      s0<<"/RPICCC/"<<hname<<"/STATE";
      _rdcState = new DimService(s0.str().c_str(),(char*) _state.c_str());
      _rdcState->updateService();
      s0.str(std::string());
      s0<<"rpiClient-"<<hname;
      
      DimServer::start(s0.str().c_str()); 
    }

    void open(std::string s)
    {
      std::cout<<"Send Open command"<<std::endl;
      _open->doIt(s);
      this->publishState("OPENED");

    }

    void status()
    {
      _status->doIt(0);
    }

    uint32_t version(){return _status->value(0);}
    uint32_t id(){return _status->value(1);}
    uint32_t mask(){return _status->value(2);}
    uint32_t spillCount(){return _status->value(3);}
    uint32_t busyCount(){return _status->value(4);}
    
    void pause()
    {
      _mask->doIt(1);
      this->publishState("PAUSED");
    }
    
    void resume()
    {
      _mask->doIt(0);
      this->publishState("RUNNING");
    }
    void reset()
    {
      _reset->doIt(0);
      
    }
    void writereg(uint32_t adr,uint32_t reg)
    {
      _writereg->doIt(adr,reg);
      
    }
    uint32_t readreg(uint32_t adr)
    {
      _readreg->doIt(adr);
      return _readreg->value();
    } 
    inline void setState(std::string s){_state.assign(s);}
    inline std::string state() const {return _state;}
    // Publish DIM services
    inline void publishState(std::string s){setState(s);_rdcState->updateService((char*) _state.c_str());}
  private:
    std::string _prefix;
    std::string _state;

    DimService* _rdcState;
    // Command
    RpcMdcClient::open* _open;
    RpcMdcClient::mask* _mask;
    RpcMdcClient::status* _status;
    RpcMdcClient::reset* _reset;
    RpcMdcClient::writereg* _writereg;
    RpcMdcClient::readreg* _readreg;
    // Info
  };
   


};
#endif
