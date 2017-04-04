#include "MDCCHandler.hh"

MDCCHandler::MDCCHandler(std::string name,uint32_t productid) : _name(name),_productid(productid),_driver(NULL)
{
}
MDCCHandler::~MDCCHandler()
{
  if (_driver!=NULL)
    this->close();
}

void MDCCHandler::open()
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
void MDCCHandler::close()
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

/*
when x"0001" => USB_data_out <= ID_register;
when x"0002" => USB_data_out <= software_veto_register;
when x"0003" => USB_data_out <= spillNb_register;
when x"0004" => USB_data_out <= Control_register;
when x"0005" => USB_data_out <= spillon_register;
when x"0006" => USB_data_out <= spilloff_register;
when x"0007" => USB_data_out <= beam_register;
when x"0008" => USB_data_out <= Calib_register;
when x"0009" => USB_data_out <= Calib_Counter_register;
when x"000A" => USB_data_out <= nb_windows_register;
when x"000B" => USB_data_out <= software_ECALveto_register;
when x"000C" => USB_data_out <= Rstdet_register;
 

when x"0010" => USB_data_out <= busy0Nb_register;
when x"0011" => USB_data_out <= busy1Nb_register;
when x"0012" => USB_data_out <= busy2Nb_register;
when x"0013" => USB_data_out <= busy3Nb_register;
when x"0014" => USB_data_out <= busy4Nb_register;
when x"0015" => USB_data_out <= busy5Nb_register;
when x"0016" => USB_data_out <= busy6Nb_register;
when x"0017" => USB_data_out <= busy7Nb_register;
when x"0018" => USB_data_out <= busy8Nb_register;
when x"0019" => USB_data_out <= busy9Nb_register;
when x"001A" => USB_data_out <= busy10Nb_register;
when x"001B" => USB_data_out <= busy11Nb_register;
when x"001C" => USB_data_out <= busy12Nb_register;
when x"001D" => USB_data_out <= busy13Nb_register;
when x"001E" => USB_data_out <= busy14Nb_register;
when x"001F" => USB_data_out <= busy15Nb_register;

when x"0020" => USB_data_out <= spare0Nb_register;
when x"0021" => USB_data_out <= spare1Nb_register;
 
when x"0100" => USB_data_out <= version;
*/
uint32_t MDCCHandler::version(){return this->readRegister(0x100);}
uint32_t MDCCHandler::id(){return this->readRegister(0x1);}
uint32_t MDCCHandler::mask(){return this->readRegister(0x2);}
void MDCCHandler::maskTrigger(){this->writeRegister(0x2,0x1);}
void MDCCHandler::unmaskTrigger(){this->writeRegister(0x2,0x0);}
uint32_t MDCCHandler::spillCount(){return this->readRegister(0x3);}
void MDCCHandler::resetCounter(){this->writeRegister(0x4,0x1);this->writeRegister(0x4,0x0);}
uint32_t MDCCHandler::spillOn(){return this->readRegister(0x5);}
uint32_t MDCCHandler::spillOff(){return this->readRegister(0x6);}
void MDCCHandler::setSpillOn(uint32_t nc){this->writeRegister(0x5,nc);}
void MDCCHandler::setSpillOff(uint32_t nc){this->writeRegister(0x6,nc);}
uint32_t MDCCHandler::beam(){return this->readRegister(0x7);}
void MDCCHandler::setBeam(uint32_t nc){this->writeRegister(0x7,nc);}
void MDCCHandler::calibOn(){this->writeRegister(0x8,0x2);}
void MDCCHandler::calibOff(){this->writeRegister(0x8,0x0);}
uint32_t MDCCHandler::calibCount(){return this->readRegister(0xa);}
void MDCCHandler::setCalibCount(uint32_t nc){this->writeRegister(0xa,nc);}

void MDCCHandler::reloadCalibCount(){
  
  this->writeRegister(0x8,0x4);
  usleep(2);
  this->writeRegister(0x8,0x0);
  this->calibOn();
}




uint32_t MDCCHandler::ecalmask(){return this->readRegister(0xB);}
void MDCCHandler::maskEcal(){this->writeRegister(0xB,0x1);}
void MDCCHandler::unmaskEcal(){this->writeRegister(0xB,0x0);}
void MDCCHandler::resetTDC(uint8_t b){this->writeRegister(0xC,b);}
uint32_t MDCCHandler::busyCount(uint8_t b){return this->readRegister(0x10+(b&0xF));}



uint32_t MDCCHandler::readRegister(uint32_t adr)
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

void MDCCHandler::writeRegister(uint32_t adr,uint32_t val)
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
