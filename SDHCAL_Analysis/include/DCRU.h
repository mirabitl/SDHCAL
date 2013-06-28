#ifndef ROOT_DCRU
#define ROOT_DCRU
//#include "DCDefinition.h"
#include <string.h>
using namespace std;
#define MAX_RU_SIZE (512*96*2)
class DCRU  {
  
public:
  int fInstance; // Instance 
  int fBx; // Bunch crossing
  int fSize; // Size of the buffer
  int fBuffer[MAX_RU_SIZE]; // 
public:
  DCRU();
  DCRU(int inst);

  inline void setInstance(int i) {fInstance =i;}
  inline void setBunchCrossing(int i) {fBx =i;}
  inline void setBufferSize(int i) {fSize =i;}
  inline void copyBuffer(int *i, int s) {fSize=s;memcpy(fBuffer,i,s*sizeof(int));}
  inline int getInstance(){ return fInstance;}
  inline int getBunchCrossing(){ return fBx;}
  inline int getBufferSize(){ return fSize;}
  inline int* getBuffer(){ return fBuffer;}
       };
#endif
