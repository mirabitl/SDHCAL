#include "UtilDefs.h"
#include "HTImage.h"

#include <ios>
#include <ostream>
#include "TLine.h"
#include "HC.h"
 using namespace std;

HTImage::HTImage(uint32_t nbinx,float xmin,float xmax,uint32_t nbiny,float ymin,float ymax) : theNbinx_(nbinx),theXmin_(xmin),theXmax_(xmax),theNbiny_(nbiny),theYmin_(ymin),theYmax_(ymax)
{
	theImage_ = new uint16_t[theNbinx_*theNbiny_];
	theOriginalImage_ = new uint16_t[60*96];

	theBinxSize_ = (theXmax_-theXmin_)/theNbinx_;
	theBinySize_ = (theYmax_-theYmin_)/theNbiny_;
	// DEBUG_PRINT("New HT Image : %d %d %x \n",theNbinx_,theNbiny_,theImage_);
	// getchar();
}
HTImage::~HTImage()
{
	delete theImage_;
	delete theOriginalImage_;
}
void HTImage::Clear()
{

	// std::cout<<"Clear "<<theNbinx_*theNbiny_*sizeof(uint16_t)<<std::endl;
	// getchar();
	for (uint32_t i=0;i<theNbinx_;i++)
	for (uint32_t j=0;j<theNbiny_;j++)
	theImage_[i*theNbiny_+j]=100;
	//memset(theImage_,0,theNbinx_*theNbiny_*sizeof(uint16_t));
	memset(theOriginalImage_,0,60*96*sizeof(uint16_t));
	// std::cout<<"Clear done "<<theNbinx_*theNbiny_*sizeof(uint16_t)<<std::endl;  
}
static TCanvas* HTImageCanvas=NULL;
void HTImage::Draw(DCHistogramHandler* h)
{

	if (HTImageCanvas==NULL)
	{
		HTImageCanvas=new TCanvas("HTImageCanvas","hugh",800,900);
		HTImageCanvas->Modified();
		HTImageCanvas->Draw();
		HTImageCanvas->Divide(1,2);
	}
	HTImageCanvas->cd();

	TH2F* hhtx = (TH2F*) h->GetTH2("HoughTransformX");
	TH2F* hx = (TH2F*) h->GetTH2("HOriginalX");
	if (hhtx==NULL)
	{
		hhtx =(TH2F*)h->BookTH2("HoughTransform",theNbinx_,theXmin_,theXmax_,theNbiny_,theYmin_,theYmax_);
		hx =(TH2F*)h->BookTH2("HOriginalX",60,0.,60.*2.8,96,0.,96*1.);
	}
	else
	{
		hhtx->Reset();
		hx->Reset();
	}

	for (uint32_t i=0;i<theNbinx_;i++)
	for (uint32_t j=0;j<theNbiny_;j++)
	if (theImage_[i*theNbiny_+j]>3)
	hhtx->SetBinContent(i+1,j+1,theImage_[i*theNbiny_+j]*1.);


	for (uint32_t i=0;i<60;i++)
	for (uint32_t j=0;j<96;j++)
	{
		hx->SetBinContent(i+1,j+1,theOriginalImage_[i*96+j]*1.);

		
	}
	std::ofstream myFile ("/tmp/data.bin", ios::out | ios::binary);
	myFile.write ((const char*) theOriginalImage_,60*96*sizeof(uint16_t));
	myFile.close();
	uint32_t mvx;
	float thx,rx;
	this->findMaximum(mvx,thx,rx);
	//     DEBUG_PRINT("Initial MVX %d \n",mvx);
	HC candx(mvx,thx,rx);


	HTImageCanvas->cd(1);
	hhtx->Draw("COLZ");
	HTImageCanvas->cd(2);
	hx->SetFillColor(3);
	hx->Draw("BOX");

	// TLine l(0.,candx.Pos(0),50.*2.8,candx.Pos(50*2.8));
	// l.SetLineColor(2);
	// l.Draw("SAME");

	HTImageCanvas->Modified();
	HTImageCanvas->Draw();
	HTImageCanvas->Update();
	getchar();

	//delete l;
}
void HTImage::addPixel(float x,float y,float w,uint32_t ch,uint32_t pad)
{
	for (uint32_t i=0;i<theNbinx_;i++)
	{
		float theta=theXmin_+(i+0.5)*theBinxSize_;
		float rx= cos(theta)*x+ sin(theta)*y;
		int32_t j= int(floorf((rx-theYmin_)/theBinySize_));

		if (j>=0 && j<theNbiny_) theImage_[i*theNbiny_+j]=theImage_[i*theNbiny_+j]+w;


		
		
		//std::cout<<i<<" "<<j <<" "<<theImage_[i*theNbiny_+j]<<std::endl;
	}
	if (ch<INT_MAX)
	{
		//std::cout<<ch<<" "<<pad<<" "<<w<<std::endl;
		theOriginalImage_[ch*96+pad]=w; 
	}
}

