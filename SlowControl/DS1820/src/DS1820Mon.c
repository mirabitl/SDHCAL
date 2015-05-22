
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

#define SERVER_NAME "SDHCAL_GIFpp_TempLog"

void ex_program(int sig);
short handle = 0;     /* The handle to a TC-08 returned by usb_tc08_open_unit() */

char *dns;			// DNS names
char serviceName[64];
char serviceName2[64];
char DataBuffer[4096];
int ServiceId=0;

char myquery[512];
char currentTime[127];
char currentDate[127];

float FormatedData;
time_t curtime;
struct tm *loctime;


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
	sprintf (serviceName,"%s/DS1820_Temperature",SERVER_NAME);
	ServiceId = dis_add_service(serviceName, "F", &FormatedData,sizeof(FormatedData), 0, 0);
	printf ("ServiceId=%d\n",ServiceId);
	
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

/*
void initialiseDb(char *dbdDir, char *filePath,char *fileName_)
{
  dbi_driver driver;
  dbi_initialize(dbdDir);
  conn_ = dbi_conn_new("sqlite3");
  driver = dbi_conn_get_driver(conn_);
    
  printf("\nDriver information:\n-------------------\n");
  printf("\tName:       %s\n"
	 "\tFilename:   %s\n"
         "\tDesc:       %s\n"
         "\tMaintainer: %s\n"
         "\tURL:        %s\n"
         "\tVersion:    %s\n"
         "\tCompiled:   %s\n", 
	 dbi_driver_get_name(driver), dbi_driver_get_filename(driver), 
	 dbi_driver_get_description(driver), dbi_driver_get_maintainer(driver), 
	 dbi_driver_get_url(driver), dbi_driver_get_version(driver),
	 dbi_driver_get_date_compiled(driver));
    

  dbi_conn_set_option(conn_, "sqlite3_dbdir", filePath);
  dbi_conn_set_option(conn_, "dbname",fileName_);

  int ier=dbi_conn_connect(conn_);
  if (ier < 0) 
	{
    printf("Could not connect. Please check the option settings %d\n",ier);
  }
}
*/

int main(void)
{
	DIR *dir;
	struct dirent *dirent;
	char dev[16];      // Dev ID
	char devPath[128]; // Path to device
	char buf[256];     // Data from device
	char tmpData[6];   // Temp C * 1000 reported by device 
	char path[] = "/sys/bus/w1/devices"; 
	ssize_t numRead;


	(void) signal(SIGINT, ex_program);

	intializeDIM();
//	initialiseDb("/usr/local/lib/dbd","/data/online/TemperatureLogger","TC08_temperatures.sqlite");

	dir = opendir (path);
	if (dir != NULL)
	{
		while ((dirent = readdir (dir)))	// 1-wire devices are links beginning with 10-
			if (dirent->d_type == DT_LNK && strstr(dirent->d_name, "10-") != NULL) 
			{ 
				strcpy(dev, dirent->d_name);
				printf("\nDevice: %s\n", dev);
			}
			(void) closedir (dir);
	}
	else
	{
		perror ("Couldn't open the w1 devices directory");
		return 1;
	}

// Assemble path to OneWire device
		sprintf(devPath, "%s/%s/w1_slave", path, dev);
	// Read temp continuously
	// Opening the device's file triggers new reading
	while(1) 
	{
		int fd = open(devPath, O_RDONLY);
		if(fd == -1)	{	perror ("Couldn't open the w1 device.");	return 1;	}
		while((numRead = read(fd, buf, 256)) > 0) 
		{
			strncpy(tmpData, strstr(buf, "t=") + 2, 5);
			float tempC = strtof(tmpData, NULL);
			printf("Device: %s  - Temp: %.3f C  \n", dev,tempC / 1000);
			FormatedData = tempC / 1000;
	 		dis_update_service(ServiceId);
		}
		usleep(1000000);
		close(fd);
	} 		
	return 0;
}
