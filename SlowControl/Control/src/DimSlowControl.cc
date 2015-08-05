
#include "DimSlowControl.h"
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <string.h>



DimSlowControl::DimSlowControl(std::string dns) :theDNS_(dns)  
{
 
}
DimSlowControl::~DimSlowControl()
{
 
  
}

void DimSlowControl::scandns()
{
  // Look for DB server
  DimBrowser* dbr=new DimBrowser(); 
  char *service, *format; 
  int type;
  // Get DB service
  cout<<"On rentre dans scandns "<<endl;

  char *server,*node;
  dbr->getServers( ); 
  cout<<"On sort \n";
  while(dbr->getNextServer(server, node)) 
    { 
      cout << server << " @ " << node << endl; 
    }
  cout<<"0"<<endl;
  theCAENHVPrefix_="";
  dbr->getServices("/CAENHV/*/CHANNEL" ); 
  cout<<"1\n";
  while(type = dbr->getNextService(service, format)) 
    { 
      cout << service << " -  " << format << endl; 
      std::string ss;
      ss.assign(service);
      size_t n=ss.find("/CHANNEL");
      cout<<ss.substr(0,n)<<endl;
      theCAENHVPrefix_=ss.substr(0,n);
      CAENHVInfo_=new DimInfo(ss.c_str(),&theCAENHVChannel_,sizeof(HVMONDescription),this);
    } 
  // Get the CCC prefix
  cout<<"2\n";
  theBMP183Prefix_="";
    
  dbr->getServices("/BMP183/*/PRESSIONREADVALUES" ); 
  while(type = dbr->getNextService(service, format)) 
    { 
      cout << service << " -  " << format << endl; 
      std::string ss;
      ss.assign(service);
      size_t n=ss.find("/PRESSIONREADVALUES");
      theBMP183Prefix_=ss.substr(0,n);
    }
  std::stringstream ss0;ss0.str(std::string());
  ss0<<theBMP183Prefix_<<"/PRESSIONREADVALUES";
  BMP183PressureInfo_=new DimInfo(ss0.str().c_str(),theBMP183Pressure_,this);
  ss0.str(std::string());
  ss0<<theBMP183Prefix_<<"/TEMPERATUREREADVALUES";
  BMP183TemperatureInfo_=new DimInfo(ss0.str().c_str(),theBMP183Temperature_,this);

    
  theDS1820Prefix_="";
  dbr->getServices("/DTEMP/*/READVALUES" ); 
  while(type = dbr->getNextService(service, format)) 
    { 
      cout << service << " -  " << format << endl; 
      std::string ss;
      ss.assign(service);
      size_t n=ss.find("/READVALUES");
      theDS1820Prefix_=ss.substr(0,n);
      DS1820Info_=new DimInfo(ss.c_str(),theDS1820Temperatures_,2*sizeof(float),this);
 
    }
  

  theGPIOPrefix_="";
  dbr->getServices("/DGPIO/*/DOREAD" ); 
  while(type = dbr->getNextService(service, format)) 
    { 
      cout << service << " -  " << format << endl; 
      std::string ss;
      ss.assign(service);
      size_t n=ss.find("/DOREAD");
      theGPIOPrefix_=ss.substr(0,n);
    } 

  runInfo_=new DimInfo("/DB/RUNFROMDB",theCurrentRun_,this);
  dbstateInfo_=new DimInfo("/DB/DBSTATE",theCurrentState_,this);

}
int DimSlowControl::getCurrentRun() { return theCurrentRun_;}
char* DimSlowControl::getCurrentState(){return theCurrentState_;}
uint32_t DimSlowControl::getCurrentHVChannel(){return theCAENHVChannel_.getHVCHANNEL();}
float DimSlowControl::getCurrentHVVoltageSet(){return theCAENHVChannel_.getVSET();}
float DimSlowControl::getCurrentHVVoltageRead(){return theCAENHVChannel_.getVMON();}
uint32_t DimSlowControl::getCurrentHVCurrentRead(){return theCAENHVChannel_.getIMON();}
uint32_t DimSlowControl::getCurrentHVStatus(){return theCAENHVChannel_.getSTATUS();}
float DimSlowControl::getDS1820Temperature(uint32_t i){return theDS1820Temperatures_[i];}
float DimSlowControl::getBMP183Temperature(){return theBMP183Temperature_;}
float DimSlowControl::getBMP183Pressure(){return theBMP183Pressure_;}
void DimSlowControl::infoHandler()
{
  DimInfo *curr = getInfo(); // get current DimInfo address 
  if (curr->getSize()==1) return;
  if (curr==runInfo_)
    {
      theCurrentRun_=curr->getInt();
      //std::cout<<"The current Run is "<<theCurrentRun_<<std::endl;
      return;
    }
  if (curr==dbstateInfo_)
    {

      strcpy(theCurrentState_,curr->getString());
      //std::cout<<"The current DbState is "<<theCurrentState_<<std::endl;
      return;
    }
  if (curr==CAENHVInfo_)
    {

      memcpy(&theCAENHVChannel_,curr->getData(),curr->getSize());
      //std::cout<<"The current DbState is "<<theCurrentState_<<std::endl;
      return;
    }
  if (curr==BMP183PressureInfo_)
    {
      theBMP183Pressure_=curr->getFloat();
      return;
    }
  if (curr==BMP183TemperatureInfo_)
    {
      theBMP183Temperature_=curr->getFloat();
      return;
    }
  if (curr==DS1820Info_)
    {

      memcpy(theDS1820Temperatures_,curr->getData(),curr->getSize());
      //std::cout<<"The current DbState is "<<theCurrentState_<<std::endl;
      return;
    }
   
}

