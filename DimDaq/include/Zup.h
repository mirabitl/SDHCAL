#ifndef _Zup_H_
#define _Zup_H_
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



class Zup
{
public:
  Zup(std::string device,uint32_t address)
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
    sprintf(hadr,":ADR%.2d;\n",address);

    wr=write(fd1,hadr,7);usleep(50000);
    printf("%d Bytes sent are %d \n",portstatus,wr);

  }
  ~Zup()
  {
    close(fd1);
  }
  void ON()
  {
    
    wr=write(fd1,":OUT1;",6);usleep(50000);
    printf("Bytes sent are %d \n",wr);
  }
  void OFF()
  {

    wr=write(fd1,":OUT0;",6);usleep(50000);
    printf("Bytes sent are %d \n",wr);

  }
  void INFO()
  {
    wr=write(fd1,":MDL?;",6);usleep(50000);
    memset(buff,0,100);rd=read(fd1,buff,100); printf("%s \n",buff);
    wr=write(fd1,":VOL!;",6);usleep(50000);
    memset(buff,0,100);rd=read(fd1,buff,100); printf("%s \n",buff);
    wr=write(fd1,":VOL?;",6);usleep(50000);
    memset(buff,0,100);rd=read(fd1,buff,100); printf("%s \n",buff);
    wr=write(fd1,":CUR?;",6);usleep(50000);
    memset(buff,0,100);rd=read(fd1,buff,100); printf("%s \n",buff);
  }
  float ReadVoltageSet()
  {
    wr=write(fd1,":VOL!;",6);usleep(50000);
    memset(buff,0,100);rd=read(fd1,buff,100); printf("%s \n",buff);
    float v;
    sscanf(buff,"SV%f",&v);
    return v;
  }
  float ReadVoltageUsed()
  {
    wr=write(fd1,":VOL?;",6);usleep(50000);
    memset(buff,0,100);rd=read(fd1,buff,100); printf("%s \n",buff);
    float v;
    sscanf(buff,"AV%f",&v);
    return v;
  }
  float ReadCurrentUsed()
  {
    wr=write(fd1,":CUR?;",6);usleep(50000);
    memset(buff,0,100);rd=read(fd1,buff,100); printf("%s \n",buff);
    float v;
    sscanf(buff,"AA%f",&v);
    return v;
  }

private:


  int fd1;

  char buff[100];

  int wr,rd,nbytes,tries;
};
#ifdef _USE_MAIN_EXAMPLE_
int main()
{
  //
  // Open the serial port. 
  //
  Zup z("/dev/ttyUSB0",1);
  z.INFO();
  //getchar();
  z.ON();
  getchar();
  z.INFO();
  z.OFF();
  getchar();
  z.INFO();
  // getchar();
}
#endif
#endif
