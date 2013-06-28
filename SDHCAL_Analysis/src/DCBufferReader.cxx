#include "DCDIF.h"
#include "DCBufferReader.h"
#include <assert.h>
#include "DCType.h"
#include "math.h"
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

float DCBufferReader::theDAQ_BC_Period=0.4;
void DCBufferReader::setDAQ_BC_Period(float x){theDAQ_BC_Period=x;}
float DCBufferReader::getDAQ_BC_Period(){return theDAQ_BC_Period;}

unsigned long DCBufferReader::swap_bytes(unsigned int n,unsigned char* buf)
{
  unsigned char Swapped[4];
  memset(Swapped,0,4);
  for (unsigned int i=0;i<n;i++)
    Swapped[i] = buf[n-1-i];
  

  unsigned long *temp =(unsigned long*) &Swapped;

  return (*temp);
}

unsigned long long DCBufferReader::GrayToBin(unsigned long long n)
{
unsigned long long ish, ans, idiv, ishmax;
ishmax = sizeof(unsigned long long)*8; 
ish = 1;
ans = n;
while(true) 
		{
			idiv = ans >> ish;
			ans ^= idiv;
			if (idiv <= 1 || ish == ishmax) return ans;
			ish <<= 1;
        }
}



//Note: two characters readed in diff bloc = one CRC  
void DCBufferReader::check_CRC(unsigned char* DataCRC,int length, unsigned short &CRC)
{
  unsigned short CrcTab[256] = 
    {
      0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,//0-7
      0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,//8-15
      0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,//16-23
      0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,//24-31
      0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,//32-39
      0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,//40-47
      0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,//48-55
      0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,//56-63
      0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,//64-71
      0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,//72-79
      0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,//80-87	
      0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,//88-95
      0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,//96-103
      0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,//104-111
      0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,//112-119
      0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,//120-127
      0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,//128-135
      0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,//136-143
      0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,//144-151
      0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,//152-159
      0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,//160-167
      0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,//168-175
      0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,//176-183
      0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,//184-191
      0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,//192-199
      0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,//200-207
      0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,//208-215
      0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,//216-223
      0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,//224-231
      0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,//232-239
      0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,//240-247
      0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0 //248-255
    };
  
  int DataTemp;
  for (int i=0;i<length;i++)
    {
      DataTemp=DataCRC[i];
      CRC=((CRC&0xFF)<<8) ^ (CrcTab[((CRC>>8)^DataTemp)]); 	
    }
  return;
}

unsigned short DCBufferReader::ReadFormat_CRC(unsigned char* buf,int &index) //Apears only once.
{

  unsigned short temp = (unsigned short) buf[index];
  index+=1;
  return temp;
}
bool DCBufferReader::ReadGlobalHeader_Computing_CRC(unsigned char* buf,int &index,unsigned short &CRC)  //Global header is: 0xB0 (dec)->176
{

	
  unsigned short GlobalHeader= (unsigned short) (buf[index] & 0xFF);
  //check_CRC(&buf[index],1,CRC);
  index+=1;
  if (GlobalHeader!=0xB0)
    {
      std::stringstream s("");
      s<<" Wrong Global header "<<std::hex<<GlobalHeader<<std::dec<<" 0xB0 is expected";
      throw s.str()+__PRETTY_FUNCTION__;
    }


  return true;
}

unsigned short DCBufferReader::ReadDIFId_Computing_CRC(unsigned char* buf,int &index,unsigned short &CRC)
{

	
  unsigned short ReadDIFId= (unsigned short) (buf[index] & 0XFF);
  //check_CRC(&buf[index],1,CRC);
  index+=1;


  return ReadDIFId;
}




unsigned long DCBufferReader::ReadDIFTriggerCounter_Computing_CRC(unsigned char* buf,int &index,unsigned short &CRC)
{
  //return SD_BUFFER_Read_Computing_CRC(f,CRC);

	
  unsigned long DIFTriggerCounter= swap_bytes(4,&buf[index]);
  //check_CRC((unsigned char*) &DIFTriggerCounter,4,CRC);


  index+=4;

  return DIFTriggerCounter;
}

unsigned long DCBufferReader::ReadAsuTriggerCounter_Computing_CRC(unsigned char* buf,int &index,unsigned short &CRC)
{

  unsigned long AsuTriggerCounter= swap_bytes(4,&buf[index]);
  //check_CRC((unsigned char*) &AsuTriggerCounter,4,CRC);
  index+=4;
  return AsuTriggerCounter;
}

unsigned long DCBufferReader::ReadGlobalTriggerCounter_Computing_CRC(unsigned char* buf,int &index, unsigned short &CRC)
{


	
  unsigned long GlobalTriggerCounter=  swap_bytes(4,&buf[index]);
  //check_CRC((unsigned char*) &GlobalTriggerCounter,4,CRC);
  index+=4;
  return GlobalTriggerCounter;	
}


double DCBufferReader::ReadLargeClock_Computing_CRC(unsigned char* buf,int &index, unsigned short &CRC)
{

  // std::cout<<"index "<<index<<std::endl;

  unsigned long LBcCharA = swap_bytes(3,&buf[index]);

  //	memcpy(&LBcCharA,&buf[index],3);
  //check_CRC((unsigned char*) &LBcCharA,3,CRC);
  index+=3;
  unsigned long LBcCharB = swap_bytes(3,&buf[index]);
  //	memcpy(&LBcCharB,&buf[index],3);
  //check_CRC((unsigned char*) &LBcCharB,3,CRC);
  index+=3;
  unsigned long long Shift=16777216ULL;//to shift the value from the 24 first bits
	
  unsigned long long LBcChar = LBcCharA*Shift + LBcCharB; //Total number of bunch cross (BC) for the 48 bits var
  //  std::cout<<" A "<<LBcCharA<<" B "<<LBcCharB<<" "<<LBcChar<<std::endl;
  return (double) LBcChar*DCBufferReader::getDAQ_BC_Period(); //return a value in micro seconds
}

