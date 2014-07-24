#include "DimCCCServer.h"
#include <unistd.h>
#include <stdint.h>

int main()
{
DimCCCServer* s=new DimCCCServer();
  while (true)
    sleep((unsigned int) 3);
}
