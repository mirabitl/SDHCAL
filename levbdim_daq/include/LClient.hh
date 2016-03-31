#ifndef _LClient_h
#define _LClient_h
#include "fsm.hh"
#include <string>
#include <json/json.h>
class LClient : public levbdim::fsmClient
{
public:
  LClient(std::string name) : levbdim::fsmClient(name)
  {
  }
  void clear()
  {
    _jpar.clear();
  }
  template<class T> void set(std::string s,T v) {_jpar[s]=v;}
  void post(std::string cmd)
  {
    _msg.clear();
    _msg["command"]=cmd;
    _msg["content"]=_jpar;
    Json::FastWriter fastWriter;
    _m.setValue(fastWriter.write(_msg));
    this->execute(&_m);
  }
  inline Json::Value reply(){return _m.content()["answer"];}
private:
  Json::Value _jpar,_msg;
  levbdim::fsmmessage _m;
};


#endif
