#ifndef _BMP183_H_
#define _BMP183_H_

#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <math.h>
#include <dirent.h>

#include <wiringPi.h>
#include <wiringPiSPI.h>

#define CAL_AC1 0xAA
#define CAL_AC2 0xAC
#define CAL_AC3 0xAE
#define CAL_AC4 0xB0
#define CAL_AC5 0xB2
#define CAL_AC6 0xB4
#define CAL_B1 0xB6
#define CAL_B2 0xB8 
#define CAL_MB 0xBA
#define CAL_MC 0xBC
#define CAL_MD 0xBE

#define CTRL_MEAS  0xF4
#define DATA 0xF6

class BMP183
{
public:
  BMP183(void)
  {
		BMP183Setup () ;
		BMP183GetCalibration();
  }
 
  ~BMP183()
  {
  }


int BMP183Setup (void)
{
  unsigned char spiData [2] ;

  if (wiringPiSPISetup (0, 10000) < 0)	// 10MHz Max
    return -1 ;

  node = wiringPiNewNode (64, 2) ;

  node->fd          = 0 ;
  node->analogWrite = myAnalogWrite ;
  node->analogRead = myAnalogRead ;

  return 0 ;
}

static void myAnalogWrite (struct wiringPiNodeStruct *node, int addr, int value)
{
  unsigned char spiData [2] ;


  spiData [0] = (addr&0x7f) ;
  spiData [1] = value&0xff ;

  wiringPiSPIDataRW (node->fd, spiData, 2) ;
}

static int myAnalogRead (struct wiringPiNodeStruct *node, int addr)
{
  unsigned char spiData [2] ;


  spiData [0] = (addr&0x7f) +0x80;
  spiData [1] = 0 ;

  wiringPiSPIDataRW (node->fd, spiData, 2) ;
  return spiData [1];
}

int BMP183GetCalibration(void)
{ 
	int tlsb, tmsb;

	tmsb=myAnalogRead(node, CAL_AC1);
	tlsb=myAnalogRead(node, CAL_AC1+1);
	AC1=tlsb+(tmsb<<8);
	
	tmsb=myAnalogRead(node, CAL_AC2);
	tlsb=myAnalogRead(node, CAL_AC2+1);
	AC2=tlsb+(tmsb<<8);

	tmsb=myAnalogRead(node, CAL_AC3);
	tlsb=myAnalogRead(node, CAL_AC3+1);
	AC3=tlsb+(tmsb<<8);

	tmsb=myAnalogRead(node, CAL_AC4);
	tlsb=myAnalogRead(node, CAL_AC4+1);
	AC4=tlsb+(tmsb<<8);

	tmsb=myAnalogRead(node, CAL_AC5);
	tlsb=myAnalogRead(node, CAL_AC5+1);
	AC5=tlsb+(tmsb<<8);

	tmsb=myAnalogRead(node, CAL_AC6);
	tlsb=myAnalogRead(node, CAL_AC6+1);
	AC6=tlsb+(tmsb<<8);

	tmsb=myAnalogRead(node, CAL_B1);
	tlsb=myAnalogRead(node, CAL_B1+1);
	B1=tlsb+(tmsb<<8);

	tmsb=myAnalogRead(node, CAL_B2);
	tlsb=myAnalogRead(node, CAL_B2+1);
	B2=tlsb+(tmsb<<8);

	tmsb=myAnalogRead(node, CAL_MB);
	tlsb=myAnalogRead(node, CAL_MB+1);
	MB=tlsb+(tmsb<<8);

	tmsb=myAnalogRead(node, CAL_MC);
	tlsb=myAnalogRead(node, CAL_MC+1);
	MC=tlsb+(tmsb<<8);

	tmsb=myAnalogRead(node, CAL_MD);
	tlsb=myAnalogRead(node, CAL_MD+1);
	MD=tlsb+(tmsb<<8);
	
	printf ("AC5=%d AC6=%d MC=%d MD=%d\n", AC5, AC6, MC, MD);
}

float BMP183TemperatureRead(void)
{
  int temp_msb, temp_lsb,UT;
	int X1, X2;
	int T;
	float temperature;

	myAnalogWrite(node, CTRL_MEAS, 0x2E);
	usleep (20000); //20ms
	temp_msb=myAnalogRead(node, DATA);
	temp_lsb=myAnalogRead(node, DATA+1);
	
	UT=(temp_msb<<8)+temp_lsb;
	X1 = (((UT - AC6) * AC5 )>>15);
	X2 = (MC <<11) / (X1 + MD); 
	
	B5= X1+X2; // mandatory for pressure calculation
	temperature = ((X1+X2) + 8) >>4;
	temperature =temperature/10;
	printf ("temperature = %f deg C\n",temperature);
	return temperature;
}

float BMP183PressionRead(void)
{
#define CCVERS
#ifdef CCVERS
	float pression;
	int UP1_lsb, UP1_msb, UP1_xsb;	
	long UP1, B3,  B6;	
        long X1, X2, X3;
	long p;
        long B7;
	unsigned long B4;
	unsigned int oss=3;
	myAnalogWrite(node, CTRL_MEAS, 0x34+(oss<<6) );
	//myAnalogWrite(node, CTRL_MEAS, 0x34 |(0x3<<4));
	usleep (50000);//50ms
	UP1_msb=myAnalogRead(node, DATA);
	UP1_lsb=myAnalogRead(node, DATA+1);
	UP1_xsb=myAnalogRead(node, DATA+2);

	UP1 = ((UP1_msb<<16)+(UP1_lsb<<8)+UP1_xsb);
	UP1=UP1>>(8-oss);
// Calculate atmospheric pressure in [Pa]
	B6 = B5 - 4000;
	X1 = (B2 * (B6 * B6)>>12)>>11;
	X2 = (AC2 * B6) >>11;
	X3 = X1 + X2;
	B3 = (((AC1 * 4 + X3) << oss) + 2 ) / 4;
	X1 = (AC3 * B6)>>13;
	X2 = (B1 * (B6 * B6)>>12)>>16;
	X3 = (X1 + X2 + 2)>>2;
	B4 = (AC4 * (unsigned long)(X3 + 32768)) >>15;
	B7 = ((unsigned long)UP1 - B3) * (50000>>oss);
	if (B7<0x80000000)
	  p=(B7*2)/B4;
	else
	  p =  (B7/ B4)*2;
	X1 = (p >>8) * ( p >>8);
	X1 = (X1 * 3038) >>16;
	X2 = (-7357 * p) >>16;
	pression= p + ((X1 + X2 +3791)>>4);
	pression= pression/100.0;
	printf ("pression = %f hPa \n",pression);
	return pression;
#else
	float pression;
	int UP1_lsb, UP1_msb, UP1_xsb;	
        int UP[3];
	long UP1, B3, B4,  B6;	
	unsigned long X1, X2, X3;
	unsigned long p;
	unsigned long B7;

	for (int i=0;i<3;i++)
{
	myAnalogWrite(node, CTRL_MEAS, 0x34 |(0x3<<4));
        for (int k=0;k<30;k++)	
	usleep (1);//50ms
	UP1_msb=myAnalogRead(node, DATA);
	UP1_lsb=myAnalogRead(node, DATA+1);
	UP1_xsb=myAnalogRead(node, DATA+2);

	UP1 = ((UP1_msb<<16)+(UP1_lsb<<8)+UP1_xsb);
        //printf("%d %x %x \n",UP1,UP1,UP1>>5);
	UP1=UP1>>5;
        UP[i]=UP1;
}
        int UPF=(UP[0]+UP[1]+UP[2])/3;
        printf("%d %x \n",UPF,UPF);
// Calculate atmospheric pressure in [Pa]
	B6 = B5 - 4000;
	X1 = (B2 * (B6 * B6)/4096)/2048;
	X2 = (AC2 * B6)/2048;
	X3 = X1 + X2;
	B3 = (((AC1 * 4 + X3) << 3)+2)/4;
	X1 = (AC3 * B6)/8192;
	X2 = (B1 * (B6 * B6)/4096)/65536;
	X3 = (X1 + X2 + 2)/4;
	B4 = (AC4 * (X3 + 32768)/32768);
	B7 = (UPF - B3) * (50000>>3);
	p =  ((B7 * 2) / B4);
        p = (B7/B4)*2;
	X1 = (p/256) * ( p /256);
	X1 = (X1 * 3038)/65536;
	X2 = (-7357 * p)/65536;
	pression= p + ((X1 + X2 +3791)/16);
	pression= pression/100.0;
	printf ("pression = %f hPa \n",pression);
	return pression;
#endif
}
 
private:
  struct wiringPiNodeStruct *node ;
	
	unsigned char spiData [2] ;
// calibration
		short AC1;
		short AC2;
		short AC3;
		unsigned short AC4;
		unsigned short AC5;
		unsigned short AC6;
		short B1;
		short B2;
		short MB;
		short MC;
		short MD;
	
		short B5;

};
#endif
