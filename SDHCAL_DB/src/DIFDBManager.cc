#include "DIFDBManager.h"
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <fcntl.h>
DIFDBManager::DIFDBManager(std::string sdifs)
{
  theDIFs_.clear();
  theDifMap_.clear();
  theAsicsMap_.clear();
  theAsicKeyMap_.clear();
  theNumberOfAsicsMap_.clear();

  uint16_t cut=0;
  uint32_t difid;
  std::string tlist(sdifs);
  if (tlist.find_first_of(",") == tlist.npos)
    {
     
      difid = atoi(tlist.c_str());
 //     printf("1 %d %d\n",atoi(tlist.c_str()),difid);
      theDIFs_.push_back(difid);
    }
  else
    {
//      printf("2\n");
      while( (cut = tlist.find_first_of(",")) != tlist.npos )
	{
	  if(cut > 0)
	    {
	      difid = atoi(tlist.substr(0,cut).c_str());
	      theDIFs_.push_back(difid);
//	      printf("%d\n",difid);
	      if (cut+1<tlist.length())
		tlist=(tlist.substr(cut+1));
	      else
		break;

	    }	
	}
      
    }
   for (uint8_t idx=0;idx<this->getNumberOfDIFs();idx++)
    {
      std::cout<<theDIFs_[idx]<<std::endl;
    }
   std::cout<<tlist<<" gives "<<this->getNumberOfDIFs()<<std::endl;
}

DIFDBManager::DIFDBManager(std::vector<uint32_t>& v)
{
  theDIFs_.clear();
  theDifMap_.clear();
  theAsicsMap_.clear();
  theNumberOfAsicsMap_.clear();

  for (std::vector<uint32_t>::iterator it=v.begin();it!=v.end();it++)
    theDIFList_.push_back(*it);
 
}

void DIFDBManager::clearMaps()
{
  for (std::map<uint32_t,UsbDIFSettings*>::iterator it=theDifMap_.begin();it!=theDifMap_.end();it++)
    delete it->second;
  theDifMap_.clear();
  for (std::map<uint32_t,SingleHardrocV2ConfigurationFrame*>::iterator it=theAsicsMap_.begin();it!=theAsicsMap_.end();it++)
    delete it->second;
  for (std::map<uint32_t,unsigned char*>::iterator it=theAsicKeyMap_.begin();it!=theAsicKeyMap_.end();it++)
    delete it->second;


  theAsicsMap_.clear();
  theAsicKeyMap_.clear();
  theNumberOfAsicsMap_.clear();
    
}

void DIFDBManager::dumpAsicMap()
{
  for (std::map<uint32_t,unsigned char*>::iterator it=theAsicKeyMap_.begin();it!=theAsicKeyMap_.end();it++)
    {
      uint32_t d,a,s;
      d=(it->first>>8)&0xff;
      a=(it->first)&0xff;
      unsigned char* buf =it->second;
      s=buf[0];
      printf("DIF %d ASIC %d Size %d \n",d,a,s);
      for (int i=0;i<s;i++)
	printf ("%02x",buf[1+i]);
      printf ("\n");
    }
}

