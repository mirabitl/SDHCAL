#include <cstdio>
#include <cstring>
#include <iostream>     // std::cout
#include <fstream>      // std::ifstream

// This is the JSON header
#include "DHCalJsonParser.h"

DHCalJsonParser::DHCalJsonParser(){;}
void DHCalJsonParser::parse(std::string config) 
{
  std::ifstream ifs (config.c_str(), std::ifstream::in);

 // Let's parse it  
 
  bool parsedSuccess = reader.parse(ifs, 
                                   root, 
                                   false);
  
  if(not parsedSuccess)
    {
      // Report failures and their locations 
      // in the document.
      std::cout<<"Failed to parse JSON"<<std::endl 
	  <<reader.getFormatedErrorMessages()
	       <<std::endl;
      return ;
    }
}

std::vector<std::string> DHCalJsonParser::getPlugginsNames()
{
  pluggins = root["plugins"];
  
  return pluggins.getMemberNames();
}
std::string DHCalJsonParser::getPlugginLibraryName(std::string plug)
{
  const Json::Value pl=pluggins.get(plug,0);
  const Json::Value jparam = pl.get("library","NONE");
  return jparam.asString();
}
std::vector<std::string> DHCalJsonParser::getPlugginParametersNames(std::string plug)
{
   const Json::Value pl=pluggins.get(plug,0);
   const Json::Value jparam = pl.get("parameters",0);
   return jparam.getMemberNames();
}
const Json::Value  DHCalJsonParser::getPlugginParameter(std::string plug,std::string name)

{
  const Json::Value pl=pluggins.get(plug,0);
  const Json::Value jparam = pl.get("parameters",0);
  const Json::Value jp = jparam.get(name,0);
  return jp;
}
#ifdef MAIN_TEST_DHCALJSONPARSER
int main(int argc, char **argv)
{
  DHCalJsonParser* dp= new DHCalJsonParser();
  dp->parse("test.cfg");
  std::vector<std::string> vpl=dp->getPlugginsNames();
  for (std::vector<std::string>::iterator ipl=vpl.begin();ipl!=vpl.end();ipl++)
    std::cout<<(*ipl)<<" " <<dp->getPlugginLibraryName((*ipl))<<" "<<dp->getPlugginParameter((*ipl),"nevent").asInt()<<std::endl;


 return 0;
}
#endif
