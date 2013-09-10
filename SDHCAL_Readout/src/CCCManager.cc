
#include "CCCManager.h"
#include <typeinfo>
#include <iostream>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <istream>
#include <ostream>
//#include <iterator>
#include <sstream>
#include <algorithm>
#include <stdlib.h>

CCCManager::CCCManager(std::string CCCName,std::string CCCType) 
{
  theCCCType_=CCCType;
  theCCCName_=CCCName;
  //theSync_ = new toolbox::BSem(toolbox::BSem::EMPTY);
  //theSync_->give();
}

std::string CCCManager::discover()
{
  std::string line;
  std::string busline,serialline;
  std::ifstream myfile ("/proc/bus/usb/devices");
  std::stringstream ccclist;
  std::vector<uint32_t> v;
  v.clear();
  uint32_t ibus,idev;
  if (myfile.is_open())
  {
    while ( myfile.good() )
    {
      getline (myfile,line);
      if (line.substr(0,2).compare("T:")==0)
			{
	  		busline=line.substr(2,line.size()-2);
			  size_t idbus,idlevel,iddevice,idspeed;
			  for (uint8_t ic=0;ic<busline.size();ic++)
	  	  {
	    	  if (busline.substr(ic,4).compare("Bus=")==0) idbus=ic;
		      if (busline.substr(ic,4).compare("Lev=")==0) idlevel=ic;
		      if (busline.substr(ic,5).compare("Dev#=")==0) iddevice=ic;
	  	    if (busline.substr(ic,4).compare("Spd=")==0) idspeed=ic;
		    }
			  ibus=atoi(busline.substr(idbus+4,idlevel-idbus-4).c_str());
			  idev=atoi(busline.substr(iddevice+5,idspeed-iddevice-5).c_str());
			}
      if (line.substr(0,2).compare("S:")==0)
			{
	  		serialline=line.substr(2,line.size()-2);
			  size_t idserial=0,idftdi=0;
			  for (uint8_t ic=0;ic<serialline.size();ic++)
	  	  {
	    	  if (serialline.substr(ic,13).compare("SerialNumber=")==0) idserial=ic;
	      	if (serialline.substr(ic,6).compare("DCCCCC")==0) idftdi=ic;
		    }
			  if (idftdi==0 || idserial==0 ) continue;
			  uint32_t cccid=atoi(serialline.substr(idftdi+5,3).c_str());
			  v.push_back(cccid);
			  std::cout << " CCC found : "<<cccid << std::endl;
			  char cmd[256];
	  		sprintf(cmd,"sudo /bin/chmod 666 /dev/bus/usb/%.3d/%.3d",ibus,idev);
			  std::cout<<"Executing ... "<<cmd<<std::endl;
	  		system(cmd);
			}
    }
    myfile.close();
  }
  else std::cout << "Unable to open file"<<std::endl; 
  if (v.size()>0)
  {
		for (uint8_t i=0;i<v.size()-1;i++)
			ccclist<<v[i]<<",";
    ccclist<<v[v.size()-1];
  }
  std::cout<<"List of CCCs is: "<<ccclist.str()<<std::endl;
  return ccclist.str();
}

void CCCManager::destroy()
{
  std::cout<<"a voir.... "<<std::endl;
}

void CCCManager::initialise()
{
  theCCC_=NULL;
  // Open
  if (theCCCType_.compare("DCC_CCC")==0)
    {
      theCCC_= new CCCReadout (theCCCName_);
      printf ("theCCC=%p\n",theCCC_);
      theCCC_->open();
    }
  else
    {
      printf ("No more RS232 CCC exist\n");
      exit(0);
    }
  theCCC_->DoSendDIFReset();

  usleep((unsigned)1);

}

void CCCManager::configure()
{
 printf ("theCCC=%p\n",theCCC_);

  theCCC_->DoSendDIFReset();
  usleep((unsigned)1);

}

void CCCManager::start( )
{
 printf ("theCCC=%p\n",theCCC_);

  theCCC_->DoSendBCIDReset();
  theCCC_->DoSendStartAcquisitionAuto();
  usleep((unsigned)1);
}

void CCCManager::stop()
{
 theCCC_->DoSendStopAcquisition();

}

void CCCManager::test()
{
  theCCC_->DoSendDIFReset();

}