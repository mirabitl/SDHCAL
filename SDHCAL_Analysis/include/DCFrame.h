
#ifndef ROOT_DCFrame
#define ROOT_DCFrame
//#include "DCDefinition.h"

#include <iostream>
#include <vector>
//#include <TH2.h>

using namespace std;


/** 
\class DCFrame
  \author  L.Mirabito 
  \date March 2010
  \version 1.0
  
 \brief Handler of frame info

*/

class DCFrame  {
  
private:
  unsigned short difId_; ///! DIF id
  unsigned short asicId_; ///! Asic Id
  double timeToTrigger_; ///! distance in clocks to trigger
  double timeThreshold_; ///! High Threshold in clocks between frame time and trigger to be synchronised 
  unsigned char nHit0_,nHit1_; ///! number of hits for the 2 thresholds
  double bc_; //! Absolute bunch crossing
  double reco_bc_; //! Corrected Time to Trigger
  bool level0_[64]; //! Hit pattern Level 0
  bool level1_[64]; //! Hit Pattern Level 1
  bool synchronised_; //! Synchronise flag
public:
  //! Constructor
  /** 
      @param dif DIF Id 
      @param asic asic id
      @param bc Bunch Crossing 
      @param l0 Level 0 hit pattern vector
      @param l1 Leval 1 hit pattern vector
      @param ti Time to trigger
      @param threshold Time threshold in clock counts
   */

  DCFrame(unsigned short dif,unsigned short asic,double bc,bool* l0,bool* l1,double ti,double threshold=2.);


  DCFrame();
  //! Destructor
  ~DCFrame();

  //! Dum the event
  void Dump();

  //! Get Dif Id
  unsigned short getDifId();

  //! get Asic Id
  unsigned short getAsicId();

  //! Get Bunch Crossing Time
  double getBunchCrossingTime();
  
  //! Get corrected time to Trigger
  double getRecoBunchCrossingTime();

  //! Get Raw Time to trigger
  double getTimeToTrigger();

  //! Get hit flag Level 0
  /**
     @param i pad number
     @return True if above Level 0
   */

   bool getLevel0(unsigned int i);

  //! Get hit flag Level 1
  /**
     @param i pad number
     @return True if above Level 1
   */
   bool  getLevel1(unsigned int i);

   //! True if fram in Time
  bool isSynchronised();

  //! Set teh synchronise flag
  void setSynchronised(bool t);

  //! Set the correcetd Time to trigger
  void setRecoBunchCrossingTime(double t);

  //! return number of Pad hit above threshold
  unsigned short getNumberOfHit(unsigned short level);

  //! Set all paramters
  void setAll(unsigned short dif,unsigned short asic,double bc,bool* l0,bool* l1,double ti,double threshold);
};

#endif
