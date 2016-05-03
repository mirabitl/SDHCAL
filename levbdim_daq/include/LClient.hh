#ifndef _LClient_h
#define _LClient_h
#include "fsm.hh"
#include <string>
#include <iostream>
#include <json/json.h>
class LClient : public levbdim::fsmClient
{
public:
  LClient(std::string name) : levbdim::fsmClient(name)
  {
    size_t ncmd=ss.find("/CMD");
    std::string fsmname =name.substr(0,ncmd);
    size_t npref=fsmname.find_last_of("/FSM/");
    _subname=fsmname.substr(npref+1);
    size_t ntype=_subname.find("-");
    _type=_subname(0,ntype);
    _host= subname(ntype+1);
    if (_type.compare("DIF")==0)
      _port=40000;
    if (_type.compare("Ccc")==0)
      _port=42000;
    if (_type.compare("Mdcc")==0)
      _port=41000;
    if (_type.compare("Zup")==0)
      _port=43000;
    
    std::cout<<"http://"<<hostname<<":"<<_port<<"/"<<_subname<<"/CMD?name=STATUS"<<std::endl;
    
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
  inline Json::Value parameters(){ return _jpar;}
  inline Json::Value reply(){
    //std::cout<<msg()->content()<<std::endl;
    //std::cout<<msg()->status().asString()<<std::endl;

    if (msg()->status().asString().compare("DONE")==0)
      return msg()->content()["answer"];
    else
      {
	Json::Value m;
	m.clear();
	return m;
      }
  }
private:
  Json::Value _jpar,_msg;
  levbdim::fsmmessage _m;
  std::string _subname;
  std::string _type;
  std::string _hostname;
  uint32_t port;
};


#endif
