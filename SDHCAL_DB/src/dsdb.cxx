
#include "WebDIFDBServer.h"
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>

int main()
{
  char hostname[256];
  memset(hostname,0,256);
  gethostname(hostname,256);
  for (uint32_t i=0;i<256;i++)
    printf("%s on demarre \n",hostname);
  WebDIFDBServer* theDIFs= new WebDIFDBServer(hostname,7000);
  while (true)
    sleep((unsigned int) 3);
}