void HTImage::findMaximum(uint32_t& maxval,float& theta,float& r)
{
	maxval=0;
	int32_t i_m=0,j_m=0;
	for (uint32_t i=0;i<theNbinx_;i++)
	for (uint32_t j=0;j<theNbiny_;j++)
	if (theImage_[i*theNbiny_+j]>maxval)
	{ i_m=i;j_m=j;maxval=theImage_[i*theNbiny_+j];}
	//std::cout<<"Maximum ===>"<<i_m<<" "<<j_m <<" "<<theImage_[i_m*theNbiny_+j_m]<<std::endl;

	//  if (nmaxl>1)
	//  getchar();
	float x_i=0,x_j=0;
	float weight=0;
	for (int32_t ik=TMath::Max(i_m-5,0);ik<TMath::Min(i_m+5,theNbinx_);ik++)
	for (int32_t jk=TMath::Max(j_m-5,0);jk<TMath::Min(j_m+5,theNbiny_);jk++)
	if (theImage_[ik*theNbiny_+jk]>3)
	{
		weight+=theImage_[ik*theNbiny_+jk];
		x_i+=theImage_[ik*theNbiny_+jk]*(ik+0.5);
		x_j+=theImage_[ik*theNbiny_+jk]*(jk+0.5);
		// theImage_[ik*theNbiny_+jk]=0.;//
	}

	if (weight>0)
	{
		x_i=x_i/weight;
		x_j=x_j/weight;
	}
	else
	{
		x_i=i_m+0.5;
		x_j=j_m+0.5;
	}
	theta=theXmin_+x_i*theBinxSize_;
	r=theYmin_+x_j*theBinySize_;
	//  std::cout<<"Maximum pondere ===>"<<x_i<<" "<<x_j <<" "<<weight<<std::endl;
	//getchar();
	// for (int32_t ik=TMath::Max(i_m-5,0);ik<TMath::Min(i_m+5,theNbinx_);ik++)
	//   for (int32_t jk=TMath::Max(j_m-5,0);jk<TMath::Min(j_m+5,theNbiny_);jk++)
	//     theImage_[ik*theNbiny_+jk]=0;
}

void HTImage::findMaxima(std::vector<uint32_t>& maxval,std::vector<float>& theta,std::vector<float>& r)
{
	maxval.clear();
	theta.clear();
	r.clear();
	int32_t i_m=0,j_m=0;
	for (int32_t i=1;i<theNbinx_-1;i++)
	for (int32_t j=1;j<theNbiny_-1;j++)
	if (theImage_[i*theNbiny_+j]>6)
	{

		bool gradient= true;
		for (int32_t ik=TMath::Max(i-2,0);ik<TMath::Min(i+2,theNbinx_);ik++)
		for (int32_t jk=TMath::Max(j-2,0);jk<TMath::Min(j+2,theNbiny_);jk++)
		gradient = gradient && theImage_[i*theNbiny_+j]>=theImage_[ik*theNbiny_+jk];

		if (gradient)
		{
			float x_i=0,x_j=0;
			float weight=0;
			for (int32_t ik=TMath::Max(i-2,0);ik<TMath::Min(i+2,theNbinx_);ik++)
			for (int32_t jk=TMath::Max(j-2,0);jk<TMath::Min(j+2,theNbiny_);jk++)
			if (theImage_[ik*theNbiny_+jk]>2)
			{
				weight+=theImage_[ik*theNbiny_+jk];
				x_i+=theImage_[ik*theNbiny_+jk]*(ik+0.5);
				x_j+=theImage_[ik*theNbiny_+jk]*(jk+0.5);
			}
			if (weight>0)
			{
				x_i=x_i/weight;
				x_j=x_j/weight;
			}
			else
			{
				x_i=i+0.5;
				x_j=j+0.5;
			}
			float theTheta=theXmin_+x_i*theBinxSize_;
			float theR=theYmin_+x_j*theBinySize_;
			maxval.push_back(theImage_[i*theNbiny_+j]);
			theta.push_back(theTheta);
			r.push_back(theR);
		}
	}
}


