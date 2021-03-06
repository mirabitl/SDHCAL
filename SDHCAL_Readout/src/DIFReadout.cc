#include "DIFReadout.h"
#include "FtdiUsbDriver.h"
#include <unistd.h>

/* Main program de readout */
DIFReadout::DIFReadout (std::string name,uint32_t productid) : FtdiDIFDriver ((char*) name.c_str(),productid),theName_(name),theAsicType_(2),theNumberOfAsics_(48),theControlRegister_(0x80181B00),	theCurrentSLCStatus_(0),
					    thePwrToPwrARegister_(0x3E8),thePwrAToPwrDRegister_(0x3E6),thePwrDToDAQRegister_(0x4E),theDAQToPwrDRegister_(0x4E),thePwrDToPwrARegister_(0x4E)
{
  sscanf(name.c_str(),"FT101%d",&theDIFId_);
}
DIFReadout::~DIFReadout()
{
  printf("%s %d \n",__PRETTY_FUNCTION__,__LINE__);
}
void DIFReadout::setPowerManagment(uint32_t P2PAReg, uint32_t PA2PDReg,uint32_t PD2DAQReg, uint32_t DAQ2DReg,uint32_t D2AReg)
{

  thePwrToPwrARegister_=P2PAReg;
  thePwrAToPwrDRegister_=PA2PDReg;
  thePwrDToDAQRegister_=PD2DAQReg;
  theDAQToPwrDRegister_=DAQ2DReg;
  thePwrDToPwrARegister_=D2AReg;
	
}
void DIFReadout::setAsicType(uint32_t asicType)
{
  theAsicType_=asicType;
}
void DIFReadout::setNumberOfAsics(uint32_t NumberOfAsics)
{
  theNumberOfAsics_=NumberOfAsics;
}

void DIFReadout::setControlRegister(uint32_t ControlRegister)
{
  theControlRegister_=ControlRegister;
}


void DIFReadout::initialise(uint32_t difid,uint32_t asicType,uint32_t NumberOfAsics,uint32_t ctrlreg,uint32_t P2PAReg, uint32_t PA2PDReg,uint32_t PD2DAQReg, uint32_t DAQ2DReg,uint32_t D2AReg)
{
  LOG4CXX_INFO(_logDIF,"Initialise");
  theDIFId_=difid;
  theAsicType_=asicType;
  theNumberOfAsics_=NumberOfAsics;
  theControlRegister_=ctrlreg;
  thePwrToPwrARegister_=P2PAReg;
  thePwrAToPwrDRegister_=PA2PDReg;
  thePwrDToDAQRegister_=PD2DAQReg;
  theDAQToPwrDRegister_=DAQ2DReg;
  thePwrDToPwrARegister_=D2AReg;
  HardrocFlushDigitalFIFO();	
  theCurrentSLCStatus_=0;
}

void DIFReadout::start()
{
  // Nothing to do yet
  LOG4CXX_INFO(_logDIF,"Start");
  HardrocFlushDigitalFIFO();	
}
void DIFReadout::stop()
{
  LOG4CXX_INFO(_logDIF,"Stop");
  HardrocFlushDigitalFIFO();
  HardrocStopDigitalAcquisitionCommand();
  HardrocFlushDigitalFIFO();
}
void DIFReadout::DoRefreshNbOfASICs()
{
  uint8_t tnbasicsl1=theNumberOfAsics_&0xFF;
  uint8_t tnbasicsl2=(theNumberOfAsics_>>8)&0xFF;
  uint8_t tnbasicsl3=(theNumberOfAsics_>>16)&0xFF;
  uint8_t tnbasicsl4=(theNumberOfAsics_>>24)&0xFF;
  uint8_t tnbasics=tnbasicsl1+tnbasicsl2+tnbasicsl3+tnbasicsl4;
//   printf ("thetheNumberOfAsics__= %d\n",theNumberOfAsics_);
//   printf ("tnbasics= %d\n",tnbasics);
//   printf ("tnbasicsl1= %d\n",tnbasicsl1);
//   printf ("tnbasicsl2= %d\n",tnbasicsl2);
//   printf ("tnbasicsl3= %d\n",tnbasicsl3);
//   printf ("tnbasicsl4= %d\n",tnbasicsl4);
  if (theAsicType_==11)
    {

      try { 
	//printf ("MicroRoc assuming all asics present, tu be completed!!\n");
	NbAsicsWrite(tnbasics,tnbasicsl1,tnbasicsl2,tnbasicsl3,tnbasicsl4);}
      catch (LocalHardwareException& e)	{	std::cout<<__PRETTY_FUNCTION__<<" "<<theName_<<"==>"<< "DIFReadout : Unable to set the correct number of hardrocs"<<std::endl;	
	LOG4CXX_ERROR(_logDIF,"Unable to set the correct number of hardrocs"<<theName_);
      }
      return;}
  else
    {
      //printf ("HR2 theNumberOfAsics_= %d\n",theNumberOfAsics_);
      try {NbAsicsWrite(	tnbasics,tnbasics,0,0,0);}
      catch (LocalHardwareException& e)	{	std::cout<<__PRETTY_FUNCTION__<<" "<<theName_<<"==>"<< "DIFReadout : Unable to set the correct number of hardrocs"<<std::endl;	
	LOG4CXX_ERROR(_logDIF,"Unable to set the correct number of hardrocs"<<theName_);
      }
      return;
    }

	
}


