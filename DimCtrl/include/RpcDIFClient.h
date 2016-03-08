#ifndef _RpcDIFClient_h
#define _RpcDIFClient_h
#include <boost/interprocess/sync/interprocess_mutex.hpp>

namespace RpcDIFClient
{
  class scan : public DimRpcInfo
  {
  public:
    scan(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),timeout,_buf,255)
    {
      memset(_buf,0,255*sizeof(int32_t));
      _sem.lock();
    }
    inline void doIt() { setData(0);_sem.lock();}
    void rpcInfoHandler()
    {
      memcpy(_buf,getData(),getSize());
      _ndif=getSize()/sizeof(int32_t);
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
    registerDB(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),timeout,-1){}
    inline void doIt(std::string s){setData(s.c_str());_sem.lock();}
    void rpcInfoHandler(){_rc=getInt();_sem.unlock();}
    inline int32_t value(){return _rc;}
  private:
    int32_t _rc;
  };
  class initialise : public DimRpcInfo
  {
  public:
    initialise(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),timeout,-1){}
    inline void doIt(int32_t &i){setData(i);_sem.lock();}
    void rpcInfoHandler(){_rc=getInt();_sem.unlock();}
    inline int32_t value(){return _rc;}
  private:
    int32_t _rc;
  };

  class configure : public DimRpcInfo
  {
  public:
    configure(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),timeout,-1){}
    inline void doIt(int32_t idif,int32_t reg){int32_t ibuf[2];ibuf[0]=idif;ibuf[1]=reg;setData(ibuf,2*sizeof(int32_t));_sem.lock();}
    void rpcInfoHandler(){_rc=getInt();_sem.unlock();}
    inline int32_t value(){return _rc;}
  private:
    int32_t _rc;
  };

  class start : public DimRpcInfo
  {
  public:
    start(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),timeout,-1){}
    inline void doIt(int32_t &i){setData(i);_sem.lock();}
    void rpcInfoHandler(){_rc=getInt();_sem.unlock();}
    inline int32_t value(){return _rc;}
  private:
    int32_t _rc;
  };
  class stop : public DimRpcInfo
  {
  public:
    stop(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),timeout,-1){}
    inline void doIt(int32_t &i){setData(i);_sem.lock();}
    void rpcInfoHandler(){_rc=getInt();_sem.unlock();}
    inline int32_t value(){return _rc;}
  private:
    int32_t _rc;
  };
  
  class destroy : public DimRpcInfo
  {
  public:
    destroy(std::string name,int timeout=-1) : DimRpcInfo((char*) name.c_str(),timeout,-1){}
    inline void doIt(int32_t &i){setData(i);_sem.lock();}
    void rpcInfoHandler(){_rc=getInt();_sem.unlock();}
    inline int32_t value(){return _rc;}
  private:
    int32_t _rc;
  };


  class rpiClient : public DimServer, DimClient
  {
    rpiClient(std::string prefix) : _prefix(prefix)
    {
      std::stringstream s0;
      s0.str(std::string());s0<<_prefix<<"/SCANDEVICES";_scan=new scan(s0.str());
      s0.str(std::string());s0<<_prefix<<"/INITIALISE";_initialise=new initialise(s0.str());
      s0.str(std::string());s0<<_prefix<<"/CONFIGURE";_configure=new configure(s0.str());
      s0.str(std::string());s0<<_prefix<<"/START";_start=new start(s0.str());
      s0.str(std::string());s0<<_prefix<<"/STOP";_stop=new stop(s0.str());
      s0.str(std::string());s0<<_prefix<<"/DESTROY";_destroy=new destroy(s0.str());
      s0.str(std::string());s0<<_prefix<<"/REGISTERDB";_registerDB=new registerDB(s0.str());

      _vdif.clear();
    }

    void scan()
    {
      _scan->doIt();
      _vdif.clear();
      for (int i=0;i<_scan->ndif();i++)
	_vdif.push_back(_scan->difid()[i]);
      this->publishState("SCANNED");
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
      for (std::vector<uint32_t>::iterator it=_vdif.begin();it!=_vdif.end();it++)
	{
	  std::stringstream s0;
	  s0.str(std::string());
	  s0<<"/DDS/DIF"<<(*it)<<"/STATE";
	  _vdifState.push_back(new DimInfo(s0.str().c_str(),_diState));
	  s0.str(std::string());
	  s0<<"/DDS/DIF"<<(*it)<<"/STATUS";
	  _vdifStatus.push_back(new DimInfo(s0.str().c_str(),&_diStatus,sizeof(DIFStatus)));
			       
	}
    }
    void infoHandler()
    {
    }
  private:
    std::string _prefix;
    std::string _state;
    std::vector<uint32_t> _vdif;

    // DimInfo
    char _diState[80];
    DIFStatus _diStatus;
    std::vector<DimInfo*> _vdifState;
    std::vector<DimInfo*> _vdifStatus;
    // Command
    scan* _scan;
    start* _start;
    stop* _stop;
    initialise* _initialise;
    configure* _configure;
    destroy* _destroy;
    registerDB* _registerDB;
    // Info
  };
   


};
#endif
