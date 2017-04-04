#ifndef _FSMWEBCLIENT_HH
#define _FSMWEBCLIENT_HH
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <json/json.h>
#include <stdint.h>
using namespace std;
//char* CurlQuery(char* AddURL,char* Chaine);
char rfc3986[256] = {0};
char html5[256] = {0};

void url_encoder_rfc_tables_init(){

  int i;

  for (i = 0; i < 256; i++){

    rfc3986[i] = isalnum( i) || i == '~' || i == '-' || i == '.' || i == '_' ? i : 0;
    html5[i] = isalnum( i) || i == '*' || i == '-' || i == '.' || i == '_' ? i : (i == ' ') ? '+' : 0;
  }
}

char *url_encode( char *table, unsigned char *s, char *enc){

  for (; *s; s++){

    if (table[*s]) sprintf( enc, "%c", table[*s]);
    else sprintf( enc, "%%%02X", *s);
    while (*++enc);
  }

  return( enc);
}
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
std::string escapeJsonString(const std::string& input) {
  std::ostringstream ss;
  for (auto iter = input.cbegin(); iter != input.cend(); iter++) {
    //C++98/03:
    //for (std::string::const_iterator iter = input.begin(); iter != input.end(); iter++) {
    switch (*iter) {
    case '\\': ss << "\\\\"; break;
    case '"': ss << "\\\""; break;
    case '/': ss << "\\/"; break;
    case '\b': ss << "\\b"; break;
    case '\f': ss << "\\f"; break;
    case '\n': ss << "\\n"; break;
    case '\r': ss << "\\r"; break;
    case '\t': ss << "\\t"; break;
    default: ss << *iter; break;
    }
  }
  return ss.str();
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

  std::string queryState()
  {
    std::string rc=this->curlQuery((char*) _url.c_str());
    
    Json::Reader reader;
    Json::Value jc;
    _parseOk = reader.parse( rc, jc);
    if (!_parseOk) return "UNKOWN";
    return jc["STATE"].asString();
   
  }
  
  std::string curlQuery(std::string url)
  {
    CURL *curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    std::string s;
    if(curl) {
      curl_easy_setopt(curl, CURLOPT_URL,url.c_str());
      /* enable all supported built-in compressions */
      //curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
      /*Do not output result to stdout but to a local string object*/
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, FCurlWrite_CallbackFunc_StdString);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
      //      curl_easy_setopt(ch, CURLOPT_CUSTOMREQUEST, "POST");
      // curl_easy_setopt(ch, CURLOPT_POSTFIELDS, json_object_to_json_string(json));
      
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
  std::string sendTransition(std::string name,Json::Value cnt=Json::Value::null)
  {
    Json::Value content=cnt;
    //    printf("On envoie %s %s\n",name.c_str(),_url.c_str());
     std::stringstream ss;
     ss<<_url<<"FSM?command="<<name;
     std::cout<<content<<std::endl;
     // printf("1 Sending %s \n",ss.str().c_str());
     
     if (!content.isNull())
       {
	 //printf("content non null\n");
	 Json::FastWriter fastWriter;
	 //ss<<"&content="<<escapeJsonString(fastWriter.write(content));
	 std::string sc=fastWriter.write(content);
	 char out[4096];
	 url_encoder_rfc_tables_init();

	 url_encode( html5,(unsigned char*) sc.c_str(),out);
	 ss<<"&content="<<out;
	 //	 printf("2 Sending %s \n",ss.str().c_str());
       }
     else
       {
	 //printf("content  null\n");
	 ss<<"&content={}";
	 // printf("3 Sending %s \n",ss.str().c_str());
       }
     //std::cout<<"4 sending "<<ss.str()<<std::endl;
     //printf("Sending %s \n",ss.str().c_str());
     //     return "none";
   
    std::string rc=curlQuery((char*) ss.str().c_str());
    //printf("return %s %s \n",ss.str().c_str(),rc.c_str());
    Json::Reader reader;
    Json::Value jsta;
    bool parsingSuccessful = reader.parse(rc,jsta);
    if (parsingSuccessful)
      if (jsta["content"].isMember("answer"))
	_answer=jsta["content"]["answer"];
      else
	_answer=Json::Value::null;
    else
      _answer=Json::Value::null;
    return rc;
  }
  std::string sendCommand(std::string name,std::string params=std::string(""))
  {
     std::stringstream s;
     s<<_url<<"CMD?name="<<name;
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
