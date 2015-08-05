#include "HVCaenInterface.h"
#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //for exit(0);
#include<sys/socket.h>
#include<errno.h> //For errno - the error number
#include<netdb.h> //hostent
#include<arpa/inet.h>
 
HVCaenInterface::HVCaenInterface(std::string host,std::string user,std::string pwd) :theHost_(host),theUser_(user),thePassword_(pwd)
{
  theHandle_=-1;

  theIp_="";
  struct hostent *he;
  struct in_addr **addr_list;
  int i;
  char ip[30];

  if ( (he = gethostbyname( theHost_.c_str() ) ) == NULL) 
    {
      // get the host info
      herror("gethostbyname");
      return ;
    }
 
  addr_list = (struct in_addr **) he->h_addr_list;
     
  for(i = 0; addr_list[i] != NULL; i++) 
    {
      //Return the first one;
      strcpy(ip , inet_ntoa(*addr_list[i]) );
      break;
    }
  theIp_.assign(ip);
  connected_=false;
}

HVCaenInterface::~HVCaenInterface()
{
  if (theHandle_!=-1) Disconnect();
}
void HVCaenInterface::Disconnect()
{
  if (theHandle_==-1) return;

  int ret = CAENHV_DeinitSystem(theHandle_);
  //if( ret == CAENHV_OK )
  //  printf("CAENHV_DeinitSystem: Connection closed (num. %d)\n\n", ret);
  if( ret != CAENHV_OK )
    printf("CAENHV_DeinitSystem: %s (num. %d)\n\n", CAENHV_GetError(theHandle_), ret);
  connected_=false;
  theHandle_=-1;
}


void HVCaenInterface::Connect()
{

  
  // Now connect to the CAEN crate
  int32_t ret,sysHndl;
  int32_t sysType=0;
  int32_t link=LINKTYPE_TCPIP;
  ret = CAENHV_InitSystem((CAENHV_SYSTEM_TYPE_t)sysType, link,(char*) theIp_.c_str(),theUser_.c_str(),thePassword_.c_str(), &sysHndl);


  if( ret == CAENHV_OK )
    {
      theID_=ret;
      theHandle_=sysHndl;
      connected_=true;
    }
  else
    printf("\nCAENHV_InitSystem: %s (num. %d) handle %d \n\n", CAENHV_GetError(sysHndl), ret,sysHndl);    

  
}

void HVCaenInterface::SetOff(uint32_t channel)
{
  char ParName[16];
  sprintf(ParName,"%s","Pw");
  int param[1];
  param[0] = 0;

  uint16_t slot=channel/6 ;
  uint16_t ChNum=1,ChList[1];
  ChList[0]=channel%6;

  int32_t ret=CAENHV_SetChParam(theHandle_,slot,ParName, ChNum, ChList,param);
  printf("CAENHV_SetChParam: %s (num. %d)\n\n", CAENHV_GetError(theHandle_), ret);
}
void HVCaenInterface::SetOn(uint32_t channel)
{
  char ParName[16];
  sprintf(ParName,"%s","Pw");
  int param[1];
  param[0] = 1;

  uint16_t slot=channel/6;
  uint16_t ChNum=1,ChList[1];
  ChList[0]=channel%6;

  int32_t ret=CAENHV_SetChParam(theHandle_,slot,ParName, ChNum, ChList,param);
  printf("CAENHV_SetChParam: %s (num. %d)\n\n", CAENHV_GetError(theHandle_), ret);
}
void HVCaenInterface::SetCurrent(uint32_t channel,float imax)
{
  char ParName[16];
  sprintf(ParName,"%s","I0Set");
  float param[1];
  param[0] = imax;

  uint16_t slot=channel/6;
  uint16_t ChNum=1,ChList[1];
  ChList[0]=channel%6;

  int32_t ret=CAENHV_SetChParam(theHandle_,slot,ParName, ChNum, ChList,param);
  //printf("CAENHV_SetChParam: %s (num. %d)\n\n", CAENHV_GetError(theHandle_), ret);
}
void HVCaenInterface::SetVoltage(uint32_t channel,float v0)
{
  char ParName[16];
  sprintf(ParName,"%s","V0Set");
  float param[1];
  param[0] = v0;

  uint16_t slot=channel/6;
  uint16_t ChNum=1,ChList[1];
  ChList[0]=channel%6;


  //printf("%d %d %d %f \n",channel,slot,ChList[0],v0);
  int32_t ret=CAENHV_SetChParam(theHandle_,slot,ParName, ChNum, ChList,param);
  //printf("CAENHV_SetChParam: %s (num. %d)\n\n", CAENHV_GetError(theHandle_), ret);
}

float HVCaenInterface::GetCurrentSet(uint32_t channel)
{
  char ParName[16];
  sprintf(ParName,"%s","I0Set");
  float param[1];
  

  uint16_t slot=channel/6;
  uint16_t ChNum=1,ChList[1];
  ChList[0]=channel%6;

  int32_t ret=CAENHV_GetChParam(theHandle_,slot,ParName, ChNum, ChList,param);
  printf("CAENHV_GetChParam: %s (num. %d)\n %f\n", CAENHV_GetError(theHandle_), ret,param[0]) ;
  return param[0];
}
float HVCaenInterface::GetVoltageSet(uint32_t channel)
{
  char ParName[16];
  sprintf(ParName,"%s","V0Set");
  float param[1];
  

  uint16_t slot=channel/6; 
  uint16_t ChNum=1,ChList[1];
  ChList[0]=channel%6;
  
  int32_t ret=CAENHV_GetChParam(theHandle_,slot,ParName, ChNum, ChList,param);
  printf("CAENHV_GetChParam: %s (num. %d)\n %f \n", CAENHV_GetError(theHandle_), ret,param[0]) ;
  return param[0];
}
float HVCaenInterface::GetCurrentRead(uint32_t channel)
{
  char ParName[16];
  sprintf(ParName,"%s","IMon");
  float param[1];
  

  uint16_t slot=channel/6;
  uint16_t ChNum=1,ChList[1];
  ChList[0]=channel%6;

  int32_t ret=CAENHV_GetChParam(theHandle_,slot,ParName, ChNum, ChList,param);
  //printf("CAENHV_GetChParam: %s (num. %d)\n\n", CAENHV_GetError(theHandle_), ret) ;
  return param[0];
}
float HVCaenInterface::GetVoltageRead(uint32_t channel)
{
  char ParName[16];
  sprintf(ParName,"%s","VMon");
  float param[1];
  

  uint16_t slot=channel/6;
  uint16_t ChNum=1,ChList[1];
  ChList[0]=channel%6;

  int32_t ret=CAENHV_GetChParam(theHandle_,slot,ParName, ChNum, ChList,param);
  //printf("CAENHV_GetChParam: %s (num. %d)\n\n", CAENHV_GetError(theHandle_), ret) ;
  return param[0];
}
uint32_t HVCaenInterface::GetStatus(uint32_t channel)
{

  char ParName[16];
  sprintf(ParName,"%s","Status");
  uint32_t param[1];
  

  uint16_t slot=channel/6;
  uint16_t ChNum=1,ChList[1];
  ChList[0]=channel%6;

  int32_t ret=CAENHV_GetChParam(theHandle_,slot,ParName, ChNum, ChList,param);
  //printf("CAENHV_GetChParam: %s (num. %d)\n\n", CAENHV_GetError(theHandle_), ret) ;
  return param[0];
}