bool DCBufferReader::ReadFrameHeader_CRC(unsigned char* buf,int &index)  //Global header is: b4
{

	
	
  unsigned short FrameHeader= (unsigned short) buf[index];
  index+=1;
  //  printf("Frame Header index %d %x \n",index,FrameHeader);
  if (FrameHeader!=0xB4) 
    {return false;}	
  else
    {return true;}
}

void DCBufferReader::ReadAsicInternalTriggerInFrame_Computing_CRC(unsigned short &AsicId, double &TS,bool* l0,bool* l1, unsigned char* buf,int &index, unsigned short &CRC)
{
  //First two chars: AsicID
  //  unsigned short PADI[16]={48,52,56,60,32,36,40,44,16,20,24,28,0,4,8,12};
  // unsigned short BITI[8]={3,3,2,2,1,1,0,0};
  unsigned short TempInFrameCRC=CRC;
  //  for (int j=0;j<20;j++) printf("%x ",buf[index+j]);
  // printf("\n");
	
  AsicId= (unsigned short) buf[index];

  //check_CRC(&buf[index],1,TempInFrameCRC);
  index+=1;
  if((AsicId>48)&&(AsicId<100)){cout<<"Asic number default. Asic find: "<<AsicId<<endl;}// getchar();}
	
  //Folowing 6 chars: BunchCross (Binary counter)
	
  int BcChar=swap_bytes(3,&buf[index]);
  //	memcpy(&BcChar,&buf[index],3);
  //check_CRC((unsigned char*) &BcChar,3,TempInFrameCRC);
  index+=3;
  unsigned long long TimeStamp  = (unsigned long long)BcChar;
  TimeStamp=GrayToBin(TimeStamp);  //BC in multiple of DCBufferReader::getDAQ_BC_Period() in micro second
  TS=TimeStamp*DCBufferReader::getDAQ_BC_Period(); //BC in micro second
	
  //std::cout<<"Asic Id: "<<AsicId<<"\t BC: "<<BcChar<<" "<<TimeStamp<<" "<<TS<<std::endl;	
				
  //Last 32 chars: PAD trigger levels status 
  //RECALL: Pad Channels Structure (96->127)(64->95)(32->63)(0->31) Be carefull, we read this way "<-"
  bool PAD [128];
  for(int i= 0; i<128; i++){PAD[i]=0;} //init PADs
  unsigned short un = 1;
  //  int initidx=index;
  for(int ik=0;ik<4;ik++)
    {
	
      // 	  unsigned char Swapped[4];
      // 	  Swapped[0] = buf[index+3];
      // 	  Swapped[1] = buf[index+2];
      // 	  Swapped[2] = buf[index+1];
      // 	  Swapped[3] = buf[index];

      // 	  unsigned long *Data =(unsigned long*) &Swapped;

      //unsigned long *Data =(unsigned long*) &buf[index];

	  
      unsigned long PadEtat= swap_bytes(4,&buf[index]);
      // unsigned long PadEtat=0;
      //memcpy(&PadEtat,&buf[index],4);
      index+=4;
      //std::cout<<std::hex<<PadEtat<<std::dec<<" ";
      //check_CRC((unsigned char*) &PadEtat,4,CRC);
		
      //if((PadEtat==2863311530)||(PadEtat==1431655765))
      //{
      //cout<<std::bitset<32>(PadEtat)<<endl;
      //cout<<"Asic: "<<AsicId<<"BC: "<<TSdouble<<endl;
      //getchar();
      //}
		
      for(int e=0;e<32;e++)
	{	
	  //cout<<"Before: "<<std::bitset<32>(PadEtat)<<endl;
	  PAD[((3-ik)*32)+(31-e)]=PadEtat & un; //binary operation





	  //PAD[(((3-ik)*32)+(31-e))]=1;
	  //cout<<"(PadEtat & un): "<<(PadEtat & un)<<endl;
	  //cout<<"Value "<<(i*32)+(31-e) <<": "<<PAD[((i-3)*32)+(31-e)]<<endl;
	  PadEtat=PadEtat>>1;	//décalage des bit de 1
	  //cout<<"After : "<<std::bitset<32>(PadEtat)<<endl<<endl;
	}
    }
  if(AsicId!=0){CRC=TempInFrameCRC;}	//In the normal case no events has to be with 0 asics zero supp is all ready done taking data


  // fill bool arrays
  for(int p=0; p<64;p++)
    {
      l0[p]=(bool)PAD[(2*p)]; //_Lev0 (PAD paire)
      l1[p]=(bool)PAD[(2*p)+1]; //_Lev1 (PAD impaires)

      //  if (l0[p]) std::cout<<l0[p]<<l1[p]<<" ";
    }
  //  std::cout<<std::endl;

  //std::cout<<"in read "<<AsicId<<std::endl;

  return;
  //	return SD_RawHit(TSdouble, PAD); 
}

bool DCBufferReader::ReadFrameTrailer_CRC(unsigned char* buf,int &index)  //Global Trailler is: A3 (163 in dec)
{
	
  unsigned short FrameTrailer= (unsigned short) buf[index];
  index+=1;
  if(FrameTrailer==0xA3){return true;} //A3(hex)
  else{return false;}
}

bool DCBufferReader::ReadGlobalTrailer_Computing_CRC(unsigned char* buf,int &index, unsigned short &CRC)  //Global Trailer is: A0 (160 in dec)
{
	
  //  printf("index %d buf %x \n",index,buf[index]);
  unsigned short GlobalTrailer=(unsigned short) (buf[index] & 0xFF);
  //check_CRC(&buf[index],1,CRC);
  index+=1;

  if (GlobalTrailer!=0xa0)
    {
      std::stringstream s("");
      s<<" Wrong Global Trailer "<<std::hex<<GlobalTrailer<<std::dec<<" 0xa0 is expected";
      throw s.str()+__PRETTY_FUNCTION__;
    }

  return true;
}

