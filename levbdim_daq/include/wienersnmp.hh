#ifndef WIENERSNMP_HH
#define WIENERSNMP_HH
#include <stdio.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <json/writer.h> 
#include "json/json.h"
#include <iostream>
#include <fstream>

#include <boost/algorithm/string.hpp>
using namespace  std;


class WienerSnmp
{
public:
  std::string exec(const char* cmd) {
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }
    pclose(pipe);
    return result;
    
  }
  
  WienerSnmp(std::string ipa) : _ip(ipa){}
  std::string getSysMainSwitch()
  {
    std::stringstream sc;
    sc<<"snmpget -v 2c -m +WIENER-CRATE-MIB -c public ";
    sc<<_ip<<" sysMainSwitch.0";
    return exec(sc.str().c_str());
  }
  std::string setOutputVoltage(uint32_t module,uint32_t voie,float tension)
  {
    std::stringstream sc;
    sc<<"snmpset -v 2c -m +WIENER-CRATE-MIB -c guru ";
    sc<<_ip<<" outputVoltage.u"<<100*module+voie<<" F "<<tension;
    return exec(sc.str().c_str());

  }
  float getOutputVoltage(uint32_t module,uint32_t voie)
  {
    std::stringstream sc;
    sc<<"snmpget -v 2c -m +WIENER-CRATE-MIB -c public ";
    sc<<_ip<<" outputVoltage.u"<<100*module+voie;
    std::string res=exec(sc.str().c_str());
    //std::cout<<__PRETTY_FUNCTION__<<res<<std::endl;
    std::vector<std::string> strs;
    boost::split(strs,res, boost::is_any_of(" "));
    return atof(strs[4].c_str());
  }
   std::string setOutputVoltageRiseRate(uint32_t module,uint32_t voie,float val)
  {
    std::stringstream sc;
    sc<<"snmpset -v 2c -m +WIENER-CRATE-MIB -c guru ";
    sc<<_ip<<" outputVoltageRiseRate.u"<<100*module+voie<<" F "<<val;
    return exec(sc.str().c_str());

  }
  float getOutputVoltageRiseRate(uint32_t module,uint32_t voie)
  {
    std::stringstream sc;
    sc<<"snmpget -v 2c -m +WIENER-CRATE-MIB -c public ";
    sc<<_ip<<" outputVoltageRiseRate.u"<<100*module+voie;
    std::string res=exec(sc.str().c_str());
    //std::cout<<__PRETTY_FUNCTION__<<res<<std::endl;
    std::vector<std::string> strs;
    boost::split(strs,res, boost::is_any_of(" "));
    return atof(strs[4].c_str());
  }
    std::string setOutputCurrentLimit(uint32_t module,uint32_t voie,float cur )
  {
    std::stringstream sc;
    sc<<"snmpset -v 2c -m +WIENER-CRATE-MIB -c guru ";
    sc<<_ip<<" outputCurrent.u"<<100*module+voie<<" F "<<cur;
    return exec(sc.str().c_str());

  }
  float getOutputCurrentLimit(uint32_t module,uint32_t voie)
  {
    std::stringstream sc;
    sc<<"snmpget -v 2c -m +WIENER-CRATE-MIB -c public ";
    sc<<_ip<<" outputCurrent.u"<<100*module+voie;
    std::string res=exec(sc.str().c_str());
    //std::cout<<__PRETTY_FUNCTION__<<res<<std::endl;
    std::vector<std::string> strs;
    boost::split(strs,res, boost::is_any_of(" "));
    return atof(strs[4].c_str());
  }
  float getOutputMeasurementSenseVoltage(uint32_t module,uint32_t voie)
  {
    std::stringstream sc;
    sc<<"snmpget -v 2c -m +WIENER-CRATE-MIB -c public ";
    sc<<_ip<<" outputMeasurementSenseVoltage.u"<<100*module+voie;
    std::string res=exec(sc.str().c_str());
    //std::cout<<__PRETTY_FUNCTION__<<res<<std::endl;
    std::vector<std::string> strs;
    boost::split(strs,res, boost::is_any_of(" "));
    return atof(strs[4].c_str());
  }
  float getOutputMeasurementCurrent(uint32_t module,uint32_t voie)
  {
    std::stringstream sc;
    sc<<"snmpget -v 2c -m +WIENER-CRATE-MIB -c public ";
    sc<<_ip<<" outputMeasurementCurrent.u"<<100*module+voie;
    std::string res=exec(sc.str().c_str());
    //std::cout<<__PRETTY_FUNCTION__<<res<<std::endl;
    std::vector<std::string> strs;
    boost::split(strs,res, boost::is_any_of(" "));
    return atof(strs[4].c_str());
  }
  std::string setOutputSwitch(uint32_t module,uint32_t voie,uint32_t val )
  {
    std::stringstream sc;
    sc<<"snmpset -v 2c -m +WIENER-CRATE-MIB -c guru ";
    sc<<_ip<<" outputSwitch.u"<<100*module+voie<<" i "<<val;
    return exec(sc.str().c_str());

  }
  std::string getOutputSwitch(uint32_t module,uint32_t voie)
  {
    std::stringstream sc;
    sc<<"snmpget -v 2c -m +WIENER-CRATE-MIB -c public ";
    sc<<_ip<<" outputSwitch.u"<<100*module+voie;
    std::string res=exec(sc.str().c_str());
    //std::cout<<__PRETTY_FUNCTION__<<res<<std::endl;
    std::vector<std::string> strs;
    boost::split(strs,res, boost::is_any_of(" "));
    return strs[3];
  }
  std::string getOutputStatus(uint32_t module,uint32_t voie)
  {
    std::stringstream sc;
    sc<<"snmpget -v 2c -m +WIENER-CRATE-MIB -c public ";
    sc<<_ip<<" outputStatus.u"<<100*module+voie;
    std::string res=exec(sc.str().c_str());
    //std::cout<<__PRETTY_FUNCTION__<<res<<std::endl;
    //std::vector<std::string> strs;
    //boost::split(strs,res, boost::is_any_of(" "));
    return res;
  }
private:
  std::string _ip;
};
#endif
