#include "DimDaqControl.h"
#include <unistd.h>
#include <stdint.h>

int main()
{
  DimDaqControl* s=new DimDaqControl("lyopc252");

  s->scandns();
  getchar();
  s->scan();
  getchar();
  s->initialise();
  getchar();
  s->print();
  getchar();
  s->destroy();
}