bool DCBufferReader::ReadCRC_And_Check(unsigned char* buf,int &index, unsigned short &CRC)
{
  unsigned short *Data =(unsigned short*) &buf[index];
  printf("%x %x \n",buf[index],buf[index+1]);
  index+=2;
  
  int CRCInFile = Data[0];
  if(CRCInFile!=CRC)
    {
      cout<<"BAD CRC "<<std::hex<<CRC<<" in file "<<CRCInFile<<std::dec<<endl; 
      //cout<<"Type ENTER to continue"<<endl;
      //getchar();
      return false; 
    }	
  else
    {
      cout<<"GOOD"<<endl;
      return true;
    }
}
//Check whats appening if diff is empty (currently it could not be the case)
bool  DCBufferReader::correctRecoTime(double tbc,std::vector<DCFrame*>& vasic)
{

  bool dump=false;
  if (vasic.size()==0) return dump;
  int JumpCnt=0;
  double BCTemp=tbc; // DIF TBC
  for(unsigned int FrameCnt = 0; FrameCnt<vasic.size(); FrameCnt++){
    //    std::cout<<FrameCnt<<" "<<vasic[FrameCnt]->getBunchCrossingTime()<<":"<<BCTemp<<std::endl;
    if((vasic[FrameCnt]->getBunchCrossingTime() - BCTemp)>(0.5*0xFFFFFF*DCBufferReader::getDAQ_BC_Period()))
      JumpCnt++;
    BCTemp= vasic[FrameCnt]->getBunchCrossingTime();
  }
  //Reminder in this case "AsicTemp.get_SD_RawHit(FrameCnt).get_BC()" and "TriggerBunchCross" are given in us
  
  
  double TriggerOffset = JumpCnt*0xFFFFFF*DCBufferReader::getDAQ_BC_Period(); 
  BCTemp=tbc;  // DIF
  for(unsigned int FrameCnt = 0; FrameCnt<vasic.size(); FrameCnt++){
    if((vasic[FrameCnt]->getBunchCrossingTime() - BCTemp)>(0.5*0xFFFFFF*DCBufferReader::getDAQ_BC_Period()))
      JumpCnt--;
    double RecoTime =(tbc + TriggerOffset)  - (vasic[FrameCnt]->getBunchCrossingTime()+(JumpCnt*0xFFFFFF*DCBufferReader::getDAQ_BC_Period()));
    vasic[FrameCnt]->setRecoBunchCrossingTime(RecoTime);
    BCTemp= vasic[FrameCnt]->getBunchCrossingTime();
    if (fabs(vasic[FrameCnt]->getRecoBunchCrossingTime()-(vasic[FrameCnt]->getTimeToTrigger()*DCBufferReader::getDAQ_BC_Period()))>2E-2)
      {
	//std::cout<<"Problem "<<vasic[FrameCnt]->getRecoBunchCrossingTime()<<" "<<(vasic[FrameCnt]->getTimeToTrigger()*DCBufferReader::getDAQ_BC_Period())<<std::endl;
	dump=true;
      }
  }

//Check decrease counting again eventual jumps
#ifdef CHECKRECOTIME
  JumpCnt=0;
  BCTemp= vasic[0]->getRecoBunchCrossingTime();
  for(int FrameCnt = 1; FrameCnt<vasic.size(); FrameCnt++){
    if(( vasic[FrameCnt]->getRecoBunchCrossingTime()- BCTemp)>(0.5*0xFFFFFF*DCBufferReader::getDAQ_BC_Period()))
      JumpCnt++;
    BCTemp= vasic[FrameCnt]->getRecoBunchCrossingTime();
  }
  assert(!JumpCnt);
#endif
  return dump;
}



