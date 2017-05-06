#ifndef _Zup_H_
#define _Genesys_H_
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
#include <string>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
using namespace std;
class Genesys
{
public:
  Genesys(std::string device,uint32_t address)
  {

    fd1=open(device.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);

    if (fd1 == -1 )

      {

	perror("open_port: Unable to open /dev/ttyS0 – ");

      }

    else

      {

	fcntl(fd1, F_SETFL,0);
	printf("Port 1 has been sucessfully opened and %d is the file description\n",fd1);
      }


    int portstatus = 0;

    struct termios options;
    // Get the current options for the port...
    tcgetattr(fd1, &options);
    // Set the baud rates to 115200...
    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);
    // Enable the receiver and set local mode...
    options.c_cflag |= (CLOCAL | CREAD);

     options.c_cflag &= ~PARENB;
     options.c_cflag &= ~CSTOPB;
     options.c_cflag &= ~CSIZE;
     options.c_cflag |= CS8;
    //options.c_cflag |= SerialDataBitsInterp(8);           /* CS8 - Selects 8 data bits */
     options.c_cflag &= ~CRTSCTS;                            // disable hardware flow control
     options.c_iflag &= ~(IXON | IXOFF | IXANY);           // disable XON XOFF (for transmit and receive)
     //options.c_cflag |= CRTSCTS;                     /* enable hardware flow control */


    options.c_cc[VMIN] = 1;     //min carachters to be read
    options.c_cc[VTIME] = 1;    //Time to wait for data (tenths of seconds)


    // Set the new options for the port...
    tcsetattr(fd1, TCSANOW, &options);


    //Set the new options for the port...
    tcflush(fd1, TCIFLUSH);
    if (tcsetattr(fd1, TCSANOW, &options)==-1)
      {
        perror("On tcsetattr:");
        portstatus = -1;
      }
    else
      portstatus = 1;



    char hadr[20];
    memset(hadr,0,20);
    sprintf(hadr,"ADR %2d\r\n\n",address);
    stringstream s;
    s<<"ADR "<<address<<"\r\n";

    this->readCommand(s.str());
    // wr=write(fd1,s.str().c_str(),s.str().length());usleep(50000);
    // printf("%d Bytes sent are %d \n",portstatus,wr);
  
