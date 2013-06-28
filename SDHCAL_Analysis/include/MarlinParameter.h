#ifndef _MarlinParameter_h
#define _MarlinParameter_h
#include <string>
class MarlinParameter
{
 public:
 MarlinParameter(std::string name,std::string type,bool b,double d,int i,std::string s) : theName_(name),theType_(type),theBool_(b),theDouble_(d),theInteger_(i),theString_(s){}

  std::string getName(){return theName_;}
  std::string getType(){return theType_;}
  bool getBoolValue() throw (std::string)
  {
    if (theType_.compare("bool")==0) 
      return theBool_;
    else
      throw ("invalid bool type"+theType_);
  }
  double getDoubleValue() throw (std::string)
  {
    if (theType_.compare("double")==0) 
      return theDouble_;
    else
      throw ("invalid double type"+theType_);
  }
  int getIntValue() throw (std::string)
  {
    if (theType_.compare("int")==0) 
      return theInteger_;
    else
      throw ("invalid int type"+theType_);
  }
  std::string getStringValue() throw (std::string)
  {
    if (theType_.compare("string")==0) 
      return theString_;
    else
      throw ("invalid string type"+theType_);
  }

 private:
  std::string theName_,theType_;
  bool theBool_;
  double theDouble_;
  int theInteger_;
  std::string theString_;
};
#endif
