#ifndef _FSMWEBCLIENT_HH
#define _FSMWEBCLIENT_HH
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <json/json.h>
#include <stdint.h>
using namespace std;
//char* CurlQuery(char* AddURL,char* Chaine);

size_t FCurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s){
  size_t newLength = size*nmemb;
  size_t oldLength = s->size();
  try{
    s->resize(oldLength + newLength);
  }
  catch(std::bad_alloc &e){
    //handle memory problem
    return 0;
  }
  
  std::copy((char*)contents,(char*)contents+newLength,s->begin()+oldLength);
  return size*nmemb;
}


class fsmwebClient
{
public:
  fsmwebClient(std::string host,uint32_t port)
  {
    std::stringstream s;
    s<<"http://"<<host<<":"<<port<<"/";
    // Check the prefix
    std::string rc=this->curlQuery((char*) s.str().c_str());
    
    Json::Reader reader;
    _parseOk = reader.parse( rc, _jConfig);
    std::cout<<"fsmwebClient failed => "<<s.str();
    if (!_parseOk) return;
    s<<_jConfig["PREFIX"].asString()<<"/";
    _url=s.str();
    std::cout<<"fsmwebClient ok url => "<<_url;
  }
  
  std::string curlQuery(std::string url)
  {
    CURL *curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    std::string s;
    if(curl) {
      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      /* enable all supported built-in compressions */
      curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
      /*Do not output result to stdout but to a local string object*/
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, FCurlWrite_CallbackFunc_StdString);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

      /* Perform the request, res will get the return code */ 
      res = curl_easy_perform(curl);
      /* Check for errors */

      if(res != CURLE_OK){
	ostringstream oss;
	oss<<"CURL connection failed : "<<curl_easy_strerror(res)<<endl;
      }

    
      /* always cleanup */ 
      curl_easy_cleanup(curl);
    
    }
    curl_global_cleanup();
    return s;

  }
  std::string sendTransition(std::string name,Json::Value content=Json::Value::null)
  {
     std::stringstream s;
     s<<_url<<"FSM?command="<<name;
     if (content!=Json::Value::null)
       s<<"&content="<<content.asString();
     else
       s<<"&content={}";
    std::string rc=curlQuery((char*) s.str().c_str());
    Json::Reader reader;
    Json::Value jsta;
    bool parsingSuccessful = reader.parse(rc,jsta);
    if (parsingSuccessful)
      _answer=jsta["content"]["answer"];
    else
      _answer=Json::Value::null;
    return rc;
  }
  std::string sendCommand(std::string name,std::string params=std::string(""))
  {
     std::stringstream s;
     s<<_url<<"FSM?command="<<name;
     if (params.length()>2)
       s<<params;
     std::string rc=curlQuery((char*) s.str().c_str());
     Json::Reader reader;
     Json::Value jsta;
     bool parsingSuccessful = reader.parse(rc,jsta);
     if (parsingSuccessful)
       _answer=jsta;
     else
       _answer=Json::Value::null;
    return rc;


  }
  Json::Value answer(){return _answer;}
private:
  bool _parseOk;
  Json::Value _jConfig;
  std::string _url;
  Json::Value _answer;
};
#endif