void DCBufferReader::ReadDIFBlock(unsigned char* buf,DCDIF* &dif,std::vector<DCFrame*>* frames,unsigned int hrtype) 
{
  //cout<<endl<<endl<<"Read a diff Block"<<endl;
  int bufidx=0;
  unsigned short CRC=0xFFFF; //Initialisation of the CRC

  // bool DataProblem = false;
	
  //Start Reading Data
  //unsigned long Time_Sec  = Time_Sec_CRC(RunFile);
  //unsigned long Time_uSec = Time_uSec_CRC(RunFile);	
  try 
    {
  if(buf!=NULL && ReadGlobalHeader_Computing_CRC(buf,bufidx, CRC)) 
    {	
      unsigned short	DIFId = ReadDIFId_Computing_CRC(buf,bufidx, CRC);
      unsigned long	DTC   = ReadDIFTriggerCounter_Computing_CRC(buf,bufidx, CRC);
      unsigned long	ATC   = ReadAsuTriggerCounter_Computing_CRC(buf,bufidx, CRC);//Jumped for now (to implement in data)
      unsigned long	GTC   = ReadGlobalTriggerCounter_Computing_CRC(buf,bufidx, CRC);
      double            LBC   = 0;
      LBC=ReadLargeClock_Computing_CRC(buf,bufidx, CRC);
      //cout<<"DIFId: "<<DIFId<<endl;
      //cout<<"GTC: "<<GTC<<endl;
	
      //Verif position je crois que christophe dit que c'est ici mais c'est faux...
      //LargeBC 


      //      std::cout<<"Absolute BCid: "<<LBC<<std::endl;
	  
      //Read the TriggerBC
      unsigned long TBcChar = swap_bytes(3,&buf[bufidx]);
      //memcpy(&TBcChar,&buf[bufidx],3);
      //check_CRC((unsigned char*) &TBcChar,3,CRC);
      bufidx+=3;
      double TriggerBunchCross = TBcChar*DCBufferReader::getDAQ_BC_Period();
      //cout<<"TrigBC: "<<TriggerBunchCross<<endl;	
      if (GTC==0)
	{
	  GTC=0XFFFFFFFF;
	  ATC=0XFFFFFFFF;
	  LBC=0XFFFFFFFF;
	  TriggerBunchCross=0XFFFFFFFF;
	}
      dif = new DCDIF(DIFId, DTC, ATC, GTC, TriggerBunchCross, LBC);
      //std::cout<<dif<<" "<<dif->getId()<<" "<<DTC<<" "<<ATC<<" "<<GTC<<" "<<TriggerBunchCross<<" "<<LBC<<std::endl;
      
      // int LastAsic = 0;
      std::vector<DCFrame*> vasic; int lastasic=0; double lasttbc=0;
      while(ReadFrameHeader_CRC(buf,bufidx)) //While you read more Frame Header (More Asics Speaking) continue...
	{
	  //Compute CRC for the frame header
	  //unsigned char fhead=0xB4;
	  //check_CRC(&fhead,1,CRC);

	  //unsigned short AsicId=0;
	  unsigned short CurAsId=0;
	  bool KillFirstFrame = (hrtype==1);
	  //int SecureCounter=0;
	  // int ChekIdBuff=0;
	  //	  bool IdError=false;
	  
	  ////// while(!ReadFrameTrailer_CRC(buf,bufidx))//Read frames until there is frames...
	  bufidx++;
	  do 
	    {
	      //RunFile.unget();
	      //RunFile.unget();
	      bufidx--;
	      bool AddHit=false;
		  
	      /* Sometimes, the file is truncated */
	      DCFrame* f = NULL;
	      try
		{
		  // Try to read the data
		  double TS;
		  bool L0[64],L1[64];
		  ReadAsicInternalTriggerInFrame_Computing_CRC(CurAsId,TS,L0,L1,buf,bufidx, CRC);
		  //std::cout<<"Frame time "<<DIFId<<" "<<dif->getTBC()<<" Asic "<<CurAsId<<" TS "<<TS<<" "<<(dif->getTBC()-TS)/0.4<<std::endl;
		  //std::cout<<"In block"<<DIFId<<" "<<CurAsId<<" "<<TS<<std::endl;
		  f = new DCFrame(DIFId,CurAsId,TS,L0,L1,(dif->getTBC()-TS)/DCBufferReader::getDAQ_BC_Period(),4);



		  bool dump=false;
		  if (lastasic!=CurAsId)
		    {
		      //std::cout<<lastasic<<" "<<CurAsId<<std::endl;
		      dump=correctRecoTime(lasttbc,vasic);
		      if (dump)
			{
			  for (unsigned int i=0;i<vasic.size();i++)
			    {
			      DCFrame* f=vasic[i];
			      if (f==0) continue;
			      //std::cout<<f->getDifId()<<" "<<f->getAsicId()<<" "<<f->getBunchCrossingTime()<<" "<<f->getTimeToTrigger()<<" "<<f->getRecoBunchCrossingTime()/0.4<<std::endl;
			    }
			  //getchar();
			}
		      lastasic=CurAsId;
		      lasttbc = dif->getTBC();
		      vasic.clear();
		      //		      getchar();		      
		    }

		  //std::cout<<dif->getId()<<" en direct  "<<dif->getTBC()<<" "<<f->getAsicId()<<" "<<f->getBunchCrossingTime()<<std::endl;
		  if (!KillFirstFrame && !dump) vasic.push_back(f);


		  if (!dump) AddHit=true;
		 
		}
	      catch (exception & err)
		{
		  // Error in the middle
		  std::cerr << "Exception: "
			    << err.what() << std::endl;
		  goto bail_out_loops;
		}
		  
	      //To kill the first data frame of each asics
	      if(KillFirstFrame){
		//std::cout<<" I will kill this frame "<<CurAsId<<std::endl;
		AddHit=false; KillFirstFrame=false;
	      }
		  
	      if(AddHit && frames!=NULL)
		{
		  frames->push_back(f);
		}
	      else
		if (f!=0) 
		  {
		    //std::cout<<"deleting in DCBUFFER "<<AddHit<<" kill "<<KillFirstFrame<<" "<<frames<<std::endl;
		  delete f;}
		  
	    } while (!ReadFrameTrailer_CRC(buf,bufidx));
	  // Correct Time for the last asic
	  //	  correctRecoTime(lasttbc,vasic);
	  bool dump=correctRecoTime(lasttbc,vasic);
	  if (dump)
	    {
	      for (unsigned int i=0;i<vasic.size();i++)
		{
		  DCFrame* f=vasic[i];
		  if (f==0) continue;
		  //		  std::cout<<f->getDifId()<<" end  "<<f->getAsicId()<<" "<<f->getBunchCrossingTime()<<" "<<f->getTimeToTrigger()<<" "<<f->getRecoBunchCrossingTime()/0.4<<std::endl;
		}
	      //getchar();
	    }








	  lastasic=CurAsId;
	  lasttbc = dif->getTBC();
	  vasic.clear();   
	  //unsigned char fhead2=0xA3;
	  //check_CRC(&fhead2,1,CRC);
	      
	      
	      
	      
	  //Travail sur la croissance des numérot d'asic de l'événement

	}
      //If it dont find any more FrameHeader, it continues and search the Global Trailer
      bufidx--;
      //RunFile.unget();
      //	RunFile.unget();
	  
    bail_out_loops: // Jump here when the file has been truncated in the middle
	  
      try {
      if( ReadGlobalTrailer_Computing_CRC(buf,bufidx,CRC))
	{
	  //unsigned short CRC =0;check_CRC(buf,bufidx-1,CRC);
	  // cout<<bufidx<<"CRC test: "<<std::hex<<CRC;
	      
	  ///ReadCRC_And_Check(buf,bufidx, CRC); 
	  //	  getchar();	      
	 	 
	  return;
	}
      }
      catch( std::string e)
	{
	  cout<<"Dont find Global Trailer in ""SD_DIF ReadDIFBlock(ifstream &RunFile)"""<<endl;
	  throw e+__PRETTY_FUNCTION__;
	}
	  
    }
    }

  catch (std::string e) //If we dont find the Global header, there is no diff speaking for this event. We got an aquisition problem.
    {
      cout<<"Dont find the Global header in ""SD_DIF ReadDIFBlock(ifstream &RunFile)"""<< e<<endl;	

      throw e+__PRETTY_FUNCTION__;
    }
  return;		
}