int32_t DIFReadout::DoReadSLCStatus()
{	
  uint32_t CurrentSLCStatus=0;
  int32_t tretry=0;
  while ((CurrentSLCStatus==0) && (tretry<5))
    {
      try {
	//printf ("before read status, tretry=%d\n",tretry);
	//getchar();
	uint32_t rx, tx, ev;
	//readStatus(&rx,&tx,&ev);
	//printf ("%ld %ld %ld\n",rx,rx,ev);
	//getchar();
	HardrocSLCStatusRead(&CurrentSLCStatus);
	//printf ("status = %x\n",CurrentSLCStatus);
			
      }
      catch (LocalHardwareException& e)	{	std::cout<<__PRETTY_FUNCTION__<<" "<<theName_<<"==>"<< "DIFReadout : Unable to send command to DIF : "+(std::string)e.what()<<std::endl;	
	LOG4CXX_ERROR(_logDIF,"Unable to send command to DIF : "<<(std::string)e.what()<<theName_);
      }
      tretry++;
      //      std::cout<<__PRETTY_FUNCTION__<<" "<<theName_<<"==>"<<toolbox::toString("****** 	CurrentSLCStatus = %ld",CurrentSLCStatus)<<std::endl;	
    }
  std::cout<<theName_<<" ";
  theCurrentSLCStatus_ = CurrentSLCStatus;
  if (theAsicType_==2)	
    {
      if ((theCurrentSLCStatus_&0x0003)==0x01) 				std::cout<<"SLC CRC OK       - ";
      else if ((theCurrentSLCStatus_&0x0003)==0x02) 	std::cout<<"SLC CRC Failed   - ";
      else 																 						std::cout<<"SLC CRC forb  - ";
      if ((theCurrentSLCStatus_&0x000C)==0x04) 				std::cout<<"All OK      - ";
      else if ((theCurrentSLCStatus_&0x000C)==0x08) 	std::cout<<"All Failed  - ";
      else 																 						std::cout<<"All forb - ";
      if ((theCurrentSLCStatus_&0x0030)==0x10) 				std::cout<<"L1 OK     - "<<std::endl;
      else if ((theCurrentSLCStatus_&0x0030)==0x20) 	std::cout<<"L1 Failed - "<<std::endl;
      else 																 						std::cout<<"L1 forb   - "<<std::endl;
    }
  else	if (theAsicType_==11)	
    {
      if ((theCurrentSLCStatus_&0x0003)==0x01) 				std::cout<<"SLC CRC OK       - ";
      else if ((theCurrentSLCStatus_&0x0003)==0x02) 	std::cout<<"SLC CRC Failed   - ";
      else 																 						std::cout<<"SLC CRC forb  - ";
      if ((theCurrentSLCStatus_&0x000C)==0x04) 				std::cout<<"All OK      - ";
      else if ((theCurrentSLCStatus_&0x000C)==0x08) 	std::cout<<"All Failed  - ";
      else 																 						std::cout<<"All forb - ";
      if ((theCurrentSLCStatus_&0x0030)==0x10) 				std::cout<<"L1 OK     - ";
      else if ((theCurrentSLCStatus_&0x0030)==0x20) 	std::cout<<"L1 Failed - ";
      else 																 						std::cout<<"L1 forb   - ";
      if ((theCurrentSLCStatus_&0x00C0)==0x40) 				std::cout<<"L2 OK     - ";
      else if ((theCurrentSLCStatus_&0x00C0)==0x80) 	std::cout<<"L2 Failed - ";
      else 																 						std::cout<<"L2 forb   - ";
      if ((theCurrentSLCStatus_&0x0300)==0x0100) 			std::cout<<"L3 OK     - ";
      else if ((theCurrentSLCStatus_&0x0300)==0x0200) std::cout<<"L3 Failed - ";
      else 																 						std::cout<<"L3 forb   - ";
      if ((theCurrentSLCStatus_&0x0C00)==0x0400) 			std::cout<<"L4 OK"<<std::endl;
      else if ((theCurrentSLCStatus_&0x0C00)==0x0800) std::cout<<"L4 Failed"<<std::endl;
      else 																 						std::cout<<"L4 forb -"<<std::endl;
    }

  return CurrentSLCStatus;
}


