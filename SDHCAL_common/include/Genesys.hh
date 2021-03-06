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
#include <time.h>

#include <unistd.h>
#include <string>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <boost/format.hpp>

using namespace std;
class Genesys
{
public:
  void setIos()
  {
    struct termios oldtio,newtio;
    char buf[255];
    /* 
       Open modem device for reading and writing and not as controlling tty
       because we don't want to get killed if linenoise sends CTRL-C.
    */
            
    tcgetattr(fd1,&oldtio); /* save current serial port settings */
    bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */
        
    /* 
       BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
       CRTSCTS : output hardware flow control (only used if the cable has
       all necessary lines. See sect. 7 of Serial-HOWTO)
       CS8     : 8n1 (8bit,no parity,1 stopbit)
       CLOCAL  : local connection, no modem contol
       CREAD   : enable receiving characters
    */
         newtio.c_cflag = B9600 | CRTSCTS | CS8 | CLOCAL | CREAD;
	 
        /*
          IGNPAR  : ignore bytes with parity errors
          ICRNL   : map CR to NL (otherwise a CR input on the other computer
                    will not terminate input)
          otherwise make device raw (no other input processing)
        */
         newtio.c_iflag = IGNPAR | ICRNL;
         
        /*
         Raw output.
        */
         newtio.c_oflag = 0;
         
        /*
          ICANON  : enable canonical input
          disable all echo functionality, and don't send signals to calling program
        */
         newtio.c_lflag = ICANON;
         
        /* 
          initialize all control characters 
          default values can be found in /usr/include/termios.h, and are given
          in the comments, but we don't need them here
        */
         newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */ 
         newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
         newtio.c_cc[VERASE]   = 0;     /* del */
         newtio.c_cc[VKILL]    = 0;     /* @ */
         newtio.c_cc[VEOF]     = 4;     /* Ctrl-d */
         newtio.c_cc[VTIME]    = 0;     /* inter-character timer unused */
         newtio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
         newtio.c_cc[VSWTC]    = 0;     /* '\0' */
         newtio.c_cc[VSTART]   = 0;     /* Ctrl-q */ 
         newtio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
         newtio.c_cc[VSUSP]    = 0;     /* Ctrl-z */
         newtio.c_cc[VEOL]     = 0;     /* '\0' */
         newtio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
         newtio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
         newtio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
         newtio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
         newtio.c_cc[VEOL2]    = 0;     /* '\0' */
        
        /* 
          now clean the modem line and activate the settings for the port
        */
         tcflush(fd1, TCIFLUSH);
         tcsetattr(fd1,TCSANOW,&newtio);
         
  }
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
#ifdef OLDWAY
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
     options.c_cflag |= CRTSCTS;                     /* enable hardware flow control */


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

#else
    setIos();
#endif

    char hadr[20];
    memset(hadr,0,20);
    sprintf(hadr,"ADR %2d\r\n",address);
    stringstream s;
    s<<"ADR "<<address<<"\r";

    this->readCommand(s.str());
    // wr=write(fd1,s.str().c_str(),s.str().length());usleep(50000);
    // printf("%d Bytes sent are %d \n",portstatus,wr);
  
    s.str(std::string());
    s<<"RMT 1\r";
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
    readCommand("OUT 1\r");
    ::sleep(1);
    this->INFO();
  }
  void OFF()
  {
    readCommand("OUT 0\r");
    ::sleep(1);
    this->INFO();
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
    
	//std::cout<<"Y avait "<<buff<<std::endl;
      }
    wr=write(fd1,cmd.c_str(),cmd.length());
    //std::cout<<"sleep "<<std::endl;
    for (int i=0;i<20;i++) usleep(1000);
    memset(buff,0,1024);
    int32_t nchar=0,rd=0;
    while (1)
      {
	FD_ZERO(&set); /* clear the set */
	FD_SET(fd1, &set); /* add our file descriptor to the set */
	
	timeout.tv_sec = 0;
	timeout.tv_usec = 480000;


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
	usleep(1);
      }

    
    //printf("nchar %d OOOLLAA %s\n",nchar,buff);
    int istart=0;
    char bufr[100];
    memset(bufr,0,100);

    for (int i=0;i<nchar;i++)
      if (buff[i]<0x5f) {bufr[istart]=buff[i];istart++;}
    //memcpy(bufr,&buff[istart],nchar-istart);
    std::string toto;toto.assign(bufr,istart-1);
    //printf(" %d %d Corrected %s\n",istart,nchar,toto.c_str());
    
    _value=toto;
  }
  void INFO()
  {
    do
      {
	this->readCommand("IDN?\r");
	std::cout<<boost::format(" Device %s \n") % _value;
      } while (_value.compare("LAMBDA,GEN6-200")!=0);
    this->readCommand("MODE?\r");
    std::cout<<boost::format(" Status %s \n") % _value;
    std::size_t found;
    do {
      this->readCommand("STT?\r");
      std::cout<<boost::format("Full Status=>\n\t %s \n") % _value;

    
    found=_value.find("MV(");
    if (found == std::string::npos) continue;;
    sscanf( _value.substr(found+3,6).c_str(),"%f",&_vRead);
    found=_value.find("PV(");
    if (found == std::string::npos) continue;;
    sscanf(_value.substr(found+3,4).c_str(),"%f",&_vSet);
    found=_value.find("MC(");
    if (found == std::string::npos) continue;;
    sscanf(_value.substr(found+3,6).c_str(),"%f",&_iRead);
    found=_value.find("PC(");
    if (found == std::string::npos) continue;;
    sscanf(_value.substr(found+3,6).c_str(),"%f",&_iSet);
    std::cout<<boost::format("Vset %f Vread %f Iset %f I read %f  \n") % _vSet % _vRead % _iSet % _iRead;
    } while (found==std::string::npos);
    this->readCommand("OUT?\r");
    std::cout<<boost::format("Output Status=>\n\t %s \n") % _value;
    _lastInfo=time(0);
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
    if (time(0)-_lastInfo > 20) this->INFO();
    return _vSet;
  }
  float ReadVoltageUsed()
  {
    if (time(0)-_lastInfo > 20) this->INFO();
    return _vRead;
  }
  float ReadCurrentUsed()
  {
    if (time(0)-_lastInfo > 20) this->INFO();
    return _iRead;

  }
  std::string readValue(){return _value;}
private:


  int fd1;

  unsigned char buff[1024];
  std::string _value;
  int wr,rd,nbytes,tries;
  float _vSet,_vRead,_iSet,_iRead;
  time_t _lastInfo;
};
#ifdef _USE_MAIN_EXAMPLE_
int main()
{
  //
  // Open the serial port. 
  //
  Genesys* z=new Genesys("/dev/ttyUSB1",6);

  printf("Set Volatge\n");
  //z->readCommand("PV 4.50\r");
  z->INFO();
  getchar();
  printf("%f %f %f \n",z->ReadVoltageSet(),z->ReadVoltageUsed(),z->ReadCurrentUsed());
  // printf("%f %f %f \n",z.ReadVoltageSet(),z.ReadVoltageUsed(),z.ReadCurrentUsed());
  z->OFF();
  getchar();
  z->ON();
  // getchar();
  // //z->INFO();
  /*
  printf("Read Volatge\n");
  printf("%f %f %f \n",z->ReadVoltageSet(),z->ReadVoltageUsed(),z->ReadCurrentUsed());
  z->OFF();
  getchar();
  z->INFO();
  
  z->ON();
  getchar();
  z->INFO();
  */
  // // getchar();
}
#endif
#endif
