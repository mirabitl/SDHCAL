#ifndef _LClient_h
#define _LClient_h
#include "fsm.hh"
#include <string>
#include <iostream>
#include <json/json.h>
//extern "C" {
//  char* CurlQuery(char* AddURL,char* Chaine);
//};

char* CurlQuery(char* AddURL,char* Chaine);
 

class LClient : public levbdim::fsmClient, DimClient
{
public:
  LClient(std::string name) : levbdim::fsmClient(name)
  {
    size_t ncmd=name.find("/CMD");
    std::string fsmname =name.substr(0,ncmd);
    //std::cout<<" FSMNAME: "<<fsmname<<std::endl;
    size_t npref=fsmname.find("/FSM/")+5;
    _subname=fsmname.substr(npref,fsmname.length()-npref);
    //std::cout<<npref<<" SUBNAME: "<<_subname<<std::endl;
    size_t ntype=_subname.find("-");
        
    _type=_subname.substr(0,ntype);
    //std::cout<<ntype<<" TYPE: "<<_type<<std::endl;
    _host= _subname.substr(ntype+1,_subname.length()-ntype);
    _port=0;
    if (_type.compare("DIF")==0)
      _port=40000;
    if (_type.compare("Ccc")==0)
      _port=42000;
    if (_type.compare("Mdcc")==0)
      _port=41000;
    if (_type.compare("Zup")==0)
      _port=43000;
    //if (_port!=0)
    // std::cout<<_type<<" has webservice at http://"<<_host<<":"<<_port<<"/"<<_subname<<"/CMD?name=XXXX"<<std::endl;
    std::stringstream s0;
    s0<<fsmname<<"/WEB";
    std::cout<<"Creating "<<s0.str()<<std::endl;
    memset(_weburl,0,512);
    _webInfo= new DimInfo(s0.str().c_str(),(char*) _weburl,512,this);
  }
  void infoHandler()
  {
    DimInfo *curr = getInfo(); // get current DimInfo address 
  //if (curr->getSize()==1) return;
    if (curr==_webInfo)
      {
	memset(_weburl,0,512);
	memcpy(_weburl,curr->getString(),curr->getSize());
	std::cout<<"URL =========>"<<_weburl<<std::endl;
      }
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
  std::string postweb(std::string saction)
  {
    std::string surl;surl.assign(_weburl);
    if (surl.substr(0,4).compare("http")!=0)
      return "NONE";
    std::stringstream s;
    s<<surl<<"/"<<saction;

    std::cout<<"Web command "<<s.str()<<std::endl;
    char AddURL[512];
    char RC[50000];  //À changer selon vos besoin
    memset(RC,0,50000);
    strncpy(AddURL,s.str().c_str(),s.str().length());
    CurlQuery((char*) s.str().c_str(),RC);
    return std::string(RC);
  }

  //Met le contenu de la page web dans la struct

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
  std::string _host;
  uint32_t _port;
  DimInfo* _webInfo;
  char _weburl[512];
};


#endif