void DIFReadout::configureRegisters()
{


  //std::cout<<__PRETTY_FUNCTION__<<"----------------------------------------->"<<theTypeOfDIF_<<std::endl;
  //for (uint32_t i=0;i<1;i++) usleep((unsigned int) 1);

  UsbSetDIFID(theDIFId_);
  uint32_t toto;
  GetControlRegister(&toto);
  //	printf ("avant control reg default value = %08lx\n",toto);
  //std::cout <<__PRETTY_FUNCTION__<<"DIFID set "<<std::endl;
  DoRefreshNbOfASICs();


  SetEventsBetweenTemperatureReadout(5);
  SetAnalogConfigureRegister(0xC0054000);
  HardrocFlushDigitalFIFO();	

  //printf("Power %d %d %d %d \n",ds->PowerAnalog,ds->PowerDAC,ds->PowerDigital,ds->PowerADC);
  uint32_t CurrentDIFFwVersion;
  UsbGetFirmwareRevision(&CurrentDIFFwVersion);	
  printf(" DIF %s Version %x - ",theName_.c_str(),CurrentDIFFwVersion);
  DIFCptReset();
  //	printf ("control reg settings to be modified!!!\n");

  uint32_t ttype;
  GetControlRegister(&toto);
  //	printf ("control reg default value = %08lx\n",toto);
  // 	toto = 0x80181B40;// BT 
  //	toto=0x81181B40; // DCC CCC et BT
  //	toto = 0x80181B40;// BT CCC
  //	toto = 0x80181B00;// ILC CCC
  //	  toto = 0x81181B00; // ilc et DCC-CCC
  toto=theControlRegister_;
  //	printf ("control reg new  value = %08lx\n",toto);
  try 
    {
      if (theAsicType_==2)   SetChipTypeRegister(0x100);	
      else if (theAsicType_==11)   SetChipTypeRegister(0x1000);	
    }
  catch (LocalHardwareException& e) { 
    LOG4CXX_ERROR(_logDIF," Unable to set asic type in dif"<<theName_);
std::cout<<__PRETTY_FUNCTION__<<" "<<theName_<<": Unable to set asic type in dif"<<std::endl;throw;}
	
  try 
    {
      GetChipTypeRegister(&ttype);	
      //	    printf ("chiptype=%x\n",ttype);
    }
  catch (LocalHardwareException& e) { 
    LOG4CXX_ERROR(_logDIF," Unable to get asic type in dif"<<theName_);
std::cout<<__PRETTY_FUNCTION__<<" "<<theName_<<": Unable to get asic type in dif"<<std::endl;throw;}
	
	
	
  try 
    {
      if (theAsicType_==2) SetControlRegister(toto);	
      else if (theAsicType_==11) SetControlRegister(toto);	
    }
  catch (LocalHardwareException& e)
    {
      LOG4CXX_ERROR(_logDIF," Unable to send control reg value"<<theName_);
      std::cout<<__PRETTY_FUNCTION__<<" "<<theName_<<": Unable to send control reg value"<<std::endl;
      throw;
    }
  GetControlRegister(&toto);
  printf ("  CtrlReg = 0x%x - ",toto);

  try 
    {
      if (theAsicType_==2) SetPwrToPwrARegister(thePwrToPwrARegister_);	
      else if (theAsicType_==11) SetPwrToPwrARegister(thePwrToPwrARegister_);	
    }
  catch (LocalHardwareException& e)
    {
      LOG4CXX_ERROR(_logDIF,"Unable to send pwr to A reg value"<<theName_);
      std::cout<<__PRETTY_FUNCTION__<<" "<<theName_<<": Unable to send pwr to A reg value"<<std::endl;
      throw;
    }

  try 
    {
      if (theAsicType_==2) SetPwrAToPwrDRegister(thePwrAToPwrDRegister_);	
      else if (theAsicType_==11) SetPwrAToPwrDRegister(thePwrAToPwrDRegister_);	
    }
  catch (LocalHardwareException& e)
    {
      LOG4CXX_ERROR(_logDIF,": Unable to send A to D reg value"<<theName_);
      std::cout<<__PRETTY_FUNCTION__<<" "<<theName_<<": Unable to send A to D reg value"<<std::endl;
      throw;
    }
  try 
    {
      if (theAsicType_==2) SetPwrDToDAQRegister(thePwrDToDAQRegister_);	
      else if (theAsicType_==11) SetPwrDToDAQRegister(thePwrDToDAQRegister_);	
    }
  catch (LocalHardwareException& e)
    {
      LOG4CXX_ERROR(_logDIF,": Unable to send D to Daq reg value"<<theName_);
      std::cout<<__PRETTY_FUNCTION__<<" "<<theName_<<": Unable to send D to Daq reg value"<<std::endl;
      throw;
    }
  try 
    {
      if (theAsicType_==2) SetDAQToPwrDRegister(theDAQToPwrDRegister_);	
      else if (theAsicType_==11) SetDAQToPwrDRegister(theDAQToPwrDRegister_);	
    }
  catch (LocalHardwareException& e)
    {
      LOG4CXX_ERROR(_logDIF,": Unable to send daq to D reg value"<<theName_);
      std::cout<<__PRETTY_FUNCTION__<<" "<<theName_<<": Unable to send daq to D reg value"<<std::endl;
      throw;
    }
  try 
    {
      if (theAsicType_==2) SetPwrDToPwrARegister(thePwrDToPwrARegister_);	
      else if (theAsicType_==11) SetPwrDToPwrARegister(thePwrDToPwrARegister_);	
    }
  catch (LocalHardwareException& e)
    {
      LOG4CXX_ERROR(_logDIF,": Unable to send D to A reg value"<<theName_);
      std::cout<<__PRETTY_FUNCTION__<<" "<<theName_<<": Unable to send D to A reg value"<<std::endl;
      throw;
    }

}


