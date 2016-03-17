#include "MDCCReadout.h"

MDCCReadout::MDCCReadout(std::string name,uint32_t productid) : _name(name),_productid(productid),_driver(NULL)
{
}
MDCCReadout::~MDCCReadout()
{
  if (_driver!=NULL)
    this->close();
}

void MDCCReadout::open()
{
  try 
    {
      std::cout<<_name<<" "<<_productid<<std::endl;
      _driver= new FtdiUsbDriver((char*) _name.c_str(),_productid);
    } 
  catch(LocalHardwareException& e)
    {
      LOG4CXX_FATAL(_logCCC," Cannot open "<<_name<<" err="<<e.message());
      return;
    }
  try
    {
      _driver->UsbRegisterRead(0x1,&_version);
      _driver->UsbRegisterRead(0x100,&_id);
    }
  catch(LocalHardwareException& e)
    {
      LOG4CXX_FATAL(_logCCC," Cannot read version and ID ");
      return;
    }
  LOG4CXX_INFO(_logCCC," MDCC "<<_name<<" ID="<<_id<<" version="<<_version);

}
void MDCCReadout::close()
{
  try 
    {
      if (_driver!=NULL)
	delete _driver;
    } 
  catch(LocalHardwareException& e)
    {
      LOG4CXX_FATAL(_logCCC," Cannot delete "<<_name<<" err="<<e.message());
      return;
    }

}
uint32_t MDCCReadout::version(){return this->readRegister(0x100);}
uint32_t MDCCReadout::id(){return this->readRegister(0x1);}
uint32_t MDCCReadout::spillCount(){return this->readRegister(0x3);}
uint32_t MDCCReadout::busyCount(){return this->readRegister(0x5);}
uint32_t MDCCReadout::mask(){return this->readRegister(0x2);}
void MDCCReadout::maskTrigger(){this->writeRegister(0x2,0x1);}
void MDCCReadout::unmaskTrigger(){this->writeRegister(0x2,0x0);}
void MDCCReadout::resetCounter(){this->writeRegister(0x4,0x1);this->writeRegister(0x4,0x0);}
uint32_t MDCCReadout::readRegister(uint32_t adr)
{
  if (_driver==NULL)
    {
       LOG4CXX_ERROR(_logCCC,"Cannot read no driver created ");
       return 0xbad;
    }
  uint32_t rc;
  try
    {
      _driver->UsbRegisterRead(adr,&rc);
    }
  catch(LocalHardwareException& e)
    {
      LOG4CXX_ERROR(_logCCC," Cannot read at adr "<<adr);
      return 0xbad;
    }
  return rc;
}

void MDCCReadout::writeRegister(uint32_t adr,uint32_t val)
{
  if (_driver==NULL)
    {
       LOG4CXX_ERROR(_logCCC,"Cannot write no driver created ");
       return;
    }
  try
    {
      _driver->UsbRegisterWrite(adr,val);
    }
  catch(LocalHardwareException& e)
    {
      LOG4CXX_ERROR(_logCCC," Cannot write at adr "<<adr);
      return;
    }
}
