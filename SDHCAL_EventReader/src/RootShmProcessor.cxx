#include "RootShmProcessor.hh"
#include "DHCalEventReader.h"
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
using namespace levbdim;

RootShmProcessor::RootShmProcessor(std::string dir,std::string setup) : theEventNumber_(0),theRunNumber_(0),_filepath(dir),_setup(setup),_started(false)
{
  _der = new DHCalEventReader();

  _der->ParseSteering(setup);
  _der->registerAnalysis("rootProcessor","/opt/dhcal/lib/");



}
std::string RootShmProcessor::getOutputFileName(uint32_t run,uint32_t seq)
{

  std::stringstream ss("");
  ss<<_filepath<<"/DHCAL_"<<run<<"_I0_"<<seq<<".slcio";
      
  return std::string(ss.str());
}


void RootShmProcessor::start(uint32_t run)//,std::string dir,std::string setup)
{

  if (run!=theRunNumber_)
    theSequence_=0;
  theRunNumber_=run;
  _der->openOutput(getOutputFileName(theRunNumber_,theSequence_));

  _der->createEvent(run,"SD-HCAL");
  _der->createRunHeader(run,"SD-HCAL");
  _der->getRunHeader()->parameters().setValue("Setup",_setup);
  _der->writeRunHeader();

  _started=true;
}
void RootShmProcessor::processEvent(uint32_t gtc,std::vector<levbdim::buffer*> vbuf)//writeEvent(uint32_t gtc,std::vector<unsigned char*> vbuf)
{
  if (!_started) return;
  _der->createEvent(theRunNumber_,"SD-HCAL");

  _der->getEvent()->setEventNumber(gtc);

  uint32_t theNumberOfDIF=vbuf.size();
  if (theEventNumber_%100==0) 
    std::cout<<"Standard completion "<<theEventNumber_<<" GTC "<<gtc<<std::endl;
  for (std::vector<levbdim::buffer*>::iterator iv=vbuf.begin();iv!=vbuf.end();iv++) 
    {
      unsigned char* cdata=(unsigned char*) (*iv)->ptr();
      int32_t* idata=(int32_t*) cdata;
      //printf("\t writing %d bytes",idata[SHM_BUFFER_SIZE]);
      int difsize=(*iv)->size();
      if (theEventNumber_%100==0)
	std::cout<<idata[0]<<" "<<idata[1]<<" "<<idata[2]<<" "<<idata[3]<<" "<<difsize<<std::endl;

      _der->addRawOnlineRU(idata,difsize/4+1);
       
    }
  printf("Pasring \n");
  _der->parseSDHCALEvent();
  printf("analyzing %d \n",theEventNumber_);
  _der->analyzeEvent();
  printf("writing %d \n",theEventNumber_);
  _der->writeEvent(false);		
   
  theEventNumber_++;
  //delete _der->getEvent();


  if (theEventNumber_%100==0)
    {
      struct stat file_status;
      stat(getOutputFileName(theRunNumber_,theSequence_).c_str(), &file_status);
      int filesize =(int) file_status.st_size ;
      printf("File size is %d \n",filesize);
      if (filesize>2*1000*1024*1024)
	{
	  _der->closeOutput();
	  theSequence_++;
	  _der->openOutput(getOutputFileName(theRunNumber_,theSequence_));
	}
    }

     

}

void RootShmProcessor::stop()
{
  if (_started)
    this->close();
}
void RootShmProcessor::close()
{
  _der->closeOutput();

}
