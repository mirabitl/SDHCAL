#ifndef _DS1820_H_
#define _DS1820_H_

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

#include <dirent.h>

class DS1820
{
public:
  DS1820(void)
  {
		sprintf (path,"/sys/bus/w1/devices"); 
		nbSensors=0;
		dir = opendir (path);
		if (dir != NULL)
		{
			while ((dirent = readdir (dir)))	// 1-wire devices are links beginning with 10-
			{
				if (dirent->d_type == DT_LNK && strstr(dirent->d_name, "10-") != NULL) 
				{ 
					strcpy(dev[nbSensors], dirent->d_name);
					printf ("dev = %s( %d)\n",dev[nbSensors]);
					nbSensors++;
				}
			}	
			(void) closedir (dir);
		}
		else
			perror ("Couldn't open the w1 devices directory");
  }
 
  ~DS1820()
  {
 		close(fd);
  }
	
	int DS1820Read(void)
	{
		for( int i=0;i<nbSensors;i++)
		{
			sprintf(devPath, "%s/%s/w1_slave", path, dev[i]);
			fd = open(devPath, O_RDONLY);
			if(fd == -1){	perror ("Couldn't open the w1 device.");	return -1;}
			while((numRead = read(fd, buf, 256)) > 0) 
			{
				strncpy(tmpData, strstr(buf, "t=") + 2, 5);
				tempC[i] = strtof(tmpData, NULL)/1000;
				printf("Device: %s  - Temp: %.3f C  \n", dev[i],tempC[i]);
			}	
			close(fd);
		}
	 	return 0;
	}
 
 	int DS1820GetTemps(float *temps)
	{
		temps[0]=tempC[0];
		temps[1]=tempC[1];
		return 0;
	}
	
 	int DS1820GetNbSensors(void)
	{
		return nbSensors;
	}
 
private:
	DIR *dir;
	struct dirent *dirent;
	char dev[16][16];      // Dev ID
	char devPath[128]; // Path to device
	char buf[256];     // Data from device
	char tmpData[6];   // Temp C * 1000 reported by device 
	char path[255]; 
	ssize_t numRead;
	int fd ;
	float tempC[16] ;
	int nbSensors ;// number of sensors on the bus	

};
#endif