void DCBufferReader::ReadDIFBlockSmall(unsigned char* cbuf,DCDIF* &dif,std::vector<DCFrame*>* frames) 
{

// BoardID : 1 octet (ordre de positionnement de PCB : 0,1,2,3,4...)
// BoardID : 4 octets (nom du PCB : A,B,C,D,E,F), A=NumHardr(1,2,3,4), B=NumHardr(5,6,7,8), C=.....
// TT trig Ext : 4 octets
// SizeFrame : 4 octets (toute les données avant les zero , octet de fin compris) : 
// TT trig Ext Acq : 4 octets
// TT trig Ext NoAcq : 4 octets
// Flag : 2 octets
// CompteurDiff : 4 octets
// DATA : 20 octets (NumChip : 1 octet, TimeStamp : 3 octets, Dac0Dac1: 16 octets)
// Fin de trame : 1 octet (0xFF, pas visible)

  unsigned char c;
  //  unsigned char Temp[4];
  unsigned int NumPCB;
  unsigned int BoardId;
  unsigned int TriggerExtTot;
  unsigned int TriggerExtEnAcq;
  unsigned int TriggerExtNonAcq;
  int AsicNumber=0;
  unsigned long long TimeStamp;
  bool PAD[128];
  int PadEtat;
  int idx=0;


  std::vector<DCFrame*> framelist;

  ////////////////////PCB NUMBER///////////////////////////////////////////////////    1 Octet
  //USED TO IDDENTIFIE PCB
  c=cbuf[idx];idx++;
  NumPCB=c+1;
	
  //	cout<<"PCB NUMBER: "<<NumPCB<<endl;
	
  /////////////////////BOARD ID////////////////////////////////////////////////////    4 Octet
  BoardId = swap_bytes(4,&cbuf[idx]);idx+=4;
  unsigned long DIFId= 1000+BoardId;
  //  cout<<"BoarID: "<<BoardId<<endl; //Sert A rien car foireux genre 10 11 12 etc..
  //NumPCB=1000+BoardId;
  /////////////////////TRIGGER EXT TOT//////////////////////////////////////////////   4 Octet

  TriggerExtTot= swap_bytes(4,&cbuf[idx]);idx+=4;
  //	cout<<std::bitset<32>(TriggerExtTot)<<endl;
  //  cout<<"TriggerExtTot: "<<TriggerExtTot<<endl;
	


	
  /////////////////////SIZE OF EVENT//////////////////////////////////////////////   4 Octet
  int SizeTot;
  SizeTot= swap_bytes(4,&cbuf[idx]);idx+=4;
  //  cout<<"SizeTot: "<<SizeTot<<endl;
  /////////////////////TRIGGER EXT EN ACQ///////////////////////////////////////////   4 Octet
  TriggerExtEnAcq= swap_bytes(4,&cbuf[idx]);idx+=4;
  // cout<<"extEnAcq" <<(TriggerExtEnAcq)<<endl;
	
  /////////////////////TRIGGER EXT NON ACQ//////////////////////////////////////////   4 Octet
  TriggerExtNonAcq= swap_bytes(4,&cbuf[idx]);idx+=4;
  // cout<<"ExtenNonAcq"<<(TriggerExtNonAcq)<<endl;
	
	
  ////////////////////Deux Flagg a virer////////////////////////////////////////////	 2 Octet
  int Flag;

  Flag = swap_bytes(2,&cbuf[idx]);idx+=2;
  // cout<<"Flag "<<Flag<<endl;

  ////////////////////Timme diff///////////////////////////////////////////////////    4 Octet
  unsigned int TimeDiff= swap_bytes(4,&cbuf[idx]);idx+=4;
  //  double TD = 0;// (double)TimeDiff * SmallDAQ_TimeDiff_Period;

  dif = new DCDIF(DIFId,TriggerExtEnAcq,TriggerExtNonAcq,TriggerExtTot,TimeDiff,0);
  //std::cout<<dif<<" "<<dif->getId()<<std::endl;
  // cout<<"TimeDif"<<(TimeDiff)<<endl;

  //	cout<<"TimeDiff: "<<R.get_timeDiff()<<endl;
	
	
  // En tout on à des packet de 10262 octet, après lecture des triggers et time diff et asic (27 oct), 
  //il reste donc 10239 octet à lire 
  //On peut lire  511 évenements de 20 oct ce qui fait un total de 10220
  //il reste à en virer 19
	
  ////////////////////Boucle sur les evenements////////////////////////////////////  10238 Octet
  // Les paquets de 20 oct sont dans l'ordre asic apres asic....
  for(int si=0; si<511; si++) 
    {
      ///////Somme des octets entre parenthèses (20 octet) represente un hit////////
      //////////////////////Lecture Numero d'ASIC////////////////////////////////   (1 Octet)
      //f.read((char *)&c,sizeof(char));
      AsicNumber=cbuf[idx];idx++;
      //cout<<"AN: "<<AsicNumber;
      if (AsicNumber==0) break;
	
      //////////////////////Lecture Time Stamp////////////////////////////////////   (3 Octet)
      TimeStamp=swap_bytes(3,&cbuf[idx]);idx+=3;
      double TS =(double)GrayToBin(TimeStamp) * SmallDAQ_BC_Period;
		
      //	cout<<"AN: "<<AsicNumber<<"\t TS: "<<TimeStamp<<" "<<TS<< endl;
		
		
      //////////////////////Scan et remplissage PAD[]////////////////////////////    (16 Octet)
		
		
      for(int i= 0; i<128; i++)
	{PAD[i]=0;}
		
      for(int ik=0;ik<4;ik++)
	{
	  int un = 1;
			
	  PadEtat=  swap_bytes(4,&cbuf[idx]);idx+=4;
			
			
	  /*
	    if((163==MonitorTrigger)&&(NumPCB==1))
	    {
	    //Affichage des trames d'état des PAD
	    if(ik==0){cout<<"PadEtat serie (detector "<<NumPCB<<") (Asic "<<AsicNumber<<") : "<<endl;}
	    cout<<std::bitset<32>(PadEtat);
	    if(ik==3){cout<<endl;}
	    }
	  */
			
	  //RAPPEL: Structure Pad (96->127)(64->95)(32->63)(0->31) Attention on lira dans ce sens "<-"
			
			
			
	  for(int e=0;e<32;e++)
	    {	
	      //cout<<"Avant: "<<std::bitset<32>(PadEtat)<<endl;
	      PAD[((3-ik)*32)+(31-e)]=PadEtat & un; //opération binaire
	      //PAD[(((3-ik)*32)+(31-e))]=1;
	      //cout<<"(PadEtat & un): "<<(PadEtat & un)<<endl;
	      //cout<<"Value "<<(i*32)+(31-e) <<": "<<PAD[((i-3)*32)+(31-e)]<<endl;
	      PadEtat=PadEtat>>1;	//décalage des bit de 1
	      //cout<<"Après: "<<std::bitset<32>(PadEtat)<<endl<<endl;
	    }
			
	}
		
      if (TS!=0)
	{
	  bool lv0[64],lv1[64];
	  for(int p=0; p<64;p++)
	    {
	      lv0[p]=(bool)PAD[(2*p)]; //_Lev0 (PAD paire)
	      lv1[p]=(bool)PAD[(2*p)+1]; //_Lev1 (PAD impaires)
	    }
	  DCFrame* f =  new DCFrame(DIFId,AsicNumber,TS,lv0,lv1,TimeDiff,200);
	  framelist.push_back(f);
	}
		
      //cout<<"TSVerif: "<<R.get_events(R.get_eventsSize()-1).getTimeStamp()<<endl;
    }
	
  /////////////Octet de fin de trame inutiles////////////////////////////////////
  for(int si=0; si<18; si++) {c=0; 
    //  f.read((char *)&c,sizeof(char));
    c=cbuf[idx];idx++;
  }


  // Pourquoi / LM	R.RemoveFirstFrames(NumOfAsicPerPCB);
	

  // Debug 
  //  std::cout << TriggerExtEnAcq<<" "<< TriggerExtNonAcq<<  " " <<TriggerExtTot<<std::endl;
  // std::cout << " Read Board "<<BoardId<<" time  is "<<TimeDiff <<" size="<<framelist.size()<< std::endl;
  int nmax=0;
  if (TimeDiff<200)

    {
      double tmax=-1;

      for (unsigned int j=0;j<framelist.size();j++)
	      
	{
	  DCFrame* f= framelist[j];
		
	  //std::cout<<"frame " << j <<" Aisc" <<f.getAsicNum()<<" Time Stamp "<<f.getTimeStamp()<<std::endl;
	  if (f->getBunchCrossingTime()>tmax) {tmax=f->getBunchCrossingTime();}

	}
      for (unsigned int j=0;j<framelist.size();j++)
	      
	{
	  DCFrame* f= framelist[j];
		
	  //std::cout<<"frame " << j <<" Aisc" <<f.getAsicNum()<<" Time Stamp "<<f.getTimeStamp()<<std::endl;
	  if (f->getBunchCrossingTime()==tmax) 
	    {
	      // std::cout<<"frame " << j <<" Aisc" <<f->getAsicId()<<" Time Stamp "<<f->getBunchCrossingTime()<<std::endl;
	      nmax++;
	      f->setSynchronised(true);
	    }
	  else
	    f->setSynchronised(false);
	}

    }
  // if (nmax>0) getchar();
  if (TriggerExtNonAcq<0)
    {
      for (unsigned int i=0;i<framelist.size();i++)
	delete framelist[i];

      framelist.clear();
      return;
    }

  for (unsigned int i=0;i<framelist.size();i++)
    frames->push_back(framelist[i]);
  
  framelist.clear();

  return;

}