void DimSlowControl::CAENHVSetVoltage(uint32_t chan,float v)
{
  if (theCAENHVPrefix_.compare("")==0) return;
  std::stringstream s0;
  int32_t data[2];
  float* fdata =(float*) data;
  data[0]=chan;
  fdata[1]=v;
  s0.str(std::string());
  s0<<theCAENHVPrefix_<<"/SETV0";
  DimClient::sendCommand(s0.str().c_str(),data,2*sizeof(int32_t));

}
void DimSlowControl::CAENHVSetCurrent(uint32_t chan,float ic)
{
  if (theCAENHVPrefix_.compare("")==0) return;
  std::stringstream s0;
  int32_t data[2];
  float* fdata =(float*) data;
  data[0]=chan;
  fdata[1]=ic;
  s0.str(std::string());
  s0<<theCAENHVPrefix_<<"/SETI0";
  DimClient::sendCommand(s0.str().c_str(),data,2*sizeof(int32_t));

}
void DimSlowControl::CAENHVSetOn(uint32_t chan)
{
  if (theCAENHVPrefix_.compare("")==0) return;
  std::stringstream s0;
  s0.str(std::string());
  s0<<theCAENHVPrefix_<<"/SETON";
  DimClient::sendCommand(s0.str().c_str(),(int) chan); 
    
  

}
void DimSlowControl::CAENHVSetOff(uint32_t chan)
{
  if (theCAENHVPrefix_.compare("")==0) return;
  std::stringstream s0;
  s0.str(std::string());
  s0<<theCAENHVPrefix_<<"/SETOFF";
  DimClient::sendCommand(s0.str().c_str(),(int) chan); 
    
  

}
void DimSlowControl::CAENHVRead(uint32_t chan)
{
  if (theCAENHVPrefix_.compare("")==0) return;
  std::stringstream s0;
  s0.str(std::string());
  s0<<theCAENHVPrefix_<<"/READCHANNEL";
  DimClient::sendCommand(s0.str().c_str(),(int) chan); 
    
  

}
void DimSlowControl::CAENHVStartMonitor(uint32_t period)
{
  if (theCAENHVPrefix_.compare("")==0) return;
  std::stringstream s0;
  s0.str(std::string());
  s0<<theCAENHVPrefix_<<"/STARTMONITOR";
  DimClient::sendCommand(s0.str().c_str(),(int) period); 
    
  

}
void DimSlowControl::CAENHVStopMonitor()
{
  if (theCAENHVPrefix_.compare("")==0) return;
  std::stringstream s0;
  s0.str(std::string());
  s0<<theCAENHVPrefix_<<"/STOPMONITOR";
  DimClient::sendCommand(s0.str().c_str(),(int) 1); 
    
  

}

