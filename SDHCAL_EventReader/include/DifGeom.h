#ifndef DifGeom_HH
#define DifGeom_HH

/**
   \class DifGeom
   \author L.Mirabito
   \date May 2010
   \version 1.0
   \brief DIF Geometry: only PAD shift and polarity is stored. It allows to swap from local DIF numbering to local chamber numbering
 */
class DifGeom
{
 public:

  static const unsigned short Max_Pad_XSize=96;
  static const unsigned short Max_Pad_YSize=32;
 
 DifGeom(unsigned int id=0,unsigned int ch=0,float x=0,float y=0,float polx=0, float poly=0) : id_(id),chId_(ch), x_(x),y_(y),polx_(polx),poly_(poly) {;}
  inline unsigned int getId(){return id_;}
  inline unsigned int getChamberId(){return chId_;}
  inline float getX(){return x_;}
  inline float getY(){return y_;}
  inline float getPolarityX(){return polx_;}
  inline float getPolarityY(){return poly_;}
  inline float toGlobalX(float x)
  {
    if (polx_>0)
      return x+x_;
    else
      return Max_Pad_XSize-x+1+x_;

  }
  inline float toGlobalY(float x)
  {
    if (poly_>0)
      return x+y_;
    else
      return (Max_Pad_YSize-x+1)+y_;

  }

  inline double toLocalX(double x)
  {
    if (polx_>0)
      return x-x_;
    else
      return Max_Pad_XSize-x+1+x_;
  }
  inline double toLocalY(double x)
  {
    if (polx_>0)
      return x-x_;
    else
      return Max_Pad_YSize-x+1+x_;
  }

