#include "MDCCReadout.h"
#include <iostream>
#include <stdio.h>
int main()
{
  std::string dev="MDCC01";
  MDCCReadout m(dev,0x6001);
  m.open();

  m.maskTrigger();
  m.resetCounter();
  printf(" After reset+unmask vers %d id %d count %d : %d  mask %d \n",m.version(),m.id(),m.spillCount(),m.busyCount(),m.mask());
  getchar();
  m.unmaskTrigger();

  for (int i=0;i<120;i++)
    {
      printf(" After %d s vers %d id %d count %d : %d  mask %d \n",i,m.version(),m.id(),m.spillCount(),m.busyCount(),m.mask());

      sleep((unsigned int) 1);
    }

  m.maskTrigger();
  getchar();
  printf(" After mask vers %d id %d count %d : %d  mask %d \n",m.version(),m.id(),m.spillCount(),m.busyCount(),m.mask());
  getchar();
  m.unmaskTrigger();
 
 
}
