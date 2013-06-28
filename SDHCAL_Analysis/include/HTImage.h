#ifndef _HTIMAGE_H

#define _HTIMAGE_H
#include <limits.h>
#include <DCHistogramHandler.h>
#include <stdint.h>

class HTImage
{
public:
	HTImage(uint32_t nbinx,float xmin,float xmax,uint32_t nbiny,float ymin,float ymax);
	~HTImage();
	void Clear();
	void addPixel(float x,float y,float w=1.,uint32_t ch=INT_MAX,uint32_t pad=INT_MAX);
	void findMaximum(uint32_t& maxval,float& theta,float& r);
	void findMaxima(std::vector<uint32_t>& maxval,std::vector<float>& theta,std::vector<float>& r);
	void Draw(DCHistogramHandler* h);
private:
	uint16_t* theImage_;
	uint16_t* theOriginalImage_;
	int32_t theNbinx_;
	float theXmin_,theXmax_;
	int32_t theNbiny_;
	float theYmin_,theYmax_,theBinxSize_,theBinySize_;
};


#endif