void DIFReadout::DoSendDIFTemperatureCommand()
{
  //std::cout<<"Usb do send dif temperature command"<<std::endl;
  try	{	HardrocCommandAskDifTemperature();	}
  catch (LocalHardwareException& e)  {  
    LOG4CXX_ERROR(_logDIF,"Unable to send dif temperature command"<<theName_);

}	
  return;
}

void DIFReadout::DoSendASUTemperatureCommand()
{
  //std::cout<<"Usb do send asu temperature command"<<std::endl;
  try	{	HardrocCommandAskAsuTemperature();	}
  catch (LocalHardwareException& e)  {  LOG4CXX_ERROR(_logDIF," "<<"==>"<< "DIFReadout : Unable to send asu temperature command");	}	
  return;
}

void DIFReadout::DoGetASUTemperatureCommand(uint32_t *ttemp1,uint32_t *ttemp2)
{
  //	std::cout<<"Usb do get asu temperature command"<<std::endl;
  try	{	GetASUTemperature(ttemp1,ttemp2);	}
  catch (LocalHardwareException& e)  {  LOG4CXX_ERROR(_logDIF," "<<"==>"<< "DIFReadout : Unable to send asu temperature command");	}	
  //	printf ("DIFReadout ASUTemp = %d %d \n",*ttemp1,*ttemp2);
  theTemperatureBuffer_[1]=(*ttemp1)*1.0;
  theTemperatureBuffer_[2]=(*ttemp2)*1.0;
  return;
}

void DIFReadout::DoGetDIFTemperatureCommand(uint32_t *ttemp)
{
  //	std::cout<<"Usb do get dif temperature command"<<std::endl;
  try	{	GetDIFTemperature(ttemp);	}
  catch (LocalHardwareException& e)  {  LOG4CXX_ERROR(_logDIF," "<<"==>"<< "DIFReadout : Unable to send asu temperature command");	}	
  //	printf ("DIFReadout DIFTemp = %d \n",*ttemp);
  theTemperatureBuffer_[0]=(*ttemp)*1.0;
  return;
}

void DIFReadout::DoSetTemperatureReadoutToAuto(uint32_t tvalue)
{
  uint32_t ttemp;
  //	std::cout<<"Usb do set dif temperature mode "<<tvalue<<std::endl;
  try	{	SetTemperatureReadoutToAuto(tvalue);	}
  catch (LocalHardwareException& e)  {  LOG4CXX_ERROR(_logDIF," "<<"==>"<< "DIFReadout : Unable to send asu temperature mode");	}	
  return;
}

void DIFReadout::DoGetTemperatureReadoutAutoStatus(uint32_t *tvalue)
{
  uint32_t ttemp;
  //	std::cout<<"Usb do get dif temperature mode "<<tvalue<<std::endl;
  try	{	GetTemperatureReadoutAutoStatus(tvalue);	}
  catch (LocalHardwareException& e)  {  LOG4CXX_ERROR(_logDIF," "<<"==>"<< "DIFReadout : Unable to send asu temperature mode");	}	
  //	printf("DIFReadout Temperature status =/n",*tvalue);
  return;
}

void DIFReadout::DoSetEventsBetweenTemperatureReadout(uint32_t tvalue)
{
  uint32_t ttemp;
  //	std::cout<<"Usb do set dif temperature mode "<<tvalue<<std::endl;
  try	{	SetEventsBetweenTemperatureReadout(tvalue);	}
  catch (LocalHardwareException& e)  {  LOG4CXX_ERROR(_logDIF," "<<"==>"<< "DIFReadout : Unable to set temperature readoutfrequency");	}	
  return;
}

