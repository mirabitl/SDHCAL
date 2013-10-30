#ifndef _RECOHIT_H

#define _RECOHIT_H
#include <limits.h>



#include "IMPL/LCTOOLS.h"
#include "EVENT/RawCalorimeterHit.h" 
#include "IMPL/LCGenericObjectImpl.h"
#include "IMPL/RawCalorimeterHitImpl.h"
#include "UTIL/CellIDDecoder.h"
#include "DifGeom.h"
#include "ChamberGeom.h"
#include <stdint.h>
class Shower;
class RecoHit
{
public:
	RecoHit(){;}
	RecoHit(DifGeom& d, ChamberGeom& c,IMPL::RawCalorimeterHitImpl* h,uint32_t hrtype=2);
	void initialise(DifGeom& d, ChamberGeom& c,IMPL::RawCalorimeterHitImpl* h,uint32_t hrtype=2);
  double X();
  double Y();
  double Z();
  int chamber();
  

  

	uint8_t I(){return chamberLocalI_;}
	uint8_t  J(){return chamberLocalJ_;}

	unsigned short dif(){return dg_.getId();}
	uint8_t getDifI(){ return difLocalI_;}
	uint8_t getDifJ(){ return difLocalJ_;}
	uint8_t getAsic(){ return 0xFF & (raw_->getCellID0()&0xFF00)>>8;}
        uint8_t getChannel(){ return 0xFF & (raw_->getCellID0()&0x3F0000)>>16;}
	uint16_t getAmplitude(){return raw_->getAmplitude();}
	//  inline void setIndices(uint32_t i,uint32_t n) { index_=i;next_=n;}
	//inline uint32_t getIndex(){return index_;}
	//inline uint32_t getNext(){return next_;}
	Shower* getShower(){return shower_;}
	void setShower(Shower* s){shower_=s;}
private:
	DifGeom dg_;
	ChamberGeom cg_;
	IMPL::RawCalorimeterHitImpl* raw_;
	uint8_t difLocalI_,difLocalJ_;
	uint8_t chamberLocalI_,chamberLocalJ_;
	// uint32_t index_,next_;
	double x_,y_;
	Shower* shower_;
};
#endif
