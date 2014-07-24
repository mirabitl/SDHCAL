#include "DimDIFServer.h"
#include <unistd.h>
#include <stdint.h>

int main()
{
DimDIFServer* s=new DimDIFServer();
  while (true)
    sleep((unsigned int) 3);
}