void DCBufferReader::PadConvert(int asicid,int ipad, int &i, int &j)
{
unsigned short AsicShiftI[49]={	0,	0,	0,	0,	0,	8,	8,	8,	8,	16,	16,	16,	16,	24,	24,	24,	24,	32,	32,	32,	32,	40,	40,	40,	40,	48,	48,	48,	48,	56,	56,	56,	56,	64,	64,	64,	64,	72,	72,	72,	72,	80,	80,	80,	80,	88,	88,	88,	88};
unsigned short AsicShiftJ[49]={	0,	0,	8,	16,	24,	24,	16,	8,	0,	0,	8,	16,	24,	24,	16,	8,	0,	0,	8,	16,	24,	24,	16,	8,	0,	0,	8,	16,	24,	24,	16,	8,	0,	0,	8,	16,	24,	24,	16,	8,	0,	0,	8,	16,	24,	24,	16,	8,	0};
unsigned short MapI[64]={2,2,2,7,5,1,3,1,4,1,6,0,3,0,3,0,7,5,4,0,6,7,2,5,3,1,4,4,6,7,5,6,7,5,6,4,3,7,1,0,5,2,6,7,0,5,4,6,0,6,0,4,4,3,3,3,5,7,1,1,2,1,2,2};
unsigned short MapJ[64]={0,1,2,0,0,0,0,2,0,1,0,0,2,2,1,1,1,1,1,3,1,2,3,2,3,3,2,3,2,3,3,3,4,4,4,4,4,5,4,4,5,4,5,6,7,6,5,6,5,7,6,6,7,6,5,7,7,7,5,6,5,7,6,7};
 i = MapI[ipad]+AsicShiftI[asicid]+1;
 j = MapJ[ipad]+AsicShiftJ[asicid]+1;
 // std::cout<<MapI[x]<<" "<<MapJ[x]<<std::endl;
}

