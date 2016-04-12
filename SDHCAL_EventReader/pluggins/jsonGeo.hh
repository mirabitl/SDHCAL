#ifndef JSONGEO_HH
#define JSONGEO_HH
#include <json/json.h>
#include <stdint.h>
#include <string>
#include <Math/PositionVector3D.h>
#include <Math/Point3Dfwd.h>

std::string itoa(int k);


class jsonGeo
{
public:
  jsonGeo(std::string config);
  inline Json::Value difGeo(uint32_t k)  {return _jroot["difs"][itoa(k)];}
  inline Json::Value chamberGeo(uint32_t k)  {return _jroot["chambers"][itoa(k)];}
  void convert(uint32_t dif,uint32_t asic,uint32_t channel,ROOT::Math::XYZPoint* point);
private:
  Json::Value _jroot;
};

#endif
