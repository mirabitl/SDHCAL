#ifndef _RpcDIFClient_h
#define _RpcDIFClient_h
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
namespace RpcDIFClient
{
  class scan : public DimRpcInfo
  {
  public:
    scan(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),_buf,255)
    {
      //std::cout<<" Creating DIMrpcinfo "<<name<<std::endl;
      memset(_buf,0,255*sizeof(int32_t));
      _sem.lock();
    }
    inline void doIt() { int d=0;setData(d);_sem.lock();}
    void rpcInfoHandler()
    {
      //std::cout<<"Receiver size "<<getSize()<<std::endl;
      memcpy(_buf,getData(),getSize());
      _ndif=getSize()/sizeof(int32_t);
      for (int i=0;i<_ndif;i++)
	std::cout<<_buf[i]<<std::endl;
      _sem.unlock();
    }
    inline int32_t ndif(){return _ndif;}
    inline int32_t* difid(){return _buf;}
  private:
    boost::interprocess::interprocess_mutex _sem;
    int32_t _buf[255];
    int32_t _ndif;
  };

  class registerDB : public DimRpcInfo
  {
  public:
    registerDB(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),-1){_sem.lock();}
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
    inline void doIt(int32_t idif,int32_t reg){int32_t ibuf[2];ibuf[0]=idif;ibuf[1]=reg;setData(ibuf,2*sizeof(int32_t));_sem.lock();}
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
      s0.str(std::string());s0<<_prefix<<"/SCANDEVICES";_scan=new RpcDIFClient::scan(s0.str());
      s0.str(std::string());s0<<_prefix<<"/INITIALISE";_initialise=new RpcDIFClient::initialise(s0.str());
      s0.str(std::string());s0<<_prefix<<"/CONFIGURE";_configure=new RpcDIFClient::configure(s0.str());
      s0.str(std::string());s0<<_prefix<<"/START";_start=new RpcDIFClient::start(s0.str());
      s0.str(std::string());s0<<_prefix<<"/STOP";_stop=new RpcDIFClient::stop(s0.str());
      s0.str(std::string());s0<<_prefix<<"/DESTROY";_destroy=new RpcDIFClient::destroy(s0.str());
      s0.str(std::string());s0<<_prefix<<"/REGISTERDB";_registerDB=new RpcDIFClient::registerDB(s0.str());

      _vdif.clear();
      cout<<"Building DIF rpiClient"<<_prefix<<endl;
      s0.str(std::string());
      char hname[80];
      gethostname(hname,80);
      _state="CREATED";
      s0<<"/RPI/"<<hname<<_prefix<<"/STATE";
      _rdcState = new DimService(s0.str().c_str(),(char*) _state.c_str());
      _rdcState->updateService();
      s0.str(std::string());
      s0<<"rpiClient-"<<hname;
      
      DimServer::start(s0.str().c_str()); 
    }

    void scan()
    {
      std::cout<<"Send Scan command"<<std::endl;
      _scan->doIt();
      std::cout<<_scan->ndif()<< " found"<<std::endl;
      _vdif.clear();
      for (int i=0;i<_scan->ndif();i++)
	_vdif.push_back(_scan->difid()[i]);
      this->publishState("SCANNED");
      _jsroot.clear();
      this->clearInfos();
      this->registerInfos();
      
    }

    void initialise()
    {
      _initialise->doIt(0);
     
      this->checkRpiState("INITIALISED","INIT_FAILED");
    }
    void registerDB(std::string dbs)
    {
      _registerDB->doIt(dbs);
      this->checkRpiState("DB_REGISTERED","DBREGISTER_FAILED");
    }
    void configure(uint32_t reg)
    {
      _configure->doIt(0,reg);
      this->checkRpiState("CONFIGURED","CONFIGURE_FAILED");

    }
    void start()
    {
      _start->doIt(0);
      this->checkRpiState("STARTED","START_FAILED");
    }
    
     void stop()
    {
      _stop->doIt(0);
      this->checkRpiState("STOPPED","STOP_FAILED");
    }
     void destroy()
    {
      _destroy->doIt(0);
      //this->checkRpiState("","DESTROY_FAILED");
      this->publishState("CREATED");
    }
    void checkRpiState(std::string stest,std::string sfail)
    {
      usleep((unsigned int) 100100); // Wait DIM to process
      if (!this->checkDifState(stest))
	this->publishState(sfail);
      else
	this->publishState(stest);

    }
    bool checkDifState(std::string stest)
    {
      const Json::Value difs = _jsroot["difs"];
      for (int i=0;i<difs.size();i++)
	if (difs[i]["state"].asString().substr(0,stest.length()).compare(stest)!=0)
	  return false;
      return true;
    }
    void clearInfos()
    {
      for (std::vector<DimInfo*>::iterator it=_vdifState.begin();it!=_vdifState.end();it++)
	if ((*it)!=NULL) delete (*it);
      _vdifState.clear();
      for (std::vector<DimInfo*>::iterator it=_vdifStatus.begin();it!=_vdifStatus.end();it++)
	if ((*it)!=NULL) delete (*it);
      _vdifStatus.clear();
    }
    void registerInfos()
    {
      _jsroot.clear();
      _jsroot["name"]=_prefix;
      _jsroot["state"]=_state;
      Json::Value array;
      for (std::vector<uint32_t>::iterator it=_vdif.begin();it!=_vdif.end();it++)
	{
	  Json::Value difi;
	  difi["id"]=(*it);
	  difi["state"]=_diState;
	  difi["slc"]=0;
	  difi["gtc"]=0;
	  difi["bcid"]=0;
	  difi["bytes"]=0;
	  array.append(difi);
	  std::stringstream s0;
	  s0.str(std::string());
	  s0<<"/DDS/DIF"<<(*it)<<"/STATE";
	  std::cout<<"Registering "<<s0.str()<<std::endl;
	  DimInfo* d=new DimInfo((char*) s0.str().c_str(),(char*) _diState,512,this);
	  _vdifState.push_back(d);
	  s0.str(std::string());
	  s0<<"/DDS/DIF"<<(*it)<<"/INFO";
	  std::cout<<"Registering "<<s0.str()<<std::endl;;
	  _vdifStatus.push_back(new DimInfo((char*) s0.str().c_str(),&_diStatus,sizeof(DIFStatus),this));
			       
	}

      _jsroot["difs"]=array;
      //Json::FastWriter fastWriter;
      //std::cout<<fastWriter.write(_jsroot);
      //getchar();
    }
    void infoHandler()
    {
      DimInfo *curr = getInfo();
      //std::cout<<"RECEIVED "<<curr->getName();
      for (std::vector<DimInfo*>::iterator it=_vdifState.begin();it!=_vdifState.end();it++)
	if (curr == (*it))
	  {
	    uint32_t difid;
	    sscanf(curr->getName(),"/DDS/DIF%d/STATE",&difid);
	    //std::cout<<"DIF id "<<difid<<" => "<<curr->getString()<<std::endl;
	    for (int i=0;i<_jsroot["difs"].size();i++)
	      if (_jsroot["difs"][i]["id"].asInt()==difid)
		{
		  //Json::Value difi=difs[i];
		  _jsroot["difs"][i]["state"]=std::string(curr->getString());

		}
	  }
      for (std::vector<DimInfo*>::iterator it=_vdifStatus.begin();it!=_vdifStatus.end();it++)
	if (curr == (*it))
	  {
	    uint32_t difid;
	    sscanf(curr->getName(),"/DDS/DIF%d/INFO",&difid);
	    for (int i=0;i<_jsroot["difs"].size();i++)
	      if (_jsroot["difs"][i]["id"].asUInt()==difid)
		{
		  memcpy(&_diStatus,curr->getData(),curr->getSize());
		  _jsroot["difs"][i]["slc"]=_diStatus.slc;
		  _jsroot["difs"][i]["gtc"]=_diStatus.gtc;
		  _jsroot["difs"][i]["bcid"]=(Json::Int64) _diStatus.bcid;
		  _jsroot["difs"][i]["bytes"]=(Json::Int64)_diStatus.bytes;
		}
	  }
    }
    inline void setState(std::string s){_state.assign(s);}
    inline std::string state() const {return _state;}
    // Publish DIM services
    inline void publishState(std::string s){setState(s);_rdcState->updateService((char*) _state.c_str());_jsroot["state"]=_state;}
    inline std::string status(){
      Json::FastWriter fastWriter;
      return fastWriter.write(_jsroot);
    }
    inline std::string prefix(){return _prefix;}
  private:
    std::string _prefix;
    std::string _state;
    std::vector<uint32_t> _vdif;

    DimService* _rdcState;
    // DimInfo
    char _diState[512];
    DIFStatus _diStatus;
    std::vector<DimInfo*> _vdifState;
    std::vector<DimInfo*> _vdifStatus;
    Json::Value _jsroot;
    // Command
    RpcDIFClient::scan* _scan;
    RpcDIFClient::start* _start;
    RpcDIFClient::stop* _stop;
    RpcDIFClient::initialise* _initialise;
    RpcDIFClient::configure* _configure;
    RpcDIFClient::destroy* _destroy;
    RpcDIFClient::registerDB* _registerDB;
    // Info
  };
   


};
#endif
