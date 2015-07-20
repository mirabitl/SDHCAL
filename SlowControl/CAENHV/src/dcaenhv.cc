#include "DimCAENHVServer.h"
#include <unistd.h>
#include <stdint.h>

int main()
{
DimCAENHVServer* s=new DimCAENHVServer();
  while (true)
	{	
	  //s->getTemperature();
	  //	s->getPression();
		
    sleep((unsigned int) 1);
	}	
}
