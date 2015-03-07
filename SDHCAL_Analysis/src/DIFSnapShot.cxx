#include "DIFSnapShot.h"
#include "DifGeom.h"
#include <string.h>
#include <TROOT.h>
#include <TStyle.h>
#include  <TCanvas.h>
#include <TH2.h>
#include <iostream>
#include <fstream>
#include <sstream>
DIFSnapShot::DIFSnapShot()
{
  this->clear();
}
DIFSnapShot::DIFSnapShot(std::vector<DIFPtr*> &difptrlist,uint32_t seed) :theSeed(seed)
{
  this->clear();
  this->fill(difptrlist,seed);
}
DIFSnapShot::DIFSnapShot(std::vector<DIFPtr*> &difptrlist,uint32_t lowtime,uint32_t hightime) :theLowTime(lowtime),theHighTime(hightime)
{
  this->clear();
  this->fill(difptrlist,lowtime,hightime);
}
void DIFSnapShot::clear()
{
  memset(theData,0,3*255*96*sizeof(uint32_t));
}
void DIFSnapShot::fill(std::vector<DIFPtr*> &difptrlist,uint32_t seed1,uint32_t seed2)
{
  if (seed2==0)
    {
      theSeed=seed1;
    }
  else
    {
      theLowTime=seed1;
      theHighTime=seed2;
    }
  uint32_t ith,idif,ix,jy;
  for (std::vector<DIFPtr*>::iterator it = difptrlist.begin();it!=difptrlist.end();it++)
    {
      DIFPtr* d = (*it);
      
      // Loop on frames
      idif=d->getID();
      //printf("%d found\n",idif);
      for (uint32_t i=0;i<d->getNumberOfFrames();i++)
	{
	  
	  if (seed2==0 && abs((int)seed1-(int) d->getFrameTimeToTrigger(i))>2) continue;
	  if (seed2!=0 && d->getFrameTimeToTrigger(i)>seed2) continue;
	  if (seed2!=0 && d->getFrameTimeToTrigger(i)<seed1) continue;
	  // Loop on Pad
	  for (uint32_t j=0;j<64;j++)
	    {
	      if (!(d->getFrameLevel(i,j,0) || d->getFrameLevel(i,j,1))) continue; // skip empty pads
	      if (d->getFrameLevel(i,j,0)) ith=1;
	      if (d->getFrameLevel(i,j,1)) ith=0;
	      if (d->getFrameLevel(i,j,1) && d->getFrameLevel(i,j,0)) ith=2;
	      
	      int x=0,y=0;
	      DifGeom::PadConvert(d->getFrameAsicHeader(i),j,x,y,2);
	      ix=int(x)-1;
	      jy=int(y)-1;
	      theData[ith][idif][ix]=(1<<jy);
	      //printf("%x\n",theData[ith][idif][ix]);
	    }
	}
    }
}
static TCanvas* TCPlot=NULL;

void DIFSnapShot::DrawEvent()
{
}

void DIFSnapShot::DrawDIF(uint32_t difid)
{
    std::stringstream s;
    s<<"DIF"<<difid;
    TH2F* hd=new TH2F(s.str().c_str(),s.str().c_str(),96,0.,96.,32,0.,32.);
    if (TCPlot==NULL)
	{
	  TCPlot=new TCanvas("TCPlot","test1",1300,600);
	  TCPlot->Modified();
	  TCPlot->Draw();
	  //TCPlot->Divide(2,2);
	}
	
    for (uint32_t i=0;i<96;i++)
      for (uint32_t j=0;j<32;j++)
      {
	if (this->hit(0,difid,i,j) || this->hit(2,difid,i,j) || this->hit(2,difid,i,j))
	  hd->SetBinContent(i+1,j+1,1.);
      }
    hd->Draw("COLZ");
    TCPlot->Modified();
    TCPlot->Draw();
    TCPlot->Update();
    char c;c=getchar();putchar(c); if (c=='.') exit(0);;
    delete hd;
}
