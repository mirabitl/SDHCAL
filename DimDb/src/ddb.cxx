#include "DimDbServer.h"
#include <unistd.h>
#include <stdint.h>

int main()
{
DimDbServer* s=new DimDbServer();
  while (true)
    sleep((unsigned int) 3);
}
