#include <limits.h>
#include "TextDIFDBManager.h"
using namespace std;
TextDIFDBManager::TextDIFDBManager(std::string sdifs,std::string directory) : DIFDBManager(sdifs)
{
  theFilePath_=directory;
}

void TextDIFDBManager::initialize()
{
}
uint32_t TextDIFDBManager::LoadDIFMicrorocParameters(uint32_t difid,SingleHardrocV2ConfigurationFrame* ConfigHR2)
{
  std::cout<< __FUNCTION__ <<" Not yet implemented"<<std::endl;
  return 0;
}

uint32_t TextDIFDBManager::LoadDIFHardrocV2Parameters(uint32_t difid,SingleHardrocV2ConfigurationFrame* ConfigHR2)
{
  uint8_t tHardroc=0;
  for (int iasic=0;iasic<MAX_NB_OF_ASICS;iasic++)
    {	
      //  first parse file
      char tFileName[100];
      unsigned long long  tdatal=0llu;
      int tchannel;
      std::string tline;
      int tdata;

      for (int i=0;i<HARDROCV2_SLC_FRAME_SIZE;i++)
	ConfigHR2[tHardroc][i]=0; // fill all  with 0s first
      ConfigHR2[tHardroc][0]=0x01; // N/U bits are 1
      ConfigHR2[tHardroc][1]=0xE0; // N/U bits are 1
	
		
      sprintf (tFileName,"%s/HR2_FT0101%d_%d.cfg",theFilePath_.c_str(),	difid,(iasic+1));// format is "DIFId_HardRocHeaderNb.cfg" stored in config/
#ifdef DEBUG_SLC
      std::cout<<"LoadCurrentDIFHardrocV2ParametersFromFile , tFileName = "<<tFileName<<std::endl;
#endif
      ifstream tFile(tFileName,ios_base::in);
      if (tFile.is_open())
	{		
	  while (	!tFile.eof())
	    {	
	      getline(tFile,tline);
#ifdef  MY_DEBUG_MORE
	      std::cout<<"line read = "<<tline<<std::endl;
#endif								
	      if (tline.find("EnOCDout1b")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"EnOCDout1b = %d\n",&tdata);	
		  ConfigHR2[tHardroc][0]|=((tdata&0x01)<<7);
		}
	      else if (tline.find("EnOCDout2b")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"EnOCDout2b = %d\n",&tdata);	
		  ConfigHR2[tHardroc][0]|=((tdata&0x01)<<6);
		}
	      else if (tline.find("EnOCTransmitOn1b")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"EnOCTransmitOn1b = %d\n",&tdata);	
		  ConfigHR2[tHardroc][0]|=((tdata&0x01)<<5);
		}
	      else if (tline.find("EnOCTransmitOn2b")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"EnOCTransmitOn2b = %d\n",&tdata);	
		  ConfigHR2[tHardroc][0]|=((tdata&0x01)<<4);
		}
	      else if (tline.find("EnOCchipSatb")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"EnOCchipSatb = %d\n",&tdata);	
		  ConfigHR2[tHardroc][0]|=((tdata&0x01)<<3);
		}
	      else if (tline.find("SelStartReadout")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"SelStartReadout = %d\n",&tdata);	
		  ConfigHR2[tHardroc][0]|=((tdata&0x01)<<2);
		}
	      else if (tline.find("SelEndReadout")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"SelEndReadout = %d\n",&tdata);	
		  ConfigHR2[tHardroc][0]|=((tdata&0x01)<<1);
		}
	      else if (tline.find("ClkMux")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"ClkMux = %d\n",&tdata);	
		  ConfigHR2[tHardroc][1]|=((tdata&0x01)<<4);
		}
	      else if (tline.find("ScOn")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"ScOn = %d\n",&tdata);	
		  ConfigHR2[tHardroc][1]|=((tdata&0x01)<<3);
		}
	      else if (tline.find("RazChnExtVal")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"RazChnExtVal = %d\n",&tdata);	
		  ConfigHR2[tHardroc][1]|=((tdata&0x01)<<2);
		}
	      else if (tline.find("RazChnIntVal")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"RazChnIntVal = %d\n",&tdata);	
		  ConfigHR2[tHardroc][1]|=((tdata&0x01)<<1);
		}
	      else if (tline.find("TrigExtVal")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"TrigExtVal = %d\n",&tdata);	
		  ConfigHR2[tHardroc][1]|=((tdata&0x01)<<0);
		}
	      else if (tline.find("DiscrOrOr")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"DiscrOrOr = %d\n",&tdata);	
		  ConfigHR2[tHardroc][2]|=((tdata&0x01)<<7);
		}
	      else if (tline.find("EnTrigOut")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"EnTrigOut = %d\n",&tdata);	
		  ConfigHR2[tHardroc][2]|=((tdata&0x01)<<6);
		}
	      else if (tline.find("Trig0b")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Trig0b = %d\n",&tdata);	
		  ConfigHR2[tHardroc][2]|=((tdata&0x01)<<5);
		}
	      else if (tline.find("Trig1b")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Trig1b = %d\n",&tdata);	
		  ConfigHR2[tHardroc][2]|=((tdata&0x01)<<4);
		}
	      else if (tline.find("Trig2b")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Trig2b = %d\n",&tdata);	
		  ConfigHR2[tHardroc][2]|=((tdata&0x01)<<3);
		}
	      else if (tline.find("OtaBgSw")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"OtaBgSw = %d\n",&tdata);	
		  ConfigHR2[tHardroc][2]|=((tdata&0x01)<<2);
		}
	      else if (tline.find("DacSw")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"DacSw = %d\n",&tdata);	
		  ConfigHR2[tHardroc][2]|=((tdata&0x01)<<1);
		}
	      else if (tline.find("SmallDac")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"SmallDac = %d\n",&tdata);	
		  ConfigHR2[tHardroc][2]|=((tdata&0x01)<<0);
		}
	      else if (tline.find("B2 =")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"B2 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][3]=((tdata>>2)&0xFF);
		  ConfigHR2[tHardroc][4]|=((tdata&0x03)<<6);
		}
	      else if (tline.find("B1 =")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"B1 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][4]|=((tdata>>4)&0x3F);
		  ConfigHR2[tHardroc][5]|=((tdata&0x0F)<<4);
		}
	      else if (tline.find("B0 =")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"B0 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][5]|=((tdata>>6)&0x0F);
		  ConfigHR2[tHardroc][6]|=((tdata&0x3F)<<2);
		}
	      else if (tline.find("Header")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Header = %d\n",&tdata);	
		  ConfigHR2[tHardroc][6]|=(tdata&0x01)<<1;
		  ConfigHR2[tHardroc][6]|=(tdata&0x02)>>1;
		  ConfigHR2[tHardroc][7]|=(tdata&0x04)<<5;
		  ConfigHR2[tHardroc][7]|=(tdata&0x08)<<3;
		  ConfigHR2[tHardroc][7]|=(tdata&0x10)<<1;
		  ConfigHR2[tHardroc][7]|=(tdata&0x20)>>1;
		  ConfigHR2[tHardroc][7]|=(tdata&0x40)>>3;
		  ConfigHR2[tHardroc][7]|=(tdata&0x80)>>5;
		}
	      else if (tline.find("Mask2")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Mask2 = %llu\n",&tdatal);	
		  ConfigHR2[tHardroc][7]|= ((tdatal&0x8000000000000000llu)>>62); //63	ok
		  ConfigHR2[tHardroc][8]|= ((tdatal&0x4000000000000000llu)>>56); //62	ok
		  ConfigHR2[tHardroc][8]|= ((tdatal&0x2000000000000000llu)>>58); //61	ok
		  ConfigHR2[tHardroc][8]|= ((tdatal&0x1000000000000000llu)>>60); //60	ok
		  ConfigHR2[tHardroc][9]|= ((tdatal&0x0800000000000000llu)>>54); //59	ok
		  ConfigHR2[tHardroc][9]|= ((tdatal&0x0400000000000000llu)>>56); //58	ok
		  ConfigHR2[tHardroc][10]|=((tdatal&0x0200000000000000llu)>>50); //57	ok
		  ConfigHR2[tHardroc][10]|=((tdatal&0x0100000000000000llu)>>52); //56	ok
		  ConfigHR2[tHardroc][10]|=((tdatal&0x0080000000000000llu)>>54); //55	ok
		  ConfigHR2[tHardroc][11]|=((tdatal&0x0040000000000000llu)>>48); //54	ok
		  ConfigHR2[tHardroc][11]|=((tdatal&0x0020000000000000llu)>>50); //53	ok
		  ConfigHR2[tHardroc][11]|=((tdatal&0x0010000000000000llu)>>52); //52	ok
		  ConfigHR2[tHardroc][12]|=((tdatal&0x0008000000000000llu)>>46); //51	ok
		  ConfigHR2[tHardroc][12]|=((tdatal&0x0004000000000000llu)>>48); //50	ok
		  ConfigHR2[tHardroc][13]|=((tdatal&0x0002000000000000llu)>>42); //49	ok
		  ConfigHR2[tHardroc][13]|=((tdatal&0x0001000000000000llu)>>44); //48	ok
		  ConfigHR2[tHardroc][13]|=((tdatal&0x0000800000000000llu)>>46); //47	ok
		  ConfigHR2[tHardroc][14]|=((tdatal&0x0000400000000000llu)>>40); //46	ok
		  ConfigHR2[tHardroc][14]|=((tdatal&0x0000200000000000llu)>>42); //45	ok
		  ConfigHR2[tHardroc][14]|=((tdatal&0x0000100000000000llu)>>44); //44	ok
		  ConfigHR2[tHardroc][15]|=((tdatal&0x0000080000000000llu)>>38); //43	ok
		  ConfigHR2[tHardroc][15]|=((tdatal&0x0000040000000000llu)>>40); //42	ok
		  ConfigHR2[tHardroc][16]|=((tdatal&0x0000020000000000llu)>>34); //41 ok	
		  ConfigHR2[tHardroc][16]|=((tdatal&0x0000010000000000llu)>>36); //40	ok
		  ConfigHR2[tHardroc][16]|=((tdatal&0x0000008000000000llu)>>38); //39	ok
		  ConfigHR2[tHardroc][17]|=((tdatal&0x0000004000000000llu)>>32); //38	ok
		  ConfigHR2[tHardroc][17]|=((tdatal&0x0000002000000000llu)>>34); //37	ok
		  ConfigHR2[tHardroc][17]|=((tdatal&0x0000001000000000llu)>>36); //36	ok
		  ConfigHR2[tHardroc][18]|=((tdatal&0x0000000800000000llu)>>30); //35	ok
		  ConfigHR2[tHardroc][18]|=((tdatal&0x0000000400000000llu)>>32); //34	ok
		  ConfigHR2[tHardroc][19]|=((tdatal&0x0000000200000000llu)>>26); //33	ok
		  ConfigHR2[tHardroc][19]|=((tdatal&0x0000000100000000llu)>>28); //32	ok 
		  ConfigHR2[tHardroc][19]|=((tdatal&0x0000000080000000llu)>>30); //31	ok
		  ConfigHR2[tHardroc][20]|=((tdatal&0x0000000040000000llu)>>24); //30	ok
		  ConfigHR2[tHardroc][20]|=((tdatal&0x0000000020000000llu)>>26); //29	ok
		  ConfigHR2[tHardroc][20]|=((tdatal&0x0000000010000000llu)>>28); //28	ok
		  ConfigHR2[tHardroc][21]|=((tdatal&0x0000000008000000llu)>>22); //27	ok
		  ConfigHR2[tHardroc][21]|=((tdatal&0x0000000004000000llu)>>24); //26	ok
		  ConfigHR2[tHardroc][22]|=((tdatal&0x0000000002000000llu)>>18); //25	ok
		  ConfigHR2[tHardroc][22]|=((tdatal&0x0000000001000000llu)>>20); //24	ok
		  ConfigHR2[tHardroc][22]|=((tdatal&0x0000000000800000llu)>>22); //23	ok
		  ConfigHR2[tHardroc][23]|=((tdatal&0x0000000000400000llu)>>16); //22	ok
		  ConfigHR2[tHardroc][23]|=((tdatal&0x0000000000200000llu)>>18); //21	ok 
		  ConfigHR2[tHardroc][23]|=((tdatal&0x0000000000100000llu)>>20); //20	ok
		  ConfigHR2[tHardroc][24]|=((tdatal&0x0000000000080000llu)>>14); //19	ok
		  ConfigHR2[tHardroc][24]|=((tdatal&0x0000000000040000llu)>>16); //18	ok
		  ConfigHR2[tHardroc][25]|=((tdatal&0x0000000000020000llu)>>10); //17	ok
		  ConfigHR2[tHardroc][25]|=((tdatal&0x0000000000010000llu)>>12); //16	ok
		  ConfigHR2[tHardroc][25]|=((tdatal&0x0000000000008000llu)>>14); //15 ok
		  ConfigHR2[tHardroc][26]|=((tdatal&0x0000000000004000llu)>>8);//14 ok
		  ConfigHR2[tHardroc][26]|=((tdatal&0x0000000000002000llu)>>10); //13 ok
		  ConfigHR2[tHardroc][26]|=((tdatal&0x0000000000001000llu)>>12); //12 ok
		  ConfigHR2[tHardroc][27]|=((tdatal&0x0000000000000800llu)>>6);//11 ok
		  ConfigHR2[tHardroc][27]|=((tdatal&0x0000000000000400llu)>>8);//10 ok
		  ConfigHR2[tHardroc][28]|=((tdatal&0x0000000000000200llu)>>2);//9 ok
		  ConfigHR2[tHardroc][28]|=((tdatal&0x0000000000000100llu)>>4);//8 ok
		  ConfigHR2[tHardroc][28]|=((tdatal&0x0000000000000080llu)>>6);//7 ok
		  ConfigHR2[tHardroc][29]|=((tdatal&0x0000000000000040llu)>>0);//6 ok
		  ConfigHR2[tHardroc][29]|=((tdatal&0x0000000000000020llu)>>2);//5 ok
		  ConfigHR2[tHardroc][29]|=((tdatal&0x0000000000000010llu)>>4);//4 ok
		  ConfigHR2[tHardroc][30]|=((tdatal&0x0000000000000008llu)<<2);//3 ok
		  ConfigHR2[tHardroc][30]|=((tdatal&0x0000000000000004llu)<<0);//2 ok
		  ConfigHR2[tHardroc][31]|=((tdatal&0x0000000000000002llu)<<6);//1 ok
		  ConfigHR2[tHardroc][31]|=((tdatal&0x0000000000000001llu)<<4);//0 ok
		}
	      else if (tline.find("Mask1")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Mask1 = %llu\n",&tdatal);		
		  ConfigHR2[tHardroc][7]|= ((tdatal&0x8000000000000000llu)>>63); //63 ok		
		  ConfigHR2[tHardroc][8]|= ((tdatal&0x4000000000000000llu)>>57); //62	ok	
		  ConfigHR2[tHardroc][8]|= ((tdatal&0x2000000000000000llu)>>59); //61	ok	
		  ConfigHR2[tHardroc][9]|= ((tdatal&0x1000000000000000llu)>>53); //60	ok	
		  ConfigHR2[tHardroc][9]|= ((tdatal&0x0800000000000000llu)>>55); //59	ok	
		  ConfigHR2[tHardroc][9]|= ((tdatal&0x0400000000000000llu)>>57); //58	ok	
		  ConfigHR2[tHardroc][10]|=((tdatal&0x0200000000000000llu)>>51); //57	ok	
		  ConfigHR2[tHardroc][10]|=((tdatal&0x0100000000000000llu)>>53); //56		ok
		  ConfigHR2[tHardroc][10]|=((tdatal&0x0080000000000000llu)>>55); //55	ok	
		  ConfigHR2[tHardroc][11]|=((tdatal&0x0040000000000000llu)>>49); //54		ok
		  ConfigHR2[tHardroc][11]|=((tdatal&0x0020000000000000llu)>>51); //53	ok	
		  ConfigHR2[tHardroc][12]|=((tdatal&0x0010000000000000llu)>>45); //52		ok
		  ConfigHR2[tHardroc][12]|=((tdatal&0x0008000000000000llu)>>47); //51		ok
		  ConfigHR2[tHardroc][12]|=((tdatal&0x0004000000000000llu)>>49); //50		ok
		  ConfigHR2[tHardroc][13]|=((tdatal&0x0002000000000000llu)>>43); //49		ok
		  ConfigHR2[tHardroc][13]|=((tdatal&0x0001000000000000llu)>>45); //48	ok	
		  ConfigHR2[tHardroc][13]|=((tdatal&0x0000800000000000llu)>>47); //47	ok	
		  ConfigHR2[tHardroc][14]|=((tdatal&0x0000400000000000llu)>>41); //46		ok
		  ConfigHR2[tHardroc][14]|=((tdatal&0x0000200000000000llu)>>43); //45		ok
		  ConfigHR2[tHardroc][15]|=((tdatal&0x0000100000000000llu)>>37); //44		ok
		  ConfigHR2[tHardroc][15]|=((tdatal&0x0000080000000000llu)>>39); //43		ok
		  ConfigHR2[tHardroc][15]|=((tdatal&0x0000040000000000llu)>>41); //42		ok
		  ConfigHR2[tHardroc][16]|=((tdatal&0x0000020000000000llu)>>35); //41		ok
		  ConfigHR2[tHardroc][16]|=((tdatal&0x0000010000000000llu)>>37); //40		ok
		  ConfigHR2[tHardroc][16]|=((tdatal&0x0000008000000000llu)>>39);  //39	ok	
		  ConfigHR2[tHardroc][17]|=((tdatal&0x0000004000000000llu)>>33); //38		ok
		  ConfigHR2[tHardroc][17]|=((tdatal&0x0000002000000000llu)>>35); //37		ok
		  ConfigHR2[tHardroc][18]|=((tdatal&0x0000001000000000llu)>>29); //36		ok
		  ConfigHR2[tHardroc][18]|=((tdatal&0x0000000800000000llu)>>31); //35		ok
		  ConfigHR2[tHardroc][18]|=((tdatal&0x0000000400000000llu)>>33); //34		ok
		  ConfigHR2[tHardroc][19]|=((tdatal&0x0000000200000000llu)>>27); //33		ok
		  ConfigHR2[tHardroc][19]|=((tdatal&0x0000000100000000llu)>>29); //32		ok
		  ConfigHR2[tHardroc][19]|=((tdatal&0x0000000080000000llu)>>31); //31	ok	
		  ConfigHR2[tHardroc][20]|=((tdatal&0x0000000040000000llu)>>25); //30		ok
		  ConfigHR2[tHardroc][20]|=((tdatal&0x0000000020000000llu)>>27); //29		ok
		  ConfigHR2[tHardroc][21]|=((tdatal&0x0000000010000000llu)>>21); //28		ok
		  ConfigHR2[tHardroc][21]|=((tdatal&0x0000000008000000llu)>>23); //27		ok
		  ConfigHR2[tHardroc][21]|=((tdatal&0x0000000004000000llu)>>25); //26		ok
		  ConfigHR2[tHardroc][22]|=((tdatal&0x0000000002000000llu)>>19); //25		ok
		  ConfigHR2[tHardroc][22]|=((tdatal&0x0000000001000000llu)>>21); //24		ok
		  ConfigHR2[tHardroc][22]|=((tdatal&0x0000000000800000llu)>>23); //23	ok	
		  ConfigHR2[tHardroc][23]|=((tdatal&0x0000000000400000llu)>>17); //22		ok
		  ConfigHR2[tHardroc][23]|=((tdatal&0x0000000000200000llu)>>19); //21		ok
		  ConfigHR2[tHardroc][24]|=((tdatal&0x0000000000100000llu)>>13); //20		ok
		  ConfigHR2[tHardroc][24]|=((tdatal&0x0000000000080000llu)>>15); //19		ok
		  ConfigHR2[tHardroc][24]|=((tdatal&0x0000000000040000llu)>>17); //18		ok
		  ConfigHR2[tHardroc][25]|=((tdatal&0x0000000000020000llu)>>11); //17		ok
		  ConfigHR2[tHardroc][25]|=((tdatal&0x0000000000010000llu)>>13); //16		ok
		  ConfigHR2[tHardroc][25]|=((tdatal&0x0000000000008000llu)>>15); //15	ok	
		  ConfigHR2[tHardroc][26]|=((tdatal&0x0000000000004000llu)>>9); //14	ok
		  ConfigHR2[tHardroc][26]|=((tdatal&0x0000000000002000llu)>>11); 	//13		ok
		  ConfigHR2[tHardroc][27]|=((tdatal&0x0000000000001000llu)>>5); //12	ok
		  ConfigHR2[tHardroc][27]|=((tdatal&0x0000000000000800llu)>>7); //11	ok
		  ConfigHR2[tHardroc][27]|=((tdatal&0x0000000000000400llu)>>9); //10	ok
		  ConfigHR2[tHardroc][28]|=((tdatal&0x0000000000000200llu)>>3); //9	ok
		  ConfigHR2[tHardroc][28]|=((tdatal&0x0000000000000100llu)>>5); //8	ok
		  ConfigHR2[tHardroc][28]|=((tdatal&0x0000000000000080llu)>>7); //7	ok
		  ConfigHR2[tHardroc][29]|=((tdatal&0x0000000000000040llu)>>1); //6	ok
		  ConfigHR2[tHardroc][29]|=((tdatal&0x0000000000000020llu)>>3); //5	ok
		  ConfigHR2[tHardroc][30]|=((tdatal&0x0000000000000010llu)<<3); //4	 ok
		  ConfigHR2[tHardroc][30]|=((tdatal&0x0000000000000008llu)<<1); //3	ok
		  ConfigHR2[tHardroc][30]|=((tdatal&0x0000000000000004llu)>>1); //2	ok
		  ConfigHR2[tHardroc][31]|=((tdatal&0x0000000000000002llu)<<5); //1	ok
		  ConfigHR2[tHardroc][31]|=((tdatal&0x0000000000000001llu)<<3); //0	ok
		}
	      else if (tline.find("Mask0")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Mask0 = %llu\n",&tdatal);	
		  ConfigHR2[tHardroc][8]|= ((tdatal&0x8000000000000000llu)>>56); //63	ok
		  ConfigHR2[tHardroc][8]|= ((tdatal&0x4000000000000000llu)>>58); //62	ok
		  ConfigHR2[tHardroc][8]|= ((tdatal&0x2000000000000000llu)>>60); //61	ok
		  ConfigHR2[tHardroc][9]|= ((tdatal&0x1000000000000000llu)>>54); //60	ok
		  ConfigHR2[tHardroc][9]|= ((tdatal&0x0800000000000000llu)>>56); //59	ok
		  ConfigHR2[tHardroc][9]|= ((tdatal&0x0400000000000000llu)>>58); //58ok	
		  ConfigHR2[tHardroc][10]|=((tdatal&0x0200000000000000llu)>>52); //57	ok
		  ConfigHR2[tHardroc][10]|=((tdatal&0x0100000000000000llu)>>54); //56	ok
		  ConfigHR2[tHardroc][11]|=((tdatal&0x0080000000000000llu)>>48); //55	ok
		  ConfigHR2[tHardroc][11]|=((tdatal&0x0040000000000000llu)>>50); //54	ok
		  ConfigHR2[tHardroc][11]|=((tdatal&0x0020000000000000llu)>>52); //53	ok
		  ConfigHR2[tHardroc][12]|=((tdatal&0x0010000000000000llu)>>46); //52	ok
		  ConfigHR2[tHardroc][12]|=((tdatal&0x0008000000000000llu)>>48); //51	ok
		  ConfigHR2[tHardroc][12]|=((tdatal&0x0004000000000000llu)>>50); //50	ok
		  ConfigHR2[tHardroc][13]|=((tdatal&0x0002000000000000llu)>>44); //49	ok
		  ConfigHR2[tHardroc][13]|=((tdatal&0x0001000000000000llu)>>46); //48	ok
		  ConfigHR2[tHardroc][14]|=((tdatal&0x0000800000000000llu)>>40); //47	ok
		  ConfigHR2[tHardroc][14]|=((tdatal&0x0000400000000000llu)>>42); //46	ok
		  ConfigHR2[tHardroc][14]|=((tdatal&0x0000200000000000llu)>>44); //45	ok
		  ConfigHR2[tHardroc][15]|=((tdatal&0x0000100000000000llu)>>38); //44	ok
		  ConfigHR2[tHardroc][15]|=((tdatal&0x0000080000000000llu)>>40); //43	ok
		  ConfigHR2[tHardroc][15]|=((tdatal&0x0000040000000000llu)>>42); //42	ok
		  ConfigHR2[tHardroc][16]|=((tdatal&0x0000020000000000llu)>>36); //41	ok
		  ConfigHR2[tHardroc][16]|=((tdatal&0x0000010000000000llu)>>38); //40	ok
		  ConfigHR2[tHardroc][17]|=((tdatal&0x0000008000000000llu)>>32); //39	ok
		  ConfigHR2[tHardroc][17]|=((tdatal&0x0000004000000000llu)>>34); //38	
		  ConfigHR2[tHardroc][17]|=((tdatal&0x0000002000000000llu)>>36); //37	ok
		  ConfigHR2[tHardroc][18]|=((tdatal&0x0000001000000000llu)>>30); //36	ok
		  ConfigHR2[tHardroc][18]|=((tdatal&0x0000000800000000llu)>>32); //35	ok
		  ConfigHR2[tHardroc][18]|=((tdatal&0x0000000400000000llu)>>34); //34ok	
		  ConfigHR2[tHardroc][19]|=((tdatal&0x0000000200000000llu)>>28); //33	ok
		  ConfigHR2[tHardroc][19]|=((tdatal&0x0000000100000000llu)>>30); //32	ok
		  ConfigHR2[tHardroc][20]|=((tdatal&0x0000000080000000llu)>>24); //31	ok
		  ConfigHR2[tHardroc][20]|=((tdatal&0x0000000040000000llu)>>26); //30	ok
		  ConfigHR2[tHardroc][20]|=((tdatal&0x0000000020000000llu)>>28); //29	ok
		  ConfigHR2[tHardroc][21]|=((tdatal&0x0000000010000000llu)>>22); //28	ok
		  ConfigHR2[tHardroc][21]|=((tdatal&0x0000000008000000llu)>>24); //27	ok
		  ConfigHR2[tHardroc][21]|=((tdatal&0x0000000004000000llu)>>26); //26	ok
		  ConfigHR2[tHardroc][22]|=((tdatal&0x0000000002000000llu)>>20); //25	ok
		  ConfigHR2[tHardroc][22]|=((tdatal&0x0000000001000000llu)>>22); //24	ok
		  ConfigHR2[tHardroc][23]|=((tdatal&0x0000000000800000llu)>>16); //23	ok
		  ConfigHR2[tHardroc][23]|=((tdatal&0x0000000000400000llu)>>18); //22	ok
		  ConfigHR2[tHardroc][23]|=((tdatal&0x0000000000200000llu)>>20); //21	ok
		  ConfigHR2[tHardroc][24]|=((tdatal&0x0000000000100000llu)>>14); //20	ok
		  ConfigHR2[tHardroc][24]|=((tdatal&0x0000000000080000llu)>>16); //19	ok
		  ConfigHR2[tHardroc][24]|=((tdatal&0x0000000000040000llu)>>18); //18	ok
		  ConfigHR2[tHardroc][25]|=((tdatal&0x0000000000020000llu)>>12); //17	ok
		  ConfigHR2[tHardroc][25]|=((tdatal&0x0000000000010000llu)>>14); //16	ok
		  ConfigHR2[tHardroc][26]|=((tdatal&0x0000000000008000llu)>>8);//15ok
		  ConfigHR2[tHardroc][26]|=((tdatal&0x0000000000004000llu)>>10); //14	ok
		  ConfigHR2[tHardroc][26]|=((tdatal&0x0000000000002000llu)>>12); 		//13	ok
		  ConfigHR2[tHardroc][27]|=((tdatal&0x0000000000001000llu)>>6);//12 ok
		  ConfigHR2[tHardroc][27]|=((tdatal&0x0000000000000800llu)>>8);//11ok
		  ConfigHR2[tHardroc][27]|=((tdatal&0x0000000000000400llu)>>10); //10ok
		  ConfigHR2[tHardroc][28]|=((tdatal&0x0000000000000200llu)>>4);//9ok
		  ConfigHR2[tHardroc][28]|=((tdatal&0x0000000000000100llu)>>6);//8ok
		  ConfigHR2[tHardroc][29]|=((tdatal&0x0000000000000080llu)>>0);//7ok
		  ConfigHR2[tHardroc][29]|=((tdatal&0x0000000000000040llu)>>2);//6ok
		  ConfigHR2[tHardroc][29]|=((tdatal&0x0000000000000020llu)>>4);//5ok
		  ConfigHR2[tHardroc][30]|=((tdatal&0x0000000000000010llu)<<2);//4 ok
		  ConfigHR2[tHardroc][30]|=((tdatal&0x0000000000000008llu)<<0);//3ok
		  ConfigHR2[tHardroc][30]|=((tdatal&0x0000000000000004llu)>>2);//2 ok
		  ConfigHR2[tHardroc][31]|=((tdatal&0x0000000000000002llu)<<4);//1 ok
		  ConfigHR2[tHardroc][31]|=((tdatal&0x0000000000000001llu)<<2);//0ok
		}
	      else if (tline.find("RS_or_Discri")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"RS_or_Discri = %d\n",&tdata);	
		  ConfigHR2[tHardroc][31]|=((tdata&0x01)<<1);
		}
	      else if (tline.find("Discri1")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Discri1 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][31]|=((tdata&0x01)<<0);
		}
	      else if (tline.find("Discri2")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Discri2 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][32]|=((tdata&0x01)<<7);
		}
	      else if (tline.find("Discri0")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Discri0 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][32]|=((tdata&0x01)<<6);
		}
	      else if (tline.find("OtaQ_PwrADC")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"OtaQ_PwrADC = %d\n",&tdata);	
		  ConfigHR2[tHardroc][32]|=((tdata&0x01)<<5);
		}
	      else if (tline.find("En_OtaQ")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"En_OtaQ = %d\n",&tdata);	
		  ConfigHR2[tHardroc][32]|=((tdata&0x01)<<4);
		}
	      else if (tline.find("Sw50f0")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Sw50f0 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][32]|=((tdata&0x01)<<3);
		}
	      else if (tline.find("Sw100f0")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Sw100f0 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][32]|=((tdata&0x01)<<2);
		}
	      else if (tline.find("Sw100k0")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Sw100k0 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][32]|=((tdata&0x01)<<1);
		}
	      else if (tline.find("Sw50k0")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Sw50k0 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][32]|=((tdata&0x01)<<0);
		}
	      else if (tline.find("PwrOnFsb1")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"PwrOnFsb1 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][33]|=((tdata&0x01)<<7);
		}
	      else if (tline.find("PwrOnFsb2")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"PwrOnFsb2 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][33]|=((tdata&0x01)<<6);
		}
	      else if (tline.find("PwrOnFsb0")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"PwrOnFsb0 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][33]|=((tdata&0x01)<<5);
		}
	      else if (tline.find("Sel1")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Sel1 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][33]|=((tdata&0x01)<<4);
		}
	      else if (tline.find("Sel0")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Sel0 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][33]|=((tdata&0x01)<<3);
		}
	      else if (tline.find("Sw50f1")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Sw50f1 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][33]|=((tdata&0x01)<<2);
		}
	      else if (tline.find("Sw100f1")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Sw100f1 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][33]|=((tdata&0x01)<<1);
		}
	      else if (tline.find("Sw100k1")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Sw100k1 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][33]|=((tdata&0x01)<<0);
		}
	      else if (tline.find("Sw50k1")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Sw50k1 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][34]|=((tdata&0x01)<<7);
		}
	      else if (tline.find("Cmdb0Fsb1")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Cmdb0Fsb1 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][34]|=((tdata&0x01)<<6);
		}
	      else if (tline.find("Cmdb1Fsb1")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Cmdb1Fsb1 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][34]|=((tdata&0x01)<<5);
		}
	      else if (tline.find("Cmdb2Fsb1")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Cmdb2Fsb1 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][34]|=((tdata&0x01)<<4);
		}
	      else if (tline.find("Cmdb3Fsb1")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Cmdb3Fsb1 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][34]|=((tdata&0x01)<<3);
		}
	      else if (tline.find("Sw50f2")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Sw50f2 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][34]|=((tdata&0x01)<<2);
		}
	      else if (tline.find("Sw100f2")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Sw100f2 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][34]|=((tdata&0x01)<<1);
		}
	      else if (tline.find("Sw100k2")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Sw100k2 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][34]|=((tdata&0x01)<<0);
		}
	      else if (tline.find("Sw50k2")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Sw50k2 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][35]|=((tdata&0x01)<<7);
		}
	      else if (tline.find("Cmdb0Fsb2")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Cmdb0Fsb2 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][35]|=((tdata&0x01)<<6);
		}
	      else if (tline.find("Cmdb1Fsb2")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Cmdb1Fsb2 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][35]|=((tdata&0x01)<<5);
		}
	      else if (tline.find("Cmdb2Fsb2")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Cmdb2Fsb2 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][35]|=((tdata&0x01)<<4);
		}
	      else if (tline.find("Cmdb3Fsb2")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"Cmdb3Fsb2 = %d\n",&tdata);	
		  ConfigHR2[tHardroc][35]|=((tdata&0x01)<<3);
		}
	      else if (tline.find("PwrOnW")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"PwrOnW = %d\n",&tdata);	
		  ConfigHR2[tHardroc][35]|=((tdata&0x01)<<2);
		}
	      else if (tline.find("PwrOnSS")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"PwrOnSS = %d\n",&tdata);	
		  ConfigHR2[tHardroc][35]|=((tdata&0x01)<<1);
		}
	      else if (tline.find("PwrOnBuff")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"PwrOnBuff = %d\n",&tdata);	
		  ConfigHR2[tHardroc][35]|=((tdata&0x01)<<0);
		}
	      else if (tline.find("SwSsc")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"SwSsc = %d\n",&tdata);	
		  ConfigHR2[tHardroc][36]|=((tdata&0x07)<<5);
		}
	      else if (tline.find("CmdB0SS")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"CmdB0SS = %d\n",&tdata);	
		  ConfigHR2[tHardroc][36]|=((tdata&0x01)<<4);
		}
	      else if (tline.find("CmdB1SS")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"CmdB1SS = %d\n",&tdata);	
		  ConfigHR2[tHardroc][36]|=((tdata&0x01)<<3);
		}
	      else if (tline.find("CmdB2SS")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"CmdB2SS = %d\n",&tdata);	
		  ConfigHR2[tHardroc][36]|=((tdata&0x01)<<2);
		}
	      else if (tline.find("CmdB3SS")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"CmdB3SS = %d\n",&tdata);	
		  ConfigHR2[tHardroc][36]|=((tdata&0x01)<<1);
		}
	      else if (tline.find("PwrOnPA")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"PwrOnPA = %d\n",&tdata);	
		  ConfigHR2[tHardroc][36]|=((tdata&0x01)<<0);
		}			
	      else if (tline.find("PaGain")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"PaGain%d = %d\n",&tchannel,&tdata);	
		  ConfigHR2[tHardroc][37+(63-tchannel)]|=(tdata&0xFF);
		}
	      else if (tline.find("cTest")!=string::npos)			
		{			
		  sscanf(tline.c_str(),"cTest = %llu\n",&tdatal);	
		  ConfigHR2[tHardroc][101]= ((tdatal&0xFF00000000000000llu)>>56);
		  ConfigHR2[tHardroc][102]= ((tdatal&0x00FF000000000000llu)>>48);
		  ConfigHR2[tHardroc][103]= ((tdatal&0x0000FF0000000000llu)>>40);
		  ConfigHR2[tHardroc][104]= ((tdatal&0x000000FF00000000llu)>>32);
		  ConfigHR2[tHardroc][105]= ((tdatal&0x00000000FF000000llu)>>24);
		  ConfigHR2[tHardroc][106]= ((tdatal&0x0000000000FF0000llu)>>16);
		  ConfigHR2[tHardroc][107]= ((tdatal&0x000000000000FF00llu)>>8);
		  ConfigHR2[tHardroc][108]= ((tdatal&0x00000000000000FFllu)>>0);
		}
	    }	//while (	!tFile.eof())
	  tFile.close();
	  tHardroc++;
	}	//if (tFile.is_open())
      else
	{
	  std::cout<<"ASIC file is missing for DIF "<<difid<<" Asic "<<(iasic+1)<<std::endl;
	}
    }//	for (int tHardroc=0;tHardroc<NbOfASICs;tHardroc++)
  return tHardroc;
}
void TextDIFDBManager::LoadDIFDefaultParameters(uint32_t difid, UsbDIFSettings* v)
{
  //  first parse file
  char tFileName[100];
  std::string tline;
  
  sprintf (tFileName,"%s/DIF_FT101%d.cfg",theFilePath_.c_str(),difid);// format is "DIF_difId.cfg" stored in config/
#ifdef DEBUG_DIF_DEFAULT
  std::cout<<"LoadCurrentDIFDefaultParametersFromFile , tFileName = "<<tFileName<<std::endl;
#endif
  ifstream tFile(tFileName,ios_base::in);
  if (tFile.is_open())
    {		
      while (	!tFile.eof())
	{	
	  getline(tFile,tline);
#ifdef  DEBUG_DIF_DEFAULT
	  std::cout<<"line read = "<<tline<<std::endl;
#endif								
	  if (tline.find("Monitored Channel ")!=string::npos)			
	    sscanf(tline.c_str(),"Monitored Channel = %d\n",&v->MonitoredChannel);	
	  else if (tline.find("Monitor Sequencer")!=string::npos)			
	    sscanf(tline.c_str(),"Monitor Sequencer = %d\n",&v->MonitorSequencer);	
	  else if (tline.find("Numerical Readout Mode")!=string::npos)			
	    sscanf(tline.c_str(),"Numerical Readout Mode = %d\n",&v->NumericalReadoutMode);	
	  else if (tline.find("Numerical Readout Start Mode")!=string::npos)			
	    sscanf(tline.c_str(),"Numerical Readout Start Mode = %d\n",&v->NumericalReadoutStartMode);	
	  else if (tline.find("AVDD Shdn")!=string::npos)			
	    sscanf(tline.c_str(),"AVDD Shdn = %d\n",&v->AVDDShdn);	
	  else if (tline.find("DVDD Shdn")!=string::npos)			
	    sscanf(tline.c_str()," DVDD Shdn = %d\n",&v->DVDDShdn);	
	  else if (tline.find("DIF IMon Gain")!=string::npos)			
	    {	
	      sscanf(tline.c_str(),"DIF IMon Gain = %d\n",&v->DIFIMonGain);
	      if (v->DIFIMonGain==50) v->DIFIMonGain=0;
	      else v->DIFIMonGain=1;
	    }
	  else if (tline.find("Slab IMon Gain")!=string::npos)			
	    {	
	      sscanf(tline.c_str(),"Slab IMon Gain = %d\n",&v->SlabIMonGain);	
	      if (v->SlabIMonGain==50) v->SlabIMonGain=0;
	      else v->SlabIMonGain=1;
	    }
	  else if (tline.find("Power Analog")!=string::npos)			
	    sscanf(tline.c_str(),"Power Analog = %d\n",&v->PowerAnalog);	
	  else if (tline.find("Power Digital")!=string::npos)			
	    sscanf(tline.c_str(),"Power Digital = %d\n",&v->PowerDigital);	
	  else if (tline.find("Power DAC")!=string::npos)			
	    sscanf(tline.c_str(),"Power DAC = %d\n",&v->PowerDAC);	
	  else if (tline.find("Power ADC")!=string::npos)			
	    sscanf(tline.c_str(),"Power ADC = %d\n",&v->PowerADC);	
	  else if (tline.find("Power SS")!=string::npos)			
	    sscanf(tline.c_str(),"Power SS = %d\n",&v->PowerSS);	
	  else if (tline.find("Timer Hold Register")!=string::npos)			
	    sscanf(tline.c_str(),"Timer Hold Register = %d\n",&v->TimerHoldRegister);	
	  else if (tline.find("Enable Monitoring")!=string::npos)			
	    sscanf(tline.c_str(),"Enable Monitoring = %d\n",&v->EnableMonitoring);	
	  else if (tline.find("Memory Display Limit Max")!=string::npos)			
	    sscanf(tline.c_str(),"Memory Display Limit Max = %d\n",&v->MemoryDisplayLimitMax);	
	  else if (tline.find("Memory Display Limit Min")!=string::npos)			
	    sscanf(tline.c_str(),"Memory Display Limit Min = %d\n",&v->MemoryDisplayLimitMin);	
	  else if (tline.find("Memory Efficiency Limit Max")!=string::npos)			
	    sscanf(tline.c_str(),"Memory Efficiency Limit Max = %d\n",&v->MemoryEfficiencyLimitMax);	
	  else if (tline.find("Memory Efficiency Limit Min")!=string::npos)			
	    sscanf(tline.c_str(),"Memory Efficiency Limit Min = %d\n",&v->MemoryEfficiencyLimitMin);	
	  if (	v->MemoryDisplayLimitMax==-1) v->MemoryDisplayLimitMax =  INT_MAX;
	  if (	v->MemoryDisplayLimitMin==1) v->MemoryDisplayLimitMin = - INT_MAX;
	  if (	v->MemoryEfficiencyLimitMax==-1) v->MemoryEfficiencyLimitMax =  INT_MAX;
	  if (	v->MemoryEfficiencyLimitMin==1) v->MemoryEfficiencyLimitMin = - INT_MAX;
	  
	  
	}	//while (	!tFile.eof())
      tFile.close();
    }	//if (tFile.is_open())
  
}
