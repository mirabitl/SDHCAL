#include "DimDaqControl.h"
#include <unistd.h>
#include <stdint.h>

int main()
{
  DimDaqControl* c=new DimDaqControl("lyopc252");
  DimDaqControlServer* s=new DimDaqControlServer(c);
  //s->scandns();
  //getchar();
  //s->scan();
  //getchar();
  //s->initialise();
  while (true)
    sleep((unsigned int) 3);
}
