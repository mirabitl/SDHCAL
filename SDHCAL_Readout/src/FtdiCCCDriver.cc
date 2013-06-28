/************************************************************************/
/* ILC test beam daq							*/	
/* C. Combaret								*/
/* V 1.0								*/
/* first release : 30-01-2008						*/
/* revs : 								*/
/************************************************************************/
#include "FtdiCCCDriver.h"
#include <iostream>
#include <sstream>
#include <sys/timeb.h>


#define MY_DEBUG

FtdiCCCDriver::FtdiCCCDriver(char * deviceIdentifier )     throw (LocalHardwareException) : FtdiUsbDriver(deviceIdentifier) 
{


}

FtdiCCCDriver::~FtdiCCCDriver()     throw (LocalHardwareException)

{
  std::cout<<"destructor called"<<std::endl;
  
 
}	



int32_t FtdiCCCDriver :: CCCCommandDIFReset(void)    throw (LocalHardwareException)
{
  uint32_t taddress=0x00;
	
  try	{		UsbCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	

int32_t FtdiCCCDriver :: CCCCommandBCIDReset(void)    throw (LocalHardwareException)
{
  uint32_t taddress=0x01;
	
  try	{		UsbCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	

int32_t FtdiCCCDriver :: CCCCommandStartAcquisitionAuto(void)    throw (LocalHardwareException)
{
  uint32_t taddress=0x02;
	printf ("sdcc send start acq\n");
  try	{		UsbCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	

int32_t FtdiCCCDriver :: CCCCommandRamfullExt(void)    throw (LocalHardwareException)
{
  uint32_t taddress=0x03;
	
  try	{		UsbCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	

int32_t FtdiCCCDriver :: CCCCommandTriggerExt(void)    throw (LocalHardwareException)
{
  uint32_t taddress=0x04;
	
  try	{		UsbCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	


int32_t FtdiCCCDriver :: CCCCommandStopAcquisition(void)    throw (LocalHardwareException)
{
  uint32_t taddress=0x05;
	
  try	{		UsbCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	

int32_t FtdiCCCDriver :: CCCCommandDigitalReadout(void)    throw (LocalHardwareException)
{
  uint32_t taddress=0x06;
	
  try	{		UsbCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	

int32_t FtdiCCCDriver :: CCCCommandTrigger(void)    throw (LocalHardwareException)
{
  uint32_t taddress=0x07;
	
  try	{		UsbCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	

int32_t FtdiCCCDriver :: CCCCommandSpillOn(void)    throw (LocalHardwareException)
{
  uint32_t taddress=0x0C;
	
  try	{		UsbCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	

int32_t FtdiCCCDriver :: CCCCommandSpillOff(void)    throw (LocalHardwareException)
{
  uint32_t taddress=0x0D;
	
  try	{		UsbCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	

int32_t FtdiCCCDriver :: CCCCommandClearMemory(void)    throw (LocalHardwareException)
{
  uint32_t taddress=0x08;
	printf (" Not implemented anymore\n");
  return -2;
}	

int32_t FtdiCCCDriver :: CCCCommandStartSingleAcquisition(void)    throw (LocalHardwareException)
{
  uint32_t taddress=0x09;
	
  try	{		UsbCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	

int32_t FtdiCCCDriver :: CCCCommandPulseLemo(void)    throw (LocalHardwareException)
{
  uint32_t taddress=0x0A;
	
  try	{		UsbCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	

int32_t FtdiCCCDriver :: CCCCommandRazChannel(void)    throw (LocalHardwareException)
{
  uint32_t taddress=0x0B;
	
  try	{		UsbCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	
/*
int32_t FtdiCCCDriver :: CCCCommandNoCommand(void)    throw (LocalHardwareException)
{
  uint32_t taddress=0x0E;
	
  try	{		CCCCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	
*/
int32_t FtdiCCCDriver :: CCCCommandResetCCC(void)    throw (LocalHardwareException)
{
  uint32_t taddress=0x0F;
	
  try	{		UsbCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	

