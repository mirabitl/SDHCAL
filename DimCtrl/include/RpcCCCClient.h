#ifndef _RpcCCCClient_h
#define _RpcCCCClient_h
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
namespace RpcCCCClient
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
  class initialise : public DimRpcInfo
  {
  public:
    initialise(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),-1){_sem.lock();}
    inline void doIt(int32_t i){int d=i;setData(d);_sem.lock();}
    void rpcInfoHandler(){_rc=getInt();_sem.unlock();}
    inline int32_t value(){return _rc;}
  private:
    boost::interprocess::interprocess_mutex _sem;
    int32_t _rc;
  };

  class configure : public DimRpcInfo
  {
  public:
    configure(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),-1){_sem.lock();}
    inline void doIt(int32_t idif){int32_t ibuf=idif;setData(ibuf);_sem.lock();}
    void rpcInfoHandler(){_rc=getInt();_sem.unlock();}
    inline int32_t value(){return _rc;}
  private:
    boost::interprocess::interprocess_mutex _sem;
    int32_t _rc;
  };

  class start : public DimRpcInfo
  {
  public:
    start(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),-1){_sem.lock();}
    inline void doIt(int32_t i){int d=i;setData(d);_sem.lock();}
    void rpcInfoHandler(){_rc=getInt();_sem.unlock();}
    inline int32_t value(){return _rc;}
  private:
    boost::interprocess::interprocess_mutex _sem;
    int32_t _rc;
  };
  class stop : public DimRpcInfo
  {
  public:
    stop(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),-1){_sem.lock();}
    inline void doIt(int32_t i){int d=i;setData(d);_sem.lock();}
    void rpcInfoHandler(){_rc=getInt();_sem.unlock();}
    inline int32_t value(){return _rc;}
  private:
    boost::interprocess::interprocess_mutex _sem;
    int32_t _rc;
  };
   class pause : public DimRpcInfo
  {
  public:
    pause(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),-1){_sem.lock();}
    inline void doIt(int32_t i){int d=i;setData(d);_sem.lock();}
    void rpcInfoHandler(){_rc=getInt();_sem.unlock();}
    inline int32_t value(){return _rc;}
  private:
    boost::interprocess::interprocess_mutex _sem;
    int32_t _rc;
  };
  class resume : public DimRpcInfo
  {
  public:
    resume(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),-1){_sem.lock();}
    inline void doIt(int32_t i){int d=i;setData(d);_sem.lock();}
    void rpcInfoHandler(){_rc=getInt();_sem.unlock();}
    inline int32_t value(){return _rc;}
  private:
    boost::interprocess::interprocess_mutex _sem;
    int32_t _rc;
  };
  
  class difreset : public DimRpcInfo
  {
  public:
    difreset(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),-1){_sem.lock();}
    inline void doIt(int32_t i){int d=i;setData(d);_sem.lock();}
    void rpcInfoHandler(){_rc=getInt();_sem.unlock();}
    inline int32_t value(){return _rc;}
  private:
    boost::interprocess::interprocess_mutex _sem;
    int32_t _rc;
  };
  
  class cccreset : public DimRpcInfo
  {
  public:
    cccreset(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),-1){_sem.lock();}
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
      s0.str(std::string());s0<<_prefix<<"/OPEN";_open=new RpcCCCClient::open(s0.str());
      s0.str(std::string());s0<<_prefix<<"/INITIALISE";_initialise=new RpcCCCClient::initialise(s0.str());
      s0.str(std::string());s0<<_prefix<<"/CONFIGURE";_configure=new RpcCCCClient::configure(s0.str());
      s0.str(std::string());s0<<_prefix<<"/START";_start=new RpcCCCClient::start(s0.str());
      s0.str(std::string());s0<<_prefix<<"/STOP";_stop=new RpcCCCClient::stop(s0.str());
      s0.str(std::string());s0<<_prefix<<"/PAUSE";_pause=new RpcCCCClient::pause(s0.str());
      s0.str(std::string());s0<<_prefix<<"/RESUME";_resume=new RpcCCCClient::resume(s0.str());
      s0.str(std::string());s0<<_prefix<<"/DIFRESET";_difreset=new RpcCCCClient::difreset(s0.str());
      s0.str(std::string());s0<<_prefix<<"/CCCRESET";_cccreset=new RpcCCCClient::cccreset(s0.str());
      s0.str(std::string());s0<<_prefix<<"/WRITEREG";_writereg=new RpcCCCClient::writereg(s0.str());
      s0.str(std::string());s0<<_prefix<<"/READREG";_readreg=new RpcCCCClient::readreg(s0.str());


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

    void initialise()
    {
      _initialise->doIt(0);
      this->publishState("INITIALISED");
    }
    void configure()
    {
      _configure->doIt(0);
      this->publishState("CONFIGURED");

    }
    void start()
    {
      _start->doIt(0);
      this->publishState("STARTED");
    }
    
     void stop()
    {
      _stop->doIt(0);
      this->publishState("STOPPED");
    }
    void pause()
    {
      _start->doIt(0);
      this->publishState("PAUSED");
    }
    
    void resume()
    {
      _stop->doIt(0);
      this->publishState("STARTED");
    }
    void difreset()
    {
      _difreset->doIt(0);
      
    }
    void cccreset()
    {
      _cccreset->doIt(0);
      
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
    RpcCCCClient::open* _open;
    RpcCCCClient::pause* _pause;
    RpcCCCClient::resume* _resume;
    RpcCCCClient::start* _start;
    RpcCCCClient::stop* _stop;
    RpcCCCClient::initialise* _initialise;
    RpcCCCClient::configure* _configure;
    RpcCCCClient::difreset* _difreset;
    RpcCCCClient::cccreset* _cccreset;
    RpcCCCClient::writereg* _writereg;
    RpcCCCClient::readreg* _readreg;
    // Info
  };
   


};
#endif
