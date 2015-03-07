#ifndef DIFSnapShot_H
#define DIFSnapShot_H
#include "DIFSlowControl.h"

class DIFSnapShot
{
 public:
  DIFSnapShot();
  DIFSnapShot(std::vector<DIFPtr*> &difptrlist,uint32_t seed);
  DIFSnapShot(std::vector<DIFPtr*> &difptrlist,uint32_t lowtime,uint32_t hightime);

  void clear();
  void fill(std::vector<DIFPtr*> &difptrlist,uint32_t seed1,uint32_t seed2=0);
  void DrawEvent();
  void DrawDIF(uint32_t difid);
  inline bool hit(uint32_t ith,uint32_t idif, uint32_t i,uint32_t j){ return ((theData[ith][idif][i]>>j)&1);}
 private:
  uint32_t theSeed;
  uint32_t theLowTime,theHighTime;
  uint32_t theData[3][255][96];
    
};
#endif
