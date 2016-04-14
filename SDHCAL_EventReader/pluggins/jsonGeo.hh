#ifndef JSONGEO_HH
#define JSONGEO_HH
#include <json/json.h>
#include <stdint.h>
#include <string>
#include <Math/PositionVector3D.h>
#include <Math/Point3Dfwd.h>
#include <Math/Vector3Dfwd.h>
#include <Math/DisplacementVector3D.h>


#define INFO_PRINT_ENABLED 1
#if DEBUG_PRINT_ENABLED
#define INFO_PRINT_ENABLED 1
#define DEBUG_PRINT printf
#else
#define DEBUG_PRINT(format, args...) ((void)0)
#endif
#if INFO_PRINT_ENABLED
#define INFO_PRINT printf
#else
#define INFO_PRINT(format, args...) ((void)0)
#endif

std::string itoa(int k);


class jsonGeo
{
public:
  jsonGeo(std::string config);
  inline Json::Value cuts()  {return _jroot["cuts"];}
  inline Json::Value difGeo(uint32_t k)  {return _jroot["difs"][itoa(k)];}
  inline Json::Value chamberGeo(uint32_t k)  {return _jroot["chambers"][itoa(k)];}
  void convert(uint32_t dif,uint32_t asic,uint32_t channel,ROOT::Math::XYZPoint* point);
private:
  Json::Value _jroot;
};

#endif