void DimSlowControl::CAENHVStartRegulation(uint32_t period)
{
  if (theCAENHVPrefix_.compare("")==0) return;
  std::stringstream s0;
  s0.str(std::string());
  s0<<theCAENHVPrefix_<<"/STARTREGUL";
  DimClient::sendCommand(s0.str().c_str(),(int) period); 
    
  

}
void DimSlowControl::CAENHVStopRegulation()
{
  if (theCAENHVPrefix_.compare("")==0) return;
  std::stringstream s0;
  s0.str(std::string());
  s0<<theCAENHVPrefix_<<"/STOPREGUL";
  DimClient::sendCommand(s0.str().c_str(),(int) 1); 
    
  

}


void DimSlowControl::OpenGPIO()
{
  if (theGPIOPrefix_.compare("")==0) return;

  std::stringstream s0;

  s0.str(std::string());
  s0<<theGPIOPrefix_<<"/OPEN";
  DimClient::sendCommand(s0.str().c_str(),(int) 1); 

}
void DimSlowControl::CloseGPIO()
{
  if (theGPIOPrefix_.compare("")==0) return;
  std::stringstream s0;

  s0.str(std::string());
  s0<<theGPIOPrefix_<<"/CLOSE";
  DimClient::sendCommand(s0.str().c_str(),(int) 1); 

}


void DimSlowControl::VMEOn()
{
  if (theGPIOPrefix_.compare("")==0) return;
  std::stringstream s0;

  s0.str(std::string());
  s0<<theGPIOPrefix_<<"/VME_ON";
  DimClient::sendCommand(s0.str().c_str(),(int) 1); 

}
void DimSlowControl::VMEOff()
{
  if (theGPIOPrefix_.compare("")==0) return;
  std::stringstream s0;

  s0.str(std::string());
  s0<<theGPIOPrefix_<<"/VME_OFF";
  DimClient::sendCommand(s0.str().c_str(),(int) 1); 

}
void DimSlowControl::DIFOn()
{
  if (theGPIOPrefix_.compare("")==0) return;
  std::stringstream s0;

  s0.str(std::string());
  s0<<theGPIOPrefix_<<"/DIF_ON";
  DimClient::sendCommand(s0.str().c_str(),(int) 1); 

}
void DimSlowControl::DIFOff()
{
  if (theGPIOPrefix_.compare("")==0) return;
  std::stringstream s0;

  s0.str(std::string());
  s0<<theGPIOPrefix_<<"/DIF_OFF";
  DimClient::sendCommand(s0.str().c_str(),(int) 1); 

}
void DimSlowControl::BMP183SetPeriod(uint32_t p)
{
  if (theBMP183Prefix_.compare("")==0) return;
  std::stringstream s0;

  s0.str(std::string());
  s0<<theBMP183Prefix_<<"/SETPERIOD";
  DimClient::sendCommand(s0.str().c_str(),(int) p); 

}
void DimSlowControl::BMP183Store(std::string s)
{
  if (theBMP183Prefix_.compare("")==0) return;
  std::stringstream s0;

  s0.str(std::string());
  s0<<theBMP183Prefix_<<"/STORE";
  DimClient::sendCommand(s0.str().c_str(),s.c_str()); 


}
void DimSlowControl::DS1820SetPeriod(uint32_t p)
{
  if (theDS1820Prefix_.compare("")==0) return;
  std::stringstream s0;

  s0.str(std::string());
  s0<<theDS1820Prefix_<<"/SETPERIOD";
  DimClient::sendCommand(s0.str().c_str(),(int) p); 

}
void DimSlowControl::DS1820Store(std::string s)
{
  if (theDS1820Prefix_.compare("")==0) return;
  std::stringstream s0;

  s0.str(std::string());
  s0<<theDS1820Prefix_<<"/STORE";
  DimClient::sendCommand(s0.str().c_str(),s.c_str()); 


}
void DimSlowControl::CAENHVInitialise(std::string s)
{
  std::stringstream s0;

  s0.str(std::string());
  s0<<theCAENHVPrefix_<<"/INITIALISE";
  DimClient::sendCommand(s0.str().c_str(),s.c_str()); 

}


