#ifndef _RpcShmClient_h
#define _RpcShmClient_h
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
namespace RpcShmClient
{

  class setup : public DimRpcInfo
  {
  public:
    setup(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),-1){_sem.lock();}
    inline void doIt(std::string s){setData((char*) s.c_str());_sem.lock();}
    void rpcInfoHandler(){_rc=getInt();_sem.unlock();}
    inline int32_t value(){return _rc;}
  private:
    boost::interprocess::interprocess_mutex _sem;
    int32_t _rc;
  };
  
  class directory : public DimRpcInfo
  {
  public:
    directory(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),-1){_sem.lock();}
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
   class destroy : public DimRpcInfo
  {
  public:
    destroy(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),-1){_sem.lock();}
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
      s0.str(std::string());s0<<_prefix<<"/INITIALISE";_initialise=new RpcShmClient::initialise(s0.str());
      s0.str(std::string());s0<<_prefix<<"/START";_start=new RpcShmClient::start(s0.str());
      s0.str(std::string());s0<<_prefix<<"/STOP";_stop=new RpcShmClient::stop(s0.str());
      s0.str(std::string());s0<<_prefix<<"/DESTROY";_destroy=new RpcShmClient::destroy(s0.str());
      s0.str(std::string());s0<<_prefix<<"/SETUP";_setup=new RpcShmClient::setup(s0.str());
      s0.str(std::string());s0<<_prefix<<"/DIRECTORY";_directory=new RpcShmClient::directory(s0.str());


      cout<<"Building rpiClient"<<endl;
      s0.str(std::string());
      char hname[80];
      gethostname(hname,80);
      _state="CREATED";
      s0<<"/RPISHM/"<<hname<<"/STATE";
      _rdcState = new DimService(s0.str().c_str(),(char*) _state.c_str());
      _rdcState->updateService();
      s0.str(std::string());
      s0<<"rpiClient-"<<hname;
      
      DimServer::start(s0.str().c_str()); 
    }


    void initialise()
    {
      _initialise->doIt(0);
      this->publishState("INITIALISED");
    }
    void start(uint32_t nd)
    {
      _start->doIt(nd);
      this->publishState("STARTED");
    }
    
     void stop()
    {
      _stop->doIt(0);
      this->publishState("STOPPED");
    }
    void destroy()
    {
      _destroy->doIt(0);
      this->publishState("DESTROYED");
    }
    void setup(std::string s)
    {
      _setup->doIt(s);
    }
    void directory(std::string s)
    {
      _directory->doIt(s);
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

    RpcShmClient::destroy* _destroy;
    RpcShmClient::start* _start;
    RpcShmClient::stop* _stop;
    RpcShmClient::initialise* _initialise;
    RpcShmClient::setup* _setup;
    RpcShmClient::directory* _directory;
  };
   


};
#endif
