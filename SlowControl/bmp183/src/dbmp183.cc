#include "DimBMP183Server.h"
#include <unistd.h>
#include <stdint.h>

int main()
{
DimBMP183Server* s=new DimBMP183Server();
  while (true)
	{	
		s->getTemperature();
		s->getPression();
		
    sleep((unsigned int) 1);
	}	
}