void DCBufferReader::checkType(unsigned char* tcbuf, unsigned int rusize, bool &slowcontrol,unsigned int& version,unsigned int &hrtype,unsigned int &id0,unsigned int &iddif,int xdaq_shift)
{
  // Version 10
// dif fw lab version (1 byte) (1 fois par slow control)
// dif fw major version (1 byte) (1 fois par slow control)
// dif fw minor version (1 byte) (1 fois par slow control)
// asic type (1 byte)(1 fois par slow control)
// data format version (8 bits) (1 fois par slow control)
// timestamp.time (32b) <- secondes depuis le 01/01/1970 (1 fois par slow control)
// timestamp.millitm (32b) <- millisecondes (1 fois par slow control)
// SLC Header (0xB1)
// DIF ID (8 bits)
// Number_of_hardrocs (8 bits)



  //static int xdaq_shift=30;
  //static int xdaq_shift=92;
  slowcontrol = false;
  version=0;
  hrtype=0;
  id0=0;
  iddif=0;

  //printf("\n");
  for (uint32_t i=0;i<48;i++)
    {
      //printf("%02x",tcbuf[xdaq_shift+i]);
      if (tcbuf[xdaq_shift+i] == 0xb0 || tcbuf[xdaq_shift+i] == 0xb1) 
      {
	id0=xdaq_shift+i;
	break;
      }
    }
  //  printf("\n");
  slowcontrol=(tcbuf[id0] == 0xb1);
 
  if (!slowcontrol) return;
  return;
  if (id0 == xdaq_shift)
    {
      version=7;
     
    }
  else
    {
      version =tcbuf[id0-9];
      hrtype=tcbuf[id0-10];
    }
 
  unsigned char* cbuf = &tcbuf[id0];
  int nasic=0,difid=0;
  int header_shift=0;
  if (version<8) 
    {
      nasic=cbuf[5];
      header_shift=6;
    }
  else
    
    {
     difid=cbuf[1];
      nasic= cbuf[2];
      header_shift=3;
    }

  if (nasic>48)
    {
      std::string s= "too many asic! weird slow control";s+=+__PRETTY_FUNCTION__;
      throw s;
    }
  int size_hardroc1 = nasic*72+header_shift+1;

  if (cbuf[size_hardroc1-1]!=0xa1) 
    size_hardroc1=0;
  else
    {hrtype=1;iddif=size_hardroc1; return;}

  int size_hardroc2 = nasic*109+header_shift+1;

  printf("DEBUG %d %d %d %d %d %d %x ",version,difid,nasic,header_shift,size_hardroc2,id0,cbuf[size_hardroc2-1]);

  if (cbuf[size_hardroc2-1]!=0xa1) 
    size_hardroc2=0;
  else
    {
      hrtype=2;
      iddif=size_hardroc2;
      std::cout<<id0<<"  "<<std::hex<<(int)cbuf[iddif]<<std::dec<<std::endl;
      return;
    }
  if (!(size_hardroc1!=0 || size_hardroc2!=0))
    {
      std::string s="unknown hardroc type";s+=__PRETTY_FUNCTION__;
      throw s;
    }


}
void DCBufferReader::RKPadConvert(int asicid,int ipad, int &i, int &j, int asicType)
{
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Upper view (ASICs SIDE)
//Asics mapping (channel 0 to 63) on small chambers  with HARDROC I
//                            0		    5		  10		15	      20        25        30        35        40        45        50        55        60
const unsigned short MapJSmallHR1[64]={2,2,2,7,5,1,3,1,4,1,6,0,3,0,3,0,7,5,4,0,6,7,2,5,3,1,4,4,6,7,5,6,7,5,6,4,3,7,1,0,5,2,6,7,0,5,4,6,0,6,0,4,4,3,3,3,5,7,1,1,2,1,2,2};
const unsigned short MapISmallHR1[64]={7,6,5,7,7,7,7,5,7,6,7,7,5,5,6,6,6,6,6,4,6,5,4,5,4,4,5,4,5,4,4,4,3,3,3,3,3,2,3,3,2,3,2,1,0,1,2,1,2,0,1,1,0,1,2,0,0,0,2,1,2,0,1,0};
//(J Axis)
//
// 7	|11|05|00|06|08|04|10|03|  	
// 6	|15|09|01|14|18|17|20|16|  	     
// 5	|13|07|02|12|26|23|28|21|  	     
// 4	|19|25|22|24|27|30|31|29|  	     
// 3	|39|38|41|36|35|33|34|32|  	     TOP VIEW (ASICs SIDE)
// 2	|48|58|60|54|46|40|42|37|  	
// 1	|50|59|62|53|51|45|47|43| 
// 0	|44|61|63|55|52|56|49|57|   
//		  0  1  2  3  4  5  6  7    (I Axis)  ----->




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Upper view (ASICs SIDE)
//ASIC 90 deg rotation in the trigonometric way compare to the MapISmallHR1/MapJSmallHR1.
//Asics mapping on large chambers with HARDROC I
//                            0		    5		  10		15	      20        25        30        35        40        45        50        55        60
const unsigned short MapJLargeHR1[64]={0,1,2,0,0,0,0,2,0,1,0,0,2,2,1,1,1,1,1,3,1,2,3,2,3,3,2,3,2,3,3,3,4,4,4,4,4,5,4,4,5,4,5,6,7,6,5,6,5,7,6,6,7,6,5,7,7,7,5,6,5,7,6,7};
const unsigned short MapILargeHR1[64]={2,2,2,7,5,1,3,1,4,1,6,0,3,0,3,0,7,5,4,0,6,7,2,5,3,1,4,4,6,7,5,6,7,5,6,4,3,7,1,0,5,2,6,7,0,5,4,6,0,6,0,4,4,3,3,3,5,7,1,1,2,1,2,2};
//(J Axis)
//
// 7	|03|16|21|29|32|37|43|57|  	
// 6	|10|20|28|31|34|42|47|49|  	     
// 5	|04|17|23|30|33|40|45|56|  	     
// 4	|08|18|26|27|35|46|51|52|  	     
// 3	|06|14|12|24|36|54|53|55|  	     TOP VIEW (ASICs SIDE)
// 2	|00|01|02|22|41|60|62|63|  	
// 1	|05|09|07|25|38|58|59|61| 
// 0	|11|15|13|19|39|48|50|44|   
//		  0  1  2  3  4  5  6  7    (I Axis)  ----->
//				|	 |
//				|DIFF|
//				|____|	


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Upper view (ASICs SIDE)
//90 deg rotation in the trigonometric way compare to the MapILargeHR1/MapJLargeHR1.
//180 deg rotation in the trigonometric way compare to the MapISmallHR1/MapJSmallHR1.
//Asics mapping on large chambers with HARDROC II & IIB
//								 0		   5		 10		   15	     20        25        30        35        40        45        50        55        60
const unsigned short MapJLargeHR2[64]={1,1,2,2,3,3,4,4,5,5,6,6,7,7,4,3,2,0,0,1,0,5,6,7,7,6,5,4,3,2,1,0,0,1,2,3,4,5,6,7,7,6,0,0,1,1,0,2,3,4,5,7,7,6,6,5,5,4,4,3,3,2,2,1};
const unsigned short MapILargeHR2[64]={1,0,1,0,1,0,1,0,0,1,0,1,0,1,2,2,2,0,1,2,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,6,5,6,7,5,5,5,5,6,7,6,7,6,7,6,7,7,6,7,6,7};
//(J Axis)
//
// 7	|46|63|61|59|58|56|54|52|  	
// 6	|43|45|62|60|57|55|53|51|  	     
// 5	|42|44|47|48|49|50|41|40|  	     
// 4	|32|33|34|35|36|37|38|39|  	     
// 3	|31|30|29|28|27|26|25|24|  	     TOP VIEW (ASICs SIDE)
// 2	|20|19|16|15|14|21|22|23|  	
// 1	|18|00|02|04|06|09|11|13| 
// 0	|17|01|03|05|07|08|10|12|   
//		  0  1  2  3  4  5  6  7    (I Axis)  ----->
//				|	 |
//				|DIFF|
//				|____|	

unsigned short AsicShiftI[49]={	0,	0,	0,	0,	0,	8,	8,	8,	8,	16,	16,	16,	16,	24,	24,	24,	24,	32,	32,	32,	32,	40,	40,	40,	40,	48,	48,	48,	48,	56,	56,	56,	56,	64,	64,	64,	64,	72,	72,	72,	72,	80,	80,	80,	80,	88,	88,	88,	88};
unsigned short AsicShiftJ[49]={	0,	0,	8,	16,	24,	24,	16,	8,	0,	0,	8,	16,	24,	24,	16,	8,	0,	0,	8,	16,	24,	24,	16,	8,	0,	0,	8,	16,	24,	24,	16,	8,	0,	0,	8,	16,	24,	24,	16,	8,	0,	0,	8,	16,	24,	24,	16,	8,	0};

 
//Small chambers  HR1
if(asicType==0){
 i = MapISmallHR1[ipad]+AsicShiftI[asicid]+1;
 j = MapJSmallHR1[ipad]+AsicShiftJ[asicid]+1;
}
 
//First square meter HR1
if(asicType==1){
 i = MapILargeHR1[ipad]+AsicShiftI[asicid]+1;
 j = MapJLargeHR1[ipad]+AsicShiftJ[asicid]+1;
}


//Second Square meter HR2
if(asicType==2){
 i = MapILargeHR2[ipad]+AsicShiftI[asicid]+1;
 j = MapJLargeHR2[ipad]+AsicShiftJ[asicid]+1;
}
 
}


