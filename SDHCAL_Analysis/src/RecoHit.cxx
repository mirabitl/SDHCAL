#include "RecoHit.h"
#include "Shower.h"
#include <stdio.h>
#include <iostream>

RecoHit::RecoHit(DifGeom& d, ChamberGeom& c,IMPL::RawCalorimeterHitImpl* h,uint32_t hrtype) : dg_(d),cg_(c),raw_(h),shower_(0)

{
	int asicid = (h->getCellID0()&0xFF00)>>8;
	int channel= (h->getCellID0()&0x3F0000)>>16;
	int x=0,y=0;
	DifGeom::PadConvert(asicid,channel,x,y,hrtype);
	difLocalI_=int(x);
	difLocalJ_=int(y);
	chamberLocalI_=dg_.toGlobalX(difLocalI_);
	chamberLocalJ_=dg_.toGlobalY(difLocalJ_);
	double zg=0;
	cg_.calculateGlobal(chamberLocalI_,chamberLocalJ_,0,x_,y_,zg);

}

void RecoHit::initialise(DifGeom& d, ChamberGeom& c,IMPL::RawCalorimeterHitImpl* h,uint32_t hrtype) 
{
	dg_=d;cg_=c;raw_=h;
	int asicid = (h->getCellID0()&0xFF00)>>8;
	int channel= (h->getCellID0()&0x3F0000)>>16;
	int x=0,y=0;
	DifGeom::PadConvert(asicid,channel,x,y,hrtype);
	difLocalI_=int(x);
	difLocalJ_=int(y);
	chamberLocalI_=dg_.toGlobalX(difLocalI_);
	chamberLocalJ_=dg_.toGlobalY(difLocalJ_);
	double zg=0;
	cg_.calculateGlobal(chamberLocalI_,chamberLocalJ_,0,x_,y_,zg);

}
double RecoHit::X(){return x_;}
double RecoHit::Y(){return y_;}
double RecoHit::Z(){ return cg_.getZ();}
int RecoHit::chamber(){return cg_.getId();}
