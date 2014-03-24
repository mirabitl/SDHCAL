
// This is the JSON header




#ifndef _DHCalJsonParser_H
#define _DHCalJsonParser_H
#include "json/json.h"
class DHCalJsonParser
{
public:
  DHCalJsonParser();
  void parse(std::string config);
  std::vector<std::string> getPlugginsNames();
  std::string getPlugginLibraryName(std::string plug);
  std::vector<std::string> getPlugginParametersNames(std::string plug);
  const Json::Value  getPlugginParameter(std::string plug,std::string name);

private:
  Json::Value root;
  Json::Reader reader;
  Json::Value  pluggins;

};

#endif