void DIFDBManager::download(std::string asicType)
{
  uint8_t nasics=0; // cc02032012
	uint8_t nasicsl1=0;
	uint8_t nasicsl2=0;
	uint8_t nasicsl3=0;
	uint8_t nasicsl4=0;
	uint8_t tamponl=0;

  this->clearMaps();
  std::cout<<"Full download "<<std::endl;
  this->LoadAsicParameters();
  std::cout<<"Specific DIF download "<<std::endl;
	printf ("nb difs = %d \n",this->getNumberOfDIFs());
  for (uint8_t idx=0;idx<this->getNumberOfDIFs();idx++)
  {
      // Create and fill DIF settings
      uint32_t difid= this->getDIFId(idx);
      UsbDIFSettings* ud=new UsbDIFSettings(difid);
      this->LoadDIFDefaultParameters(this->getDIFId(idx),ud);
      if (!ud->Masked)
			{
	  std::pair<uint32_t,UsbDIFSettings*> p(difid,ud);
	  theDifMap_.insert(p);
	  // Fill Asics for this DIF
	  SingleHardrocV2ConfigurationFrame* v = new  SingleHardrocV2ConfigurationFrame[MAX_NB_OF_ASICS];
//#define OLDWAY
#undef OLDWAY
#ifdef OLDWAY
	  uint8_t nasics=0;
	

	  if (asicType.compare("HR2")==0)
	    nasics=this->LoadDIFHardrocV2Parameters(difid,v);
	  else
	    if (asicType.compare("MR")==0)
	      nasics=this->LoadDIFMicrorocParameters(difid,v);
	    else
	      {
		std::cout<<"unknown chip type" << asicType <<std::endl;
		return;
	      }
#else
	  nasics=0; // cc02032012
	 nasicsl1=0;
	 nasicsl2=0;
	 nasicsl3=0;
	 nasicsl4=0;
	 tamponl=0;

	  for (std::map<uint32_t,unsigned char*>::iterator ik=theAsicKeyMap_.begin();ik!=theAsicKeyMap_.end();ik++)
	    {
	      // printf("New key %x \n",ik->first);
	      if (((ik->first>>8)&0xFF)==difid) 
				{
				  //			printf ("ik->second[0] = %d\n",ik->second[0]);
					if (ik->second[0]==109) //HR2
					{
						nasics++;
						nasicsl1++;
					}	
					else if (ik->second[0]==74) //MR
					{
						nasics++;
						if (nasics%8==0) tamponl=1;
						else if (nasics%8<5) tamponl=(nasics%8);
						else tamponl=9-nasics%8;
						if (tamponl==1) nasicsl1++;
						else if (tamponl==2) nasicsl2++;
						else if (tamponl==3) nasicsl3++;
						else if (tamponl==4) nasicsl4++;
					}
				}
//				nasics++;
	    }
#endif
	  std::pair<uint32_t,SingleHardrocV2ConfigurationFrame*> pa(difid,v);
	  theAsicsMap_.insert(pa);
	  printf ("difdbmanager difid =%d  nasics=%d %d %d %d %d\n",difid,nasics,nasicsl1,nasicsl2,nasicsl3,nasicsl4);
	  std::pair<uint32_t,uint32_t> pn(difid,nasicsl1+(nasicsl2<<8)+(nasicsl3<<16)+(nasicsl4<<24));
	  theNumberOfAsicsMap_.insert(pn);
	}
    }
}

uint8_t DIFDBManager::getAsicsNumber(uint32_t difid)
{
	uint8_t tnbl1, tnbl2,tnbl3,tnbl4;
	tnbl1=theNumberOfAsicsMap_[difid]&0xFF;
	tnbl2=(theNumberOfAsicsMap_[difid]>>8)&0xFF;
	tnbl3=(theNumberOfAsicsMap_[difid]>>16)&0xFF;
	tnbl4=(theNumberOfAsicsMap_[difid]>>24)&0xFF;
  	printf ("in DIFDBManager theNumberOfAsicsMap_[%d]=%d \n",difid,theNumberOfAsicsMap_[difid]);
	return tnbl1+tnbl2+tnbl3+tnbl4;
}
	
uint8_t DIFDBManager::getAsicsNumberl1(uint32_t difid)
{
	return theNumberOfAsicsMap_[difid]&0xFF;
}
uint8_t DIFDBManager::getAsicsNumberl2(uint32_t difid)
{
	return (theNumberOfAsicsMap_[difid]>>8)&0xFF;
}
uint8_t DIFDBManager::getAsicsNumberl3(uint32_t difid)
{
	return (theNumberOfAsicsMap_[difid]>>16)&0xFF;
}
uint8_t DIFDBManager::getAsicsNumberl4(uint32_t difid)
{
	return (theNumberOfAsicsMap_[difid]>>24)&0xFF;
}


void DIFDBManager::dumpToTree(std::string rootPath,std::string setupName)
{
 	  for (std::map<uint32_t,unsigned char*>::iterator ik=theAsicKeyMap_.begin();ik!=theAsicKeyMap_.end();ik++)
	    {
	      // printf("New key %x \n",ik->first);
				uint32_t difid=((ik->first>>8)&0xFF);
				uint32_t asicid =(ik->first)&0xFF;
				std::stringstream s("");
				s<<rootPath<<"/"<<setupName;
				int status = mkdir(s.str().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
				std::cout<<status<<" "<<s.str()<<std::endl;

				s<<"/"<<difid;
				status = mkdir(s.str().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
				std::cout<<status<<" "<<s.str()<<std::endl;
				std::stringstream sf("");
				sf<<s.str()<<"/"<<asicid;
				int fd= ::open(sf.str().c_str(),O_CREAT| O_RDWR | O_NONBLOCK,S_IRWXU);
				if (fd<0)
					{
						perror("No way to store to file :");
		//std::cout<<" No way to store to file"<<std::endl;
						return;
					}
				int ier=write(fd,ik->second,ik->second[0]+1);
				if (ier!=ik->second[0]+1) 
				{
					std::cout<<"pb in write "<<ier<<std::endl;
					return;
				}
				::close(fd);
			}
				
	 
}

