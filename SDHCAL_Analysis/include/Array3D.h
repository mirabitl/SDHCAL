#ifndef _ARRAY3D_H

#define _ARRAY3D_H
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

template <class T>
class array3D
{
	T* ptr_;
	uint32_t xs_,ys_,zs_;
public:
	array3D(){ptr_=0;}
	inline void initialise(T* p,uint32_t x_size1=60,uint32_t y_size1=96,uint32_t z_size1=96)
	{
		ptr_=p;
		xs_=x_size1;
		ys_=y_size1;
		zs_=z_size1;
	}

	inline void Fill(T* p){ memcpy(ptr_,p,xs_*ys_*zs_*sizeof(T));}
	inline void clear(){memset(ptr_,0,xs_*ys_*zs_*sizeof(T));}
	inline T* getPtr(){return ptr_;}
	inline T  getValue(uint32_t i,uint32_t j,uint32_t k){return ptr_[(i*ys_+j)*zs_+k];}
	inline void setValue(uint32_t i,uint32_t j,uint32_t k,T val){ptr_[(i*ys_+j)*zs_+k]=val;}

	inline uint32_t getXSize(){return xs_;}
	inline uint32_t getYSize(){return ys_;}
	inline uint32_t getZSize(){return zs_;}
};

#endif
