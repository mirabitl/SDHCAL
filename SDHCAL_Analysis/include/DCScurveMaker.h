#ifndef _DCSCURVEMAKER_H
#define _DCSCURVEMAKER_H
#include "DHCalEventReader.h"
#include "DCHistogramHandler.h"
#include "DHCALAnalyzer.h"
/** 
\class DCScurveMaker
  \author M.Vander Donckt
  \date April 2010
  \version 1.0

   \brief Scurve Analysis Class

   <h2> Description</h2>
   The <i> DCScurveMaker </i> 
   <ul>
   <li> uses DHCalEventReader to handle the events
   <li> uses DCHistogramHandler to handle the histograms 
   <li> produces histograms per pad of pedestal and scurve vs treshold.
   <li> at the end of job : produces gain optimized cfg files put in /tmp/ScurveMaker.
   </ul>   
 */
class DCScurveMaker : public DHCALAnalyzer 
{

 public:
  //! Constructor
  DCScurveMaker(DHCalEventReader* r,DCHistogramHandler* h);
  //! Destructor
  ~DCScurveMaker();
//! Books Histograms for one asic.
/** Done for each new asic encountered, not for the dead asics. */
  void initHistograms(int dif, int asic);

//! Fills the pedestal and scurve histograms for each pad of each asic.
  void processEvent();
//! Finds on which pad the charge is injected.
  void processRunHeader();
  //! empty
  virtual void initJob(){;}
  //! that's where the optimized gains are computed.
  /** Produces gain optimized cfg files put in /tmp/ScurveMaker */
  void endJob();
//! Initialisation after condition changes
  void initRun(){PadGain_=0;LowThresh_=0;};
//! Make sure to reset the runheader...
  virtual void endRun(){ reader_->setRunHeader(NULL);}
 private:
  int   hrtype_;         /// Asic type
  std::vector<short> InjectedDif_;   /// number of the DIF we are studying
  std::vector<bool> InjectedPad_;  ///which pad gets the charge injection
  std::vector<short> InjectedPadVector_;  ///which pad gets the charge injection
  short PadGain_;    ///The current tested gain
  short LowThresh_;  ///Lowest threshold for histo booking
  DHCalEventReader* reader_; ///event reader
  DCHistogramHandler* handler_; ///histogram handler
  vector<short> UsedGains_; ///vector of tested gains.
  std::map<short,vector<short> > ActivePads_;
};


#endif