//Check whats appening if diff is empty (currently it could not be the case)
#ifdef PEUTETREUNJOUR
void  DCBufferReader::computeRecoTime(double tbc,std::vector<uint32_t>& vrawtime, std::vector<uint32_t>& vrecotime)
{
  bool dump=false;
  if (vasic.size()==0) return dump;
  int JumpCnt=0;
  double BCTemp=tbc; // DIF TBC
  for(unsigned int FrameCnt = 0; FrameCnt<vasic.size(); FrameCnt++){
    //    std::cout<<FrameCnt<<" "<<vasic[FrameCnt]->getBunchCrossingTime()<<":"<<BCTemp<<std::endl;
    if((vrawtime[FrameCnt] - BCTemp)>(0.5*0xFFFFFF*DCBufferReader::getDAQ_BC_Period()))
      JumpCnt++;
    BCTemp= vrawtime[FrameCnt];
  }
  //Reminder in this case "AsicTemp.get_SD_RawHit(FrameCnt).get_BC()" and "TriggerBunchCross" are given in us
  
  
  double TriggerOffset = JumpCnt*0xFFFFFF*DCBufferReader::getDAQ_BC_Period(); 
  BCTemp=tbc;  // DIF
  for(unsigned int FrameCnt = 0; FrameCnt<vasic.size(); FrameCnt++){
    if((vrawtime[FrameCnt] - BCTemp)>(0.5*0xFFFFFF*DCBufferReader::getDAQ_BC_Period()))
      JumpCnt--;
    double RecoTime =(tbc + TriggerOffset)  - (vasic[FrameCnt]->getBunchCrossingTime()+(JumpCnt*0xFFFFFF*DCBufferReader::getDAQ_BC_Period()));
    vrecotime[FrameCnt]=RecoTime;
    BCTemp= vrawtime[FrameCnt];
  }

  return;
}
#endif