    s.str(std::string());
    s<<"RMT 1\r\n";
    this->readCommand(s.str());
    // wr=write(fd1,s.str().c_str(),s.str().length());usleep(50000);
    // printf("%d Bytes sent are %d \n",portstatus,wr);

  }
  ~Genesys()
  {
    if (fd1>0)
      close(fd1);
  }
  void ON()
  {
    stringstream s;
    s<<"OUT 1\r\n\n";
    wr=write(fd1,s.str().c_str(),s.str().length());usleep(50000);
    printf("Bytes sent are %d \n",wr);

  }
  void OFF()
  {

    wr=write(fd1,":OUT0;",6);usleep(50000);
    //printf("Bytes sent are %d \n",wr);

  }

  void readCommand(std::string cmd)
  {
    memset(buff,0,1024);
    fd_set set;
    struct timeval timeout;
    int rv;

    FD_ZERO(&set); /* clear the set */
    FD_SET(fd1, &set); /* add our file descriptor to the set */

    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;

    rv = select(fd1 + 1, &set, NULL, NULL, &timeout);
    if(rv == -1)
      perror("select"); /* an error accured */
    else if(rv != 0)
      {
	read( fd1, buff, 100 ); /* there was data to read */
    
	std::cout<<"Y avait "<<buff<<std::endl;
      }
    wr=write(fd1,cmd.c_str(),cmd.length());
    //std::cout<<"sleep "<<std::endl;
    //for (int i=0;i<2500;i++) usleep(100);
    memset(buff,0,1024);
    int32_t nchar=0,rd=0;
    while (1)
      {
	FD_ZERO(&set); /* clear the set */
	FD_SET(fd1, &set); /* add our file descriptor to the set */
	
	timeout.tv_sec = 0;
	timeout.tv_usec = 500000;


	rv = select(fd1 + 1, &set, NULL, NULL, &timeout);
	if(rv == -1)
	  {
	    perror("select"); /* an error accured */
	  }
	else if(rv == 0)
	  {
	    //printf("Nothing in select \n"); /* a timeout occured */
	    break;
	  }
	else
	  {

	    rd=read(fd1,&buff[nchar],100);
	    //printf(" rd = %d nchar %d %s\n",rd,nchar,buff);
	    if (rd>0)
	      nchar+=rd;
	  }
	usleep(10);
      }

    
    //printf("nchar %d OOOLLAA %s\n",nchar,buff);
    int istart=0;
    char bufr[100];
    memset(bufr,0,100);

    for (int i=0;i<nchar;i++)
      if (buff[i]<0x5f) {bufr[istart]=buff[i];istart++;}
    //memcpy(bufr,&buff[istart],nchar-istart);
    std::string toto;toto.assign(bufr,istart);
    //printf(" %d %d Corrected %s\n",istart,nchar,toto.c_str());
    _value=toto;
  }
  void INFO()
  {
    //this->readCommand("IDN?\r\n");
    this->readCommand("PV?\r\n");
    this->readCommand("MV?\r\n");
    this->readCommand("PC?\r\n");
    this->readCommand("MC?\r\n");
    /*
    wr=write(fd1,":MDL?;",6);usleep(50000);
    memset(buff,0,100);rd=read(fd1,buff,100); printf("%s \n",buff);
    wr=write(fd1,":VOL!;",6);usleep(50000);
    memset(buff,0,100);rd=read(fd1,buff,100); printf("%s \n",buff);
    wr=write(fd1,":VOL?;",6);usleep(50000);
    memset(buff,0,100);rd=read(fd1,buff,100); printf("%s \n",buff);
    wr=write(fd1,":CUR?;",6);usleep(50000);
    memset(buff,0,100);rd=read(fd1,buff,100); printf("%s \n",buff);
    */
  }
  float ReadVoltageSet()
  {
    this->readCommand("PV?\r\n");
    /*
    wr=write(fd1,":VOL!;",6);usleep(50000);
    memset(buff,0,100);rd=read(fd1,buff,100); printf("%s \n",buff);
    */
    float v;
    sscanf(_value.c_str(),"%f",&v);
    //sscanf(buff,"SV%f",&v);
    return v;
  }
  float ReadVoltageUsed()
  {
    this->readCommand("MV?\r\n");
    /*
    wr=write(fd1,":VOL?;",6);usleep(50000);
    memset(buff,0,100);rd=read(fd1,buff,100); printf("%s \n",buff);
    */
    float v;
    sscanf(_value.c_str(),"%f",&v);
    return v;
  }
  float ReadCurrentUsed()
  {
    this->readCommand("MC?\r\n");
    /*
    wr=write(fd1,":CUR?;",6);usleep(50000);
    memset(buff,0,100);rd=read(fd1,buff,100); printf("%s \n",buff);
    */
    float v;
    //sscanf(buff,"AA%f",&v);
    sscanf(_value.c_str(),"%f",&v);
    return v;
  }
  std::string readValue(){return _value;}
private:


  int fd1;

  unsigned char buff[1024];
  std::string _value;
  int wr,rd,nbytes,tries;
};
#ifdef _USE_MAIN_EXAMPLE_
int main()
{
  //
  // Open the serial port. 
  //
  Genesys* z=new Genesys("/dev/ttyUSB1",6);

  printf("Set Volatge\n");
  z->readCommand("PV 4.50\r\n");
  //z->INFO();
  // printf("%f %f %f \n",z.ReadVoltageSet(),z.ReadVoltageUsed(),z.ReadCurrentUsed());
  // //getchar();
  // z.ON();
  // getchar();
  // //z->INFO();
  printf("Read Volatge\n");
  printf("%f %f %f \n",z->ReadVoltageSet(),z->ReadVoltageUsed(),z->ReadCurrentUsed());
  // z->OFF();
  // getchar();
  // //z->INFO();
  // printf("%f %f %f \n",z->ReadVoltageSet(),z->ReadVoltageUsed(),z->ReadCurrentUsed());
  // // getchar();
}
#endif
#endif
