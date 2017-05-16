#include "tdcrb.hh"
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <map>
#include <bitset>
#include "DHCalEventReader.h"
#include "TApplication.h"
#include "TCanvas.h"
#include <dirent.h>
#include <fnmatch.h>

int main(int argc, char** argv )
{
  tdcrb bs("/tmp");
  TApplication theApp("tapp", &argc, argv);
  bs.geometry("/home/acqilc/SDHCAL/SDHCAL_EventReader/pluggins/m3_avril2015.json");

  std::stringstream spat;
  int runask=atol(argv[1]);
  spat<<"*"<<atol(argv[1])<<"*.dat";
  struct dirent **namelist;
  int n;
  std::cout<<"Pattern "<<spat.str()<<std::endl;
  std::string dirp="/data/NAS/RPCH4/";
  n = scandir(dirp.c_str(), &namelist, NULL, alphasort);
  if (n < 0)
    perror("scandir");
  else {
    while (n--) {

      if (fnmatch(spat.str().c_str(), namelist[n]->d_name, 0)==0)
	{
	  printf("%s %d \n", namelist[n]->d_name,fnmatch(spat.str().c_str(), namelist[n]->d_name, 0));
	  printf("found\n");
	  std::stringstream sf;
	  sf<<dirp<<"/"<< namelist[n]->d_name;
	  bs.addRun(runask,sf.str());
	}
      free(namelist[n]);
    }
    free(namelist);
  }

  
  DCHistogramHandler rootHandler;
  DHCalEventReader  dher;

  bs.Read();
  bs.setRun(runask);
  bs.end();
}