void DIFReadout::DoSetAnalogConfigureRegister(uint32_t tdata)    
{
  uint32_t ttemp;
  //	std::cout<<"Usb do set dif temperature mode "<<tvalue<<std::endl;
  try	{	SetAnalogConfigureRegister(tdata);	}
  catch (LocalHardwareException& e)  {  LOG4CXX_ERROR(_logDIF," "<<"==>"<< "DIFReadout : Unable to set analog  conf register");	}	
  return;
}

static unsigned char vframe[HARDROCV2_SLC_FRAME_SIZE]; 
int32_t DIFReadout::configureChips(SingleHardrocV2ConfigurationFrame* slow) throw( LocalHardwareException ) 
{
  // send configure command
  unsigned short tCRC;
  unsigned char CurrentCRC[2];
  uint32_t framesize=0;
  // set default register values
  uint32_t regctrl;
//   UsbRegisterRead(2,&regctrl);
//   printf("Apres test  %x \n",regctrl);
//   getchar();
//   UsbRegisterRead(0,&regctrl);
//   printf("Apres Reg0  %x \n",regctrl);
//   getchar();
  if (theAsicType_==2) framesize = HARDROCV2_SLC_FRAME_SIZE;
  else if (theAsicType_==11) framesize = MICROROC_SLC_FRAME_SIZE;
  else 	std::cout <<"Bad Asic type"<<std::endl;

  //UsbRegisterWrite2(0,0x100);
  tCRC=0xFFFF;							// initial value of the CRC
  //printf ("avant HardrocCommandSLCWrite\n");
  //getchar();
  try 
    {
      HardrocCommandSLCWrite();	
    }
  catch (LocalHardwareException& e)
    {
      std::cout<< theName_<<": Unable to send start SLC command to DIF"<<std::endl;
      throw e;
    }
  // printf("avant Frame \n");
  //getchar();

  for (int tAsic=theNumberOfAsics_;tAsic>0;tAsic--)
    {
      //      std::cout<<"Configuring "<<tAsic<<std::endl;
      //unsigned char vframe[framesize];
      for (int tbyte=0;tbyte<framesize;tbyte++)
	{
	
//slow[tAsic-1][tbyte]=0x40;//tbyte + (((tAsic-1)&0x01)<<7);


	  printf("%02x",slow[tAsic-1][tbyte] );
	  vframe[tbyte]=slow[tAsic-1][tbyte];
	}
      printf("\n");

      for (int tbyte=0;tbyte<framesize;tbyte++) 
	{
	  tCRC=((tCRC&0xFF)<<8) ^ ( FtdiUsbDriver::CrcTable[ (tCRC>>8)^slow[tAsic-1][tbyte] ] );
	} 
      try 
	{
	  //HardrocCommandSLCWriteSingleSLCFrame(&(theSlowBuffer_[tAsic-1][0]));
	  if (theAsicType_==2)	CommandSLCWriteSingleSLCFrame(vframe,framesize);
	  if (theAsicType_==11)	CommandSLCWriteSingleSLCFrame(vframe,framesize);
	  //printf("apres Frame %d \n",framesize);
	  // getchar();
			
	}
      catch (LocalHardwareException& e)
	{
	  LOG4CXX_ERROR(_logDIF," "<<":Unable to send SLC frame to DIF");	
	  throw e;
	}

    }	//for (int tAsic=NbOfASICs;tAsic>0;tAsic--)
  //  printf("avant CRC\n");
  // getchar();

  CurrentCRC[0]= tCRC>>8;				// MSB first
  CurrentCRC[1]=tCRC&0xFF;
  try 
    {	
      HardrocCommandSLCWriteCRC(&CurrentCRC[0]);
    }
  catch (LocalHardwareException& e) 
    {
      LOG4CXX_ERROR(_logDIF," "<<":Unable to send CRC");	
      throw;
    }

  //  checkReadWrite(0x100,10);
  //printf("avant Test\n");
  //getchar();
  //UsbRegisterRead(2,&regctrl);
  //printf("Avant SLCStatus  %x \n",regctrl);
  //getchar();

  usleep(400000);// was 500 ms
  //  int tretry=0;
  //  printf ("before slc status\n");
  theCurrentSLCStatus_=this->DoReadSLCStatus();

  return theCurrentSLCStatus_;
}


