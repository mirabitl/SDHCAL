#define MAX_DIF_NUMBER 255
#define MAX_CHAMBER_NUMBER 60
#define MAX_TIME_WORD 50000
#include <stdint.h>
#include <string.h>
class FlagTime
{
public:
  FlagTime(){
    difSet_ = new uint32_t[MAX_DIF_NUMBER*MAX_TIME_WORD];
    chamberSet_ = new uint32_t[MAX_CHAMBER_NUMBER*MAX_TIME_WORD];
    difCount_= new unsigned char[MAX_TIME_WORD*32];
    chamberCount_= new unsigned char[MAX_TIME_WORD*32];
    this->clear();}
  inline void clear(){memset(difSet_,0,MAX_DIF_NUMBER*MAX_TIME_WORD*sizeof(uint32_t));memset(chamberSet_,0,MAX_CHAMBER_NUMBER*MAX_TIME_WORD*sizeof(uint32_t));max_time_=MAX_TIME_WORD;memset(chamberCount_,0,MAX_TIME_WORD*32);memset(difCount_,0,MAX_TIME_WORD*32);}
  inline void setDifTime(uint32_t dif,uint32_t t) {if ((t/32)>=MAX_TIME_WORD) return; difSet_[dif*MAX_TIME_WORD+ t/32]|=(1<<(t%32)); difCount_[t]+=1;}
  inline void setChamberTime(uint32_t dif,uint32_t t) {if ((t/32)>=MAX_TIME_WORD) return;chamberSet_[dif*MAX_TIME_WORD+t/32]|=(1<<(t%32)); chamberCount_[t]+=1;}
  inline bool isChamberSet(uint32_t dif,uint32_t t) {if ((t/32)>=max_time_) return false; return (chamberSet_[dif*MAX_TIME_WORD+t/32]&(1<<(t%32)))!=0;}
  inline bool isDifSet(uint32_t dif,uint32_t t) {if ((t/32)>=max_time_) return false; return (difSet_[dif*MAX_TIME_WORD+t/32]&(1<<(t%32)))!=0;}
  inline uint32_t countDif(uint32_t t){return difCount_[t];}
  inline uint32_t countChamber(uint32_t t){return chamberCount_[t];}
  inline void setMaxTime(uint32_t t){max_time_=t+1;}
  inline void summarize()
  {
    memset(chamberCount_,0,max_time_);
    memset(difCount_,0,max_time_);
    for (uint32_t it=0;it<max_time_;it++)
      {
	for (uint32_t i=0;i<MAX_CHAMBER_NUMBER;i++)
	  if (isChamberSet(i,it)) chamberCount_[it]=chamberCount_[it]+1;
	for (uint32_t i=0;i<MAX_DIF_NUMBER;i++)
	  if (isDifSet(i,it)) difCount_[it]=difCount_[it]+1;
      }
  }
private:
  /* uint32_t difSet_[MAX_DIF_NUMBER][MAX_TIME_WORD]; */
  /* uint32_t chamberSet_[MAX_CHAMBER_NUMBER][MAX_TIME_WORD]; */
  /* unsigned char chamberCount_[MAX_TIME_WORD*32]; */
  /* unsigned char difCount_[MAX_TIME_WORD*32]; */
  uint32_t *difSet_;
  uint32_t *chamberSet_;
  unsigned char *chamberCount_;
  unsigned char *difCount_;
  uint32_t max_time_;
};
