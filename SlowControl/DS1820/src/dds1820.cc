#include "DimDS1820Server.h"
#include <unistd.h>
#include <stdint.h>

int main()
{
DimDS1820Server* s=new DimDS1820Server();
  while (true)
	{	
		s->getTemperature();
    sleep((unsigned int) 1);
	}	
}
