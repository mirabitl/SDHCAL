#ifndef _DCBUFFER_READER_H
#define _DCBUFFER_READER_H
#include "DCDIF.h"
#include "DCFrame.h"

class DCBufferReader
{
 public:
  static unsigned long swap_bytes(unsigned int n,unsigned char* buf);
  static unsigned long long GrayToBin(unsigned long long n);
  static  void check_CRC(unsigned char* DataCRC,int length, unsigned short &CRC);
  static unsigned short ReadFormat_CRC(unsigned char* buf,int &index); //Apears only once.
  static bool ReadGlobalHeader_Computing_CRC(unsigned char* buf,int &index,unsigned short &CRC);  //Global header is: 0xB0 (dec)->176
  static unsigned short ReadDIFId_Computing_CRC(unsigned char* buf,int &index,unsigned short &CRC);
  static unsigned long ReadDIFTriggerCounter_Computing_CRC(unsigned char* buf,int &index,unsigned short &CRC);
  static unsigned long ReadAsuTriggerCounter_Computing_CRC(unsigned char* buf,int &index,unsigned short &CRC);
  static unsigned long ReadGlobalTriggerCounter_Computing_CRC(unsigned char* buf,int &index, unsigned short &CRC);
  static double ReadLargeClock_Computing_CRC(unsigned char* buf,int &index, unsigned short &CRC);
  static bool ReadFrameHeader_CRC(unsigned char* buf,int &index);  //Global header is: b4
  static void ReadAsicInternalTriggerInFrame_Computing_CRC(unsigned short &AsicId, double &TS,bool* l0,bool* l1, unsigned char* buf,int &index, unsigned short &CRC);
  static bool ReadFrameTrailer_CRC(unsigned char* buf,int &index);  //Global Trailler is: A3 (163 in dec)
  static bool ReadGlobalTrailer_Computing_CRC(unsigned char* buf,int &index, unsigned short &CRC);  //Global Trailer is: A0 (160 in dec)
  static bool ReadCRC_And_Check(unsigned char* buf,int &index, unsigned short &CRC);
 public:
  static void ReadDIFBlock(unsigned char* buf,DCDIF* &dif,std::vector<DCFrame*>* frames,unsigned int hrtype=1); 
  static void ReadDIFBlockSmall(unsigned char* buf,DCDIF* &dif,std::vector<DCFrame*>* frames); 
  static void PadConvert(int asicid,int ipad, int &i, int &j);
  static void RKPadConvert(int asicid,int ipad, int &i, int &j,int hrtype);
  static bool correctRecoTime(double tbc,std::vector<DCFrame*>& vasic);
  static void checkType(unsigned char* tcbuf, unsigned int rusize, bool &slowcontrol,unsigned int& version,unsigned int &hrtype,unsigned int &id0,unsigned int& iddif,int xdaq_shift=92);

  static void setDAQ_BC_Period(float x);
  static float getDAQ_BC_Period();


  static float theDAQ_BC_Period;
};
#endif
