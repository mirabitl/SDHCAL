#ifndef _DBCACHE_HH
#define _DBCACHE_HH
#include "OracleDIFDBInterface.hh"
#include "baseApplication.hh"
class DbCache : public levbdim::baseApplication
{
public:
  DbCache(std::string name);
  void configure(levbdim::fsmmessage* m);
  void destroy(levbdim::fsmmessage* m);
  // action
  void c_status(Mongoose::Request &request, Mongoose::JsonResponse &response);
private:
  void ls(std::string sourcedir,std::vector<std::string>& res);
  
  std::string _mode;
  std::string _stateName;
  std::string _path;
  State* _state;
};
#endif
