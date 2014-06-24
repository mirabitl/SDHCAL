#include "DimZupServer.h"
#include <unistd.h>
#include <stdint.h>

int main()
{
DimZupServer* s=new DimZupServer();
  while (true)
    sleep((unsigned int) 30);
}
