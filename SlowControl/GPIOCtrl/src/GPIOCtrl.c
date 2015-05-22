#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>

extern "C"
{
#include "/home/pi/dim_v20r13/dim/dim.h"
#include "/home/pi/dim_v20r13/dim/dis.h"
}

#define SERVER_NAME "SDHCAL_GIFpp_GPIOCtrl"


#define IN  0
#define OUT 1
 
#define LOW  0
#define HIGH 1
 
#define PVME  24 /* P1-16 */
#define PDIF 23  /* P1-18 */

#define DIRECTION_MAX 35
#define VALUE_MAX 30
#define BUFFER_MAX 3

void ex_program(int sig);
short handle = 0;     /* The handle to a TC-08 returned by usb_tc08_open_unit() */

char *dns;			// DNS names
char serviceName[64];
char DIFCdeName[64];
char VMECdeName[64];
char DataBuffer[4096];
int ServiceId=0;

int FormatedData[2];		// DIF : VME
time_t curtime;
struct tm *loctime;

int VMEPower=0;
int DIFPower=0;

static int
GPIOExport(int pin)
{
	char buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int fd;
  char tcommand[80];

	strcpy(tcommand,"sudo chmod 777 -R  /sys/class/gpio/export");
	system(tcommand);
	
	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open export for writing!\n");
		return(-1);
	}
 
	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return(0);
}
 
static int
GPIOUnexport(int pin)
{
	char buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int fd;
  char tcommand[80];
	strcpy(tcommand,"sudo chmod 777 -R  /sys/class/gpio/unexport");
	system(tcommand);
 
	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open unexport for writing!\n");
		return(-1);
	}
 
	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return(0);
}
 
static int
GPIODirection(int pin, int dir)
{
	static const char s_directions_str[]  = "in\0out";
 
	char path[DIRECTION_MAX];
	int fd;
   
	char tcommand[80];
	sprintf(tcommand,"sudo chmod 777 -R  /sys/class/gpio/gpio%d/",pin);
	system(tcommand);

	snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
	fd = open(path, O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open gpio direction for writing on pin %d!\n", pin );
		return(-1);
	}
 
	if (-1 == write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3)) 
	{
		fprintf(stderr, "Failed to set direction!\n");
		return(-1);
	}
 
	close(fd);
	return(0);
}
 
static int GPIORead(int pin)
{
	char path[VALUE_MAX];
	char value_str[3];
	int fd;
 
	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_RDONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open gpio value for reading!\n");
		return(-1);
	}
 
	if (-1 == read(fd, value_str, 3)) 
	{
		fprintf(stderr, "Failed to read value!\n");
		return(-1);
	}
 	close(fd);
 	return(atoi(value_str));
}
 
static int GPIOWrite(int pin, int value)
{
	static const char s_values_str[] = "01";
 
	char path[VALUE_MAX];
	int fd;
 printf ("writing %d %d...\n",pin,value);
	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_WRONLY);
	if (-1 == fd) 
	{
		fprintf(stderr, "Failed to open gpio value for writing!\n");
		return(-1);
	}
 
	if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) 
	{
		fprintf(stderr, "Failed to write value!\n");
		return(-1);
	}
 
	close(fd);
	return(0);
}

void SetVMEPwr(void *tag,void *cmd_buffer,int *size)
{	
	char tstr[64];
	sprintf (tstr,"%s",(char *)cmd_buffer);
	VMEPower=tstr[0];
	printf ("setting VME power to = %d\n",VMEPower);

	if (-1 == GPIOWrite(PVME, VMEPower))
		printf ("unable to set VME Power\n");
}

void SetDIFPwr(void *tag,void *cmd_buffer,int *size)
{
	char tstr[64];
	sprintf (tstr,"%s",(char *)cmd_buffer);
	DIFPower=tstr[0];
	printf ("setting DIF power to = %d\n",DIFPower);

	if (-1 == GPIOWrite(PDIF, DIFPower))
		printf ("unable to set DIF Power\n");
}

int initializeGPIO (void)
{

// GPIO23 : VME
// GPIO24 : DIF
	
/*  setup_io();		// Set up gpi pointer for direct register access
 
  INP_GPIO(23); // must use INP_GPIO before we can use OUT_GPIO
  INP_GPIO(24); // must use INP_GPIO before we can use OUT_GPIO
  OUT_GPIO(23);			
  OUT_GPIO(24);
	*/

if (geteuid()==0)  seteuid(getuid());

// Enable GPIO pins

	if (-1 == GPIOExport(PVME) || -1 == GPIOExport(PDIF))
		return(1);
 
// Set GPIO directions
	if (-1 == GPIODirection(PVME, OUT) || -1 == GPIODirection(PDIF, OUT))
		return(2);
	return 0;
}

int freeGPIO (void)
{	
	if (-1 == GPIOUnexport(PDIF) || -1 == GPIOUnexport(PVME))
		return(4);
	return 0;
}

int intializeDIM(void)
{
//printf ("getenv(DIM_DNS_NODE)=%s\n",getenv("DIM_DNS_NODE"));
	dns = getenv("DIM_DNS_NODE");
//	printf ("dns=%s\n",dns);
	if (dns == 0) 
	{
		printf("No DIM_DNS_NODE specified. \n");
		exit(203);
	}
	else 
		printf ("DIM_DNS_NODE = %s\n", dns);
		
	printf ("ServerName=%s\n",SERVER_NAME);
		
	// -- add item as service --
	sprintf (serviceName,"%s/RPI_GPIOstatus",SERVER_NAME);
	ServiceId = dis_add_service(serviceName, "I:2", &FormatedData,sizeof(FormatedData), 0, 0);
	printf ("ServiceId=%d\n",ServiceId);

	sprintf(DIFCdeName,"%s/RPC_DIFPWRCde",SERVER_NAME);
	dis_add_cmnd(DIFCdeName,"I",SetDIFPwr,1);
	 

	sprintf(VMECdeName,"%s/RPC_VMEPWRCde",SERVER_NAME);
	dis_add_cmnd(VMECdeName,"I",SetVMEPwr,1);
	
	dis_start_serving("Power");
	int tstatus= dis_start_serving(SERVER_NAME);
	printf ("tstatus=%d\n",tstatus);
	return 0;
}


void ex_program(int sig) 
{
	if (sig==2)
	{
 		(void) signal(SIGINT, SIG_DFL);
 		exit(1);
	}	
}

int main(void)
{
	(void) signal(SIGINT, ex_program);
	initializeGPIO();
printf ("a\n");
	intializeDIM();
printf ("b\n");

	while(1) 
	{
		FormatedData[0]=GPIORead(PDIF);//DIFPower ;					// DIF
		FormatedData[1]=GPIORead(PVME);//VMEPower ;					// VME
 		dis_update_service(ServiceId);
//printf ("\033[A\033[K");
//printf(" %d %d\n",FormatedData[0], FormatedData[1]);
		usleep(1000000);
	} 		
	printf ("c\n");
	freeGPIO();
	return 0;
}