uint32_t DIFReadout::DoHardrocV2ReadoutDigitalData(unsigned char* CurrentDIFDigitalData)
{

  unsigned int ComputedCRC=0xFFFF;
  unsigned int ReceivedCRC=0xFFFF;
  unsigned char tdata;
  unsigned char tdata32[42];
  unsigned char tdataana[MAX_ANALOG_DATA_SIZE];
  int tHardrocIndex=0;
  int tMemoryIndex=0;
  int tBunchIndex=0;
  int tindex=0;
  double tefficacity=0;
  int tDIFIDIndex=0;	
  char tdisplay[600];			
  unsigned  tCurrentAbsoluteBCIDMSB;
  uint32_t tCurrentAbsoluteBCIDLSB;

  memset(CurrentDIFDigitalData,0,MAX_EVENT_SIZE);
  uint16_t theadersize;
  // on calcule le CRC a la volee			
  try	{	 UsbReadByte(&tdata); }  // Global header
  catch (LocalHardwareException& e)	  {	 
    LOG4CXX_DEBUG(_logDIF," "<<"==>"<<"DIF : no data, exiting "<<e.what()); 
    return 0;}	
  uint32_t CurrentNbOfEventsPerTrigger=0;

  //  printf ("%02x",tdata);

  if ((tdata != 0xB0) && (tdata != 0xBB))		// global header
    {	
      LOG4CXX_ERROR(_logDIF," "<<"==>"<<"DIF : Bad global header("<<std::hex<< (int) tdata<<std::dec<<" instead of 0xb0 or 0xbb), exiting");	
      HardrocFlushDigitalFIFO();
      return 0;
    }	
  tindex=0;

  CurrentDIFDigitalData[tindex++]=tdata;		// global header
  ComputedCRC=((ComputedCRC&0xFF)<<8) ^ ( FtdiUsbDriver::CrcTable[ ((ComputedCRC>>8)^(tdata&0xFF))&0xFF ] );			// global header

  tDIFIDIndex=tindex;
  // DIF ID  			
  //DIF trigger counter  (32 bits)	
  //acq trigger counter  (32 bits)		
  //global trigger counter  (32 bits)
  //absolute bcid counter (48bits, natural binary, MSB first) 
  //timedif counter (24bits, natural binary, MSB first)
  if ( tdata ==0xb0) theadersize=23; 
  else theadersize = 32;

  try	{	    UsbReadnBytes(tdata32,theadersize); 	  }  
  catch (LocalHardwareException& e)
    {
      LOG4CXX_ERROR(_logDIF," "<<"==> no DIF Header");	
      HardrocFlushDigitalFIFO();
      return 0;
    }
	
  for (int ti=0;ti<theadersize;ti++)
    {
      //printf ("%02x",tdata32[ti]);
      CurrentDIFDigitalData[tindex++]=tdata32[ti];
      ComputedCRC=((ComputedCRC&0xFF)<<8) ^ ( FtdiUsbDriver::CrcTable[ ((ComputedCRC>>8)^(tdata32[ti]&0xFF))&0xFF ] );		// dif_id
    }
  //printf ("\n");
  uint32_t CurrentDifId = tdata32[0];
  // printf ("DIF = %d\n",CurrentDifId);
  if (CurrentDifId!=theDIFId_)
    {
      LOG4CXX_ERROR(_logDIF," "<<"==>"<<"DIF "<<CurrentDifId<<" :  Invalid  DIF ID "<<theDIFId_);	
      HardrocFlushDigitalFIFO();
      return 0;
    }
  CurrentDIFDigitalData[tDIFIDIndex]=CurrentDifId;

  uint32_t CurrentDIFTriggerCounter=tdata32[1]<<24;
  CurrentDIFTriggerCounter+=tdata32[2]<<16;
  CurrentDIFTriggerCounter+=tdata32[3]<<8;
  CurrentDIFTriggerCounter+=tdata32[4];
  //  std::cout<<CurrentDIFTriggerCounter<<" is read"<<std::endl;
  uint32_t CurrentAcqTriggerCounter=tdata32[5]<<24;
  CurrentAcqTriggerCounter+=tdata32[6]<<16;
  CurrentAcqTriggerCounter+=tdata32[7]<<8;
  CurrentAcqTriggerCounter+=tdata32[8];
	
  uint32_t CurrentGlobalTriggerCounter=tdata32[9]<<24;
  CurrentGlobalTriggerCounter+=tdata32[10]<<16;
  CurrentGlobalTriggerCounter+=tdata32[11]<<8;
  CurrentGlobalTriggerCounter+=tdata32[12];

  tCurrentAbsoluteBCIDMSB=tdata32[13]<<8;
  tCurrentAbsoluteBCIDMSB+=tdata32[14];

  tCurrentAbsoluteBCIDLSB=tdata32[15]<<24;
  tCurrentAbsoluteBCIDLSB+=tdata32[16]<<16;
  tCurrentAbsoluteBCIDLSB+=tdata32[17]<<8;
  tCurrentAbsoluteBCIDLSB+=tdata32[18];

  uint32_t CurrentTimeDif=tdata32[19]<<16;
  CurrentTimeDif+=tdata32[20]<<8;
  CurrentTimeDif+=tdata32[21];
	
  uint32_t CurrentNbLines=tdata32[22]>>4;
  //std::cout<<"CurrentNbLines="<<CurrentNbLines<<std::endl;
  uint32_t CurrentTemperatureASU1;
  uint32_t CurrentTemperatureASU2;
  uint32_t CurrentTemperatureDIF;
  if ( tdata ==0xbb) 
    {
      CurrentTemperatureASU1=tdata32[23]<<24;
      CurrentTemperatureASU1+=tdata32[24]<<16;
      CurrentTemperatureASU1+=tdata32[25]<<8;
      CurrentTemperatureASU1+=tdata32[26];

      CurrentTemperatureASU2=tdata32[27]<<24;
      CurrentTemperatureASU2+=tdata32[28]<<16;
      CurrentTemperatureASU2+=tdata32[29]<<8;
      CurrentTemperatureASU2+=tdata32[30];

      CurrentTemperatureDIF=tdata32[31];
		
      //		printf ("Temps = %d %d %d \n",CurrentTemperatureDIF,CurrentTemperatureASU1,CurrentTemperatureASU2);
    }

  uint32_t CurrentHardrocHeader=0;
  int tcontinue=1;
  while (tcontinue)
    {	
      try	
	{	
	  UsbReadByte(&tdata);	
	}//frame hearder or Global trailer 
      catch (LocalHardwareException& e)
	{
	  LOG4CXX_ERROR(_logDIF," "<<"==>"<<"DIF" << theDIFId_<<":  There should be a frame header/global trailer");	
	  HardrocFlushDigitalFIFO();
	  return 0;
	}	
      CurrentDIFDigitalData[tindex++]=tdata;
      uint32_t theader=tdata;

      if (theader==0xC4) 				  // analog frame header
	{	
	  for (uint32_t tl=0;tl<CurrentNbLines;tl++)	
	    {
	      try {  UsbReadByte(&tdata);	}//nb of chips on line tl 
	      catch (LocalHardwareException& e){std::cout<<"DIF %s :  There should be number of chip on line %d"<<theDIFId_<<" "<<tl<<std::endl;	HardrocFlushDigitalFIFO();	return 0;}	
				
	      CurrentDIFDigitalData[tindex++]=tdata;		
	      uint32_t tanasize = tdata*64*2;
	      if (tdata>12) printf ("erreur taille data analogiques = %d\n",tdata);
	      try	{	    UsbReadnBytes(tdataana,tanasize); 	  }  
	      catch (LocalHardwareException& e){std::cout<<"DIF %s :  There should analog data"<<theDIFId_<<std::endl;	HardrocFlushDigitalFIFO();return 0;}
	      for (uint32_t ti=0;ti<tanasize;ti++)
		{
		  CurrentDIFDigitalData[tindex++]=tdataana[ti];
		}
	    } //for (uint32_t tl=0;tl<	CurrentNbLines;tl++)				
	  try {	 UsbReadByte(&tdata);	}//0xD4 
	  catch (LocalHardwareException& e){
	    LOG4CXX_ERROR(_logDIF,"DIF :  There should be a 0xD4"<<theName_);	
	    std::cout<< __FILE__<<" "<< __LINE__<<" "<< __FUNCTION__ <<" "<<e.what()<<std::endl;
	    HardrocFlushDigitalFIFO();return 0;}	
	  //			printf ("%02x\n",tdata);
	  CurrentDIFDigitalData[tindex++]=tdata;		//0xD4
	}
      else if (theader==0xB4) 				  // frame header
	{
	  ComputedCRC=((ComputedCRC&0xFF)<<8) ^ ( FtdiUsbDriver::CrcTable[ ((ComputedCRC>>8)^(theader&0xFF))&0xFF ] );			// B4 in crc but not C4
	  while (1)
	    {	
	      try {	UsbReadByte(&tdata);	}
	      catch (LocalHardwareException& e){
		std::cout<< __FILE__<<" "<< __LINE__<<" "<< __FUNCTION__ <<" "<<e.what()<<std::endl;

		LOG4CXX_ERROR(_logDIF," There should be a valid frame trailer/hardroc header"<<theName_); 
		HardrocFlushDigitalFIFO();return 0;}	
	      if ((tdata != 0xA3)&&(tdata != 0xC3))		//not a frame trailer, so a hardroc header
		{	
		  if (tMemoryIndex>ASIC_MEM_DEPTH)
		    {
		      std::cout<< __FILE__<<" "<< __LINE__<<" "<< __FUNCTION__ <<" unable to read more than 128 Memory indexes"<<std::endl;

		      LOG4CXX_ERROR(_logDIF," unable to read more than 128 Memory indexes"<<theName_);	
		      HardrocFlushDigitalFIFO();
		      return 0;
		    }		
		  ComputedCRC=((ComputedCRC&0xFF)<<8) ^ ( FtdiUsbDriver::CrcTable[ ((ComputedCRC>>8)^(tdata&0xFF))&0xFF ] );			// hardroc header
		  CurrentDIFDigitalData[tindex++]=tdata;
		  CurrentHardrocHeader=tdata;
		  CurrentNbOfEventsPerTrigger++;
		  try {	 UsbReadnBytes(tdata32,19);	}  // BCID
		  catch (LocalHardwareException& e){
		    std::cout<< __FILE__<<" "<< __LINE__<<" "<< __FUNCTION__ <<" "<<e.what()<<std::endl;

		    LOG4CXX_ERROR(_logDIF," There should be a hardroc frame"<<theName_);		
		    HardrocFlushDigitalFIFO();return 0;}	
		  // bcid (3 bytes)
		  // data (16bytes)	
		  //printf("ASIC %d :",tdata);
		  for (int ti=0;ti<19;ti++)
		    {
		      // printf("%02x",tdata32[ti]);
		      CurrentDIFDigitalData[tindex++]=tdata32[ti];
		      ComputedCRC=((ComputedCRC&0xFF)<<8) ^ ( FtdiUsbDriver::CrcTable[ ((ComputedCRC>>8)^(tdata32[ti]&0xFF))&0xFF ] );		
		    }
		  //printf("\n");
		  tMemoryIndex++;
		  if( tMemoryIndex>ASIC_MEM_DEPTH) 
		    {
		      std::cout<< __FILE__<<" "<< __LINE__<<" "<< __FUNCTION__ <<" tMemoryIndex > ASIC_MEM_DEPTH"<<std::endl;

		      LOG4CXX_ERROR(_logDIF,"tMemoryIndex > ASIC_MEM_DEPTH"<<theName_);			      
		      return 0;
		    }
		}
	      else //if ((tdata != 0xA3)&&(tdata != 0xC3))
		{			//frame trailer	
		  if (tdata==0xC3)	
		    {
		      std::cout<< __FILE__<<" "<< __LINE__<<" "<< __FUNCTION__ <<"Incomplete frame received (0xC3) "<<std::endl;

		      LOG4CXX_ERROR(_logDIF," Incomplete frame received (0xC3)"<<theName_);			      
		      HardrocFlushDigitalFIFO();
		      return 0;
		    }	
		  ComputedCRC=((ComputedCRC&0xFF)<<8) ^ ( FtdiUsbDriver::CrcTable[ ((ComputedCRC>>8)^(tdata&0xFF))&0xFF ] );		// frame trailer
		  CurrentDIFDigitalData[tindex++]=tdata;
		  tMemoryIndex=0;			// next hardroc, so mem index should be resetedSBad glo
		  tHardrocIndex++;
		  if( tHardrocIndex>MAX_NB_OF_ASICS+1)		    {
		      std::cout<< __FILE__<<" "<< __LINE__<<" "<< __FUNCTION__ <<" tHardrocIndex= "<< tHardrocIndex<<" > MAX_NB_OF_ASICS"<<std::hex<<(int) tdata <<std::dec<<std::endl;

		      for (int ic=0;ic<tindex;ic++)
			printf("%02x",CurrentDIFDigitalData[ic]);
		      printf("\n");

		      //LOG4CXX_ERROR(_logDIF," "<<"==>"<<toolbox::toString("tHardrocIndex > MAX_NB_OF_ASICS"));	
						
		      HardrocFlushDigitalFIFO();
		      return 0;
		    }
		  break;
		} //if ((tdata != 0xA3)&&(tdata != 0xC3))
	    }	//while (1)
	}	//if (tdata==0xB4)
      else if (tdata == 0xA0) 		// global trailer	
	{	
	  try	{	  UsbRead2Bytes(&ReceivedCRC);	}		// CRC
	  catch (LocalHardwareException& e)
	    {
	      std::cout<< __FILE__<<" "<< __LINE__<<" "<< __FUNCTION__ <<" "<<e.what()<<std::endl;

	      LOG4CXX_ERROR(_logDIF," There should be a valid CRC"<<theName_);		     
	      HardrocFlushDigitalFIFO();
	      return 0;
	    }	
	  CurrentDIFDigitalData[tindex++]=(ReceivedCRC>>8)&0xFF;
	  CurrentDIFDigitalData[tindex++]=(ReceivedCRC>>0)&0xFF;
			
	  if (ComputedCRC == ReceivedCRC)
	    {	
	      tcontinue =0;
	    }	
	  else
	    {			
#ifdef DISPLAY_CRC
	      LOG4CXX_ERROR(_logDIF," CRC mismatch ( received"<<ReceivedCRC<<" instead "<<ComputedCRC<<" "<<theName_);	
#endif
	      tcontinue =0;
	    }
	}	//else if (tdata == 0xA0)
    }	//while (tcontinue) 
	
	/*
	  printf ("\n");
	  for (int i=0;i<tindex;i++)
	  printf ("%02x", CurrentDIFDigitalData[i]);
	  printf ("\n");
	*/
	//this->append(CurrentDIFDigitalData,tindex);
  return tindex;
}

