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
//	open1=0;
//	open2=0;
		sprintf (path,"/sys/bus/w1/devices"); 

		dir = opendir (path);
//		open1++;
		if (dir != NULL)
		{
			while ((dirent = readdir (dir)))	// 1-wire devices are links beginning with 10-
			{
				if (dirent->d_type == DT_LNK && strstr(dirent->d_name, "10-") != NULL) 
				{ 
					strcpy(dev, dirent->d_name);
				}
			}	
			(void) closedir (dir);
//			open1--;
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
//		printf ("%d %d - ",open1,open2);
		sprintf(devPath, "%s/%s/w1_slave", path, dev);
		fd = open(devPath, O_RDONLY);
//		open2++;
		if(fd == -1)	
		{	
			perror ("Couldn't open the w1 device.");	return -1;	
		}
		while((numRead = read(fd, buf, 256)) > 0) 
		{
			strncpy(tmpData, strstr(buf, "t=") + 2, 5);
			tempC = strtof(tmpData, NULL);
			printf("Device: %s  - Temp: %.3f C  \n", dev,tempC / 1000);
		}	
		close(fd);
//		open2--;
 		return tempC / 1000;
	}
 
 
private:
	DIR *dir;
	struct dirent *dirent;
	char dev[16];      // Dev ID
	char devPath[128]; // Path to device
	char buf[256];     // Data from device
	char tmpData[6];   // Temp C * 1000 reported by device 
	char path[255]; 
	ssize_t numRead;
	int fd ;
	float tempC ;
	
//	int open1;
//	int open2;
};
#endif
