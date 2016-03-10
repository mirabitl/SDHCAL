#ifndef _RpcDbClient_h
#define _RpcDbClient_h
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
namespace RpcDbClient
{

  class download : public DimRpcInfo
  {
  public:
    download(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),-1){_sem.lock();}
    inline void doIt(std::string s){setData((char*) s.c_str());_sem.lock();}
    void rpcInfoHandler(){_rc=getInt();_sem.unlock();}
    inline int32_t value(){return _rc;}
  private:
    boost::interprocess::interprocess_mutex _sem;
    int32_t _rc;
  };
  class newrun : public DimRpcInfo
  {
  public:
    newrun(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),-1){_sem.lock();}
    inline void doIt(int32_t i){int d=i;setData(d);_sem.lock();}
    void rpcInfoHandler(){_rc=getInt();_sem.unlock();}
    inline int32_t value(){return _rc;}
  private:
    boost::interprocess::interprocess_mutex _sem;
    int32_t _rc;
  };

  class deletedb : public DimRpcInfo
  {
  public:
    deletedb(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),-1){_sem.lock();}
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
      s0.str(std::string());s0<<_prefix<<"/DOWNLOAD";_download=new RpcDbClient::download(s0.str());
      s0.str(std::string());s0<<"/DB/NEWRUN";_newrun=new RpcDbClient::newrun(s0.str());
      s0.str(std::string());s0<<_prefix<<"/DELETE";_deletedb=new RpcDbClient::deletedb(s0.str());



      cout<<"Building rpiClient"<<endl;
      s0.str(std::string());
      char hname[80];
      gethostname(hname,80);
      _state="CREATED";
      s0<<"/RPIDB/"<<hname<<"/STATE";
      _rdcState = new DimService(s0.str().c_str(),(char*) _state.c_str());
      _rdcState->updateService();
      s0.str(std::string());
      s0<<"rpiClient-"<<hname;
      
      DimServer::start(s0.str().c_str()); 
    }

    void download(std::string s)
    {
      std::cout<<"Send Download command"<<std::endl;
      _download->doIt(s);
      this->publishState("DOWNLOADED");

    }

    uint32_t newrun()
    {
      _newrun->doIt(0);
      return _newrun->value();
    }
    void deletedb()
    {
      _deletedb->doIt(0);
      this->publishState("DELETED");
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
    RpcDbClient::download* _download;
    RpcDbClient::deletedb* _deletedb;
    RpcDbClient::newrun* _newrun;
    // Info
  };
   


};
#endif
