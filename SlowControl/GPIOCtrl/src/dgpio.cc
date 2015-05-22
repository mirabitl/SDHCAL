#include "DimGPIOServer.h"
#include <unistd.h>
#include <stdint.h>

int main()
{
DimGPIOServer* s=new DimGPIOServer();
  while (true)
    sleep((unsigned int) 30);
}
