#include "LCIOWritterInterface.h"
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include "DIFReadoutConstant.h"
#include <iostream>
#include <sstream>

LCIOWritterInterface::LCIOWritterInterface() : theEventNumber_(0),theRunNumber_(0)
{
  theMonitor_ = new DHCalEventReader();


}
std::string LCIOWritterInterface::getOutputFileName(uint32_t run,uint32_t seq)
{

  std::stringstream ss("");
  ss<<theDirectory_<<"/DHCAL_"<<run<<"_I0_"<<seq<<".slcio";
      
  return std::string(ss.str());
}


void LCIOWritterInterface::openFile(uint32_t run,std::string dir,std::string setup)
{
  theDirectory_=dir;
  if (run!=theRunNumber_)
    theSequence_=0;
  theRunNumber_=run;
  theMonitor_->openOutput(getOutputFileName(theRunNumber_,theSequence_));

  theMonitor_->createEvent(run,"SD-HCAL");
  theMonitor_->createRunHeader(run,"SD-HCAL");
  theMonitor_->getRunHeader()->parameters().setValue("Setup",setup);
  theMonitor_->writeRunHeader();


}
void LCIOWritterInterface::writeEvent(uint32_t gtc,std::vector<unsigned char*> vbuf)
{

  theMonitor_->createEvent(theRunNumber_,"SD-HCAL");

  theMonitor_->getEvent()->setEventNumber(gtc);

  uint32_t theNumberOfDIF=vbuf.size();
  if (theEventNumber_%100==0) 
    std::cout<<"Standard completion "<<theEventNumber_<<" GTC "<<gtc<<std::endl;
  for (std::vector<unsigned char*>::iterator iv=vbuf.begin();iv!=vbuf.end();iv++) 
    {
      unsigned char* cdata=(*iv);
      int32_t* idata=(int32_t*) cdata;
      //printf("\t writing %d bytes",idata[SHM_BUFFER_SIZE]);
      int difsize=idata[SHM_BUFFER_SIZE];
      theMonitor_->addRawOnlineRU(idata,difsize/4+1);
       
    }
   
  theMonitor_->writeEvent(false);		
   
  theEventNumber_++;
 


  if (theEventNumber_%100==0)
    {
      struct stat file_status;
      stat(getOutputFileName(theRunNumber_,theSequence_).c_str(), &file_status);
      int filesize =(int) file_status.st_size ;
      printf("File size is %d \n",filesize);
      if (filesize>2*1000*1024*1024)
	{
	  theMonitor_->closeOutput();
	  theSequence_++;
	  theMonitor_->openOutput(getOutputFileName(theRunNumber_,theSequence_));
	}
    }

     

}

void LCIOWritterInterface::closeFile()
{
  theMonitor_->closeOutput();

}