  static void PadConvert(int asicid,int ipad, int &i, int &j, int asicType)
  {
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Upper view (ASICs SIDE)
    //Asics mapping (channel 0 to 63) on small chambers  with HARDROC I
    //                            0		    5		  10		15	      20        25        30        35        40        45        50        55        60
    const unsigned short MapJSmallHR1[64]={2,2,2,7,5,1,3,1,4,1,6,0,3,0,3,0,7,5,4,0,6,7,2,5,3,1,4,4,6,7,5,6,7,5,6,4,3,7,1,0,5,2,6,7,0,5,4,6,0,6,0,4,4,3,3,3,5,7,1,1,2,1,2,2};
    const unsigned short MapISmallHR1[64]={7,6,5,7,7,7,7,5,7,6,7,7,5,5,6,6,6,6,6,4,6,5,4,5,4,4,5,4,5,4,4,4,3,3,3,3,3,2,3,3,2,3,2,1,0,1,2,1,2,0,1,1,0,1,2,0,0,0,2,1,2,0,1,0};
    //(J Axis)
    //
    // 7	|11|05|00|06|08|04|10|03|  	
    // 6	|15|09|01|14|18|17|20|16|  	     
    // 5	|13|07|02|12|26|23|28|21|  	     
    // 4	|19|25|22|24|27|30|31|29|  	     
    // 3	|39|38|41|36|35|33|34|32|  	     TOP VIEW (ASICs SIDE)
    // 2	|48|58|60|54|46|40|42|37|  	
    // 1	|50|59|62|53|51|45|47|43| 
    // 0	|44|61|63|55|52|56|49|57|   
    //		  0  1  2  3  4  5  6  7    (I Axis)  ----->




    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Upper view (ASICs SIDE)
    //ASIC 90 deg rotation in the trigonometric way compare to the MapISmallHR1/MapJSmallHR1.
    //Asics mapping on large chambers with HARDROC I
    //                            0		    5		  10		15	      20        25        30        35        40        45        50        55        60
    const unsigned short MapJLargeHR1[64]={0,1,2,0,0,0,0,2,0,1,0,0,2,2,1,1,1,1,1,3,1,2,3,2,3,3,2,3,2,3,3,3,4,4,4,4,4,5,4,4,5,4,5,6,7,6,5,6,5,7,6,6,7,6,5,7,7,7,5,6,5,7,6,7};
    const unsigned short MapILargeHR1[64]={2,2,2,7,5,1,3,1,4,1,6,0,3,0,3,0,7,5,4,0,6,7,2,5,3,1,4,4,6,7,5,6,7,5,6,4,3,7,1,0,5,2,6,7,0,5,4,6,0,6,0,4,4,3,3,3,5,7,1,1,2,1,2,2};
    //(J Axis)
    //
    // 7	|03|16|21|29|32|37|43|57|  	
    // 6	|10|20|28|31|34|42|47|49|  	     
    // 5	|04|17|23|30|33|40|45|56|  	     
    // 4	|08|18|26|27|35|46|51|52|  	     
    // 3	|06|14|12|24|36|54|53|55|  	     TOP VIEW (ASICs SIDE)
    // 2	|00|01|02|22|41|60|62|63|  	
    // 1	|05|09|07|25|38|58|59|61| 
    // 0	|11|15|13|19|39|48|50|44|   
    //		  0  1  2  3  4  5  6  7    (I Axis)  ----->
    //				|	 |
    //				|DIFF|
    //				|____|	


    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Upper view (ASICs SIDE)
    //90 deg rotation in the trigonometric way compare to the MapILargeHR1/MapJLargeHR1.
    //180 deg rotation in the trigonometric way compare to the MapISmallHR1/MapJSmallHR1.
    //Asics mapping on large chambers with HARDROC II & IIB
    //								 0		   5		 10		   15	     20        25        30        35        40        45        50        55        60
    const unsigned short MapJLargeHR2[64]={1,1,2,2,3,3,4,4,5,5,6,6,7,7,4,3,2,0,0,1,0,5,6,7,7,6,5,4,3,2,1,0,0,1,2,3,4,5,6,7,7,6,0,0,1,1,0,2,3,4,5,7,7,6,6,5,5,4,4,3,3,2,2,1};
    const unsigned short MapILargeHR2[64]={1,0,1,0,1,0,1,0,0,1,0,1,0,1,2,2,2,0,1,2,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,6,5,6,7,5,5,5,5,6,7,6,7,6,7,6,7,7,6,7,6,7};
    //(J Axis)
    //
    // 7	|46|63|61|59|58|56|54|52|  	
    // 6	|43|45|62|60|57|55|53|51|  	     
    // 5	|42|44|47|48|49|50|41|40|  	     
    // 4	|32|33|34|35|36|37|38|39|  	     
    // 3	|31|30|29|28|27|26|25|24|  	     TOP VIEW (ASICs SIDE)
    // 2	|20|19|16|15|14|21|22|23|  	
    // 1	|18|00|02|04|06|09|11|13| 
    // 0	|17|01|03|05|07|08|10|12|   
    //		  0  1  2  3  4  5  6  7    (I Axis)  ----->
    //				|	 |
    //				|DIFF|
    //				|____|	

    unsigned short AsicShiftI[49]={	0,	
					0,	0,	0,	0,	
					8,	8,	8,	8,	
					16,	16,	16,	16,	
					24,	24,	24,	24,	
					32,	32,	32,	32,	
					40,	40,	40,	40,	
					48,	48,	48,	48,	
					56,	56,	56,	56,	
					64,	64,	64,	64,	
					72,	72,	72,	72,	
					80,	80,	80,	80,	
					88,	88,	88,	88};
    unsigned short AsicShiftJ[49]={	0,		
					0,	8,	16,	24,
					24,	16,	8,	0,	
					0,	8,	16,	24,	
					24,	16,	8,	0,	
					0,	8,	16,	24,	
					24,	16,	8,	0,	
					0,	8,	16,	24,	
					24,	16,	8,	0,	
					0,	8,	16,	24,	
					24,	16,	8,	0,	
					0,	8,	16,	24,	
					24,	16,	8,	0

    };

 
    //Small chambers  HR
    if(asicType==0){
      i = MapISmallHR1[ipad]+AsicShiftI[asicid]+1;
      j = MapJSmallHR1[ipad]+AsicShiftJ[asicid]+1;
    }
 
    //First square meter HR1
    if(asicType==1){
      i = MapILargeHR1[ipad]+AsicShiftI[asicid]+1;
      j = MapJLargeHR1[ipad]+AsicShiftJ[asicid]+1;
    }


    //Second Square meter HR2
    if(asicType==2){
      i = MapILargeHR2[ipad]+AsicShiftI[asicid]+1;
      j = MapJLargeHR2[ipad]+AsicShiftJ[asicid]+1;
      j=33-j;
    }
    if(asicType==11){
      int jligne = 3-(asicid-1)/12;
      int icol = (asicid-1)%12;
      int ispad= ((ipad-1)/8)+1;
      int jspad=(ipad-1)%8+1;
      i = icol*8+ispad;
      j =  jligne*8+jspad;
      //j =  (jligne+1)*8-jspad;
    }
 
  }



 private:
  unsigned int id_;
  unsigned int chId_;
  double x_,y_,polx_,poly_;


};
#endif
