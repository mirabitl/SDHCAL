#include "LcioShmProcessor.hh"
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

LcioShmProcessor::LcioShmProcessor(std::string dir,std::string setup) : theEventNumber_(0),theRunNumber_(0),_filepath(dir),_setup(setup),_started(false)
{
  _der = new DHCalEventReader();


}
std::string LcioShmProcessor::getOutputFileName(uint32_t run,uint32_t seq)
{

  std::stringstream ss("");
  ss<<_filepath<<"/DHCAL_"<<run<<"_I0_"<<seq<<".slcio";
      
  return std::string(ss.str());
}


void LcioShmProcessor::start(uint32_t run)//,std::string dir,std::string setup)
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
void LcioShmProcessor::processRunHeader(std::vector<uint32_t> header)
{
 if (!_started) return;
 _der->createEvent(theRunNumber_,"SD-HCAL");

 _der->getEvent()->setEventNumber(0);

  
 uint32_t ibuf[256];
 for (int i=0;i<header.size();i++) ibuf[i]=header[i];
 // Construct one levbdim buffer with header content
 levbdim::buffer b(128+header.size());
 b.setDetectorId(255);
 b.setDataSourceId(1);
 b.setEventId(0);
 b.setBxId(0);
 b.setPayload(ibuf,header.size()*sizeof(uint32_t));
 unsigned char* cdata=(unsigned char*) b.ptr();
 int32_t* idata=(int32_t*) cdata;
 int difsize=b.size();
 _der->addRawOnlineRU(idata,difsize/4+1);
 _der->writeEvent(false);            
}

void LcioShmProcessor::processEvent(uint32_t gtc,std::vector<levbdim::buffer*> vbuf)//writeEvent(uint32_t gtc,std::vector<unsigned char*> vbuf)
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
      _der->addRawOnlineRU(idata,difsize/4+1);
       
    }
  _der->parseSDHCALEvent();
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

void LcioShmProcessor::stop()
{
  if (_started)
    this->close();
}
void LcioShmProcessor::close()
{
  _der->closeOutput();

}
