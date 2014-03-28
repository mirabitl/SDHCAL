#ifndef _DIFHit_H_
#define _DIFHit_H_
#include <stdint>
class DIFHit
{
 public:
  DIFHit(uint16_t difid,uint16_t asicid,uint16_t pad);
  inline uint16_t getDIFId(){ return theDIFId_;}
  inline uint16_t getAsic(){ return theAsic_;}
  inline uint16_t getPad(){return thePad_;}
  inline uint16_t I(){return theI_;}
  inline uint16_t J(){return theJ_;}
  inline float XLocal(){return theI_*100./96.;}
  inline float YLocal(){return theJ_*100./96.;}
 private:
  uint16_t theDIFId_;
  uint16_t theAsic_;
  uint16_t thePad_;
  uint16_t theI_;
  uint16_t theJ_;

  
};
#endif
