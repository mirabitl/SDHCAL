#include "HVCaenInterface.h"
#include "MyInterface.h"
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
int main()
{
  MyInterface m("acqilc/RPC_2008@localhost:GIFRPC2015");
  
  m.connect();

  // find rack id
  m.executeSelect("select IDX,HOSTNAME,USERNAME,PWD from HVRACK WHERE HOSTNAME='lyoac28' AND VALID=1");
  std::string hostname,username,pwd;
  int32_t rackid=-1;
  MYSQL_ROW row=NULL;
  while ((row=m.getNextRow())!=0)
    {
      rackid=atoi(row[0]);
      
      hostname.assign(row[1]);
      username.assign(row[2]);
      pwd.assign(row[3]);
    }
  std::cout<<hostname<<" "<<username<< " "<<pwd<<std::endl;
  HVCaenInterface h(hostname,username,pwd);
  h.Connect();

  std::stringstream stmt;
  for (int kt=0;kt<30;kt++)
    {
  for (int i=0;i<48;i++)
    {
      if (h.GetVoltageSet(i)<10) continue;
      printf("%d %f %f %f %d \n",i,h.GetVoltageSet(i),h.GetVoltageRead(i),h.GetCurrentRead(i),h.GetStatus(i));
       stmt.str(std::string());
       stmt<<"INSERT INTO HVMON(HVRACKID,HVCHANNEL,VSET,VMON,IMON,STATUS) VALUES("<<
	 rackid<<","<<
	 i<<","<<
	 h.GetVoltageSet(i)<<","<<
	 h.GetVoltageRead(i)<<","<<
	 h.GetCurrentRead(i)<<","<<
	 h.GetStatus(i)<<")";
       m.executeQuery(stmt.str());
    }
  sleep((unsigned int) 10);
    }
  h.Disconnect();
  m.disconnect();
}
