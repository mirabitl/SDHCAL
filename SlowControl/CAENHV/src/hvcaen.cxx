#include "HVCaenInterface.h"
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <fstream>

#include <string.h>
#include <string>
#include <stdlib.h>
int main(int argc, char **argv)
{
  int opt;
  std::string host,user,pwd,action;
  uint32_t slot,channel;
  float value;
  while ((opt = getopt (argc, argv, "h:u:p:s:c:a:v:")) != -1)
  {
    switch (opt)
    {
      case 'h':

		host.assign(optarg);
                break;
      case 'u':

		user.assign(optarg);
                break;
      case 'p':

		pwd.assign(optarg);
                break;
      case 'a':

		action.assign(optarg);
                break;
      case 'c':

		channel =atoi(optarg);
                break;
      case 'v':

		value =atof(optarg);
                break;
    }
  }
  /*  std::cout<<host<<std::endl;
  std::cout<<user<<std::endl;
  std::cout<<pwd<<std::endl;  
  std::cout<<action<<std::endl;
  std::cout<<channel<<std::endl;
  std::cout<<value<<std::endl;  
  */

  HVCaenInterface h(host,user,pwd);
  h.Connect();
  if (action.compare("READ")==0)
    {
      std::ofstream myfile;
      myfile.open ("/dev/shm/caenhv.txt");
      myfile<<channel<<" ";
      myfile<<h.GetVoltageSet(channel)<<" ";
      myfile<<h.GetVoltageRead(channel)<<" ";
      myfile<<h.GetCurrentRead(channel)<<" ";
      myfile<<h.GetStatus(channel)<<std::endl;
      myfile.close();
    }
  if (action.compare("V0")==0)
    {
      h.SetVoltage(channel,value);
    }
  if (action.compare("I0")==0)
    {
      h.SetCurrent(channel,value);
    }
  if (action.compare("ON")==0)
    {
      h.SetOn(channel);
    }
  if (action.compare("OFF")==0)
    {
      h.SetOff(channel);
    }
  h.Disconnect();
}
